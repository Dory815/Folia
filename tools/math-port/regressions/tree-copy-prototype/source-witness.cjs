'use strict';

// Pinned MathJax 4.1.3 Node.copy witness for the PropertyList copy fix.
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const root = path.resolve(__dirname, '../../../..');
const ts = path.join(root, '.reference/MathJax-src/ts/core/Tree/Node.ts');
const runtime = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/Tree/Node.js');
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(ts)).digest('hex'),
  '532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0');
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(runtime)).digest('hex'),
  '94174004b89f19bf3c98e8eec5fabdd0bcbf1e13e0b740963fa69554ad8a12a7');

const { AbstractNode } = require(runtime);
class Probe extends AbstractNode {
  get kind() { return 'probe'; }
}
const factory = { create(kind, properties = {}, children = []) {
  assert.equal(kind, 'probe');
  return new Probe(factory, properties, children);
} };

const inherited = { inherited: 7 };
const source = new Probe(factory);
Object.defineProperty(source.properties, '__proto__', {
  value: inherited, enumerable: true, writable: true, configurable: true
});
const objectCopy = source.copy();
assert.equal(Object.hasOwn(objectCopy.properties, '__proto__'), false);
assert.equal(Object.getPrototypeOf(objectCopy.properties), inherited);
assert.equal(objectCopy.getProperty('inherited'), 7);

const scalarSource = new Probe(factory);
Object.defineProperty(scalarSource.properties, '__proto__', {
  value: 13, enumerable: true, writable: true, configurable: true
});
const scalarCopy = scalarSource.copy();
assert.equal(Object.hasOwn(scalarCopy.properties, '__proto__'), false);
assert.equal(Object.getPrototypeOf(scalarCopy.properties), Object.prototype);
assert.equal(scalarCopy.getProperty('__proto__'), Object.prototype);

process.stdout.write('pinned Node.copy __proto__ assignment semantics passed\n');
