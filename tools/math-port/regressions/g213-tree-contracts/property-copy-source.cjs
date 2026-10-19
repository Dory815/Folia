'use strict';
// Source witness for Object.assign as called by pinned MathJax 4.1.3 AbstractNode.copy().
// CJS Node.js SHA-256: 94174004b89f19bf3c98e8eec5fabdd0bcbf1e13e0b740963fa69554ad8a12a7
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const cjsPath = path.resolve(__dirname, '..', '..', '..', 'mathjax-bundle', 'node_modules', '@mathjax', 'src', 'cjs', 'core', 'Tree', 'Node.js');
const digest = '94174004b89f19bf3c98e8eec5fabdd0bcbf1e13e0b740963fa69554ad8a12a7';
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(cjsPath)).digest('hex'), digest);
const { AbstractNode } = require(cjsPath);
const factory = { create() { return new AbstractNode(factory); } };
const node = new AbstractNode(factory);
const props = node.properties;
const flip = Symbol('flip-to-enumerable');
const drop = Symbol('flip-to-nonenumerable');
const deleted = Symbol('deleted-during-getter');
const late = Symbol('added-during-getter');
let reads = 0;
Object.defineProperty(props, 'first', { enumerable: true, configurable: true, get() {
  ++reads;
  Object.defineProperty(props, 'hidden', { enumerable: true });
  Object.defineProperty(props, 'gone', { enumerable: false });
  delete props[deleted];
  Object.defineProperty(props, 'lateString', { enumerable: true, value: 8 });
  Object.defineProperty(props, late, { enumerable: true, value: 9 });
  Object.defineProperty(props, flip, { enumerable: true });
  Object.defineProperty(props, drop, { enumerable: false });
  return 1;
} });
Object.defineProperty(props, 'hidden', { enumerable: false, configurable: true, value: 2 });
Object.defineProperty(props, 'gone', { enumerable: true, configurable: true, value: 3 });
Object.defineProperty(props, 'deletedString', { enumerable: true, configurable: true, value: 4 });
Object.defineProperty(props, flip, { enumerable: false, configurable: true, value: 5 });
Object.defineProperty(props, drop, { enumerable: true, configurable: true, value: 6 });
Object.defineProperty(props, deleted, { enumerable: true, configurable: true, value: 7 });
const inherited = { marker: 10 };
Object.defineProperty(props, '__proto__', { enumerable: true, value: inherited });
const copied = node.copy().properties;
assert.equal(reads, 1);
assert.deepEqual(Object.keys(copied), ['first', 'hidden', 'deletedString']);
assert.equal(copied.first, 1);
assert.equal(copied.hidden, 2); // existing own key became enumerable before its turn
assert.equal(Object.hasOwn(copied, 'gone'), false); // existing key became non-enumerable
assert.equal(Object.hasOwn(copied, 'lateString'), false); // new key was outside the key snapshot
assert.equal(copied[flip], 5); // snapshotted Symbol became enumerable before its turn
assert.equal(Object.hasOwn(copied, drop), false); // snapshotted Symbol became non-enumerable
assert.equal(Object.hasOwn(copied, deleted), false); // deleted snapshotted Symbol skipped
assert.equal(Object.hasOwn(copied, late), false); // new Symbol was outside the key snapshot
assert.equal(Object.getPrototypeOf(copied), inherited);
assert.equal(Object.hasOwn(copied, '__proto__'), false);
console.log('pinned AbstractNode.copy Object.assign witness passed');
