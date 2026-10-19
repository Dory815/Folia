'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../../../..');
const pkg = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
assert.equal(require(path.join(pkg, 'package.json')).version, '4.1.3');
const file = path.join(pkg, 'cjs/input/tex/Tags.js');
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex'),
  'e027c7edb6f7207f3b3ce7e7203dcfc763219b1fa37a3b39925ae83c1beef896');
const { AbstractTags, TagInfo, Label } = require(file);
const tags = new AbstractTags();
assert(tags.currentTag instanceof TagInfo);
assert.deepEqual(tags.history, []);
assert.deepEqual(tags.stack, []);
const first = tags.currentTag;
tags.start('outer', true, true);
const second = tags.currentTag;
assert(tags.stack.includes(first));
tags.end();
assert(tags.history.includes(second));
assert.equal(tags.currentTag, first);
tags.resetTag();
assert.deepEqual(tags.history, []);
assert(tags.currentTag instanceof TagInfo);
assert(tags.labels instanceof Object);
assert(Object.values(tags.labels).every(value => value instanceof Label));

// Array.prototype.find reads inherited indexed values in a hole. Such a value
// is reachable to Source even though an own-index-only scan does not see it.
const inheritedTag = new TagInfo('inherited', true, true);
const sparseHistory = new Array(1);
const customArrayPrototype = Object.create(Array.prototype);
Object.defineProperty(customArrayPrototype, '0', {
  value: inheritedTag, writable: true, enumerable: true, configurable: true
});
Object.setPrototypeOf(sparseHistory, customArrayPrototype);
assert.equal(sparseHistory.find(value => value === inheritedTag), inheritedTag);
assert.equal(Object.hasOwn(sparseHistory, '0'), false);
process.stdout.write('Pinned MathJax 4.1.3 TagInfo root witness passed; inherited-hole reachability reproduced.\n');
