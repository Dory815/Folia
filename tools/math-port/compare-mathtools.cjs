'use strict';

// Strict complete comparison for the pinned Mathtools Source observation.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const originalPath = path.join(root, 'build/math-port/mathtools-original.json');
const nativePath = path.resolve(process.argv[2] || path.join(root, 'build/math-port/mathtools-native.json'));
const lock = JSON.parse(fs.readFileSync(path.join(root, 'docs/math-port/evidence/mathtools-source-lock.json'), 'utf8'));
const hash = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const originalBytes = fs.readFileSync(originalPath);
const original = JSON.parse(originalBytes);
const nativeBytes = fs.readFileSync(nativePath);
const native = JSON.parse(nativeBytes);
const originalSha256 = hash(originalBytes);
if (originalSha256 !== lock.oracleOutputSha256)
  throw new Error(`Mathtools Source output hash differs from lock: ${originalSha256}`);
const differences = [];
function equal(a, b, at) {
  if (Object.is(a, b)) return;
  if (a === null || b === null || typeof a !== 'object' || typeof b !== 'object' || Array.isArray(a) !== Array.isArray(b)) {
    differences.push({path: at, source: a, native: b}); return;
  }
  if (Array.isArray(a)) {
    if (a.length !== b.length) differences.push({path: `${at}.length`, source: a.length, native: b.length});
    for (let i = 0; i < Math.min(a.length, b.length); i++) equal(a[i], b[i], `${at}[${i}]`);
    return;
  }
  const ak = Object.keys(a), bk = Object.keys(b);
  if (ak.length !== bk.length || ak.some((key, i) => bk[i] !== key))
    differences.push({path: `${at}.[keys]`, source: ak, native: bk});
  for (const key of ak) if (Object.hasOwn(b, key)) equal(a[key], b[key], `${at}.${key}`);
}
equal(original, native, '$');
const report = {accepted: differences.length === 0, cases: original.cases.length,
  matchedCases: original.cases.filter((item, i) => JSON.stringify(item) === JSON.stringify(native.cases?.[i])).length,
  maps: Object.keys(original.maps).length,
  matchedMaps: Object.keys(original.maps).filter(name => JSON.stringify(original.maps[name]) === JSON.stringify(native.maps?.[name])).length,
  originalSha256, nativeSha256: hash(nativeBytes), differences};
const output = path.join(root, 'build/math-port/mathtools-comparison.json');
fs.writeFileSync(output, `${JSON.stringify(report, null, 2)}\n`);
console.log(JSON.stringify({...report, firstDifference: differences[0] ?? null,
  output: path.relative(root, output)}));
process.exitCode = report.accepted ? 0 : 1;
