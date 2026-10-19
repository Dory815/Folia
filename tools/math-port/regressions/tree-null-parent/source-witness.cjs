'use strict';
// Ignored draft witness. Pinned to MathJax 4.1.3 CJS Node.js SHA-256:
// 94174004B89F19BF3C98E8EEC5FABDD0BCBF1E13E0B740963FA69554AD8A12A7
const assert = require('node:assert/strict');
const fs = require('node:fs');
const crypto = require('node:crypto');
const path = require('node:path');
const source = path.resolve(__dirname, '..', '..', '..', 'mathjax-bundle', 'node_modules', '@mathjax', 'src', 'cjs', 'core', 'Tree', 'Node.js');
const expectedHash = '94174004b89f19bf3c98e8eec5fabdd0bcbf1e13e0b740963fa69554ad8a12a7';
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(source)).digest('hex'), expectedHash);
const { AbstractNode } = require(source);
const p = new AbstractNode(null);
const a = new AbstractNode(null);
const trace = [];
function observe(label, operation) {
  try { operation(); trace.push(`${label}:return`); }
  catch (error) { trace.push(`${label}:${error.name}:${error.message}`); }
}
observe('append-null', () => p.appendChild(null));
assert.equal(p.childNodes.length, 1);
assert.equal(p.childIndex(null), 0);
observe('replace-a-for-null', () => p.replaceChild(a, null));
assert.equal(p.childNodes.length, 1);
assert.equal(p.childNodes[0], a); // write and newChild.parent happen before oldChild.parent throws
assert.equal(a.parent, p);
observe('replace-null-for-a', () => p.replaceChild(null, a));
assert.equal(p.childNodes[0], null); // newChild.parent throws after replacing the array slot
assert.equal(a.parent, p); // oldChild.parent has not been read yet
observe('remove-null', () => p.removeChild(null));
assert.equal(p.childNodes.length, 0);
console.log(trace.join('\n'));
