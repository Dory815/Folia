'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../../..', '..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourceRoot = path.join(root, '.reference/MathJax-src');
const digest = (bytes) => crypto.createHash('sha256').update(bytes).digest('hex');
const pins = {
  'ts/core/MmlTree/MmlNode.ts': '2694e886f6a2ed1f957d81f75e3967f1f2cf49076fd01142b59a8d0a4eb6bfb9',
  'cjs/core/MmlTree/MmlNode.js': '86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45',
  'ts/core/MmlTree/MmlFactory.ts': 'e792675db055402962e9a80a245c5be8948ef53ac4db7b41eba06154f4bf91ab',
  'cjs/core/MmlTree/MmlFactory.js': 'd0c4e25237d04a9d21d12ee44776e46d30777e7f284db6f4f3ed1cc625b752d3',
  'ts/core/MmlTree/MmlNodes/mrow.ts': 'db8f0e5a8055ad240bef35034df074d8800841e1e0e164e40581913449c637a5',
  'cjs/core/MmlTree/MmlNodes/mrow.js': '5846d2b0334223b759a4059dfb0ac5e31aef1f5a96c041da45b3de94c0689896'
};
for (const [relative, expected] of Object.entries(pins)) {
  const installed = fs.readFileSync(path.join(packageRoot, relative));
  assert.equal(digest(installed), expected, `pinned ${relative}`);
  if (relative.startsWith('ts/')) {
    const locked = fs.readFileSync(path.join(sourceRoot, relative));
    assert.equal(installed.toString('utf8').replaceAll('\r\n', '\n'), locked.toString('utf8').replaceAll('\r\n', '\n'));
  }
}
const treeRoot = path.join(packageRoot, 'cjs/core/MmlTree/');
const {MmlFactory} = require(path.join(treeRoot, 'MmlFactory.js'));
const {AbstractMmlNode} = require(path.join(treeRoot, 'MmlNode.js'));
const factory = new MmlFactory();
let factoryGetterCalls = 0;
let factorySetterCalls = 0;
const otherFactory = {marker: 'foreign'};
Object.defineProperty(AbstractMmlNode.prototype, 'factory', {
  configurable: true,
  get() { ++factoryGetterCalls; return otherFactory; },
  set(_value) { ++factorySetterCalls; }
});
const merror = factory.create('merror', {}, []);
assert.equal(factoryGetterCalls, 0, 'constructor initialization itself does not read the factory accessor');
assert.ok(factorySetterCalls > 0, 'constructor writes the factory parameter through the inherited setter');
const observedFactory = merror.factory;
assert.equal(observedFactory, otherFactory, 'an explicit factory read observes the inherited getter');
assert.equal(factoryGetterCalls, 1);
const mrow = factory.create('mrow');
assert.equal(mrow.isInferred, false, 'pinned MmlNode.isInferred returns false for an ordinary mrow');
console.log(JSON.stringify({
  source: 'pinned @mathjax/src CJS MmlNode/MmlFactory/MmlMrow',
  factoryGetterCallsAfterExplicitRead: factoryGetterCalls,
  factorySetterCalls,
  explicitFactoryReadMatchesAccessor: observedFactory === otherFactory,
  mrowIsInferred: mrow.isInferred
}, null, 2));