'use strict';

// Independent pinned MathJax table-profile Source witness.  Inputs are real
// MathML compilations through the pinned profile's SVG/Common/NewCM output.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

assert.ok(process.argv.length === 2 ||
  (process.argv.length === 3 && process.argv[2] === '--write-fixture'),
'usage: source-table-profile.cjs [--write-fixture]');
const root = path.resolve(__dirname, '../..');
const scopePath = path.join(root, 'docs/math-port/scope.json');
const scope = JSON.parse(fs.readFileSync(scopePath, 'utf8'));
const sha = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const {createReference} = require('./reference-profile.cjs');
const ref = createReference();
assert.deepEqual(ref.provenance.packages, scope.profile.tex_packages);
const branchTableAttrs = 'columnspacing="1em" rowspacing="4pt" data-frame-styles="" framespacing=".5em .125em"';
const oldOperatorSeed = '\\begin{array}{c}abcdefghij+klmnopqrst\\end{array}';
const breakOperatorCells = [...'abcdefghij'].map(char=>`<mi data-latex="${char}">${char}</mi>`).join('')+
  '<mo data-latex="+">+</mo>'+[...'klmnopqrst'].map(char=>`<mi data-latex="${char}">${char}</mi>`).join('');

const cases = [
  {name:'table-profile-label-left-width-includes',mathml:'<math xmlns="http://www.w3.org/1998/Math/MathML" display="block"><mtable side="left" align="left" minlabelspacing="0.4em" data-width-includes-label="true"><mlabeledtr><mtd><mtext>LeftA</mtext></mtd><mtd><mi>x</mi></mtd><mtd><mn>1</mn></mtd></mlabeledtr><mlabeledtr><mtd><mtext>LeftB</mtext></mtd><mtd><mi>y</mi></mtd><mtd><mn>2</mn></mtd></mlabeledtr></mtable></math>',display:true},
  {name:'table-profile-label-right-width-excludes',mathml:'<math xmlns="http://www.w3.org/1998/Math/MathML" display="block"><mtable side="right" align="right" minlabelspacing="0.7em" data-width-includes-label="false"><mlabeledtr><mtd><mtext>RightA</mtext></mtd><mtd><mi>x</mi></mtd><mtd><mn>1</mn></mtd></mlabeledtr><mlabeledtr><mtd><mtext>RightB</mtext></mtd><mtd><mi>y</mi></mtd><mtd><mn>2</mn></mtd></mlabeledtr></mtable></math>',display:true},
  {name:'table-profile-percent-child-nested',mathml:'<math xmlns="http://www.w3.org/1998/Math/MathML" display="block"><mtable columnwidth="6em"><mtr><mtd><mrow><mi>a</mi><mo>+</mo><mtable width="65%" columnwidth="45% auto"><mtr><mtd><mi>bb</mi></mtd><mtd><mi>c</mi></mtd></mtr><mtr><mtd><mi>d</mi></mtd><mtd><mi>ee</mi></mtd></mtr></mtable></mrow></mtd></mtr></mtable></math>',display:true},
  {name:'table-profile-ragged-row-cell-align',mathml:'<math xmlns="http://www.w3.org/1998/Math/MathML" display="block"><mtable rowalign="top bottom" columnalign="left right"><mtr><mtd><mi>a</mi></mtd><mtd><mi>bb</mi></mtd></mtr><mtr><mtd rowalign="center"><mi>ccc</mi></mtd></mtr></mtable></math>',display:true},
  {name:'table-profile-horizontal-stretch-column',mathml:'<math xmlns="http://www.w3.org/1998/Math/MathML" display="block"><mtable columnwidth="3em"><mtr><mtd><mo stretchy="true">→</mo></mtd></mtr></mtable></math>',display:true},
  {name:'table-profile-break-text',mathml:`<math xmlns="http://www.w3.org/1998/Math/MathML" display="block" overflow="linebreak"><mtable ${branchTableAttrs} data-latex="\\begin{array}{c}\\text{x x}\\end{array}" width="auto" columnwidth="0.5em"><mtr data-latex="{c}"><mtd><mtext data-latex="\\text{x x}">x x</mtext></mtd></mtr></mtable></math>`,display:true,width:1280},
  {name:'table-profile-break-operator',mathml:`<math xmlns="http://www.w3.org/1998/Math/MathML" display="block" overflow="linebreak"><mtable ${branchTableAttrs} data-latex="${oldOperatorSeed}" width="auto" columnwidth="2em"><mtr data-latex="{c}"><mtd>${breakOperatorCells}</mtd></mtr></mtable></math>`,display:true,width:100},
];

