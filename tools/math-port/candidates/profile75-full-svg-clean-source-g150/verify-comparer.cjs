'use strict';
// Synthetic comparer checks only; these records are not renderer evidence.
const fs = require('node:fs');
const path = require('node:path');
const cp = require('node:child_process');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../../../..');
const sourcePath = path.join(root, 'build/math-port/profile75-full-svg-source-clean-g150.json');
const scratch = path.join(root, 'build/math-port/profile75-full-svg-clean-comparer-g150');
const nativePath = path.join(scratch, 'native.json');
const comparisonPath = path.join(scratch, 'comparison.json');
const comparer = path.join(__dirname, 'compare.cjs');
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
assert.equal(source.records.length, 150, 'run capture.cjs first');
const annotated = JSON.parse(fs.readFileSync(path.join(root,
  'build/math-port/profile75-full-svg-native-boundary-g150.json'), 'utf8'));
const native = {
  schema: 1,
  renderer: 'folia.math::Renderer',
  groups: source.groups,
  corpusSha256: source.source.corpusSha256,
  caseInventorySha256: source.source.caseInventorySha256,
  boundaryEvidence: annotated.boundaryEvidence,
  runtimeBoundaries: {
    continuous: { isolation: 'shared-owner-across-cases', owner: 'one folia.math::Renderer with one NativeRenderSession/Context' },
    'fresh-session-per-case': { isolation: 'isolated-owner-per-case', owner: 'fresh folia.math::Renderer/NativeRenderSession/Context per record' }
  },
  records: source.records.map(row => ({ ...row, sceneNodes: row.svg === null ? 0 : 1 }))
};
const invoke = () => cp.spawnSync(process.execPath, [comparer, sourcePath, nativePath, comparisonPath],
  { cwd: root, encoding: 'utf8', maxBuffer: 512 * 1024 * 1024 });
fs.mkdirSync(scratch, { recursive: true });
fs.writeFileSync(nativePath, JSON.stringify(native));
let result = invoke();
assert.equal(result.status, 0, result.stderr || result.stdout);
assert.equal(JSON.parse(fs.readFileSync(comparisonPath, 'utf8')).matched, 150);

native.runtimeBoundaries['fresh-session-per-case'].owner = 'one reused renderer';
fs.writeFileSync(nativePath, JSON.stringify(native));
result = invoke();
assert.notEqual(result.status, 0, 'boundary mismatch must be rejected');

native.runtimeBoundaries['fresh-session-per-case'].owner = 'fresh folia.math::Renderer/NativeRenderSession/Context per record';
native.records[0].svg += '<!-- intentional synthetic mismatch -->';
fs.writeFileSync(nativePath, JSON.stringify(native));
result = invoke();
assert.equal(result.status, 1, result.stderr || result.stdout);
const comparison = JSON.parse(fs.readFileSync(comparisonPath, 'utf8'));
assert.equal(comparison.matched, 149);
assert.equal(comparison.failures.length, 1);
console.log(JSON.stringify({ positiveMatched: 150, boundaryMismatchRejected: true,
  svgMismatchMatched: 149, parityClaim: false }));
