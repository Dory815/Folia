'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
function repoRootFromArgs() {
  const args = process.argv.slice(2);
  const at = args.indexOf('--repo-root');
  assert.ok(at >= 0 && args[at + 1], 'usage: node common-scripts-real-source-witness.cjs --repo-root <repository-root>');
  return path.resolve(args[at + 1]);
}
const root = repoRootFromArgs();
const src = path.join(root, '.reference/MathJax-src/ts');
const cjs = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const font = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/mathjax-newcm-font');
const lock = JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/common-source/scripts-real-source-lock.json'), 'utf8'));
const files = {
  'core/MmlTree/MmlFactory.js': path.join(cjs, 'core/MmlTree/MmlFactory.js'),
  'core/MmlTree/MmlNodes/msubsup.js': path.join(cjs, 'core/MmlTree/MmlNodes/msubsup.js'),
  'core/MmlTree/MmlNodes/mmultiscripts.js': path.join(cjs, 'core/MmlTree/MmlNodes/mmultiscripts.js'),
  'input/mathml.js': path.join(cjs, 'input/mathml.js'),
  'output/common.js': path.join(cjs, 'output/common.js'),
  'output/common/Wrappers/scriptbase.js': path.join(cjs, 'output/common/Wrappers/scriptbase.js'),
  'output/common/Wrappers/msubsup.js': path.join(cjs, 'output/common/Wrappers/msubsup.js'),
  'output/common/Wrappers/mmultiscripts.js': path.join(cjs, 'output/common/Wrappers/mmultiscripts.js'),
  'output/svg.js': path.join(cjs, 'output/svg.js'),
  'output/svg/Wrappers/msubsup.js': path.join(cjs, 'output/svg/Wrappers/msubsup.js'),
  'output/svg/Wrappers/mmultiscripts.js': path.join(cjs, 'output/svg/Wrappers/mmultiscripts.js'),
  'newcm/cjs/svg.js': path.join(font, 'cjs/svg.js'),
  'newcm/cjs/common.js': path.join(font, 'cjs/common.js'),
  'ts/core/MmlTree/MmlFactory.ts': path.join(src, 'core/MmlTree/MmlFactory.ts'),
  'ts/core/MmlTree/MmlNodes/msubsup.ts': path.join(src, 'core/MmlTree/MmlNodes/msubsup.ts'),
  'ts/core/MmlTree/MmlNodes/mmultiscripts.ts': path.join(src, 'core/MmlTree/MmlNodes/mmultiscripts.ts'),
  'ts/input/mathml.ts': path.join(src, 'input/mathml.ts'),
  'ts/output/common.ts': path.join(src, 'output/common.ts'),
  'ts/output/common/Wrappers/scriptbase.ts': path.join(src, 'output/common/Wrappers/scriptbase.ts'),
  'ts/output/common/Wrappers/msubsup.ts': path.join(src, 'output/common/Wrappers/msubsup.ts'),
  'ts/output/common/Wrappers/mmultiscripts.ts': path.join(src, 'output/common/Wrappers/mmultiscripts.ts'),
  'ts/output/svg.ts': path.join(src, 'output/svg.ts'),
  'ts/output/svg/Wrappers/msubsup.ts': path.join(src, 'output/svg/Wrappers/msubsup.ts'),
  'ts/output/svg/Wrappers/mmultiscripts.ts': path.join(src, 'output/svg/Wrappers/mmultiscripts.ts'),
};
const sha256 = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
assert.equal(JSON.parse(fs.readFileSync(path.join(cjs, '../package.json'))).version, '4.1.3');
assert.equal(JSON.parse(fs.readFileSync(path.join(font, 'package.json'))).version, '4.1.3');
for (const [name, file] of Object.entries(files))
  assert.equal(sha256(file), lock.sha256[name], `pinned Source/runtime witness changed: ${name}`);

