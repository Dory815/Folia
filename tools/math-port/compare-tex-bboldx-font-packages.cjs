'use strict';

// Strict comparison for the BBM/BBoldx/Dsfont native candidate.  The Source oracle
// and native candidate both serialize actual trees, packageData, registered
// parse maps, and runtime errors; this tool does not normalize or omit fields.
const fs = require('node:fs');
const path = require('node:path');

const root = path.resolve(__dirname, '../..');
const sourcePath = path.join(root, 'build/math-port/tex-bboldx-font-packages-original.json');
const nativePath = path.resolve(process.argv[2] || path.join(root, 'build/math-port/tex-bboldx-font-packages-native.json'));
const source = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));

function firstDiff(a, b, path = '$') {
  if (Object.is(a, b)) return null;
  if (a === null || b === null || typeof a !== 'object' || typeof b !== 'object') {
    return { path, source: a, native: b };
  }
  if (Array.isArray(a) !== Array.isArray(b)) return { path, source: a, native: b };
  const ak = Object.keys(a).sort(), bk = Object.keys(b).sort();
  if (ak.length !== bk.length || ak.some((key, i) => key !== bk[i])) {
    return { path: `${path}.[keys]`, source: ak, native: bk };
  }
  for (const key of ak) {
    const diff = firstDiff(a[key], b[key], Array.isArray(a) ? `${path}[${key}]` : `${path}.${key}`);
    if (diff) return diff;
  }
  return null;
}

const failures = [];
if (source.schema !== native.schema) failures.push({ path: 'schema', source: source.schema, native: native.schema });
if (!Array.isArray(source.cases) || !Array.isArray(native.cases)) {
  failures.push({ path: 'cases', source: Array.isArray(source.cases), native: Array.isArray(native.cases) });
} else {
  if (source.cases.length !== native.cases.length) failures.push({ path: 'cases.length', source: source.cases.length, native: native.cases.length });
  const length = Math.min(source.cases.length, native.cases.length);
  for (let i = 0; i < length; i++) {
    const expected = source.cases[i], actual = native.cases[i];
    if (expected.name !== actual.name) {
      failures.push({ path: `cases[${i}].name`, source: expected.name, native: actual.name });
      continue;
    }
    const diff = firstDiff(expected, actual, `cases[${i}](${expected.name})`);
    if (diff) failures.push(diff);
  }
}

const report = {
  accepted: failures.length === 0,
  total: Array.isArray(source.cases) ? source.cases.length : 0,
  matched: Array.isArray(source.cases) && Array.isArray(native.cases)
    ? source.cases.filter((item, i) => native.cases[i] && !firstDiff(item, native.cases[i])).length
    : 0,
  source: sourcePath,
  native: nativePath,
  failures,
};
const output = path.join(root, 'build/math-port/tex-bboldx-font-packages-comparison.json');
fs.mkdirSync(path.dirname(output), { recursive: true });
fs.writeFileSync(output, JSON.stringify(report, null, 2) + '\n');
console.log(JSON.stringify({ accepted: report.accepted, total: report.total, matched: report.matched, firstDiff: failures[0] || null, output }));
process.exitCode = report.accepted ? 0 : 1;
