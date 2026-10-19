'use strict';

// Enumerate all Source/native differences without changing either record.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const root = path.resolve(__dirname, '../..');
const [sourceArg, nativeArg, summaryArg] = process.argv.slice(2);
if (!sourceArg || !nativeArg) {
  throw new Error('Usage: diagnose-tex-package-differences.cjs <source.json> <native.json> [summary.json]');
}
const sourcePath = path.resolve(root, sourceArg);
const nativePath = path.resolve(root, nativeArg);
const summaryPath = path.resolve(root, summaryArg || `${nativeArg}.all-differences.json`);
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));

function digest(file) {
  return crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
}

function enumerate(expected, actual, at, output) {
  if (Object.is(expected, actual)) return;
  if (expected === null || actual === null || typeof expected !== 'object' || typeof actual !== 'object') {
    output.push({ path: at, source: expected, native: actual });
    return;
  }
  if (Array.isArray(expected) !== Array.isArray(actual)) {
    output.push({ path: at, sourceType: Array.isArray(expected) ? 'array' : 'object',
      nativeType: Array.isArray(actual) ? 'array' : 'object' });
    return;
  }
  if (Array.isArray(expected)) {
    if (expected.length !== actual.length) output.push({ path: `${at}.length`, source: expected.length, native: actual.length });
    const length = Math.min(expected.length, actual.length);
    for (let i = 0; i < length; i++) enumerate(expected[i], actual[i], `${at}[${i}]`, output);
    return;
  }
  const sourceKeys = Object.keys(expected).sort();
  const nativeKeys = Object.keys(actual).sort();
  if (sourceKeys.length !== nativeKeys.length || sourceKeys.some((key, i) => key !== nativeKeys[i])) {
    output.push({ path: `${at}.[keys]`, source: sourceKeys, native: nativeKeys });
  }
  for (const key of sourceKeys) {
    if (Object.hasOwn(actual, key)) enumerate(expected[key], actual[key], `${at}.${key}`, output);
    else output.push({ path: `${at}.${key}`, source: expected[key], native: { $missing: true } });
  }
  for (const key of nativeKeys) {
    if (!Object.hasOwn(expected, key)) output.push({ path: `${at}.${key}`, source: { $missing: true }, native: actual[key] });
  }
}

const rows = [];
if (Array.isArray(source.cases) && Array.isArray(native.cases)) {
  if (source.cases.length !== native.cases.length) rows.push({ name: '<cases>', differences: [{ path: 'cases.length', source: source.cases.length, native: native.cases.length }] });
  for (let i = 0; i < source.cases.length; i++) {
    const expected = source.cases[i];
    const actual = native.cases[i];
    const differences = [];
    enumerate(expected, actual, `cases[${i}](${expected.name})`, differences);
    rows.push({ name: expected.name, differences });
  }
} else {
  enumerate(source, native, '$', rows.push.bind(rows));
}

const groups = new Map();
for (const row of rows) for (const difference of row.differences || []) {
  const pathKey = difference.path.replace(/^cases\[\d+\]\([^)]*\)\./, 'case.');
  const prior = groups.get(pathKey) || { path: pathKey, count: 0, samples: [] };
  prior.count++;
  if (prior.samples.length < 5) prior.samples.push({ case: row.name, source: difference.source, native: difference.native });
  groups.set(pathKey, prior);
}
const summary = {
  source: path.relative(root, sourcePath), sourceSha256: digest(sourcePath),
  native: path.relative(root, nativePath), nativeSha256: digest(nativePath),
  cases: rows.length,
  matched: rows.filter(row => !(row.differences || []).length).length,
  totalDifferences: rows.reduce((sum, row) => sum + (row.differences || []).length, 0),
  groups: [...groups.values()].sort((a, b) => b.count - a.count || a.path.localeCompare(b.path)),
  rows,
};
fs.mkdirSync(path.dirname(summaryPath), { recursive: true });
fs.writeFileSync(summaryPath, JSON.stringify(summary, null, 2) + '\n');
process.stdout.write(JSON.stringify({ cases: summary.cases, matched: summary.matched,
  totalDifferences: summary.totalDifferences, groups: summary.groups.length,
  firstGroups: summary.groups.slice(0, 12), output: path.relative(root, summaryPath) }) + '\n');
