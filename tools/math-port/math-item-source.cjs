'use strict';
// Executes actual MathItem state/compile, Tags start/finish and MathDocument compile loop.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const source = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const load = file => require(path.join(source, 'cjs', file));
const {AbstractMathItem, STATE} = load('core/MathItem.js');
const {AbstractMathDocument} = load('core/MathDocument.js');
const {AbstractTags} = load('input/tex/Tags.js');
const {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const nf = new MmlFactory();
const cases = [];
{
  const item = new AbstractMathItem('x', null, false);
  cases.push({name: 'actual-initial-defaults', source: item.math, display: item.display, escaped: item.isEscaped,
    root: item.root, typesetRoot: item.typesetRoot, metrics: item.metrics, inputData: item.inputData,
    outputData: item.outputData, state: item.state()});
  assert.deepEqual(item.metrics, {}); assert.equal(item.state(), 0);
  const escaped = new AbstractMathItem('$', null, null);
  assert.equal(escaped.isEscaped, true);
  cases.push({name: 'escaped-is-distinct-from-inline', display: escaped.display, escaped: escaped.isEscaped});
}
{
  const item = new AbstractMathItem('x', null, false);
  const rootNode = item.root = nf.create('math');
  // Output marker is unrelated to state transition implementation; Source state never dereferences it.
  const outputMarker = item.typesetRoot = {marker: 'typeset-owner'};
  item.setMetrics(16, 8, 400, 1);
  item.inputData.recompile = {state: 0, counter: 7}; item.outputData.mtextFamily = 'serif';
  item.state(STATE.TYPESET); item.state(STATE.COMPILED - 1);
  assert.deepEqual(item.inputData, {}); assert.deepEqual(item.outputData, {});
  assert.equal(item.root, rootNode); assert.equal(item.typesetRoot, outputMarker);
  cases.push({name: 'rollback-clears-data-at-thresholds-but-retains-roots-and-metrics', state: item.state(),
    inputData: item.inputData, outputData: item.outputData, rootRetained: item.root === rootNode,
    typesetRootRetained: item.typesetRoot === outputMarker, metrics: item.metrics});
}
{
  const tags = new AbstractTags(); tags.allCounter = 7;
  const rounds = [];
  const jax = {compile(item) {
    // This is a bounded input-jax stub; observed MathItem/Tags methods are actual originals.
    tags.resetTag(); tags.startEquation(item);
    const before = {state: item.state(), counter: tags.counter, allCounter: tags.allCounter,
      refUpdate: tags.refUpdate, recompile: structuredClone(item.inputData.recompile || null)};
    tags.counter += 2; tags.redo = rounds.length === 0;
    tags.ids['observed-id'] = true;
    tags.labels['observed-label'] = {tag: '9', id: 'observed-id'};
    tags.finishEquation(item);
    rounds.push({before, after: {state: item.state(), counter: tags.counter, allCounter: tags.allCounter,
      refUpdate: tags.refUpdate, recompile: structuredClone(item.inputData.recompile || null)}});
    return nf.create('math');
  }};
  const item = new AbstractMathItem('x', jax, false);
  const calls = [];
  const document = {
    math: [item],
    // MathDocument's processed bitset and error policy are unrelated services explicitly stubbed.
    processed: {isSet() {return false;}, set(name) {calls.push(name);}},
    options: {compileError(_doc, _item, error) {throw error;}},
    compileMath: AbstractMathDocument.prototype.compileMath
  };
  AbstractMathDocument.prototype.compile.call(document);
  assert.equal(rounds.length, 2); assert.equal(rounds[0].before.state, 0);
  assert.deepEqual(rounds[0].after.recompile, {state: 0, counter: 7});
  assert.equal(rounds[0].after.allCounter, 9); assert.equal(rounds[1].before.counter, 7);
  assert.equal(rounds[1].before.refUpdate, true); assert.equal(rounds[1].after.allCounter, 9);
  assert.equal(item.state(), 20); assert.deepEqual(item.inputData.recompile, {state: 0, counter: 7});
  cases.push({name: 'actual-document-recompile-loop-restores-state-and-record-before-second-input-pass',
    rounds, final: {state: item.state(), recompile: item.inputData.recompile,
      allCounter: tags.allCounter, allIds: tags.allIds, allLabels: tags.allLabels, processedCalls: calls}});
}
{
  const tags = new AbstractTags(); tags.allCounter = 10; tags.resetTag();
  const item = new AbstractMathItem('x', null, true);
  item.state(19); item.inputData.recompile = {state: 19, counter: 3};
  tags.startEquation(item); tags.counter = 5; tags.redo = true; tags.finishEquation(item);
  assert.equal(tags.allCounter, 10); assert.deepEqual(item.inputData.recompile, {state: 19, counter: 10});
  cases.push({name: 'redo-during-refupdate-stores-current-precommit-session-counter-and-exact-item-state',
    state: item.state(), refUpdate: tags.refUpdate, counter: tags.counter, allCounter: tags.allCounter,
    recompile: item.inputData.recompile});
}
const files = ['core/MathItem', 'core/MathDocument', 'input/tex', 'input/tex/Tags',
  'input/tex/base/BaseMethods', 'input/tex/bussproofs/BussproofsUtil'];
const sourceHashes = {};
const hash = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
for (const file of files) for (const [folder, extension] of [['cjs', '.js'], ['mjs', '.js'], ['ts', '.ts']]) {
  sourceHashes[folder + '/' + file + extension] = hash(path.join(source, folder, file + extension));
}
const output = {version: '4.1.3', sourceHashes, cases};
const expected = path.join(root, 'tests/math/fixtures/core-source/math-item-source-expected.json');
assert.deepEqual(output, JSON.parse(fs.readFileSync(expected)), 'MathItem Source observations changed');
process.stdout.write(`PASS: ${cases.length} MathItem/Tags Source cases, ${Object.keys(sourceHashes).length} file hashes.\n`);
