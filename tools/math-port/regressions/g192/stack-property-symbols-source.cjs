'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../../../..');
const packageRoot = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const manifest = JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'), 'utf8'));
assert.equal(manifest.version, '4.1.3');
const cjsPath = path.join(packageRoot, 'cjs/input/tex/StackItem.js');
const expectedCjsSha256 = '53baA91d7c27475a91a9b6993235071ab32b04a29736c5920bdda2836ab5a91b'.toLowerCase();
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(cjsPath)).digest('hex'), expectedCjsSha256);
const tsPath = path.join(root, '.reference/MathJax-src/ts/input/tex/StackItem.ts');
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(tsPath)).digest('hex'),
  'ac138d321e1efbc54afbd961a4b6bfa24f7d0923eddba1c8d891dee9424602b5');
const { BaseItem } = require(path.join(packageRoot, 'cjs/input/tex/StackItem.js'));
const item = new BaseItem({});
const properties = item._properties;
assert.equal(Object.getPrototypeOf(properties), Object.prototype);
const absent = Symbol('absent');
assert.equal(properties[absent], undefined);
assert.deepEqual(Reflect.ownKeys(properties), []);
const alpha = Symbol('alpha'), beta = Symbol('beta'), delta = Symbol('delta');
item.setProperty(alpha, 17);
assert.equal(item.getProperty(alpha), 17);
Object.defineProperty(properties, beta, { value: 23, writable: true, enumerable: false, configurable: true });
item.setProperty('z', 'z-value');
item.setProperty('2', 'two');
item.setProperty('1', 'one');
assert.equal(item.getProperty(alpha), 17);
assert.equal(Object.getOwnPropertyDescriptor(properties, alpha).enumerable, true);
assert.equal(Object.getOwnPropertyDescriptor(properties, beta).enumerable, false);
let setterCalls = 0, setterValue;
Object.defineProperty(properties, delta, { set(value) { setterCalls++; setterValue = value; }, enumerable: true, configurable: true });
item.setProperties({ [alpha]: 41, [delta]: 47, [beta]: 49 });
assert.equal(item.getProperty(alpha), 41);
assert.equal(setterCalls, 1);
assert.equal(setterValue, 47);
assert.deepEqual(Reflect.ownKeys(properties), ['1', '2', 'z', alpha, beta, delta]);
assert.deepEqual(Object.getOwnPropertySymbols(properties), [alpha, beta, delta]);
const assigned = Object.assign({}, properties);
assert.equal(assigned[alpha], 41);
assert.equal(Object.hasOwn(assigned, beta), false);
assert.equal(assigned.z, 'z-value');
assert.equal(Object.prototype.toString.call(properties), '[object Object]');
assert.equal(delete properties[alpha], true);
assert.equal(item.getProperty(alpha), undefined);
assert.deepEqual(Object.getOwnPropertySymbols(properties), [beta, delta]);
const sourceOrder = [], readSymbol = Symbol('read-symbol'), lateSymbol = Symbol('late-symbol'), skippedSymbol = Symbol('skipped-symbol');
const getterSource = {};
Object.defineProperty(getterSource, 'first', {
  enumerable: true,
  get() { sourceOrder.push('string'); delete getterSource[lateSymbol]; return 53; }
});
Object.defineProperty(getterSource, readSymbol, {
  enumerable: true,
  get() { sourceOrder.push('symbol'); return 59; }
});
Object.defineProperty(getterSource, lateSymbol, {
  enumerable: true,
  configurable: true,
  get() { sourceOrder.push('late'); return 61; }
});
Object.defineProperty(getterSource, skippedSymbol, {
  enumerable: false,
  get() { sourceOrder.push('non-enumerable'); return 67; }
});
item.setProperties(getterSource);
assert.deepEqual(sourceOrder, ['string', 'symbol']);
assert.equal(item.getProperty('first'), 53);
assert.equal(item.getProperty(readSymbol), 59);
assert.equal(item.getProperty(lateSymbol), undefined);
assert.equal(item.getProperty(skippedSymbol), undefined);
process.stdout.write('Pinned MathJax 4.1.3 StackItem CJS Symbol-key witness passed.\n');

