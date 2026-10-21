'use strict';

// Independent pinned MathML -> Common -> SVG witness for XMLNode/foreignObject.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

assert.ok(process.argv.length === 2 ||
  (process.argv.length === 3 && process.argv[2] === '--write-fixture'),
  'usage: common-xml-source.cjs [--write-fixture]');
const root = path.resolve(__dirname, '../..');
const scopePath = path.join(root, 'docs/math-port/scope.json');
const scope = JSON.parse(fs.readFileSync(scopePath, 'utf8'));
const cases = [
  {name: 'xml-unknown-measurement-mixed-payload', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'auto', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><section class="xml-probe" data-order="a&amp;b"><span title="A &amp; B">Alpha &amp; Beta</span><!--inside--><br/><svg xmlns="http://www.w3.org/2000/svg"><text>G</text></svg></section></annotation-xml></semantics></math>'},
  {name: 'xml-force-nonzero-hdw', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'force', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><div data-mjx-hdw="1em .25em 3em"><span>F</span></div></annotation-xml></semantics></math>'},
  {name: 'xml-use-nonzero-hdw', canMeasureNodes: true,
    outputOptions: {htmlHDW: 'use', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><div data-mjx-hdw=".75em .125em 2.5em"><i>U</i></div></annotation-xml></semantics></math>'},
  {name: 'xml-ignore-hdw-measures', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'ignore', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><div data-mjx-hdw="1em .25em 3em"><b>I</b></div></annotation-xml></semantics></math>'},
  {name: 'xml-automatic-measurable-policy', canMeasureNodes: true,
    outputOptions: {htmlHDW: 'auto', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><div data-mjx-hdw=".5em .2em 2em"><span>A</span></div></annotation-xml></semantics></math>'},
  {name: 'xml-automatic-force-policy', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'auto', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><div data-mjx-hdw=".5em .2em 2em"><span>B</span></div></annotation-xml></semantics></math>'},
  {name: 'xml-scale-not-one', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'ignore', scale: 2},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><article><em>S</em></article></annotation-xml></semantics></math>'},
  {name: 'xml-scale-two-force-hdw', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'force', scale: 2},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><div data-mjx-hdw="1em .25em 3em"><span>Scale</span></div></annotation-xml></semantics></math>'},
  // Source CommonWrapper consumes the authored parent font-family style into
  // removedStyles before CommonXmlNode.getFontStyles; this case deliberately
  // records the resulting Times fallback, not a hit on the authored-family tier.
  {name: 'xml-authored-font-style-removed-times-fallback', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'ignore', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml style="font-family: ProbeFamily" encoding="application/xhtml+xml"><div>font</div></annotation-xml></semantics></math>'},
  {name: 'xml-after-preceding-math', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'force', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><mrow><mi>x</mi><semantics><annotation-xml encoding="application/xhtml+xml"><div><br/><svg xmlns="http://www.w3.org/2000/svg"><text>Y</text></svg></div></annotation-xml></semantics></mrow></math>'},
  {name: 'xml-empty-element', canMeasureNodes: false,
    outputOptions: {htmlHDW: 'ignore', scale: 1},
    mathml: '<math xmlns="http://www.w3.org/1998/Math/MathML"><semantics><annotation-xml encoding="application/xhtml+xml"><div/></annotation-xml></semantics></math>'}
].map(entry => ({...entry, metrics: {em: 16, ex: 8, container_width: 640, display: false}}));

const {createMathmlSourceRuntime} = require('./common-mathml-source-runtime.cjs');
const runtime = createMathmlSourceRuntime({root, scope, scopePath,
  captureXmlNode(node, sourceNode, adaptor) {
    assert.ok(sourceNode && adaptor, 'Source XMLNode must retain its real Lite payload and adaptor');
    function visit(current) {
      const kind = adaptor.kind(current);
      const result = {kind, attributes: {}, children: []};
      if (kind === '#text' || kind === '#comment') {
        result.text = adaptor.value(current);
        return result;
      }
      for (const attribute of adaptor.allAttributes(current))
        result.attributes[attribute.name] = attribute.value;
      result.children = adaptor.childNodes(current).map(visit);
      return result;
    }
    const payload = visit(sourceNode);
    payload.serialized = adaptor.outerHTML(sourceNode);
    return payload;
  },
  observeWrapper(wrapper, result) {
    if (wrapper.node.kind === 'annotation-xml') {
      result.annotationXmlStyles = wrapper.constructor.styles;
      return;
    }
    if (wrapper.node.kind !== 'XML') return;
    const html = wrapper.getHTML();
    const start = wrapper.jax.math.start.node;
    const measurementParent = wrapper.adaptor.parent(start);
    const bbox = wrapper.getBBox();
    result.xml = {
      bbox: Object.fromEntries(['w', 'h', 'd', 'scale', 'rscale'].map(key => [key, bbox[key]])),
      metrics: {em: wrapper.metrics.em, ex: wrapper.metrics.ex, scale: wrapper.metrics.scale,
        family: wrapper.metrics.family || null},
      htmlHDW: wrapper.jax.options.htmlHDW,
      canMeasureNodes: wrapper.adaptor.canMeasureNodes,
      outputScale: wrapper.jax.options.scale,
      styles: wrapper.getFontStyles(),
      getHTML: wrapper.adaptor.outerHTML(html),
      wrapperStyles: wrapper.constructor.styles,
      measurementParent: {startKind: wrapper.adaptor.kind(start),
        parentKind: measurementParent ? wrapper.adaptor.kind(measurementParent) : null}
    };
  }
});

function cppString(value) { return JSON.stringify(value); }
function cppU16(value) { return `u${cppString(value)}`; }
function cppRaw(value) {
  const result = [];
  let part = '', bytes = 0;
  for (const scalar of value) {
    const size = Buffer.byteLength(scalar);
    if (bytes + size > 3500 && part) { result.push(part); part = ''; bytes = 0; }
    part += scalar;
    bytes += size;
  }
  if (part || !result.length) result.push(part);
  return result.map(text => {
    let delimiter = 'FOLIAXML';
    for (let suffix = 0; text.includes(`)${delimiter}"`); ++suffix)
      delimiter = `FOLIAXML${suffix + 1}`;
    return `R"${delimiter}(${text})${delimiter}"`;
  }).join('\n      ');
}
function cppAttribute(name, value) {
  const key = cppU16(name);
  if (typeof value === 'boolean')
    return `{${key}, mathml_scene_test::AttributeKind::boolean, {}, 0, ${value}}`;
  if (typeof value === 'number') {
    assert.ok(Number.isFinite(value), `Nonfinite Source attribute ${name}`);
    return `{${key}, mathml_scene_test::AttributeKind::number, {}, ${value}, false}`;
  }
  assert.equal(typeof value, 'string', `Unsupported Source attribute ${name}`);
  return `{${key}, mathml_scene_test::AttributeKind::text, ${cppU16(value)}, 0, false}`;
}

function cppSourceStyles(sourceStyles) {
  const entries = [];
  for (const [selector, declarations] of Object.entries(sourceStyles)) {
    assert.ok(declarations && typeof declarations === 'object' && !Array.isArray(declarations),
      `Unsupported Source style declaration for ${selector}`);
    for (const [property, value] of Object.entries(declarations)) {
      if (typeof value === 'string')
        entries.push(`  {${cppU16(selector)}, ${cppU16(property)}, XmlSourceStyleKind::text, ${cppU16(value)}, 0}`);
      else if (typeof value === 'number' && Number.isFinite(value))
        entries.push(`  {${cppU16(selector)}, ${cppU16(property)}, XmlSourceStyleKind::number, {}, ${value}}`);
      else
        assert.fail(`Unsupported Source style value ${selector}.${property}`);
    }
  }
  assert.ok(entries.length, 'Source XML/HTML class style table is empty');
  return entries;
}

function cppTree(record, index) {
  const prefix = `xml${index}`;
  const nodes = [], attributes = [], xmlPayloads = [];
  function visit(node, parent) {
    assert.equal(node.isInferred, node.kind === 'inferredMrow');
    const at = nodes.length, first = attributes.length;
    const own = Object.entries(node.attributes || {});
    attributes.push(...own.map(([name, value]) => cppAttribute(name, value)));
    let xmlIndex = 'std::numeric_limits<std::size_t>::max()';
    if (node.kind === 'XML') {
      assert.ok(node.xml?.serialized, `${record.name}: Source XML payload missing`);
      xmlIndex = String(xmlPayloads.length);
      xmlPayloads.push(node.xml.serialized);
    }
    nodes.push(`  {${cppString(node.kind)}, ${node.isInferred}, ${parent}, ${first}, ${own.length}, ` +
      `${cppU16(node.kind === 'text' ? node.text || '' : '')}, ${xmlIndex}}`);
    node.children.forEach(child => visit(child, at));
  }
  visit(record.mml, 'std::numeric_limits<std::size_t>::max()');
  assert.ok(xmlPayloads.length, `${record.name}: expected an XML payload`);
  const declarations =
    `constexpr std::array<mathml_scene_test::Node, ${nodes.length}> ${prefix}Nodes{{\n${nodes.join(',\n')}\n}};\n` +
    `constexpr std::array<mathml_scene_test::Attribute, ${attributes.length}> ${prefix}Attributes{{\n` +
    `${attributes.length ? '  ' + attributes.join(',\n  ') : ''}\n}};\n` +
    `constexpr std::array<std::u16string_view, ${xmlPayloads.length}> ${prefix}XmlPayloads{{\n  ` +
    `${xmlPayloads.map(cppU16).join(',\n  ')}\n}};\n`;
  const xmlWrappers = record.wrappers.filter(wrapper => wrapper.kind === 'XML');
  assert.equal(xmlWrappers.length, xmlPayloads.length, `${record.name}: Source XML wrappers/payloads`);
  const observations = xmlWrappers.map(wrapper => {
    assert.ok(wrapper.xml && Object.values(wrapper.xml.bbox).every(Number.isFinite),
      `${record.name}: XML wrapper bbox capture`);
  return `  {${wrapper.xml.bbox.w}, ${wrapper.xml.bbox.h}, ${wrapper.xml.bbox.d}, ` +
      `${wrapper.xml.bbox.scale}, ${wrapper.xml.bbox.rscale}, ${cppU16(wrapper.xml.styles['font-family'])}, ` +
      `${cppU16(wrapper.xml.styles['font-size'])}, ${cppRaw(wrapper.xml.getHTML)}, ` +
      `${cppRaw(JSON.stringify(wrapper.xml.wrapperStyles))}, ${cppString(wrapper.xml.htmlHDW)}, ` +
      `${wrapper.xml.canMeasureNodes}, ${wrapper.xml.outputScale}, ${wrapper.xml.metrics.scale}, ${wrapper.xml.metrics.em}, ` +
      `${wrapper.xml.metrics.ex}, ${cppU16(wrapper.xml.metrics.family || '')}}`;
  });
  const observationDeclaration =
    `constexpr std::array<XmlSourceObservation, ${observations.length}> ${prefix}Observations{{\n` +
    `${observations.join(',\n')}\n}};\n`;
  const source = `mathml_scene_test::Case{${cppString(record.name)},` +
    `mathml_scene_test::Metrics{${record.metrics.em},${record.metrics.ex},${record.metrics.container_width},${record.metrics.display}},` +
    `${prefix}Nodes,${prefix}Attributes,${cppRaw(record.svg)}}`;
  const option = record.outputOptions.htmlHDW;
  const policy = {auto: 'automatic', force: 'force', use: 'use', ignore: 'ignore'}[option];
  assert.ok(policy, `${record.name}: unsupported htmlHDW option ${option}`);
  const measurementParent = record.measurementParent;
  assert.ok(measurementParent?.startKind && measurementParent.parentKind,
    `${record.name}: actual Source math.start parent capture missing`);
  return {declarations: declarations + observationDeclaration,
    value: `{${source}, XmlHdwPolicy::${policy}, ${record.outputOptions.scale}, ` +
    `${record.canMeasureNodes}, ${cppString(measurementParent.startKind)}, ${cppString(measurementParent.parentKind)}, ` +
    `${prefix}XmlPayloads, ${prefix}Observations}`};
}

function find(node, kind, output = []) {
  if (node.kind === kind) output.push(node);
  node.children.forEach(child => find(child, kind, output));
  return output;
}
function validate(records) {
  for (const record of records) {
    const xml = find(record.mml, 'XML');
    const wrappers = record.wrappers.filter(wrapper => wrapper.kind === 'XML');
    assert.ok(xml.length > 0, `${record.name}: compiled Source MML has XMLNode`);
    assert.equal(xml.length, wrappers.length, `${record.name}: each XMLNode wrapper observed`);
    assert.ok(record.svg.includes('<foreignObject'), `${record.name}: Source emits foreignObject`);
    assert.ok(wrappers.every(wrapper => wrapper.xml?.getHTML.includes('mjx-html')),
      `${record.name}: capture actual CommonXmlNode.getHTML result`);
    const annotationWrappers = record.wrappers.filter(wrapper => wrapper.kind === 'annotation-xml');
    assert.ok(annotationWrappers.length > 0 &&
      annotationWrappers.every(wrapper => wrapper.annotationXmlStyles),
      `${record.name}: capture actual Source annotation-xml constructor.styles`);
    if (record.name === 'xml-unknown-measurement-mixed-payload') {
      const payload = xml[0].xml;
      assert.ok(payload.serialized.includes('section') && payload.serialized.includes('data-order="a&amp;b"'),
        'Source frozen XML payload retains ordered attributes and escaped text');
      assert.ok(payload.serialized.includes('<!--inside-->') && payload.serialized.includes('<br'),
        'nested comments and HTML void elements are in Source payload');
      assert.ok(find(payload, 'svg').length && find(payload, 'text').length,
        'mixed SVG namespace subtree remains nested in the Source payload');
      assert.ok(Object.values(wrappers[0].xml.bbox).slice(0, 3).every(value => value === 0),
        'actual pinned Lite nodeBBox makes unknown content dimensions zero');
    }
    if (record.name === 'xml-force-nonzero-hdw' || record.name === 'xml-use-nonzero-hdw' ||
        record.name === 'xml-automatic-force-policy' || record.name === 'xml-scale-two-force-hdw')
      assert.ok(wrappers[0].xml.bbox.w > 0 && wrappers[0].xml.bbox.h > 0,
        `${record.name}: Source HDW supplies nonzero dimensions`);
    assert.equal(wrappers[0].xml.outputScale, record.outputOptions.scale,
      `${record.name}: actual Source output option scale is recorded`);
    assert.ok(Number.isFinite(wrappers[0].xml.metrics.scale) && wrappers[0].xml.metrics.scale > 0,
      `${record.name}: actual Source wrapper metrics scale is recorded`);
    if (record.name === 'xml-authored-font-style-removed-times-fallback') {
      const annotation = find(record.mml, 'annotation-xml')[0];
      assert.equal(annotation.attributes.style, 'font-family: ProbeFamily',
        'Source MML retains the authored parent font style');
      assert.equal(wrappers[0].xml.styles['font-family'], 'Times',
        'Source removes the authored family before the XML wrapper font-style tier');
      assert.equal(wrappers[0].xml.metrics.family, null,
        'Source metrics family is unset, so the actual adaptor Times fallback applies');
    }
    if (record.name === 'xml-after-preceding-math') {
      const firstGlyph = record.svg.indexOf('data-c=');
      const foreign = record.svg.indexOf('<foreignObject');
      assert.ok(firstGlyph >= 0 && foreign > firstGlyph,
        'Source foreignObject follows preceding math content');
    }
    assert.ok(record.measurementParent?.startKind && record.measurementParent.parentKind,
      `${record.name}: actual math.start.node parent is recorded`);
  }
}

async function main() {
  const records = [];
  for (const entry of cases) {
    const metrics = {em: entry.metrics.em, ex: entry.metrics.ex,
      containerWidth: entry.metrics.container_width, display: entry.metrics.display};
    records.push({...entry, ...await runtime.convert(entry, metrics)});
  }
  validate(records);
  const {provenance, modules} = runtime.provenance('MathML', __filename);
  const header = [
    `// Source MathJax ${scope.upstream.version} ${scope.upstream.commit}`,
    '// Input: pinned direct MathML XMLNode/foreignObject witness; not TeX package coverage.',
    `// scope-sha256 ${provenance.scopeSha256}`,
    ...modules.map(module => `// ${module.compiled} ${module.compiledSha256} | ${module.source} ${module.sourceSha256}`)
  ].join('\n');
  const generated = records.map(cppTree);
  const sourceWrapperStyles = records[0].wrappers.find(wrapper => wrapper.kind === 'XML').xml.wrapperStyles;
  const sourceAnnotationXmlStyles = records[0].wrappers.find(
    wrapper => wrapper.kind === 'annotation-xml').annotationXmlStyles;
  for (const record of records)
    assert.deepEqual(record.wrappers.find(wrapper => wrapper.kind === 'XML').xml.wrapperStyles,
      sourceWrapperStyles, `${record.name}: Source XML wrapper style registration remains stable`);
  for (const record of records)
    assert.deepEqual(record.wrappers.find(wrapper => wrapper.kind === 'annotation-xml').annotationXmlStyles,
      sourceAnnotationXmlStyles, `${record.name}: Source annotation-xml wrapper style registration remains stable`);
  const sourceStyleEntries = cppSourceStyles(sourceWrapperStyles);
  const sourceAnnotationXmlStyleEntries = cppSourceStyles(sourceAnnotationXmlStyles);
  const fixture = header + '\n' +
    'enum class XmlHdwPolicy { automatic, use, force, ignore };\n' +
    'enum class XmlSourceStyleKind { text, number };\n' +
    'struct XmlSourceStyleEntry { std::u16string_view selector,property; XmlSourceStyleKind kind; std::u16string_view text; double number; };\n' +
    'struct XmlSourceObservation { double w,h,d,scale,rscale; std::u16string_view family,font_size; std::string_view html,wrapper_styles,source_html_hdw; bool can_measure_nodes; double output_scale,metrics_scale,em,ex; std::u16string_view metrics_family; };\n' +
    'struct XmlSourceCase { mathml_scene_test::Case source; XmlHdwPolicy hdw; double scale; bool can_measure_nodes; std::string_view start_kind,start_parent_kind; std::span<std::u16string_view const> payloads; std::span<XmlSourceObservation const> observations; };\n' +
    `constexpr XmlSourceStyleEntry sourceXmlStyleEntries[]{\n${sourceStyleEntries.join(',\n')}\n};\n` +
    `constexpr XmlSourceStyleEntry sourceAnnotationXmlStyleEntries[]{\n${sourceAnnotationXmlStyleEntries.join(',\n')}\n};\n` +
    generated.map(item => item.declarations).join('\n') +
    '\nconstexpr XmlSourceCase sourceXmlCases[]{\n' + generated.map(item => `  ${item.value}`).join(',\n') + '\n};\n';
  const evidencePath = path.join(root, 'build/math-port/common-source/xml-mathml-scene.json');
  fs.mkdirSync(path.dirname(evidencePath), {recursive: true});
  fs.writeFileSync(evidencePath, JSON.stringify({provenance, cases: records}, null, 2) + '\n');
  const target = path.join(root, 'tests/math/fixtures/common-source/xml-mathml-scene.inc');
  const previous = fs.existsSync(target) ? fs.readFileSync(target, 'utf8') : null;
  if (process.argv[2] === '--write-fixture') {
    if (previous !== fixture) fs.writeFileSync(target, fixture);
  } else {
    assert.equal(previous, fixture, 'Independent XML Source fixture differs; review before --write-fixture');
  }
  process.stdout.write(`Verified ${records.length} independent XML Source cases\n`);
}

main().catch(error => {process.stderr.write(error.stack + '\n'); process.exitCode = 1;});
