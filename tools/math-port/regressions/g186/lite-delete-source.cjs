'use strict';

// Pinned MathJax 4.1.3 CJS observation for Lite own-field deletion and
// accessor replacement. The hash is the one recorded by
// tools/math-port/lite-adaptor-original-oracle.cjs.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const packageRoot = path.resolve(__dirname, '../../../mathjax-bundle/node_modules/@mathjax/src');
const version = JSON.parse(fs.readFileSync(path.join(packageRoot, 'package.json'))).version;
if (version !== '4.1.3') throw new Error(`Expected @mathjax/src 4.1.3, found ${version}`);
const elementPath = path.join(packageRoot, 'cjs/adaptors/lite/Element.js');
const elementHash = crypto.createHash('sha256').update(fs.readFileSync(elementPath)).digest('hex');
if (elementHash !== '81626bfcd19aaf7b5d8a0c461d5f9c9f3a3d3c150b283c7641f9c38e6fcca9c6')
  throw new Error(`Pinned LiteElement CJS changed: ${elementHash}`);
const {LiteElement} = require(elementPath);

const element = new LiteElement('span');
const initialKeys = Object.keys(element);
const deleteResult = delete element.kind;
const afterDelete = {
  deleteResult,
  hasOwn: Object.hasOwn(element, 'kind'),
  value: element.kind,
  keys: Object.keys(element)
};
Object.defineProperty(element, 'kind', {
  configurable: true,
  enumerable: true,
  get() { return 'custom-kind'; }
});
const afterDefineAccessor = {
  hasOwn: Object.hasOwn(element, 'kind'),
  value: element.kind,
  keys: Object.keys(element),
  descriptor: Object.getOwnPropertyDescriptor(element, 'kind')
};
delete element.kind;
const afterAccessorDelete = {
  hasOwn: Object.hasOwn(element, 'kind'),
  value: element.kind,
  keys: Object.keys(element)
};

const result = {version, elementHash, initialKeys, afterDelete, afterDefineAccessor, afterAccessorDelete};
if (JSON.stringify(initialKeys) !== JSON.stringify(['kind', 'attributes', 'children', 'styles']) ||
    !afterDelete.deleteResult || afterDelete.hasOwn || afterDelete.value !== undefined ||
    JSON.stringify(afterDelete.keys) !== JSON.stringify(['attributes', 'children', 'styles']) ||
    !afterDefineAccessor.hasOwn || afterDefineAccessor.value !== 'custom-kind' ||
    afterDefineAccessor.descriptor.configurable !== true || afterDefineAccessor.descriptor.enumerable !== true ||
    afterAccessorDelete.hasOwn || afterAccessorDelete.value !== undefined)
  throw new Error(`Unexpected pinned Source behavior: ${JSON.stringify(result)}`);
process.stdout.write(JSON.stringify(result, null, 2) + '\n');
