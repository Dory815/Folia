const assert = require('node:assert/strict');
const fs = require('node:fs');
const crypto = require('node:crypto');
const path = require.resolve(`${process.cwd()}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/Tree/Node.js`);
const expectedHash = '94174004b89f19bf3c98e8eec5fabdd0bcbf1e13e0b740963fa69554ad8a12a7';
const hash = crypto.createHash('sha256').update(fs.readFileSync(path)).digest('hex');
assert.equal(hash, expectedHash, 'pinned MathJax 4.1.3 Tree/Node.js changed');

const { AbstractNode } = require(path);
const prototype = AbstractNode.prototype;
const method = prototype.toString;
const descriptor = Object.getOwnPropertyDescriptor(prototype, 'toString');
assert.equal(method.name, '');
assert.equal(method.length, 0);
assert.deepEqual(
  { enumerable: descriptor.enumerable, writable: descriptor.writable, configurable: descriptor.configurable },
  { enumerable: true, writable: true, configurable: true },
);

let calls = 0;
const receiver = {
  kind: 'mrow',
  childNodes: { join(separator) { calls++; assert.equal(arguments.length, 1); return `joined(${separator})`; } },
};
assert.equal(method.call(receiver), 'mrow(joined(,))');
assert.equal(calls, 1);
assert.equal(method.call({ kind: 'mi', childNodes: [] }), 'mi()');
console.log(`Tree/Node.js ${hash}: toString metadata, receiver, join argument, and output match`);
