#!/usr/bin/env node
'use strict';

const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');

function firstDiff(expected, actual, at = '$') {
  if (Object.is(expected, actual)) return null;
  if (expected === null || actual === null ||
      typeof expected !== 'object' || typeof actual !== 'object')
    return {path:at, expected, actual};
  if (Array.isArray(expected) !== Array.isArray(actual))
    return {path:at, expected:Array.isArray(expected)?'array':typeof expected,
      actual:Array.isArray(actual)?'array':typeof actual};
  if (Array.isArray(expected)) {
    if (expected.length !== actual.length)
      return {path:`${at}.length`, expected:expected.length, actual:actual.length};
    for (let i = 0; i < expected.length; i++) {
      const difference = firstDiff(expected[i], actual[i], `${at}[${i}]`);
      if (difference) return difference;
    }
    return null;
  }
  const expectedKeys = Object.keys(expected).sort();
  const actualKeys = Object.keys(actual).sort();
  if (JSON.stringify(expectedKeys) !== JSON.stringify(actualKeys))
    return {path:`${at}.keys`, expected:expectedKeys, actual:actualKeys};
  for (const key of expectedKeys) {
    const difference = firstDiff(expected[key], actual[key], `${at}.${key}`);
    if (difference) return difference;
  }
  return null;
}

function compare(label, sourcePath, nativePath) {
  const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
  const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));
  if (source.source !== 'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e') {
    process.stderr.write(`${label}: unexpected Source provenance ${JSON.stringify(source.source)}\n`);
    return false;
  }
  const difference = firstDiff(source.results, native.results);
  if (difference) {
    process.stderr.write(`${label}: ${JSON.stringify(difference, null, 2)}\n`);
    return false;
  }
  process.stdout.write(`${label}: exact Source match.\n`);
  return true;
}

const methodsPath = path.resolve(process.argv[2] ||
  path.join(root, 'build/math-port/font-methods-native.json'));
const tuplePath = path.resolve(process.argv[3] ||
  path.join(root, 'build/math-port/font-tuple-native.json'));
const methodsOk = compare('Font methods',
  path.join(root, 'build/math-port/font-methods-original.json'), methodsPath);
const tupleOk = compare('Font tuple presence',
  path.join(root, 'build/math-port/font-tuple-presence-original.json'), tuplePath);
if (!methodsOk || !tupleOk) process.exitCode = 1;
