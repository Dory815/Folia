// Compare exact source MML trees and parser state with the native candidate.
'use strict';
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {isDeepStrictEqual} = require('node:util');
const root = path.resolve(__dirname, '../..');
const sourcePath = process.argv[2] || path.join(root, 'build/math-port/base-methods-original.json');
const nativePath = process.argv[3] || path.join(root, 'build/math-port/base-methods-native.json');
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));
function same(label, expected, actual) {
  try { assert.deepStrictEqual(actual, expected); return true; }
  catch (error) { console.error(`${label}: ${error.message.split('\n').slice(0, 8).join('\n')}`); return false; }
}
let failures = 0;
let checks = 0;
if (!same('methodNames', source.methodNames, native.methodNames)) failures++;
checks++;
const expectedNames = source.cases.map(item => item.name);
const actualNames = Object.keys(native.cases);
if (!same('case names', expectedNames, actualNames)) failures++;
checks++;
for (const item of source.cases) {
  const actual = native.cases[item.name];
  if (!actual) continue;
  checks++;
  if (!same(`${item.name}/packages`, item.packages, actual.packages)) failures++;
  if (item.error) {
    checks++;
    if (!same(`${item.name}/error`, item.error, actual.error)) failures++;
    continue;
  }
  if (actual.error) { console.error(`${item.name}: native threw ${JSON.stringify(actual.error)}`); failures++; continue; }
  checks++;
  if (!same(`${item.name}/tree`, item.result.tree, actual.result?.tree)) failures++;
  checks++;
  if (!same(`${item.name}/parser`, item.result.parser, actual.result?.parser)) failures++;
}
const mismatched = expectedNames.filter(name => {
  const item = source.cases.find(entry => entry.name === name);
  const actual = native.cases[name];
  if (!actual) return true;
  if (!isDeepStrictEqual(item.packages, actual.packages)) return true;
  if (item.error) return !isDeepStrictEqual(item.error, actual.error);
  return !!actual.error || !isDeepStrictEqual(item.result.tree, actual.result?.tree) ||
    !isDeepStrictEqual(item.result.parser, actual.result?.parser);
});
console.log(`compared ${expectedNames.length} real TeX-to-MML cases; mismatches=${mismatched.length}; comparisons=${checks}`);
if (failures) process.exitCode = 1;
