#!/usr/bin/env node
'use strict';

// Strict comparator for the independent DynamicFile raw-field witness. It
// deliberately performs no normalization of errors, property order, or values.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const originalPath = process.argv[2] || path.join(root,
  'build/math-port/font-dynamic-file-raw-source.json');
const nativePath = process.argv[3] || path.join(root,
  'build/math-port/font-dynamic-file-raw-native.json');
const original = JSON.parse(fs.readFileSync(originalPath, 'utf8'));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));
assert.deepStrictEqual(native.mapReplacement, original.mapReplacement);
assert.deepStrictEqual(native.changingExtension, original.changingExtension);
assert.deepStrictEqual(native.first, original.first);
assert.deepStrictEqual(native.rawReturnIdentity, original.rawReturnIdentity);
assert.deepStrictEqual(native.descriptorReplacement, original.descriptorReplacement);
assert.deepStrictEqual(native.throwingGetter, original.throwingGetter);
process.stdout.write('DynamicFile Map replacement, raw extension reads and descriptor transitions: exact match\n');
