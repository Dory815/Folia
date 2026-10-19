#!/usr/bin/env node
'use strict';

const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const originalPath = process.argv[2] || path.join(root, 'build/math-port/font-instance-extension-original.json');
const nativePath = process.argv[3] || path.join(root, 'build/math-port/font-instance-extension-native.json');
const original = JSON.parse(fs.readFileSync(originalPath, 'utf8'));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));

function firstDifference(a, b, at = '$') {
  if (Object.is(a, b)) return null;
  if (a === null || b === null || typeof a !== 'object' || typeof b !== 'object')
    return {path: at, source: a, native: b};
  if (Array.isArray(a) !== Array.isArray(b)) return {path: at, source: a, native: b};
  if (Array.isArray(a)) {
    if (a.length !== b.length) return {path: at + '.length', source: a.length, native: b.length};
    for (let i = 0; i < a.length; ++i) {
      const difference = firstDifference(a[i], b[i], `${at}[${i}]`);
      if (difference) return difference;
    }
    return null;
  }
  const aKeys = Object.keys(a).sort(), bKeys = Object.keys(b).sort();
  if (aKeys.length !== bKeys.length || aKeys.some((key, i) => key !== bKeys[i]))
    return {path: at + '.$keys', source: aKeys, native: bKeys};
  for (const key of aKeys) {
    const difference = firstDifference(a[key], b[key], `${at}.${key}`);
    if (difference) return difference;
  }
  return null;
}

const expected = original.nativeEquivalent;
const actual = native.nativeEquivalent;
if (!Array.isArray(expected) || !Array.isArray(actual)) {
  process.stderr.write('font instance extension comparison requires both nativeEquivalent arrays\n');
  process.exit(2);
}
const rows = Math.min(expected.length, actual.length);
let matched = 0;
const initialDifference = firstDifference(original.initialClassKeys, native.initialClassKeys, '$.initialClassKeys');
if (initialDifference) process.stdout.write(`${JSON.stringify(initialDifference)}\n`);
for (let i = 0; i < rows; ++i) {
  const expectedRow = expected[i] ?? {};
  const actualRow = actual[i] ?? {};
  let rowMatched = true;
  const fields = [...new Set([...Object.keys(expectedRow), ...Object.keys(actualRow)])].sort();
  for (const field of fields) {
    const difference = firstDifference(expectedRow[field], actualRow[field], `$[${i}].${field}`);
    if (difference) {
      rowMatched = false;
      process.stdout.write(`${JSON.stringify({row: i, name: expectedRow.extensionName, ...difference})}\n`);
    }
  }
  if (rowMatched) ++matched;
}
if (expected.length !== actual.length)
  process.stdout.write(`${JSON.stringify({path: '$.length', source: expected.length, native: actual.length})}\n`);
process.stdout.write(`${matched}/${expected.length} instance-extension rows matched\n`);
if (initialDifference || matched !== expected.length || actual.length !== expected.length) process.exitCode = 1;
