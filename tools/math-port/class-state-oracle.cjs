'use strict';
// Source-only finite TeX-class spacing oracle. Kept at tools/math-port when promoted.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const { root } = require('./mo-pattern-reference.cjs');

assert.equal(process.version, 'v24.13.0');
const npm = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourcePath = 'ts/core/MmlTree/MmlNode.ts';
const runtimePath = 'cjs/core/MmlTree/MmlNode.js';
const factorySourcePath = 'ts/core/Tree/NodeFactory.ts';
const factoryRuntimePath = 'cjs/core/Tree/NodeFactory.js';
const sha256 = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const sourceBytes = fs.readFileSync(path.join(root, '.reference/MathJax-src', sourcePath));
assert.equal(sha256(sourceBytes), 'cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656');
const sourceText = sourceBytes.toString('utf8').replaceAll('\r\n', '\n');
assert.equal(fs.readFileSync(path.join(npm, sourcePath), 'utf8').replaceAll('\r\n', '\n'), sourceText);
assert.equal(sha256(fs.readFileSync(path.join(npm, runtimePath))), '86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45');
const factorySource = fs.readFileSync(path.join(root, '.reference/MathJax-src', factorySourcePath));
assert.equal(sha256(factorySource), 'b02c5ac2cabb61755785d35def6b57f8246322c626f548df600ca81af27b4599');
assert.equal(fs.readFileSync(path.join(npm, factorySourcePath), 'utf8').replaceAll('\r\n', '\n'), factorySource.toString('utf8').replaceAll('\r\n', '\n'));
assert.equal(sha256(fs.readFileSync(path.join(npm, factoryRuntimePath))), '148c9a3a2e469fc464036b88deb7067cd779989a43356812d31365a84ec75c54');

// Independent Source literal. The lock below rejects drift in either pinned TS copy.
const TEXSPACE = [
  [ 0, -1,  2,  3,  0,  0,  0,  1],
  [-1, -1,  0,  3,  0,  0,  0,  1],
  [ 2,  2,  0,  0,  2,  0,  0,  2],
  [ 3,  3,  0,  0,  3,  0,  0,  3],
  [ 0,  0,  0,  0,  0,  0,  0,  0],
  [ 0, -1,  2,  3,  0,  0,  0,  1],
  [ 1,  1,  0,  1,  1,  1,  1,  1],
  [ 1, -1,  2,  3,  1,  0,  1,  1],
];
const TEXSPACELENGTH = ['', 'thinmathspace', 'mediummathspace', 'thickmathspace'];
const CLASSES = ['ORD', 'OP', 'BIN', 'REL', 'OPEN', 'CLOSE', 'PUNCT', 'INNER'];
const NONE = -1;

function sourceArray(name) {
  const start = sourceText.indexOf(`const ${name} = [`);
  assert.notEqual(start, -1, `pinned Source ${name} declaration`);
  const end = sourceText.indexOf('\n];', start);
  assert.notEqual(end, -1, `pinned Source ${name} terminator`);
  return sourceText.slice(start, end + 3);
}
const sourceRows = [...sourceArray('TEXSPACE').matchAll(/^\s*\[([^\]]*)\]/gm)]
  .map(([, row]) => row.split(',').map(value => Number(value.trim())));
assert.equal(sourceRows.length, 8, 'Source TEXSPACE has eight rows');
assert.ok(sourceRows.every(row => row.length === 8 && row.every(Number.isInteger)), 'Source TEXSPACE is 8 by 8 integers');
assert.deepEqual(sourceRows, TEXSPACE, 'independent literal agrees with pinned Source TEXSPACE');
const sourceLengths = [...sourceArray('TEXSPACELENGTH').matchAll(/'([^']*)'/g)].map(([, value]) => value);
assert.deepEqual(sourceLengths, TEXSPACELENGTH, 'spacing names agree with pinned Source TEXSPACELENGTH');

const { MmlFactory } = require(path.join(npm, 'cjs/core/MmlTree/MmlFactory.js'));
const factory = new MmlFactory();
function expected(previous, current, previousLevel, currentLevel) {
  if (previous === NONE || current === NONE) return '';
  const cell = TEXSPACE[previous][current];
  if ((previousLevel > 0 || currentLevel > 0) && cell >= 0) return '';
  return TEXSPACELENGTH[Math.abs(cell)];
}
const observations = [];
function observe(previous, current, previousLevel, currentLevel, mode) {
  const node = factory.create('mo');
  node.prevClass = previous;
  node.texClass = current;
  node.prevLevel = previousLevel;
  node.attributes.set('scriptlevel', currentLevel);
  const actual = node.texSpacing();
  const want = expected(previous, current, previousLevel, currentLevel);
  const previousName = previous === NONE ? 'NONE' : CLASSES[previous];
  const currentName = current === NONE ? 'NONE' : CLASSES[current];
  const key = `${previousName}>${currentName}:${mode}:pl${previousLevel}:sl${currentLevel}`;
  assert.equal(actual, want, key);
  observations.push({ key, previous, current, previousLevel, currentLevel, spacing: actual });
}
for (let previous = 0; previous < 8; previous++) {
  for (let current = 0; current < 8; current++) {
    observe(previous, current, 0, 0, 'normal');
    observe(previous, current, 1, 0, 'previous-level-1');
    observe(previous, current, 0, 1, 'current-scriptlevel-1');
  }
}
observe(NONE, 3, 0, 0, 'previous-none');
observe(0, NONE, 0, 0, 'current-none');
assert.equal(observations.length, 194);
console.log(JSON.stringify({ source: sourcePath, source_sha256: sha256(sourceBytes), matrix: '8x8', observations: observations.length, mode: process.argv.includes('--check') ? 'check' : 'verify', cases: observations }));
