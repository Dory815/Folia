'use strict';
const assert = require('node:assert/strict');
const { createReference } = require('../../reference-profile.cjs');
const reference = createReference();
const { adaptor } = reference;

const node = adaptor.node('div');
const arrayLike = { length: 2, 0: 'left', 1: 'right' };
node.children = arrayLike;
const copied = adaptor.childNodes(node);
assert.notEqual(copied, arrayLike);
assert.deepEqual(copied, ['left', 'right']);
assert.equal(adaptor.firstChild(node), 'left');
assert.equal(adaptor.childNode(node, 1), 'right');

node.children = 'xy';
const fromString = adaptor.childNodes(node);
assert.deepEqual(fromString, ['x', 'y']);
assert.equal(adaptor.firstChild(node), 'x');

const sparse = ['unused', 'tail'];
delete sparse[0];
sparse[Symbol.iterator] = null;
node.children = sparse;
const sparseCopy = adaptor.childNodes(node);
assert.notEqual(sparseCopy, sparse);
assert.equal(sparseCopy.length, 2);
assert.equal(Object.hasOwn(sparseCopy, '0'), false);
assert.equal(Object.hasOwn(sparseCopy, '1'), true);
assert.equal(sparseCopy[1], 'tail');

const trace = [];
const accessorNode = {};
Object.defineProperty(accessorNode, 'children', {
  configurable: true,
  get() { trace.push('children'); return arrayLike; },
});
assert.deepEqual(adaptor.childNodes(accessorNode), ['left', 'right']);
assert.deepEqual(trace, ['children']);

let nullError;
node.children = null;
try { adaptor.childNodes(node); }
catch (error) { nullError = { name: error.name, message: error.message }; }
assert.equal(nullError?.name, 'TypeError');

process.stdout.write(JSON.stringify({
  provenance: reference.provenance.upstream,
  arrayLike: { copy: copied, first: 'left', second: 'right' },
  string: fromString,
  sparseCopy: { length: sparseCopy.length, own0: Object.hasOwn(sparseCopy, '0'),
    own1: Object.hasOwn(sparseCopy, '1'), value1: sparseCopy[1] },
  accessorReads: trace,
  nullError,
}, null, 2) + '\n');