const {MathML} = require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/mathml.js'));
const input = new MathML();
const document = ref.mathjax.document('', {InputJax:input, OutputJax:ref.output});

function flattenMml(rootNode) {
  const nodes = [], attributes = [], indexByNode = new Map();
  function visit(node, parent) {
    const index = nodes.length;
    indexByNode.set(node, index);
    const first = attributes.length;
    for (const [name, value] of Object.entries(node.attributes?.getAllAttributes?.() ?? {})) {
      if (value === undefined || value === null) continue;
      if (typeof value === 'boolean') attributes.push({name, kind:'boolean', boolean:value});
      else if (typeof value === 'number') {
        assert.ok(Number.isFinite(value), `nonfinite MML attribute ${name}`);
        attributes.push({name, kind:'number', number:value});
      } else if (typeof value === 'string') attributes.push({name, kind:'text', text:value});
      else throw new Error(`unsupported compiled MML attribute ${name}: ${typeof value}`);
    }
    nodes.push({kind:node.kind, inferred:node.isKind?.('inferredMrow') ?? false,
      parent, first, count:attributes.length-first,
      text:node.kind === 'text' ? node.getText() : ''});
    for (const child of node.childNodes ?? []) visit(child, index);
  }
  visit(rootNode, null);
  return {nodes, attributes, indexByNode};
}

function box(value) {
  const result = Object.fromEntries(['w','h','d','scale','rscale','L','R'].map(key => [key, value[key]]));
  assert.ok(Object.values(result).every(Number.isFinite), 'nonfinite observed BBox');
  return result;
}

function findNodes(rootNode, kind, out=[]) {
  if (rootNode.kind === kind) out.push(rootNode);
  for (const child of rootNode.childNodes ?? []) findNodes(child, kind, out);
  return out;
}

function captureTable(output, node, flat) {
  const wrapper = Array.from(output.nodeMap.values()).find(value => value.node === node);
  assert.ok(wrapper, `compiled ${node.kind} has no Common wrapper`);
  const data = wrapper.getTableData();
  const record = {
    node_index: flat.indexByNode.get(node), bbox:box(wrapper.getBBox()),
    rows:wrapper.numRows, cols:wrapper.numCols, has_labels:wrapper.hasLabels,
    is_top:wrapper.isTop, pwidth:wrapper.pWidth, cWidths:[...wrapper.cWidths],
    H:[...data.H], D:[...data.D], W:[...data.W], NH:[...data.NH], ND:[...data.ND],
    label_width:data.L
  };
  assert.ok(Number.isFinite(record.pwidth));
  for (const vector of [record.H,record.D,record.W,record.NH,record.ND])
    assert.ok(vector.every(Number.isFinite));
  assert.ok(Number.isFinite(record.label_width));
  return record;
}

const originalProcess = ref.output.processMath;
const originalMeasure = ref.output.measureText;
const cachePrototype = Object.getPrototypeOf(ref.output.fontCache);
const originalUseLocalID = cachePrototype.useLocalID;
let active = null;
cachePrototype.useLocalID = function(...args) {
  const result = originalUseLocalID.apply(this, args);
  if (active) active.local_cache_prefixes.push(this.localID);
  return result;
};
ref.output.measureText = function(text, variant, font) {
  const result = originalMeasure.apply(this, arguments);
  if (active) active.measurements.push({text, variant, font_family:font?.[0] ?? '',
    italic:font?.[1] ?? false, bold:font?.[2] ?? false,
    w:result.w, h:result.h, d:result.d});
  return result;
};
ref.output.processMath = function(wrapper, node) {
  const result = originalProcess.apply(this, arguments);
  if (active) {
    const flat = flattenMml(wrapper.node);
    active.mml = flat;
    active.root_bbox ??= box(wrapper.getBBox());
    const tables = findNodes(wrapper.node, 'mtable');
    assert.ok(tables.length, `${active.entry.name}: actual MML has no mtable`);
    active.tables = tables.map(table => captureTable(this, table, flat));
    active.stretch_operators = findNodes(wrapper.node, 'mo').map(operator => {
      const common = Array.from(this.nodeMap.values()).find(value => value.node === operator);
      assert.ok(common, `${active.entry.name}: mo has no Common wrapper`);
      return {node_index:flat.indexByNode.get(operator), bbox:box(common.getBBox()),
        stretchy:operator.attributes.get('stretchy') ?? null};
    });
    active.display = Boolean(this.math?.display);
  }
  return result;
};

