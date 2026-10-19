'use strict';
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {execFileSync} = require('node:child_process');
const root = path.resolve(__dirname, '../..');
const sourcePath = path.join(root, 'build/math-port/common-styles-original.json');
const nativePath = path.resolve(process.argv[2] || path.join(root, 'build/math-port/common-styles-native.json'));
const oraclePath = path.join(root, 'tools/math-port/common-styles-original-oracle.cjs');
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const fresh = JSON.parse(execFileSync(process.execPath, [oraclePath], {encoding: 'utf8'}));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));
for (const field of ['version', 'hashes', 'cjsHashes'])
  assert.deepStrictEqual(fresh[field], source[field], `pinned original ${field} changed`);
assert.deepStrictEqual(fresh.records, source.records, 'pinned original behavior differs from frozen gold');
assert.equal(native.version, source.version, 'native version differs');
assert.deepStrictEqual(native.records.map(row => row.name), source.records.map(row => row.name), 'native row set/order differs');
for (let i = 0; i < source.records.length; ++i) {
  try { assert.deepStrictEqual(native.records[i], source.records[i]); }
  catch (error) {
    error.message = `${source.records[i].name}: ${error.message}`;
    throw error;
  }
}
console.log(`Styles/StyleJson strict Source differential passed: ${source.records.length} records`);
