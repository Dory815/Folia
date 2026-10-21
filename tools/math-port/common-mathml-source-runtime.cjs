'use strict';

// One pinned MathJax MathML->Common->SVG capture boundary shared by the
// table-specific and generic direct-MathML witnesses.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

function createMathmlSourceRuntime({root, scope, scopePath, captureWrapperBBoxes = true,
                                    observeWrapper = null, captureXmlNode = null}) {
  const hash = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
  const {createReference} = require('./reference-profile.cjs');
  const ref = createReference();
  assert.deepEqual(ref.provenance.packages, scope.profile.tex_packages);
  const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
  const cjsRoot = path.join(packageRoot, 'cjs');
  const {MathML} = require(path.join(cjsRoot, 'input/mathml.js'));
  const document = ref.mathjax.document('', {InputJax: new MathML(), OutputJax: ref.output});
  const fields = ['w', 'h', 'd', 'scale', 'rscale', 'L', 'R', 'pwidth'];

  function captureMml(node) {
    if (['xml', 'XML', 'html', 'HTML'].includes(node.kind) && !captureXmlNode)
      throw new Error(`Unsupported embedded ${node.kind} payload in direct-MathML Source fixture`);
    const result = {
      kind: node.kind,
      isInferred: node.isKind('inferredMrow'),
      attributes: node.attributes?.getAllAttributes() ?? {},
      children: node.childNodes.map(captureMml)
    };
    if (node.kind === 'XML') result.xml = captureXmlNode(node, node.getXML(), node.adaptor);
    if (node.kind === 'text') result.text = node.getText();
    return result;
  }

  function captureWrapper(wrapper) {
    const box = wrapper.getBBox();
    const result = {
      kind: wrapper.node.kind,
      bbox: Object.fromEntries(fields.map(key => [key, box[key]]))
    };
    return result;
  }

  let active = null;
  const processMath = ref.output.processMath;
  ref.output.processMath = function(wrapper, node) {
    const result = processMath.apply(this, arguments);
    assert.ok(active, 'Source capture must have an active input');
    active.mml = captureMml(wrapper.node);
    const start = captureXmlNode && this.math?.start?.node;
    if (start) {
      const parent = this.adaptor.parent(start);
      active.measurementParent = {startKind: this.adaptor.kind(start),
        parentKind: parent ? this.adaptor.kind(parent) : null};
    }
    if (captureWrapperBBoxes) {
      active.wrappers = Array.from(this.nodeMap.values(), wrapper => {
        const captured = captureWrapper(wrapper);
        if (observeWrapper) observeWrapper(wrapper, captured);
        return captured;
      });
    }
    return result;
  };

  async function convert(entry, metrics) {
    assert.equal(active, null, 'MathML Source conversion cannot be nested');
    active = {};
    const prior = {};
    if (Object.hasOwn(entry, 'canMeasureNodes')) {
      prior.canMeasureNodes = ref.adaptor.canMeasureNodes;
      ref.adaptor.canMeasureNodes = entry.canMeasureNodes;
    }
    const outputOptions = entry.outputOptions || {};
    if (Object.hasOwn(outputOptions, 'htmlHDW')) {
      prior.htmlHDW = ref.output.options.htmlHDW;
      ref.output.options.htmlHDW = outputOptions.htmlHDW;
      if (outputOptions.htmlHDW === 'auto') ref.output.setAdaptor(ref.adaptor);
    }
    for (const [name, value] of Object.entries(outputOptions)) {
      if (name === 'htmlHDW') continue;
      prior[name] = ref.output.options[name];
      ref.output.options[name] = value;
    }
    try {
      const node = await document.convertPromise(entry.mathml, metrics);
      const svg = ref.adaptor.serializeXML(node);
      assert.ok(active.mml && (active.wrappers || !captureWrapperBBoxes) && svg,
        `${entry.name}: incomplete Source output`);
      const result = {mml: active.mml, wrappers: active.wrappers, svg};
      if (captureXmlNode && active.measurementParent) result.measurementParent = active.measurementParent;
      return result;
    } finally {
      active = null;
      for (const [name, value] of Object.entries(prior)) {
        if (name === 'canMeasureNodes') ref.adaptor.canMeasureNodes = value;
        else ref.output.options[name] = value;
      }
    }
  }

  function mathmlModules() {
    const prefix = path.join(cjsRoot, 'input/mathml');
    return Object.keys(require.cache).filter(file => file.startsWith(prefix)).sort()
      .map(file => {
        const compiled = path.relative(cjsRoot, file).replaceAll(path.sep, '/');
        const source = compiled.replace(/\.js$/, '.ts');
        const sourcePath = path.join(root, '.reference/MathJax-src/ts', source);
        return {compiled, compiledSha256: hash(file), source, sourceSha256: hash(sourcePath)};
      });
  }

  function provenance(input, casesPath) {
    const modules = mathmlModules();
    return {
      provenance: {upstream: scope.upstream, input, inputJaxInCurrentScope: false,
        scopeSha256: hash(scopePath), casesSha256: hash(casesPath), outputProfile: ref.provenance,
        mathmlModules: modules},
      modules
    };
  }

  return {convert, provenance};
}

module.exports = {createMathmlSourceRuntime};
