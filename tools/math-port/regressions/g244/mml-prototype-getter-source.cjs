'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../../../..');
const relative = 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlFactory.js';
const file = path.join(root, relative);
const sha256 = crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
assert.equal(sha256, 'd0c4e25237d04a9d21d12ee44776e46d30777e7f284db6f4f3ed1cc625b752d3');
const {MmlFactory} = require(file);
const prototype = MmlFactory.prototype;
const original = Object.getOwnPropertyDescriptor(prototype, 'MML');
assert.equal(typeof original.get, 'function');
assert.equal(original.enumerable, false);
assert.equal(original.configurable, true);
const node = {};
assert.equal(original.get.call({node}), node);
for (let index = 0; index < 4; ++index) {
  new MmlFactory();
  assert.equal(Object.getOwnPropertyDescriptor(prototype, 'MML').get, original.get);
}
Object.defineProperty(prototype, 'MML', {value: 'custom', writable: false, configurable: true});
assert.equal(new MmlFactory().MML, 'custom');
delete prototype.MML;
new MmlFactory();
assert.equal(Object.hasOwn(prototype, 'MML'), false);
console.log(JSON.stringify({source: relative, sha256, stableGetter: true, replacementPreserved: true, deletionPreserved: true}));