async function capture(entry, cache) {
  const output = ref.output;
  output.options.fontCache = cache;
  output.options.localID = null;
  output.options.displayOverflow = entry.overflow ?? 'linebreak';
  output.clearFontCache();
  active = {entry, local_cache_prefixes:[], measurements:[], mml:null,
    tables:null, root_bbox:null, display:false};
  try {
    const node = await document.convertPromise(entry.mathml, {display:entry.display,
      em:entry.em ?? 16, ex:entry.ex ?? 8, containerWidth:entry.width ?? 1280});
    const svg = ref.adaptor.serializeXML(node);
    assert.ok(active.mml && active.tables && active.tables.length,
      `${entry.name}/${cache}: Source did not complete table wrappers`);
    assert.ok(svg, `${entry.name}/${cache}: Source returned empty SVG`);
    return {name:`${entry.name}-${cache}`, mathml:entry.mathml, requested_display:entry.display,
      display:active.display, em:entry.em ?? 16, ex:entry.ex ?? 8, width:entry.width ?? 1280,
      font_cache:cache, root_bbox:active.root_bbox, error_name:'', error_message:'', svg,
      local_cache_prefixes:active.local_cache_prefixes.slice(), tables:active.tables,
      measurements:active.measurements.slice(), stretch_operators:active.stretch_operators,
      nodes:active.mml.nodes,
      attributes:active.mml.attributes};
  } catch (error) {
    process.stderr.write(`${entry.name}-${cache}: Source error ${error.name}: ${error.message}\n`);
    throw error;
  } finally { active = null; }
}

function attribute(record, nodeIndex, name) {
  const node = record.nodes[nodeIndex];
  if (!node || node.first > record.attributes.length ||
      node.count > record.attributes.length-node.first) return undefined;
  const value = record.attributes.slice(node.first,node.first+node.count).find(item=>item.name===name);
  return value?.kind === 'text' ? value.text : value?.kind === 'boolean' ? value.boolean : value?.number;
}

function validateSourceRecord(record) {
  assert.equal(record.error_name,'',`${record.name}: Source conversion error`);
  assert.equal(record.display,true,`${record.name}: actual display state`);
  assert.ok(record.tables.length,`${record.name}: has table wrappers`);
  if (record.name.startsWith('table-profile-label-')) {
    const table = record.tables[0], attrs = ['side','align','minlabelspacing','data-width-includes-label'];
    assert.equal(table.has_labels,true,`${record.name}: labels consumed`);
    assert.equal(table.rows,2); assert.equal(table.cols,2);
    assert.ok(table.label_width > 0,`${record.name}: Source measured label width`);
    assert.ok(attrs.every(name=>attribute(record,table.node_index,name)!==undefined));
    const isLeft=record.name.includes('label-left');
    assert.equal(attribute(record,table.node_index,'side'),isLeft?'left':'right');
    assert.equal(attribute(record,table.node_index,'align'),isLeft?'left':'right');
    assert.equal(attribute(record,table.node_index,'data-width-includes-label'),isLeft);
  } else if (record.name.startsWith('table-profile-percent-child-nested')) {
    assert.equal(record.tables.length,2,`${record.name}: nested table wrappers`);
    const [parent,child] = record.tables;
    assert.ok(parent.cWidths.some(value=>value===6),`${record.name}: containing column is fixed at 6em`);
    assert.ok(child.pwidth > 0 && child.pwidth < 6,`${record.name}: percentage child width is consumed`);
    assert.ok(child.bbox.w > 0 && child.bbox.w <= 6,`${record.name}: nested table final width`);
    assert.equal(attribute(record,child.node_index,'width'),'65%');
  } else if (record.name.startsWith('table-profile-ragged-row-cell-align')) {
    const table = record.tables[0];
    const rows = record.nodes.map((node,index)=>({node,index})).filter(item=>item.node.parent===table.node_index&&item.node.kind==='mtr');
    const cellCounts = rows.map(row=>record.nodes.filter(node=>node.parent===row.index&&node.kind==='mtd').length);
    assert.deepEqual(cellCounts,[2,1],`${record.name}: second MML row is genuinely ragged`);
    const secondCell=record.nodes.findIndex(node=>node.parent===rows[1].index&&node.kind==='mtd');
    assert.ok(secondCell>=0,`${record.name}: second row has an actual cell`);
    assert.equal(attribute(record,secondCell,'rowalign'),'center',`${record.name}: cell-level override`);
  } else if (record.name.startsWith('table-profile-horizontal-stretch-column')) {
    const table=record.tables[0], op=record.stretch_operators[0];
    assert.equal(record.stretch_operators.length,1);
    assert.equal(op.stretchy,true,`${record.name}: input mo is stretchy`);
    assert.ok(Math.abs(table.W[0]-3)<1e-9,`${record.name}: constrained column stretched to 3em`);
    assert.ok(Math.abs(op.bbox.w-3)<1e-9,`${record.name}: Common operator selected the stretched width`);
  } else if (record.name.startsWith('table-profile-break-')) {
    assert.ok(record.svg.includes('data-mjx-linestack'),`${record.name}: Source linebreak stack`);
    assert.ok((record.svg.match(/data-mjx-linebox/g)||[]).length>=2,
      `${record.name}: Source produced multiple line boxes`);
  }
  if (record.font_cache==='local') assert.equal(record.local_cache_prefixes.length,1,`${record.name}: actual local cache prefix`);
  else assert.equal(record.local_cache_prefixes.length,0,`${record.name}: none cache has no local prefix`);
}

