'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const root = path.resolve(__dirname, '../..');
const bundle = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const lock = {
  source: {
    'input/tex/NodeFactory.ts': 'BF20E8D4F7ED43450DF42D65556CCBCEEA1469541B214C37544F180AA205C840',
    'input/tex/ParseOptions.ts': 'AB60A07DBB24040C0FB9BD67B942755FFC5B0B18980ABF1938736D8D90B22209',
  },
  cjs: {
    'input/tex/NodeFactory.js': '426E3CA8822D209EDC8A22B5E9F11C5631E52213B232220A2A55E81828374DB5',
    'input/tex/ParseOptions.js': 'FF19D70539CB76911C5F1D7673DDFD908183DCF5849C91CB068B19E58DA8A9EC',
    'core/MmlTree/MmlFactory.js': 'D0C4E25237D04A9D21D12EE44776E46D30777E7F284DB6F4F3ED1CC625B752D3',
  },
};
const sha = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex').toUpperCase();
for (const [file, digest] of Object.entries(lock.source))
  assert.equal(sha(path.join(root, '.reference/MathJax-src/ts', file)), digest, `pinned TS changed: ${file}`);
for (const [file, digest] of Object.entries(lock.cjs))
  assert.equal(sha(path.join(bundle, 'src/cjs', file)), digest, `pinned CJS changed: ${file}`);
assert.equal(JSON.parse(fs.readFileSync(path.join(bundle, 'src/package.json'), 'utf8')).version, '4.1.3');

const { NodeFactory } = require(path.join(bundle, 'src/cjs/input/tex/NodeFactory.js'));
const ParseOptions = require(path.join(bundle, 'src/cjs/input/tex/ParseOptions.js')).default;
const { MmlFactory } = require(path.join(bundle, 'src/cjs/core/MmlTree/MmlFactory.js'));
const factory = new NodeFactory();
const mmlFactory = new MmlFactory();
const registrations = [];
const requestedKinds = [];
const configuration = Object.create(ParseOptions.prototype);
configuration.nodeLists = {};
const sourceAddNode = ParseOptions.prototype.addNode;
configuration.addNode = function (kind, node) {
  registrations.push({ kind, actualKind: node.kind });
  return sourceAddNode.call(this, kind, node);
};
const sourceCreate = mmlFactory.create.bind(mmlFactory);
mmlFactory.create = function (kind, ...args) {
  requestedKinds.push(kind);
  return sourceCreate(kind, ...args);
};
factory.configuration = configuration;
factory.setMmlFactory(mmlFactory);

const text = factory.create('text', 'standalone');
const token = factory.create('token', 'mi', {}, '𝑥');
const error = factory.create('error', 'bad input');
const inferred = factory.create('node', 'inferredMrow');
let invalidTokenError = '';
try { factory.create('token', 'not-a-kind', {}, 'z'); }
catch (error) { invalidTokenError = error.message; }

assert.equal(text.kind, 'text');
assert.deepEqual(token.childNodes.map(node => node.kind), ['text']);
assert.deepEqual(error.childNodes.map(node => node.kind), ['inferredMrow']);
assert.deepEqual(error.childNodes[0].childNodes.map(node => node.kind), ['mtext']);
assert.equal(inferred.kind, 'inferredMrow');
assert.deepEqual(registrations, [
  { kind: 'mi', actualKind: 'mi' },
  { kind: 'mtext', actualKind: 'mtext' },
  { kind: 'merror', actualKind: 'merror' },
  { kind: 'inferredMrow', actualKind: 'inferredMrow' },
]);
assert.deepEqual(requestedKinds, [
  'text', 'text', 'mi', 'text', 'mtext', 'merror',
  'inferredMrow', 'inferredMrow', 'text', 'not-a-kind',
]);
assert.equal(invalidTokenError, 'this.node[kind] is not a function');
process.stdout.write(JSON.stringify({
  registrations,
  mmlFactoryCreateOrder: requestedKinds,
  tokenChildren: token.childNodes.map(node => node.kind),
  errorChildren: error.childNodes.map(node => node.kind),
  errorInferredChildren: error.childNodes[0].childNodes.map(node => node.kind),
  inferredKind: inferred.kind,
  invalidTokenError,
  limitation: 'Source invalid-token path creates its text node before the generic unknown-kind failure; the registration observer receives no event for that text node.',
}, null, 2) + '\n');
