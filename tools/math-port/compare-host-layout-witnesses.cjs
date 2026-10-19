'use strict';

// Strictly compare the original Source observations for Array.unshift and
// CommonOutputJax.getBBox with their native Context observations. The native
// records may append nativeChecks, but every Source field stays in place and
// is compared recursively without key sorting or value normalization.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const root = path.resolve(__dirname, '../..');
const args = process.argv.slice(2);
if (args.length !== 4 && args.length !== 5) {
  console.error('usage: node tools/math-port/compare-host-layout-witnesses.cjs <source-unshift.json> <native-unshift.json> <source-getbbox.json> <native-getbbox.json> [report.json]');
  process.exit(2);
}

const [sourceUnshiftPath, nativeUnshiftPath, sourceBBoxPath, nativeBBoxPath] =
  args.slice(0, 4).map(value => path.resolve(value));
const reportPath = args[4] ? path.resolve(args[4]) : null;
const unshiftOracle = path.join(__dirname, 'drafts/array-unshift-source-oracle.cjs');
const bboxOracle = path.join(__dirname, 'drafts/common-getbbox-source-oracle.cjs');
const failures = [];

function sha256(file) {
  return crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
}

function readJSON(file, label) {
  try {
    return JSON.parse(fs.readFileSync(file, 'utf8'));
  } catch (error) {
    failures.push({ path: label, error: String(error) });
    return undefined;
  }
}

function sameKeys(actual, expected, at) {
  if (!actual || typeof actual !== 'object' || Array.isArray(actual)) {
    failures.push({ path: at, expected: 'object', actual: actual === null ? null : typeof actual });
    return false;
  }
  const a = Object.keys(actual);
  if (a.length !== expected.length || a.some((key, i) => key !== expected[i])) {
    failures.push({ path: `${at}.[key-order]`, expected, actual: a });
    return false;
  }
  return true;
}

function compareValue(expected, actual, at) {
  if (Object.is(expected, actual)) return;
  if (expected === null || actual === null || typeof expected !== 'object' ||
      typeof actual !== 'object' || Array.isArray(expected) !== Array.isArray(actual)) {
    failures.push({ path: at, expected, actual });
    return;
  }
  const expectedKeys = Object.keys(expected);
  const actualKeys = Object.keys(actual);
  if (expectedKeys.length !== actualKeys.length ||
      expectedKeys.some((key, i) => key !== actualKeys[i])) {
    failures.push({ path: `${at}.[key-order]`, expected: expectedKeys, actual: actualKeys });
  }
  const keys = [...new Set([...expectedKeys, ...actualKeys])];
  for (const key of keys) {
    if (!Object.prototype.hasOwnProperty.call(expected, key)) {
      failures.push({ path: `${at}.${key}`, expected: '<absent>', actual: actual[key] });
    } else if (!Object.prototype.hasOwnProperty.call(actual, key)) {
      failures.push({ path: `${at}.${key}`, expected: expected[key], actual: '<absent>' });
    } else {
      compareValue(expected[key], actual[key], Array.isArray(expected) ? `${at}[${key}]` : `${at}.${key}`);
    }
  }
}

function compareRecord(label, source, native, sourceKeys, nativeChecksKeys) {
  if (!source || typeof source !== 'object' || Array.isArray(source)) return;
  if (!native || typeof native !== 'object' || Array.isArray(native)) return;
  sameKeys(source, sourceKeys, `${label}.source`);
  sameKeys(native, [...sourceKeys, 'nativeChecks'], `${label}.native`);
  // Compare every field in the Source record, including nested object key order.
  for (const key of sourceKeys) {
    if (!Object.prototype.hasOwnProperty.call(source, key)) {
      failures.push({ path: `${label}.source.${key}`, expected: 'Source field', actual: '<absent>' });
    } else if (!Object.prototype.hasOwnProperty.call(native, key)) {
      failures.push({ path: `${label}.native.${key}`, expected: source[key], actual: '<absent>' });
    } else {
      compareValue(source[key], native[key], `${label}.${key}`);
    }
  }
  if (!Object.prototype.hasOwnProperty.call(native, 'nativeChecks')) {
    failures.push({ path: `${label}.nativeChecks`, expected: 'present', actual: '<absent>' });
  } else {
    sameKeys(native.nativeChecks, nativeChecksKeys, `${label}.nativeChecks`);
  }
}