function cppString(value, prefix) {
  let rest = String(value), chunks = [];
  while (Buffer.byteLength(rest, 'utf8') > 3200) {
    let end = Math.min(rest.length, 3200);
    while (end > 0 && Buffer.byteLength(rest.slice(0,end), 'utf8') > 3200) end--;
    if (end > 0 && end < rest.length && /[\uD800-\uDBFF]/.test(rest[end-1])) end--;
    assert.ok(end > 0);
    chunks.push(rest.slice(0,end)); rest = rest.slice(end);
  }
  chunks.push(rest);
  return chunks.map((chunk,i) => {
    const delimiter = `${prefix}${i}`;
    assert.ok(delimiter.length <= 16 && !chunk.includes(`)${delimiter}"`));
    return `R"${delimiter}(${chunk})${delimiter}"`;
  }).join('\n    ');
}

function cppU16(value) {
  let out='u"';
  for (const char of String(value)) {
    const cp=char.codePointAt(0);
    if (char==='"') out+='\\"'; else if (char==='\\') out+='\\\\';
    else if (char==='\n') out+='\\n'; else if (char==='\r') out+='\\r';
    else if (char==='\t') out+='\\t'; else if (cp<0x20 || cp===0x7f) out+=`\\u${cp.toString(16).padStart(4,'0')}`;
    else out+=char;
  }
  return out+'"';
}

function cppAttribute(attribute) {
  const name = cppU16(attribute.name);
  if (attribute.kind === 'boolean') return `{${name}, mathml_scene_test::AttributeKind::boolean, {}, 0, ${attribute.boolean}}`;
  if (attribute.kind === 'number') return `{${name}, mathml_scene_test::AttributeKind::number, {}, ${attribute.number}, false}`;
  return `{${name}, mathml_scene_test::AttributeKind::text, ${cppU16(attribute.text)}, 0, false}`;
}

