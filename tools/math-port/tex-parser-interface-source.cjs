'use strict';
// Executes pinned methods. Configuration services below are explicitly bounded stubs.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const source = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const load = x => require(path.join(source, 'cjs', x));
const TexParser = load('input/tex/TexParser.js').default;
const {TextParser} = load('input/tex/textmacros/TextParser.js');
const Factory = load('input/tex/StackItemFactory.js').default;
const {BaseItem} = load('input/tex/StackItem.js');
const Items = load('input/tex/base/BaseItems.js');
const {MmlFactory} = load('core/MmlTree/MmlFactory.js');
const {NodeFactory} = load('input/tex/NodeFactory.js');
assert.equal(JSON.parse(fs.readFileSync(path.join(source, 'package.json'))).version, '4.1.3');
const cases = [];

// Actual TextParser ctor/Parse/Push and actual Stack/items/factories. Empty input
// deliberately avoids package dispatch. Tags.finalize and node indexing are stubs.
{
  const events = [];
  const factory = new Factory({base: BaseItem, start: Items.StartItem,
    stop: Items.StopItem, mml: Items.MmlItem});
  const nf = new NodeFactory(); nf.setMmlFactory(new MmlFactory());
  const configuration = {options: {}, nodeFactory: nf,
    pushParser(p) { events.push(['enter', p.string, p.i, p.currentCS, p.macroCount]); },
    addNode() {}, tags: {finalize(node) { return node; }}, itemFactory: factory};
  factory.configuration = configuration; nf.configuration = configuration;
  const create = factory.create;
  factory.create = function(kind, ...args) {
    events.push(['create', kind]); return create.call(this, kind, ...args);
  };
  const parse = TextParser.prototype.Parse, push = TextParser.prototype.Push;
  TextParser.prototype.Parse = function() {
    events.push(['derived-Parse', this.level === undefined]);
    return parse.call(this);
  };
  TextParser.prototype.Push = function(item) {
    events.push(['derived-Push', item.kind, this.level === undefined]);
    return push.call(this, item);
  };
  try {
    const seed = {font: 'normal', isInner: false};
    const p = new TextParser('', seed, configuration, 7);
    cases.push({name: 'derived-construction', events, level: p.level,
      nodes: p.nodes.length, text: p.text, envStack: p.envStack.length,
      seedInnerDeleted: !Object.hasOwn(seed, 'isInner'),
      envIsCopy: p.stack.env !== seed, env: {...p.stack.env},
      isInner: p.stack.global.isInner});
  } finally { TextParser.prototype.Parse = parse; TextParser.prototype.Push = push; }
}

// Execute parse itself, while a handler records the explicit input receiver and
// saveI visible during dispatch. Prev returns null, avoiding unrelated latex work.
{
  const p = Object.create(TexParser.prototype), foreign = Object.create(TexParser.prototype);
  const events = []; let prev = 0;
  Object.assign(p, {_string: '\u{1D6FC}x', i: 2, saveI: 9,
    stack: {Prev(noPop) { assert.equal(noPop, true); prev++; return null; }},
    configuration: {handlers: {get(kind) { return {parse(input) {
      events.push([kind, input[0] === foreign, input[1], p.saveI]); return false;
    }}; }}}});
  const outcomes = [p.parse('character', [foreign, '\u{1D6FC}']),
    p.parse('macro', [foreign, 'x']), p.parse('character', [foreign, '&'])];
  cases.push({name: 'parse-receiver-and-utf16', events, outcomes,
    finalSaveI: p.saveI, prevCalls: prev});
}

// Execute Push/PushAll/setter on a real frame/real inferred MML. Stack.Push is
// a recording boundary, so this case asserts parser stamping/flattening only.
{
  const p = Object.create(TexParser.prototype), pushed = [];
  Object.assign(p, {_string: '\u{1D6FC}x', i: 2, saveI: 0,
    stack: {Push(x) { pushed.push(x); }}});
  const factory = new Factory({base: BaseItem, open: Items.OpenItem});
  const frame = factory.create('open');
  p.Push(frame);
  const old = {source: frame.startStr, start: frame.startI, stop: frame.stopI};
  p.string = 'Z';
  const positionAfterReplacement = p.i;
  p.i = 0; p.Push(frame);
  const mml = new MmlFactory(), a = mml.create('mi'), b = mml.create('mn');
  const inferred = mml.create('inferredMrow', {}, [a, b]);
  p.Push(inferred);
  const plain = factory.create('open');
  const bulkBefore = [plain.startStr, plain.startI, plain.stopI];
  p.PushAll([plain]);
  cases.push({name: 'push-provenance-and-flattening', old, positionAfterReplacement,
    current: {source: frame.startStr, start: frame.startI, stop: frame.stopI},
    sameFrameTwice: pushed[0] === pushed[1],
    inferredChildrenInOrder: pushed[2] === a && pushed[3] === b,
    bulkDoesNotStamp: [plain.startStr, plain.startI, plain.stopI]
      .every((value, i) => value === bulkBefore[i]), total: pushed.length});
}

const actual = {cases};
const expected = JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/tex-source/parser-interface-source-expected.json')));
assert.deepEqual(actual, expected);
const files = Object.keys(require.cache).filter(x => x.startsWith(source + path.sep))
  .map(x => path.relative(source, x).replaceAll('\\', '/')).sort();
const hashes = Object.fromEntries(files.map(x => [x,
  crypto.createHash('sha256').update(fs.readFileSync(path.join(source, x))).digest('hex')]));
assert.deepEqual(hashes, JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/tex-source/parser-interface-source-lock.json'))));
const mapHashes = JSON.parse(fs.readFileSync(path.join(root, 'tests/math/fixtures/tex-source/parser-interface-method-map-source-lock.json')));
for (const [file, expectedHash] of Object.entries(mapHashes)) {
  assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(source, file)))
    .digest('hex'), expectedHash, file);
}
process.stdout.write(JSON.stringify({status: 'passed', cases: cases.length,
  runtimeSourceFiles: files.length, methodMapSources: Object.keys(mapHashes).length}) + '\n');
