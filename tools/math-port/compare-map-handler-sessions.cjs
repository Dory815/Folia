'use strict';

// Strictly compares the native two-runtime observation with its complete,
// hash-pinned Source witness and its separately scoped native checks.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');

const root = path.resolve(__dirname, '../..');
const originalPath = path.resolve(
    process.argv[2] || path.join(root, 'build/math-port/map-handler-session-original.json'));
const nativePath = path.resolve(
    process.argv[3] || path.join(root, 'build/math-port/map-handler-session-native.json'));
const lockPath = path.join(
    root, 'tools/math-port/map-handler-session-reference/source-lock.json');
const sha256 = bytes => crypto.createHash('sha256').update(bytes).digest('hex');

const expectedLockSha256 =
    '53ccdf3ee3b017c46be3a697efeb789a355ed64d4e865e88471d9f3c57f14afe';
const expectedOriginalSha256 =
    'f7a03711e682f19c70f071c3412a127341c377929b9cefd9ad563e8061053e9e';
const expectedCase = {
  name: 'map-handler-two-module-instances-interleaved',
  values: {
    sameModuleOwnersShareMap: true,
    moduleAStillOwnsOriginalMapAfterModuleBRegistersSameName: true,
    moduleBOwnsItsMap: true,
    distinctModuleMaps: true,
    interleavedCallbacks: ['A', 'B', 'A', 'B'],
  },
};
const expectedNativeCheckKeys = [
  'callbackIdentitiesPreserved',
  'subHandlersShareMap',
  'replacementVisibleToFutureAdds',
  'replacementPreservesExistingHandler',
  'tokenMapHostUsesSelectedOwner',
  'deadOwnerRejected',
  'registryRelease',
];

function assertKeyOrder(value, expected, label) {
  assert(value && typeof value === 'object' && !Array.isArray(value),
      `${label} must be an object`);
  assert.deepEqual(Object.keys(value), expected, `${label} keys/order differ`);
}

const lockBytes = fs.readFileSync(lockPath);
const lock = JSON.parse(lockBytes);
assert.equal(sha256(lockBytes), expectedLockSha256,
    'MapHandler Source lock changed');
assert.equal(lock.schema, 1, 'unexpected MapHandler Source lock schema');
assert.equal(lock.mathJaxVersion, '4.1.3', 'unexpected MathJax Source version');
assert.deepEqual(lock.sources.map(source => source.file),
    ['ts/input/tex/MapHandler.ts'], 'MapHandler Source lock file set changed');

const originalBytes = fs.readFileSync(originalPath);
const nativeBytes = fs.readFileSync(nativePath);
const original = JSON.parse(originalBytes);
const native = JSON.parse(nativeBytes);
const originalSha256 = sha256(originalBytes);
assert.equal(originalSha256, expectedOriginalSha256,
    'MapHandler Source oracle output changed');
assert.equal(original.sourceLockSha256, sha256(lockBytes),
    'MapHandler Source output does not identify the pinned Source lock');
assertKeyOrder(original, ['sourceLockSha256', 'cases'], 'Source output');
assert.deepEqual(original.cases, [expectedCase],
    'Source witness case or complete value shape changed');

assertKeyOrder(native, ['cases', 'nativeChecks'], 'native output');
assertKeyOrder(native.cases?.[0], ['name', 'values'], 'native case');
assertKeyOrder(native.cases[0].values,
    Object.keys(expectedCase.values), 'native Source values');
assert.deepEqual(native.cases, original.cases,
    'native Source case differs from the complete original observation');
assert.equal(native.cases.length, 1, 'native case count differs');
assertKeyOrder(native.nativeChecks, expectedNativeCheckKeys, 'nativeChecks');
for (const key of expectedNativeCheckKeys)
  assert.equal(native.nativeChecks[key], true,
      `native-only check failed: ${key}`);

console.log(JSON.stringify({
  accepted: true,
  cases: native.cases.length,
  nativeChecks: expectedNativeCheckKeys.length,
  sourceLockSha256: sha256(lockBytes),
  originalSha256,
  nativeSha256: sha256(nativeBytes),
}));