function emitTree(record, index) {
  const prefix = `tableProfile${index}`;
  assert.ok(record.nodes.length);
  const nodes = record.nodes.map(node => `{${cppString(node.kind,`K${index}`)},${node.inferred},`+
    `${node.parent === null ? 'std::numeric_limits<std::size_t>::max()' : node.parent},`+
    `${node.first},${node.count},${cppU16(node.text)}}`).join(',\n');
  const attrs = record.attributes.map(cppAttribute).join(',\n');
  let declarations = `constexpr std::array<mathml_scene_test::Node,${record.nodes.length}> ${prefix}Nodes{{\n${nodes}\n}};\n`+
    `constexpr std::array<mathml_scene_test::Attribute,${record.attributes.length}> ${prefix}Attributes{{\n${attrs}\n}};\n`;
  const tableValues = record.tables.map((table, tableIndex) => {
    const arrays = {};
    for (const key of ['H','D','W','NH','ND']) {
      assert.ok(table[key].length, `${record.name}: empty ${key}`);
      arrays[key] = `${prefix}T${tableIndex}${key}`;
      declarations += `constexpr std::array<double,${table[key].length}> ${arrays[key]}{{${table[key].join(',')}}};\n`;
    }
    return `TableProfileTable{${table.node_index},TableProfileBBox{${Object.values(table.bbox).join(',')}},`+
      `${table.rows},${table.cols},${table.has_labels},${table.is_top},${table.pwidth},`+
      `${arrays.H},${arrays.D},${arrays.W},${arrays.NH},${arrays.ND},${table.label_width}}`;
  }).join(',\n');
  declarations += `constexpr std::array<TableProfileTable,${record.tables.length}> ${prefix}Tables{{\n${tableValues}\n}};\n`;
  declarations += `constexpr std::array<std::string_view,${record.local_cache_prefixes.length}> ${prefix}Prefixes{{`+
    record.local_cache_prefixes.map((value,i)=>cppString(value,`P${index}${i}`)).join(',')+'}};\n';
  const measurementValues = record.measurements.map((measure,i) =>
    `TableProfileMeasurement{${cppU16(measure.text)},${cppString(measure.variant,`V${index}${i}`)},`+
    `${cppString(measure.font_family,`F${index}${i}`)},${measure.italic},${measure.bold},${measure.w},${measure.h},${measure.d}}`).join(',\n');
  declarations += `constexpr std::array<TableProfileMeasurement,${record.measurements.length}> ${prefix}Measurements{{\n${measurementValues}\n}};\n`;
  const value = `TableProfileSourceCase{${cppString(record.name,`N${index}`)},${cppString(record.mathml,`I${index}`)},`+
    `${record.requested_display},${record.display},${record.em},${record.ex},${record.width},`+
    `${cppString(record.font_cache,`C${index}`)},TableProfileBBox{${Object.values(record.root_bbox).join(',')}},`+
    `${cppString(record.error_name,`E${index}`)},${cppString(record.error_message,`M${index}`)},`+
    `${cppString(record.svg,`S${index}`)},${prefix}Prefixes,${prefix}Tables,${prefix}Measurements,${prefix}Nodes,${prefix}Attributes}`;
  return {declarations,value};
}

async function main() {
  const records = [];
  for (const entry of cases) for (const cache of ['none','local']) records.push(await capture(entry,cache));
  for (const record of records) validateSourceRecord(record);
  const provenance = {upstream:scope.upstream,profile_sha256:scope.profile.sha256,
    dependency_lock_sha256:scope.dependency_lock_sha256,output_profile:ref.provenance,
    scope_sha256:sha(fs.readFileSync(scopePath)), node:process.version};
  const generated = records.map(emitTree);
  const header = `// Pinned MathJax ${scope.upstream.version} ${scope.upstream.commit}; independent Source observations.\n`+
    `// full38 profile/NewCM output with actual MathML compiler; SVG bytes originate from Source.\n`+
    `// packages ${scope.profile.tex_packages.join(',')}\n`+
    `// dependency-lock-sha256 ${provenance.dependency_lock_sha256}\n`+
    `// compiled-source-sha256 ${ref.provenance.compiled_source_sha256}\n`+
    `// font-packages ${scope.font_packages.map(font=>`${font.package}@${font.version}:${font.integrity}`).join(' | ')}\n`+
    `// scope-sha256 ${provenance.scope_sha256}\n// profile-sha256 ${provenance.profile_sha256}\n`;
  const fixture = header + generated.map(item=>item.declarations).join('\n')+
    `\nconstexpr TableProfileSourceCase sourceTableProfileCases[]{\n`+
    generated.map(item=>'  '+item.value).join(',\n')+'\n};\n';
  const target = path.join(root,'tests/math/fixtures/table_profile_source.hpp');
  const previous = fs.existsSync(target) ? fs.readFileSync(target,'utf8') : null;
  const evidence = path.join(root,'build/math-port/table-profile-source.json');
  fs.mkdirSync(path.dirname(evidence),{recursive:true});
  fs.writeFileSync(evidence,JSON.stringify({provenance,records},null,2)+'\n');
  if (process.argv[2] === '--write-fixture') {
    if (previous !== fixture) fs.writeFileSync(target,fixture);
  } else assert.equal(previous,fixture,'Independent table profile fixture differs; review before --write-fixture');
  process.stdout.write(`Verified ${records.length} pinned Source table profile cases; evidence ${path.relative(root,evidence)}\n`);
}

main().catch(error=>{process.stderr.write(error.stack+'\n');process.exitCode=1;});
