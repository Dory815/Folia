'use strict';

// MathML is used only to reach table attributes without a TeX profile entry.
// Native tests consume the actual compiled Source tree, not a second parser.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

assert.ok(process.argv.length === 2 ||
  (process.argv.length === 3 && process.argv[2] === '--write-fixture'),
  'usage: common-table-mathml-source.cjs [--write-fixture]');
const root = path.resolve(__dirname, '../..');
const hash = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const scopePath = path.join(root, 'docs/math-port/scope.json');
const casesPath = path.join(__dirname, 'table-mathml-cases.json');
const scope = JSON.parse(fs.readFileSync(scopePath, 'utf8'));
const cases = JSON.parse(fs.readFileSync(casesPath, 'utf8'));
assert.equal(cases.source.commit, scope.upstream.commit);
const {createReference} = require('./reference-profile.cjs');
const ref = createReference();
assert.deepEqual(ref.provenance.packages, scope.profile.tex_packages);
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const cjsRoot = path.join(packageRoot, 'cjs');
const {MathML} = require(path.join(cjsRoot, 'input/mathml.js'));
const document = ref.mathjax.document('', {InputJax: new MathML(), OutputJax: ref.output});
const fields = ['w', 'h', 'd', 'scale', 'rscale', 'L', 'R', 'pwidth'];

function captureMml(node) {
  assert.notEqual(node.kind, 'text', 'This table corpus contains no text nodes');
  return {
    kind: node.kind,
    isInferred: node.isKind('inferredMrow'),
    attributes: node.attributes?.getAllAttributes() ?? {},
    children: node.childNodes.map(captureMml)
  };
}

function captureWrapper(wrapper) {
  const box = wrapper.getBBox();
  const result = {
    kind: wrapper.node.kind,
    bbox: Object.fromEntries(fields.map(key => [key, box[key]]))
  };
  if (wrapper.node.kind === 'mtable') {
    const {H, D, W, L} = wrapper.getTableData();
    Object.assign(result, {tableData: {H, D, W, L},
      rowRscales: wrapper.childNodes.map(row => row.getBBox().rscale)});
  }
  return result;
}

let active = null;
const processMath = ref.output.processMath;
ref.output.processMath = function(wrapper, node) {
  const result = processMath.apply(this, arguments);
  assert.ok(active, 'Source capture must have an active input');
  active.mml = captureMml(wrapper.node);
  active.wrappers = Array.from(this.nodeMap.values(), captureWrapper);
  return result;
};

function findNode(node, kind) {
  if (node.kind === kind) return node;
  for (const child of node.children) {
    const found = findNode(child, kind);
    if (found) return found;
  }
  return null;
}

function cppAttribute(name, value) {
  const key = 'u' + JSON.stringify(name);
  switch (typeof value) {
    case 'boolean': return `{${key}, MathmlAttributeKind::boolean, {}, 0, ${value}}`;
    case 'number':
      assert.ok(Number.isFinite(value), `Nonfinite own attribute: ${name}`);
      return `{${key}, MathmlAttributeKind::number, {}, ${value}, false}`;
    case 'string':
      return `{${key}, MathmlAttributeKind::text, u${JSON.stringify(value)}, 0, false}`;
    default: throw new Error(`Unsupported own attribute type: ${name}`);
  }
}

function cppTree(rootNode, prefix) {
  const nodes = [], attributes = [];
  function visit(node, parent) {
    assert.equal(node.isInferred, node.kind === 'inferredMrow');
    const index = nodes.length;
    const first = attributes.length;
    const own = Object.entries(node.attributes);
    attributes.push(...own.map(([name, value]) => cppAttribute(name, value)));
    nodes.push(`  {${JSON.stringify(node.kind)}, ${node.isInferred}, ${parent}, ${first}, ${own.length}}`);
    node.children.forEach(child => visit(child, index));
  }
  visit(rootNode, 'std::size_t(-1)');
  assert.ok(attributes.length > 0, 'The table corpus must have explicit attributes');
  return `constexpr MathmlSceneNode ${prefix}Nodes[]{\n${nodes.join(',\n')}\n};\n` +
    `constexpr MathmlSceneAttribute ${prefix}Attributes[]{\n  ${attributes.join(',\n  ')}\n};\n`;
}