const exprs = {
  msub: '<msub><mi>x</mi><mi>i</mi></msub>',
  msup: '<msup><mi>x</mi><mn>2</mn></msup>',
  msubsup: '<msubsup><mi>x</mi><mi>i</mi><mn>2</mn></msubsup>',
  mmultiscripts: '<mmultiscripts><mi>x</mi><mi>i</mi><mn>2</mn><mprescripts/><mi>j</mi><mn>3</mn></mmultiscripts>',
};
const field = (box, names) => Object.fromEntries(names.filter(k => box[k] !== undefined).map(k => [k, box[k]]));
const boxFields = ['w', 'h', 'd', 'ic', 'sk', 'dx', 'rscale', 'scale'];
function mmlState(node) {
  const attrs = node.attributes?.getAllInherited?.() || {};
  const inherited = {};
  for (const key of ['displaystyle', 'scriptlevel', 'mathvariant', 'dir']) {
    const value = attrs[key];
    if (value !== undefined) inherited[key] = value;
  }
  return {
    kind: node.kind,
    text: node.getText?.() || '',
    texClass: node.texClass ?? null,
    inherited,
    children: (node.childNodes || []).map(mmlState),
  };
}
function findWrapper(wrapper, kind) {
  if (wrapper.node.kind === kind) return wrapper;
  for (const child of wrapper.childNodes || []) {
    const found = findWrapper(child, kind);
    if (found) return found;
  }
  return null;
}
function wrapperState(wrapper) {
  const bbox = wrapper.getOuterBBox();
  const state = {
    kind: wrapper.node.kind,
    text: wrapper.node.getText?.() || '',
    bbox: field(bbox, boxFields),
    texClass: wrapper.node.texClass ?? null,
    children: (wrapper.childNodes || []).map(wrapperState),
  };
  if (typeof wrapper.getOffset === 'function') state.offset = wrapper.getOffset();
  if (typeof wrapper.getUVQ === 'function') state.uvq = wrapper.getUVQ();
  if (wrapper.scriptData) {
    const d = wrapper.scriptData;
    state.scripts = {
      numScripts: d.numScripts,
      numPrescripts: d.numPrescripts,
      base: field(d.base, boxFields), sub: field(d.sub, boxFields),
      sup: field(d.sup, boxFields), psub: field(d.psub, boxFields), psup: field(d.psup, boxFields),
      firstPrescript: wrapper.firstPrescript,
    };
  }
  return state;
}
async function run() {
  const { mathjax } = require(path.join(cjs, 'mathjax.js'));
  const { MathML } = require(path.join(cjs, 'input/mathml.js'));
  const { MmlFactory } = require(path.join(cjs, 'core/MmlTree/MmlFactory.js'));
  assert.equal(typeof MmlFactory, 'function');
  const { SVG } = require(path.join(cjs, 'output/svg.js'));
  const { liteAdaptor } = require(path.join(cjs, 'adaptors/liteAdaptor.js'));
  const { RegisterHTMLHandler } = require(path.join(cjs, 'handlers/html.js'));
  const { MathJaxNewcmFont } = require(path.join(font, 'cjs/svg.js'));
  const adaptor = liteAdaptor({fontSize: 16});
  RegisterHTMLHandler(adaptor);
  const cases = {};
  for (const [name, expression] of Object.entries(exprs)) {
    const output = new SVG({fontCache: 'none', linebreaks: {inline: true}, fontData: new MathJaxNewcmFont()});
    let actualRootWrapper;
    const processMath = output.processMath;
    output.processMath = function(wrapper, parent) {
      actualRootWrapper = wrapper;
      return processMath.call(this, wrapper, parent);
    };
    const document = mathjax.document('', {InputJax: new MathML(), OutputJax: output});
    await document.convertPromise(`<math xmlns="http://www.w3.org/1998/Math/MathML">${expression}</math>`,
      {display: false, em: 16, ex: 8, containerWidth: 640});
    assert.ok(actualRootWrapper, `${name}: real SVG processMath path was not invoked`);
    const rootNode = output.math.root;
    assert.equal(rootNode.kind, 'math');
    assert.deepEqual(rootNode.childNodes.map(n => n.kind), ['inferredMrow']);
    assert.equal(rootNode.getProperty('inlineMarked'), true);
    assert.equal(rootNode.getProperty('inlineForced'), true);
    const inferred = rootNode.childNodes[0];
    assert.equal(inferred.childNodes.length, 1);
    assert.equal(inferred.childNodes[0].kind, name);
    const rootState = wrapperState(actualRootWrapper);
    const scriptWrapper = findWrapper(actualRootWrapper, name);
    assert.ok(scriptWrapper);
    const rootBox = actualRootWrapper.getOuterBBox();
    cases[name] = {
      input: expression,
      setup: {root: 'math', inferredChild: 'inferredMrow', display: false, inlineBreaks: true,
        inlineMarked: rootNode.getProperty('inlineMarked'), inlineForced: rootNode.getProperty('inlineForced'),
        inheritedInputTree: mmlState(rootNode)},
      rootBBox: field(rootBox, boxFields),
      mmlTree: rootState,
      script: wrapperState(scriptWrapper),
    };
  }
  return {mathjax: '4.1.3', font: '@mathjax/mathjax-newcm-font 4.1.3', metrics: {em:16,ex:8,containerWidth:640,display:false}, cases};
}
run().then(result => {
  const outFile = path.join(root, 'build/math-port/common-scripts-real-source-actual.json');
  fs.mkdirSync(path.dirname(outFile), {recursive: true});
  fs.writeFileSync(outFile, `${JSON.stringify(result, null, 2)}\n`);
  const expected = JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/common-source/scripts-real-source-expected.json'), 'utf8'));
  assert.deepEqual(result, expected, 'actual MathML/SVG/NewCM outputs differ from locked observations');
  process.stdout.write(`${JSON.stringify({ok:true, cases:Object.keys(result.cases), output:path.relative(root, outFile)})}\n`);
}).catch(error => { console.error(error); process.exitCode = 1; });
