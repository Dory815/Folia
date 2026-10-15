'use strict';
// Supplementary pinned CJS witness for an inherited BaseItem constructor and
// StartItem's ordinary (factory, global) constructor arguments.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');

const root = path.resolve(__dirname, '../../../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourceFiles = [
  ['ts/input/tex/StackItem.ts', 'AC138D321E1EFBC54AFBD961A4B6BFA24F7D0923EDDBA1C8D891DEE9424602B5'],
  ['ts/input/tex/base/BaseItems.ts', '0C0D8448F3577BE6642C95A412682310362819D2D383AB83B817D2888E4E5F0C'],
];
const cjsFiles = [
  ['cjs/input/tex/StackItem.js', '53BAA91D7C27475A91A9B6993235071AB32B04A29736C5920BDDA2836AB5A91B'],
  ['cjs/input/tex/base/BaseItems.js', '5310D0429B7B62D405AE9B93C8D21AD17B787E5EB3FD1F01439C62F2094F7D1A'],
];
const sha256 = (file) => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex').toUpperCase();
for (const [relative, expected] of sourceFiles) {
  assert.equal(sha256(path.join(root, '.reference/MathJax-src', relative)), expected, `${relative} source lock`);
}
for (const [relative, expected] of cjsFiles) {
  assert.equal(sha256(path.join(packageRoot, relative)), expected, `${relative} CJS lock`);
}

const { TeX } = require(path.join(packageRoot, 'cjs/input/tex.js'));
const { BaseItem } = require(path.join(packageRoot, 'cjs/input/tex/StackItem.js'));
const { StartItem } = require(path.join(packageRoot, 'cjs/input/tex/base/BaseItems.js'));
const jax = new TeX({ packages: ['base'] });
const factory = jax.parseOptions.itemFactory;

class AlphaItem extends BaseItem {
  constructor(factory) { super(factory); }
  get kind() { return 'alpha'; }
  toString() { return 'α'; }
}
assert.equal(AlphaItem.length, 1);
const alphaKind = Object.getOwnPropertyDescriptor(AlphaItem.prototype, 'kind');
const alphaToString = Object.getOwnPropertyDescriptor(AlphaItem.prototype, 'toString');
assert.ok(alphaKind && typeof alphaKind.get === 'function');
assert.ok(alphaToString && typeof alphaToString.value === 'function');
assert.equal(alphaToString.value.length, 0);
assert.equal(alphaToString.enumerable, false);
const alpha = new AlphaItem(factory);
assert.equal(alpha.factory, factory);
assert.equal(alpha.toString(), 'α');

const global = { from: 'StartItem argument' };
const start = new StartItem(factory, global);
assert.equal(StartItem.length, 2);
assert.equal(start.factory, factory);
assert.equal(start.global, global);
console.log(JSON.stringify({ alphaLength: AlphaItem.length, startItemLength: StartItem.length,
  alphaOwnKind: Object.hasOwn(AlphaItem.prototype, 'kind'),
  alphaOwnToString: Object.hasOwn(AlphaItem.prototype, 'toString'),
  alphaUsesInheritedFactory: alpha.factory === factory,
  startUsesFactoryAndGlobalArguments: start.factory === factory && start.global === global }));
