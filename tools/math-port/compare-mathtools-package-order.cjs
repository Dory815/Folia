'use strict';

// Strict complete comparison for the package-order Mathtools observation.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const originalPath =
    path.join(root, 'build/math-port/mathtools-package-order-original.json');
const nativePath = path.resolve(
    process.argv[2] ||
    path.join(root, 'build/math-port/mathtools-package-order-native.json'));
const lock = JSON.parse(fs.readFileSync(
    path.join(
        root,
        'docs/math-port/evidence/mathtools-package-order-source-lock.json'),
    'utf8'));
const hash = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const originalBytes = fs.readFileSync(originalPath);
const nativeBytes = fs.readFileSync(nativePath);
const original = JSON.parse(originalBytes);
const native = JSON.parse(nativeBytes);
const originalSha256 = hash(originalBytes);
if (originalSha256 !== lock.oracleOutputSha256)
  throw new Error(`Mathtools package-order Source hash differs from lock: ${
      originalSha256}`);
const differences = [];
function equal(a, b, at) {
  if (Object.is(a, b))
    return;
  if (a === null || b === null || typeof a !== 'object' ||
      typeof b !== 'object' || Array.isArray(a) !== Array.isArray(b)) {
    differences.push({path : at, source : a, native : b});
    return;
  }
  if (Array.isArray(a)) {
    if (a.length !== b.length)
      differences.push(
          {path : `${at}.length`, source : a.length, native : b.length});
    for (let i = 0; i < Math.min(a.length, b.length); i++)
      equal(a[i], b[i], `${at}[${i}]`);
    return;
  }
  const ak = Object.keys(a), bk = Object.keys(b);
  if (ak.length !== bk.length || ak.some((key, i) => bk[i] !== key))
    differences.push({path : `${at}.[keys]`, source : ak, native : bk});
  for (const key of ak)
    if (Object.hasOwn(b, key))
      equal(a[key], b[key], `${at}.${key}`);
}
equal(original, native, '$');
const report = {
  accepted : differences.length === 0,
  cases : original.cases.length,
  matchedCases : original.cases
                     .filter((item, i) => JSON.stringify(item) ===
                                          JSON.stringify(native.cases?.[i]))
                     .length,
  originalSha256,
  nativeSha256 : hash(nativeBytes),
  differences
};
const output =
    path.join(root, 'build/math-port/mathtools-package-order-comparison.json');
fs.writeFileSync(output, `${JSON.stringify(report, null, 2)}\n`);
console.log(JSON.stringify({
  ...report,
  firstDifference : differences[0] ?? null,
  output : path.relative(root, output)
}));
process.exitCode = report.accepted ? 0 : 1;
