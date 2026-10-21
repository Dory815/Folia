'use strict';

// Independent pinned MathML -> SVG source witness for the SVG merror wrapper.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

assert.ok(process.argv.length === 2 ||
  (process.argv.length === 3 && process.argv[2] === '--write-fixture'),
  'usage: common-merror-source.cjs [--write-fixture]');

const root = path.resolve(__dirname, '../..');
const scopePath = path.join(root, 'docs/math-port/scope.json');
const scope = JSON.parse(fs.readFileSync(scopePath, 'utf8'));
const cases = [
  {name: 'merror-basic-text', mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><merror><mrow><mi>x</mi><mo>+</mo><mi>y</mi></mrow></merror></math>'},
  {name: 'merror-empty-title', mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><merror title=""><mi>x</mi></merror></math>'},
  {name: 'merror-escaped-title', mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><merror title="Title &amp; &quot;quoted&quot; &lt;tag&gt;"><mi>x</mi></merror></math>'},
  {name: 'merror-multiple-children', mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><merror><mi>a</mi><mo>+</mo><mi>b</mi></merror></math>'},
  {name: 'merror-nested', mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><merror title="outer"><mrow><mi>a</mi><merror title="inner"><mrow><mi>x</mi><mo>+</mo><mi>y</mi></mrow></merror></mrow></merror></math>'},
  {name: 'merror-authored-blue', mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><merror><mi mathcolor="blue">x</mi><mi mathcolor="blue">y</mi></merror></math>'},
  {name: 'merror-linked-nested-svg-child', mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><merror><mrow href="https://example.test/merror"><mo stretchy="true" minsize="5em">(</mo><mi>x</mi><mo stretchy="true" minsize="5em">)</mo></mrow></merror></math>'},
].map(entry => ({...entry, metrics: {em: 16, ex: 8, container_width: 640, display: false}}));

const {createMathmlSourceRuntime} = require('./common-mathml-source-runtime.cjs');
const runtime = createMathmlSourceRuntime({root, scope, scopePath,
  observeWrapper(wrapper, result) {
    if (wrapper.node.kind !== 'merror') return;
    const box = wrapper.getBBox();
    result.merror = {
      bbox: {w: box.w, h: box.h, d: box.d, L: box.L, R: box.R},
      title: wrapper.node.attributes.get('title') || '',
      styles: wrapper.constructor.styles
    };
  }
});

function findNodes(node, kind, output = []) {
  if (node.kind === kind) output.push(node);
  node.children.forEach(child => findNodes(child, kind, output));
  return output;
}

const expectedStyles = {
  'g[data-mml-node="merror"] > g': {fill: 'red', stroke: 'red'},
  'g[data-mml-node="merror"] > rect[data-background]': {fill: 'yellow', stroke: 'none'}
};

function validate(records) {
  for (const record of records) {
    const merrors = findNodes(record.mml, 'merror');
    const wrapperRecords = record.wrappers.filter(wrapper => wrapper.kind === 'merror');
    assert.equal(wrapperRecords.length, merrors.length, `${record.name}: Source merror wrappers captured`);
    assert.ok(merrors.length > 0, `${record.name}: Source merror MML node present`);
    assert.ok(wrapperRecords.every(wrapper => wrapper.merror &&
      Object.values(wrapper.merror.bbox).every(Number.isFinite)),
    `${record.name}: Source merror bbox and class styles captured`);
    for (const wrapper of wrapperRecords)
      assert.deepEqual(wrapper.merror.styles, expectedStyles,
        `${record.name}: actual SvgMerror class style rules remain pinned`);
    const backgrounds = (record.svg.match(/<rect\b[^>]*data-background/g) || []).length;
    assert.equal(backgrounds, merrors.length, `${record.name}: one Source background rect per merror`);
    if (record.name === 'merror-empty-title')
      assert.equal((record.svg.match(/<title\b/g) || []).length, 0,
        'empty Source title is omitted');
    if (record.name === 'merror-escaped-title') {
      assert.ok(record.svg.includes('<title>Title & "quoted" <tag></title>'),
        'Source LiteAdaptor serializes the captured title text verbatim');
      assert.equal(merrors[0].attributes.title, 'Title & "quoted" <tag>',
        'captured Source MML retains the authored title value');
    }
    if (record.name === 'merror-multiple-children') {
      assert.equal(findNodes(merrors[0], 'mi').length, 2,
        'multiple authored identifier children remain in Source MML');
      assert.equal(findNodes(merrors[0], 'mo').length, 1,
        'multiple authored operator children remain in Source MML');
    }
    if (record.name === 'merror-nested')
      assert.equal(merrors.length, 2, 'nested Source merror remains nested');
    if (record.name === 'merror-authored-blue') {
      const blueChildren = findNodes(merrors[0], 'mi')
        .filter(node => node.attributes.mathcolor === 'blue');
      assert.equal(blueChildren.length, 2, 'authored blue child colors remain in Source MML');
      assert.ok((record.svg.match(/fill="blue"/g) || []).length >= 2,
        'authored SVG blue fill survives the red merror class selector');
    }
    if (record.name === 'merror-linked-nested-svg-child') {
      assert.ok(record.svg.includes('href="https://example.test/merror"'),
        'Source child link remains authored');
      assert.ok((record.svg.match(/<svg\b/g) || []).length > 1,
        'Source assembled operator child is emitted as nested SVG');
    }
  }
}

function cppString(value) { return JSON.stringify(value); }
function cppU16(value) { return `u${cppString(value)}`; }
function cppAttribute(name, value) {
  const key = cppU16(name);
  switch (typeof value) {
    case 'boolean': return `{${key}, mathml_scene_test::AttributeKind::boolean, {}, 0, ${value}}`;
    case 'number':
      assert.ok(Number.isFinite(value), `Nonfinite Source attribute ${name}`);
      return `{${key}, mathml_scene_test::AttributeKind::number, {}, ${value}, false}`;
    case 'string': return `{${key}, mathml_scene_test::AttributeKind::text, ${cppU16(value)}, 0, false}`;
    default: throw new Error(`Unsupported Source attribute type for ${name}`);
  }
}

function cppRaw(value) {
  const parts = [];
  let part = '', bytes = 0;
  for (const codePoint of value) {
    const size = Buffer.byteLength(codePoint);
    if (bytes + size > 3500 && part) { parts.push(part); part = ''; bytes = 0; }
    part += codePoint;
    bytes += size;
  }
  if (part || !parts.length) parts.push(part);
  return parts.map(text => {
    let delimiter = 'FOLIAMERROR';
    for (let suffix = 0; text.includes(`)${delimiter}"`); ++suffix)
      delimiter = `FOLIAMERROR${suffix + 1}`;
    return `R"${delimiter}(${text})${delimiter}"`;
  }).join('\n      ');
}

function cppTree(record, index) {
  const prefix = `merror${index}`;
  const nodes = [], attributes = [];
  function visit(node, parent) {
    assert.equal(node.isInferred, node.kind === 'inferredMrow');
    const nodeIndex = nodes.length, first = attributes.length;
    const own = Object.entries(node.attributes);
    attributes.push(...own.map(([name, value]) => cppAttribute(name, value)));
    nodes.push(`  {${cppString(node.kind)}, ${node.isInferred}, ${parent}, ${first}, ${own.length}, ${node.kind === 'text' ? cppU16(node.text || '') : 'u""'}}`);
    node.children.forEach(child => visit(child, nodeIndex));
  }
  visit(record.mml, 'std::numeric_limits<std::size_t>::max()');
  const declarations =
    `constexpr std::array<mathml_scene_test::Node, ${nodes.length}> ${prefix}Nodes{{\n${nodes.join(',\n')}\n}};\n` +
    `constexpr std::array<mathml_scene_test::Attribute, ${attributes.length}> ${prefix}Attributes{{\n` +
    `${attributes.length ? '  ' + attributes.join(',\n  ') : ''}\n}};\n`;
  const mmlErrors = findNodes(record.mml, 'merror');
  const observed = record.wrappers.filter(wrapper => wrapper.kind === 'merror');
  const unused = new Set(observed);
  const boxes = mmlErrors.map(node => {
    const title = node.attributes.title || '';
    const match = observed.find(wrapper => unused.has(wrapper) && wrapper.merror.title === title);
    assert.ok(match, `${record.name}: Source bbox associates with MML merror preorder`);
    unused.delete(match);
    return match.merror.bbox;
  });
  assert.equal(unused.size, 0, `${record.name}: all observed Source merror boxes are assigned`);
  assert.ok(boxes.length > 0 && boxes.length <= 2, `${record.name}: expected one or two merror boxes`);
  const boxArray = `std::array<MerrorSourceBBox, 2>{{${boxes.map(box =>
    `{${box.w},${box.h},${box.d},${box.L},${box.R}}`).join(',')}${boxes.length === 1 ? ',{}' : ''}}}`;
  const {em, ex, container_width, display} = record.metrics;
  const sourceCase = `mathml_scene_test::Case{${cppString(record.name)},` +
    `mathml_scene_test::Metrics{${em},${ex},${container_width},${display}},` +
    `${prefix}Nodes,${prefix}Attributes,${cppRaw(record.svg)}}`;
  return {declarations, value: `{${sourceCase},${boxArray},${boxes.length}}`};
}

function cppSourceStyles(records) {
  const observed = records.flatMap(record => record.wrappers
    .filter(wrapper => wrapper.kind === 'merror')
    .map(wrapper => wrapper.merror.styles));
  assert.ok(observed.length > 0, 'Source merror class styles were observed');
  const styles = observed[0];
  assert.deepEqual(styles, expectedStyles, 'all Source merror wrappers share actual class styles');
  return `constexpr std::array<MerrorSourceStyleRule, ${Object.keys(styles).length}> ` +
    `sourceMerrorStyleRules{{\n${Object.entries(styles).map(([selector, paint]) =>
      `  {${cppU16(selector)}, ${cppU16(paint.fill)}, ${cppU16(paint.stroke)}}`).join(',\n')}` +
    '\n}};\n';
}

async function main() {
  const records = [];
  for (const entry of cases)
    records.push({...entry, ...await runtime.convert(entry, {
      em: entry.metrics.em, ex: entry.metrics.ex,
      containerWidth: entry.metrics.container_width, display: entry.metrics.display
    })});
  validate(records);
  const {provenance, modules} = runtime.provenance('MathML', __filename);
  const header = [
    `// Source MathJax ${scope.upstream.version} ${scope.upstream.commit}`,
    '// Input: pinned direct MathML merror wrapper cases; not TeX input coverage.',
    `// scope-sha256 ${provenance.scopeSha256}`,
    ...modules.map(module => `// ${module.compiled} ${module.compiledSha256} | ${module.source} ${module.sourceSha256}`)
  ].join('\n');
  const generated = records.map(cppTree);
  const fixture = header + '\nstruct MerrorSourceBBox { double width, height, depth, left, right; };\n' +
    'struct MerrorSourceCase { mathml_scene_test::Case source; std::array<MerrorSourceBBox, 2> boxes; std::size_t box_count; };\n' +
    'struct MerrorSourceStyleRule { std::u16string_view selector, fill, stroke; };\n' +
    cppSourceStyles(records) +
    generated.map(item => item.declarations).join('\n') +
    '\nconstexpr MerrorSourceCase sourceMerrorCases[]{\n' + generated.map(item => `  ${item.value}`).join(',\n') + '\n};\n';
  const evidencePath = path.join(root, 'build/math-port/common-source/merror-mathml-scene.json');
  fs.mkdirSync(path.dirname(evidencePath), {recursive: true});
  fs.writeFileSync(evidencePath, JSON.stringify({provenance, cases: records}, null, 2) + '\n');
  const target = path.join(root, 'tests/math/fixtures/common-source/merror-mathml-scene.inc');
  const previous = fs.existsSync(target) ? fs.readFileSync(target, 'utf8') : null;
  if (process.argv[2] === '--write-fixture') {
    if (previous !== fixture) fs.writeFileSync(target, fixture);
  } else {
    assert.equal(previous, fixture, 'Independent merror Source fixture differs');
  }
  process.stdout.write(`Verified ${records.length} direct MathML merror Source cases\n`);
}

main().catch(error => {process.stderr.write(error.stack + '\n'); process.exitCode = 1;});
