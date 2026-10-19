'use strict';
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const originalPath = path.join(root, 'build/math-port/font-dynamic-extension-map-original.json');
const nativePath = process.argv[2];
if (!nativePath) throw new Error('usage: node tools/math-port/compare-font-dynamic-extension-map.cjs <native-json>');
const original = JSON.parse(fs.readFileSync(originalPath, 'utf8'));
const native = JSON.parse(fs.readFileSync(path.resolve(nativePath), 'utf8'));
let failures = 0;
function compare(pathName, actual, expected) {
  if (JSON.stringify(actual) !== JSON.stringify(expected)) {
    console.error(`${pathName}: native=${JSON.stringify(actual)} source=${JSON.stringify(expected)}`);
    failures++;
  }
}
const got = native.replacement;
const want = original.replacement;
if (!got) {
  console.error('replacement: native observation missing');
  failures++;
} else {
  for (const key of ['assignedSameObject', 'replacementDiffersFromPrevious', 'descriptor', 'oldAliasRetainsRecord',
    'oldAliasRetainsLaterMutation', 'oldAliasHasExtension', 'replacementKeys', 'addExtensionResult']) {
    compare(`replacement.${key}`, got[key], want[key]);
  }
  const record = got.insertedRecord;
  if (!record) {
    console.error('replacement.insertedRecord: native observation missing');
    failures++;
  } else {
    compare('replacement.insertedRecord.identity', record.identity, want.insertedRecord.identity);
    for (const key of ['keys', 'name', 'prefix', 'sizeN', 'stretchN', 'fileKeys'])
      compare(`replacement.insertedRecord.${key}`, record[key], want.insertedRecord[key]);
  }
}
if (failures) process.exitCode = 1;
else console.log('DynamicExtension Map replacement observations match the pinned Source oracle.');

