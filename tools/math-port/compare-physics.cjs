'use strict';

// Strict structural comparison of the pinned Physics package corpus.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const sourcePath = path.join(root, 'build/math-port/physics-original.json');
const nativePath = path.resolve(process.argv[2] || path.join(root, 'build/math-port/physics-native.json'));
const lock = JSON.parse(fs.readFileSync(path.join(root,
  'docs/math-port/evidence/physics-source-lock.json'), 'utf8'));
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));
const sourceSha256 = crypto.createHash('sha256').update(fs.readFileSync(sourcePath)).digest('hex');
if (sourceSha256 !== lock.oracleOutputSha256) {
  throw new Error(`Physics Source oracle output hash differs from lock: ${sourceSha256}`);
}
const failures = [];
function compare(a, b, at) {
  if (Object.is(a, b)) return;
  if (a === null || b === null || typeof a !== 'object' || typeof b !== 'object' || Array.isArray(a) !== Array.isArray(b)) {
    failures.push({ path: at, source: a, native: b }); return;
  }
  if (Array.isArray(a)) {
    if (a.length !== b.length) failures.push({ path: `${at}.length`, source: a.length, native: b.length });
    for (let i = 0; i < Math.min(a.length, b.length); i++) compare(a[i], b[i], `${at}[${i}]`);
    return;
  }
  const ak = Object.keys(a), bk = Object.keys(b);
  if (ak.length !== bk.length) failures.push({ path: `${at}.[keyCount]`, source: ak, native: bk });
  if (ak.some((key, i) => bk[i] !== key)) failures.push({ path: `${at}.[keyOrder]`, source: ak, native: bk });
  for (const key of ak) if (Object.hasOwn(b, key)) compare(a[key], b[key], `${at}.${key}`);
}
compare(source, native, '$');
const report = {
  accepted: failures.length === 0,
  cases: source.cases?.length ?? 0,
  matchedCases: source.cases?.filter((item, i) => native.cases?.[i] && JSON.stringify(item) === JSON.stringify(native.cases[i])).length ?? 0,
  maps: Object.keys(source.maps ?? {}).length,
  matchedMaps: Object.keys(source.maps ?? {}).filter(name => JSON.stringify(source.maps[name]) === JSON.stringify(native.maps?.[name])).length,
  sourceSha256,
  source: path.relative(root, sourcePath),
  native: path.relative(root, nativePath),
  nativeSha256: crypto.createHash('sha256').update(fs.readFileSync(nativePath)).digest('hex'),
  failures,
};
const destination = path.join(root, 'build/math-port/physics-comparison.json');
fs.mkdirSync(path.dirname(destination), { recursive: true });
fs.writeFileSync(destination, `${JSON.stringify(report, null, 2)}\n`);
console.log(JSON.stringify({ accepted: report.accepted, cases: report.cases, matchedCases: report.matchedCases,
  maps: report.maps, matchedMaps: report.matchedMaps, firstDiff: failures[0] ?? null,
  failureCount: failures.length, output: path.relative(root, destination) }));
process.exitCode = report.accepted ? 0 : 1;
