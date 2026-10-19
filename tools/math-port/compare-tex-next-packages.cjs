'use strict';
// Exact field comparison for the Colortbl, TagFormat, and SetOptions profile.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');

const root = path.resolve(__dirname, '../..');
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const nativePath = process.argv[2];
if (!nativePath) throw new Error('Usage: compare-tex-next-packages.cjs <native-json> [summary-json] [source-json]');
const sourcePath = path.resolve(root, process.argv[4] || 'build/math-port/tex-next-packages-original.json');
const summaryPath = path.resolve(root, process.argv[3] || 'build/math-port/tex-next-packages-comparison.json');
const scopePath = path.join(root, 'docs/math-port/scope.json');
const scope = JSON.parse(fs.readFileSync(scopePath, 'utf8'));
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const native = JSON.parse(fs.readFileSync(path.resolve(root, nativePath), 'utf8'));
assert.equal(source.scopeSha256, digest(scopePath), 'Source oracle scope hash differs');

function firstDifference(expected, actual, at = '$') {
  if (Object.is(expected, actual)) return null;
  if (typeof expected !== typeof actual || expected === null || actual === null)
    return { path: at, source: expected, native: actual };
  if (Array.isArray(expected) || Array.isArray(actual)) {
    if (!Array.isArray(expected) || !Array.isArray(actual) || expected.length !== actual.length)
      return { path: `${at}.length`, source: Array.isArray(expected) ? expected.length : typeof expected,
        native: Array.isArray(actual) ? actual.length : typeof actual };
    for (let i = 0; i < expected.length; i++) {
      const difference = firstDifference(expected[i], actual[i], `${at}[${i}]`);
      if (difference) return difference;
    }
    return null;
  }
  if (typeof expected === 'object') {
    const expectedKeys = Object.keys(expected).sort();
    const actualKeys = Object.keys(actual).sort();
    if (JSON.stringify(expectedKeys) !== JSON.stringify(actualKeys))
      return { path: `${at}.keys`, source: expectedKeys, native: actualKeys };
    for (const key of expectedKeys) {
      const difference = firstDifference(expected[key], actual[key], `${at}.${key}`);
      if (difference) return difference;
    }
    return null;
  }
  return { path: at, source: expected, native: actual };
}

const expectedCases = source.cases;
const actualCases = native.cases || [];
const failures = [];
if (actualCases.length !== expectedCases.length)
  failures.push({ path: 'cases.length', source: expectedCases.length, native: actualCases.length });
let matched = 0;
for (let i = 0; i < expectedCases.length; i++) {
  const difference = firstDifference(expectedCases[i], actualCases[i], `cases[${i}](${expectedCases[i].name})`);
  if (difference) failures.push(difference);
  else matched++;
}
const summary = {
  source: path.relative(root, sourcePath), sourceSha256: digest(sourcePath),
  sourceLocks: scope.files.length, native: path.relative(root, path.resolve(root, nativePath)),
  nativeSha256: digest(path.resolve(root, nativePath)), total: expectedCases.length, matched,
  accepted: matched === expectedCases.length && failures.length === 0, failures
};
fs.mkdirSync(path.dirname(summaryPath), { recursive: true });
fs.writeFileSync(summaryPath, JSON.stringify(summary, null, 2) + '\n');
console.log(JSON.stringify({ total: summary.total, matched, accepted: summary.accepted,
  firstFailure: failures[0] || null, summary: path.relative(root, summaryPath) }));
process.exitCode = summary.accepted ? 0 : 1;