const sourceUnshift = readJSON(sourceUnshiftPath, 'source.unshift.json');
const nativeUnshift = readJSON(nativeUnshiftPath, 'native.unshift.json');
const sourceBBox = readJSON(sourceBBoxPath, 'source.getBBox.json');
const nativeBBox = readJSON(nativeBBoxPath, 'native.getBBox.json');

const unshiftKeys = ['source', 'sourceSha256', 'method', 'inherited', 'sparsePresence',
  'order', 'zeroArgTrace', 'partial', 'nullReceiverThrows'];
const bboxKeys = ['source', 'sourceSha256', 'method', 'item', 'measurements',
  'replacementCalls', 'exceptionPropagated'];
compareRecord('unshift', sourceUnshift, nativeUnshift, unshiftKeys,
  ['ordinary', 'generic', 'nativeView', 'zeroArgResult', 'readOnlyLengthThrows', 'partialWriteThrows']);
compareRecord('getBBox', sourceBBox, nativeBBox, bboxKeys,
  ['sourceSetMetricsExposed', 'nativeMathItemSetMetricsUsed', 'firstCanonical',
    'secondCanonical', 'firstWidthBefore', 'firstWidthAfter', 'liveField', 'replacementForwarded']);

if (nativeUnshift?.nativeChecks) {
  const checks = nativeUnshift.nativeChecks;
  const expected = {
    ordinary: ['one', 'two', 'three'],
    generic: ['one', 'head', 'inherited-tail'],
    nativeView: { length: 3, values: ['start', 'left', 'right'] },
    zeroArgResult: 2,
    readOnlyLengthThrows: true,
    partialWriteThrows: true,
  };
  compareValue(expected, checks, 'unshift.nativeChecks');
}

if (nativeBBox?.nativeChecks) {
  const checks = nativeBBox.nativeChecks;
  compareValue({
    sourceSetMetricsExposed: false,
    nativeMathItemSetMetricsUsed: true,
    firstCanonical: true,
    secondCanonical: true,
    firstWidthBefore: sourceBBox?.measurements?.[0]?.w,
    firstWidthAfter: sourceBBox?.measurements?.[0]?.w + 0.25,
    liveField: true,
    replacementForwarded: true,
  }, checks, 'getBBox.nativeChecks');
}

const paths = [unshiftOracle, bboxOracle, sourceUnshiftPath, nativeUnshiftPath,
  sourceBBoxPath, nativeBBoxPath];
const hashes = {};
for (const file of paths) {
  try { hashes[path.relative(root, file)] = sha256(file); }
  catch (error) { failures.push({ path: `hash:${file}`, error: String(error) }); }
}
const comparatorPath = path.join(__dirname, 'compare-host-layout-witnesses.cjs');
try { hashes[path.relative(root, comparatorPath)] = sha256(comparatorPath); }
catch (error) { failures.push({ path: 'hash:comparator', error: String(error) }); }

const report = {
  accepted: failures.length === 0,
  comparison: 'strict Source fields including nested key order; exact nativeChecks',
  sourceOracleScripts: [path.relative(root, unshiftOracle), path.relative(root, bboxOracle)],
  records: {
    unshift: { source: sourceUnshiftPath, native: nativeUnshiftPath },
    getBBox: { source: sourceBBoxPath, native: nativeBBoxPath },
  },
  sha256: hashes,
  failures,
};
const output = JSON.stringify(report, null, 2) + '\n';
if (reportPath) {
  fs.mkdirSync(path.dirname(reportPath), { recursive: true });
  fs.writeFileSync(reportPath, output);
}
process.stdout.write(output);
process.exitCode = report.accepted ? 0 : 1;
