'use strict';

// Strict comparison for the supplementary BussproofsUtil export/configuration
// observation, separate from the locked package corpus.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const root = path.resolve(__dirname, '../..');
const sourcePath = path.join(root, 'build/math-port/bussproofs-util-surface-original.json');
const nativePath = path.resolve(process.argv[2] || path.join(root, 'build/math-port/bussproofs-util-surface-native.json'));
const expectedSourceSha256 = '4e4e5cf596d80637d9cc961f28803a5c5c1de3262e5a093db01be483e7a027f0';
const sourceBytes = fs.readFileSync(sourcePath);
const sourceSha256 = crypto.createHash('sha256').update(sourceBytes).digest('hex');
if (sourceSha256 !== expectedSourceSha256) {
  throw new Error(`BussproofsUtil Source observation differs from its recorded hash: ${sourceSha256}`);
}
const source = JSON.parse(sourceBytes.toString('utf8'));
const native = JSON.parse(fs.readFileSync(nativePath, 'utf8'));
const failures = [];

function compare(left, right, at) {
  if (Object.is(left, right)) return;
  if (left === null || right === null || typeof left !== 'object' ||
      typeof right !== 'object' || Array.isArray(left) !== Array.isArray(right)) {
    failures.push({ path: at, source: left, native: right });
    return;
  }
  if (Array.isArray(left)) {
    if (left.length !== right.length) {
      failures.push({ path: `${at}.length`, source: left.length, native: right.length });
    }
    for (let i = 0; i < Math.min(left.length, right.length); i++) {
      compare(left[i], right[i], `${at}[${i}]`);
    }
    return;
  }
  const sourceKeys = Object.keys(left);
  const nativeKeys = Object.keys(right);
  if (sourceKeys.length !== nativeKeys.length ||
      sourceKeys.some((key, i) => nativeKeys[i] !== key)) {
    failures.push({ path: `${at}.[keys]`, source: sourceKeys, native: nativeKeys });
  }
  for (const key of sourceKeys) {
    if (Object.hasOwn(right, key)) compare(left[key], right[key], `${at}.${key}`);
  }
}

compare(source, native, '$');
const report = {
  accepted: failures.length === 0,
  exports: source.utilExports?.length ?? 0,
  preprocessors: source.configuration?.preprocessors?.length ?? 0,
  postprocessors: source.configuration?.postprocessors?.length ?? 0,
  sourceSha256,
  nativeSha256: crypto.createHash('sha256').update(fs.readFileSync(nativePath)).digest('hex'),
  failures,
};
const destination = path.join(root, 'build/math-port/bussproofs-util-surface-comparison.json');
fs.writeFileSync(destination, `${JSON.stringify(report, null, 2)}\n`);
console.log(JSON.stringify({ accepted: report.accepted, exports: report.exports,
  preprocessors: report.preprocessors, postprocessors: report.postprocessors,
  failureCount: failures.length, firstDiff: failures[0] ?? null,
  output: path.relative(root, destination) }));
process.exitCode = report.accepted ? 0 : 1;