function cppCase(record, index) {
  const table = record.wrappers.find(wrapper => wrapper.kind === 'mtable');
  const tableNode = findNode(record.mml, 'mtable');
  assert.ok(table && tableNode, `${record.name}: missing compiled table`);
  const {H, D, W, L} = table.tableData;
  const {w, h, d, L: left, R: right} = table.bbox;
  assert.ok([w, h, d, left, right, L, ...H, ...D, ...W,
    ...table.rowRscales].every(Number.isFinite), `${record.name}: nonfinite geometry`);
  const attrs = tableNode.attributes;
  const dottedLines = (attrs.rowlines === 'dotted' ? H.length - 1 : 0) +
    (attrs.columnlines === 'dotted' ? W.length - 1 : 0);
  const dottedFrames = attrs.frame === 'dotted' ? 1 : 0;
  assert.ok(Buffer.byteLength(record.svg) <= 3500, 'Split long MSVC literals before extending corpus');
  const prefix = 'mathmlTable' + index;
  let declarations = cppTree(record.mml, prefix);
  for (const [name, values] of Object.entries({H, D, W, RowScales: table.rowRscales})) {
    assert.ok(values.length > 0, 'The table corpus must have measured rows and columns');
    declarations += `constexpr double ${prefix}${name}[]{${values.join(',')}};\n`;
  }
  const {em, ex, containerWidth, display} = cases.metrics;
  const value = `MathmlTableSceneCase{${JSON.stringify(record.name)},` +
    `MathmlSceneMetrics{${em},${ex},${containerWidth},${display}},` +
    `${prefix}Nodes,${prefix}Attributes,${prefix}H,${prefix}D,${prefix}W,${L},${prefix}RowScales,` +
    `${dottedLines},${dottedFrames},MathmlSceneBBox{${w},${h},${d},${left},${right}},` +
    `${JSON.stringify(record.svg)}}`;
  return {declarations, value};
}

async function main() {
  const records = [];
  for (const entry of cases.cases) {
    active = {};
    const node = await document.convertPromise(entry.mathml, cases.metrics);
    const svg = ref.adaptor.serializeXML(node);
    assert.ok(active.mml && active.wrappers && svg, `${entry.name}: incomplete Source output`);
    records.push({name: entry.name, mathml: entry.mathml, ...active, svg});
    active = null;
  }
  const prefix = path.join(cjsRoot, 'input/mathml');
  const modules = Object.keys(require.cache).filter(file => file.startsWith(prefix)).sort()
    .map(file => {
      const compiled = path.relative(cjsRoot, file).replaceAll(path.sep, '/');
      const source = compiled.replace(/\.js$/, '.ts');
      const sourcePath = path.join(root, '.reference/MathJax-src/ts', source);
      return {compiled, compiledSha256: hash(file), source, sourceSha256: hash(sourcePath)};
    });
  const provenance = {upstream: scope.upstream, input: 'MathML', inputJaxInCurrentScope: false,
    scopeSha256: hash(scopePath), casesSha256: hash(casesPath), outputProfile: ref.provenance,
    mathmlModules: modules};
  const header = [
    `// Source MathJax ${scope.upstream.version} ${scope.upstream.commit}`,
    '// Input: pinned MathML compiler; output: shared SVG/Common/NewCM; not TeX profile coverage.',
    `// scope-sha256 ${provenance.scopeSha256}`,
    `// cases-sha256 ${provenance.casesSha256}`,
    `// profile-sha256 ${scope.profile.sha256}`,
    ...modules.map(module => `// ${module.compiled} ${module.compiledSha256}` +
      ` | ${module.source} ${module.sourceSha256}`)
  ].join('\n');
  const generated = records.map(cppCase);
  const fixture = header + '\n' + generated.map(item => item.declarations).join('\n') +
    '\nconstexpr MathmlTableSceneCase sourceTableCases[]{\n' +
    generated.map(item => '  ' + item.value).join(',\n') + '\n};\n';
  const evidence = path.join(root, 'build/math-port/common-source/table-mathml-scene.json');
  fs.mkdirSync(path.dirname(evidence), {recursive: true});
  fs.writeFileSync(evidence, JSON.stringify({provenance, records}, null, 2) + '\n');
  const target = path.join(root, 'tests/math/fixtures/common-source/table-mathml-scene.inc');
  const previous = fs.existsSync(target) ? fs.readFileSync(target, 'utf8') : null;
  if (process.argv[2] === '--write-fixture') {
    if (previous !== fixture) fs.writeFileSync(target, fixture);
  } else {
    assert.equal(previous, fixture, 'Independent Source fixture differs; review before --write-fixture');
  }
  process.stdout.write(`Verified ${records.length} direct MathML table Source cases\n`);
}

main().catch(error => {process.stderr.write(error.stack + '\n'); process.exitCode = 1;});
