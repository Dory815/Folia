'use strict';
// Real profile ParseOptions/MmlFactory and configured creators; no replacement parser.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const {createReference} = require('./reference-profile.cjs');
const root = path.resolve(__dirname, '../..');
const locks = {
  '.reference/MathJax-src/ts/input/tex/NodeFactory.ts': 'bf20e8d4f7ed43450df42d65556ccbceea1469541b214c37544f180aa205c840',
  '.reference/MathJax-src/ts/input/tex/NodeUtil.ts': 'bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2',
  '.reference/MathJax-src/ts/input/tex/boldsymbol/BoldsymbolConfiguration.ts': 'f017f7d31a5f6bf17a2d9d2468cf515b1f7e5948009a55304578bb93ffddb334',
  'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/boldsymbol/BoldsymbolConfiguration.js': 'b552f0c41e5929e19d967897118cb92960f873eb7a8a21e467d532158f6dfccb'
};
for (const [file, hash] of Object.entries(locks))
  assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(root, file))).digest('hex'), hash, file);
const ref = createReference();
assert.equal(ref.provenance.compiled_source_sha256, 'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const options = ref.input.parseOptions;
const factory = options.nodeFactory;
factory.setMmlFactory(ref.document.mmlFactory);
const Factory = factory.constructor;
const original = Object.fromEntries(['node', 'token', 'text', 'error'].map(kind => [kind, factory.get(kind)]));
const events = [];
const save = options.addNode;
options.addNode = function(kind, node) { events.push(`save:${kind}`); return save.call(this, kind, node); };
const node = (f, kind, children, definition, text) => {
  events.push(`node:${kind}`);
  return Factory.createNode(f, kind, children, definition, text);
};
factory.setCreators({
  node,
  token: (f, ...args) => { events.push('token'); return Factory.createToken(f, ...args); },
  text: (f, value) => { events.push('text'); return Factory.createText(f, value); },
  error: (f, value) => { events.push('error'); return Factory.createError(f, value); }
});
const token = factory.create('token', 'mi', {}, 'x');
assert.deepEqual(events, ['token', 'text', 'node:mi', 'save:mi']);
assert.equal(token.childNodes[0].getText(), 'x');
assert.equal(token.childNodes[0].parent, token);
events.length = 0;
const failure = factory.create('error', 'bad');
assert.deepEqual(events, ['error', 'text', 'node:mtext', 'save:mtext', 'node:merror', 'save:merror']);
assert.equal(failure.childNodes[0].isInferred, true);
assert.equal(failure.childNodes[0].childNodes[0].kind, 'mtext');
assert.equal(failure.childNodes[0].childNodes[0].childNodes[0].getText(), 'bad');
assert.equal(failure.attributes.get('data-mjx-error'), 'bad');
events.length = 0;
factory.set('text', (f, value) => {
  events.push('change-text');
  f.set('node', (inner, ...args) => { events.push('changed-node'); return Factory.createNode(inner, ...args); });
  return Factory.createText(f, value);
});
factory.create('token', 'mi', {}, 'x');
assert.deepEqual(events, ['token', 'change-text', 'changed-node', 'save:mi']);
factory.setCreators({text: original.text, node: (f, ...args) => {
  Factory.createNode(f, ...args); throw new Error('creator failure');
}});
events.length = 0;
assert.throws(() => factory.create('node', 'mi', [], {}), /creator failure/);
assert.deepEqual(events, []);
factory.setCreators({node: Factory.createNode, token: Factory.createToken, text: Factory.createText, error: Factory.createError});
const mo = factory.create('node', 'mo', [], {texClass: 1, movablelimits: false, fnOP: true});
assert.equal(mo.texClass, 1);
assert.equal(mo.getProperty('texClass'), 1);
assert.equal(mo.getProperty('movablelimits'), false);
assert.equal(mo.attributes.getExplicit('movablelimits'), false);
assert.equal(mo.getProperty('fnOP'), true);
assert.equal(mo.attributes.getExplicit('fnOP'), undefined);
const mi = factory.create('node', 'mi', [], {movablelimits: false});
assert.equal(mi.getProperty('movablelimits'), false);
assert.equal(mi.attributes.getExplicit('movablelimits'), undefined);
assert.equal(factory.create('text', null), null);
assert.equal(factory.create('text', '').getText(), '');

// Execute the real configured boldsymbol creator during real TeX parsing.
factory.setCreators(original);
const observed = [];
factory.set('token', (f, kind, definition, text) => {
  const result = original.token(f, kind, definition, text);
  observed.push({kind, text, marked: result.getProperty('fixBold') === true});
  return result;
});
ref.document.convert(String.raw`\boldsymbol{x}y`, {display: false, em: 16, ex: 8, containerWidth: 1280});
assert.equal(observed.some(row => row.kind === 'mi' && row.text === 'x' && row.marked), true);
assert.equal(observed.some(row => row.kind === 'mi' && row.text === 'y' && !row.marked), true);
// With no active parser, mtext still short-circuits the environment read.
assert.equal(options.parser, undefined);
factory.create('token', 'mtext', {}, 'text');
assert.equal(factory.create('token', 'mtext', {}, null).childNodes.length, 0);
assert.equal(factory.create('token', 'mtext', {}).childNodes.length, 1);
assert.equal(factory.create('token', 'mtext', {}, '').childNodes.length, 1);
events.length = 0;
assert.throws(() => factory.create('token', 'mi', {}, 'outside'));
assert.deepEqual(events, ['save:mi']);

const {rewriteBoldTokens} = ref.require('@mathjax/src/cjs/input/tex/boldsymbol/BoldsymbolConfiguration.js');
const variants = [
  ['normal', 'bold'], ['italic', 'bold-italic'], ['fraktur', 'bold-fraktur'], ['script', 'bold-script'],
  ['sans-serif', 'bold-sans-serif'], ['-tex-calligraphic', '-tex-bold-calligraphic'],
  ['-tex-oldstyle', '-tex-bold-oldstyle'], ['-tex-mathit', 'bold-italic'],
  ['double-struck', 'double-struck'], ['custom', 'custom']
];
for (const [from, to] of variants) {
  const current = Factory.createToken(factory, 'mi', {mathvariant: from}, 'x');
  current.setProperty('fixBold', true);
  options.root = current;
  options.nodeLists.fixBold = [current];
  rewriteBoldTokens({data: options});
  assert.equal(current.attributes.get('mathvariant'), to);
  assert.equal(current.getProperty('fixBold'), undefined);
  current.setProperty('fixBold', false);
  rewriteBoldTokens({data: options});
  assert.equal(current.getProperty('fixBold'), false);
}
const {TeX} = ref.require('@mathjax/src/cjs/input/tex.js');
const {MmlFactory} = ref.require('@mathjax/src/cjs/core/MmlTree/MmlFactory.js');
const {AbstractMathItem} = ref.require('@mathjax/src/cjs/core/MathItem.js');
const jax = new TeX({packages: ['base', 'boldsymbol']});
jax.setMmlFactory(new MmlFactory());
function actualTokens(tex) {
  const math = new AbstractMathItem(tex, jax, false), nodes = [];
  jax.compile(math, ref.document).walkTree(n => {if (n.isToken) nodes.push(n);});
  return nodes;
}
let nodes = actualTokens(String.raw`\boldsymbol{x+\boldsymbol{y}}z`);
assert.deepEqual(nodes.map(n => n.getText()), ['x','+','y','z']);
assert.deepEqual(nodes.map(n => n.attributes.get('mathvariant')), ['bold-italic','bold','bold-italic','italic']);
assert.equal(jax.parseOptions.getList('fixBold').length, 3);
assert.ok(jax.parseOptions.getList('fixBold').every(n => n.getProperty('fixBold') === undefined));
assert.equal(jax.parseOptions.parsers.length, 0);
nodes = actualTokens(String.raw`\boldsymbol{\Gamma+\alpha+1+\mathbb{R}+\mathrm{x}}z`);
assert.deepEqual(nodes.map(n => n.attributes.get('mathvariant')), ['bold','bold','bold-italic','bold','bold','bold','double-struck','bold','bold','italic']);
nodes = actualTokens(String.raw`\boldsymbol{\text{hello}}y`);
assert.deepEqual(nodes.map(n => n.kind), ['mtext','mi']);
assert.deepEqual(nodes.map(n => n.attributes.get('mathvariant')), ['normal','italic']);
assert.equal(jax.parseOptions.getList('fixBold').length, 0);
console.log(JSON.stringify({status: 'passed', sourceLocks: 4, compiledProfileLocked: true,
  variants: variants.length, actualPackageCases: 3}));
