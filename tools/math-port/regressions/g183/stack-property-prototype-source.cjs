'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const root = process.cwd();
const pkg = require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/package.json'));
assert.equal(pkg.version, '4.1.3');
const source = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/StackItem.js');
const hash = crypto.createHash('sha256').update(fs.readFileSync(source)).digest('hex');
assert.equal(hash, '53baa91d7c27475a91a9b6993235071ab32b04a29736c5920bdda2836ab5a91b');

const { BaseItem } = require(source);
const item = new BaseItem({});
assert.equal(Object.getPrototypeOf(item._properties), Object.prototype);
assert.equal(Object.hasOwn(item._properties, 'toString'), false);
assert.equal(item.getProperty('missing'), undefined);
assert.equal(item.getProperty('toString'), Object.prototype.toString);
assert.equal(item.getProperty('toString').call(item._properties), '[object Object]');
console.log(JSON.stringify({
  version: pkg.version,
  stackItemSha256: hash,
  propertiesPrototypeIsObjectPrototype: Object.getPrototypeOf(item._properties) === Object.prototype,
  missingProperty: item.getProperty('missing'),
  inheritedToStringIdentity: item.getProperty('toString') === Object.prototype.toString,
  inheritedToStringResult: item.getProperty('toString').call(item._properties)
}, null, 2));


