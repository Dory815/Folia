'use strict';

const fs = require('node:fs');
const path = require('node:path');
const cp = require('node:child_process');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const [nativeArg, reportArg] = process.argv.slice(2);
if (!nativeArg || !reportArg) throw Error('Native observation and report paths are required');
const hash = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const nativePath = path.resolve(nativeArg);
const native = JSON.parse(fs.readFileSync(nativePath));
const sourceRecords = {};
const sourceProvenance = [];
for (const [name, script] of [
  ['nodeFactory', 'tools/math-port/drafts/parser-projection-original-oracle.cjs'],
  ['parseOptions', 'tools/math-port/drafts/parser-options-projection-original-oracle.cjs'],
]) {
  const output = cp.execFileSync(process.execPath, [script], {cwd: root, encoding: 'utf8'});
  sourceRecords[name] = JSON.parse(output);
  sourceProvenance.push({script, scriptSha256: hash(script),
    outputSha256: crypto.createHash('sha256').update(output).digest('hex')});
}
function compare(source, actual, at) {
  if (source !== null && typeof source === 'object') {
    assert.equal(Array.isArray(actual), Array.isArray(source), `${at}: array type`);
    assert.ok(actual !== null && typeof actual === 'object', `${at}: object type`);
    assert.deepStrictEqual(Object.keys(actual), Object.keys(source), `${at}: complete key order`);
    for (const key of Object.keys(source)) compare(source[key], actual[key], `${at}.${key}`);
  } else assert.deepStrictEqual(actual, source, at);
}
const failures = [];
for (const name of Object.keys(sourceRecords)) {
  try { compare(sourceRecords[name], native[name], name); }
  catch (error) { failures.push({record: name, difference: error.message}); }
}
const ownerKeys = ['variantMapStorageIdentity', 'functionListOwnerIdentity',
  'nodeFactoryConfigurationValueOwnerRetainsReplacement', 'nodeFactoryNativeCallableBodiesShared',
  'nodeFactoryContextFunctionIdentityIsDistinct', 'nodeFactoryContextFunctionPropertiesAreIndependent'];
try {
  assert.deepStrictEqual(Object.keys(native), ['nodeFactory', 'parseOptions', 'runtimeOwners']);
  assert.deepStrictEqual(Object.keys(native.runtimeOwners), ownerKeys);
  for (const key of ownerKeys) assert.equal(native.runtimeOwners[key], true, key);
} catch (error) { failures.push({record: 'runtimeOwners', difference: error.message}); }
const report = {accepted: failures.length === 0, sourceRecords: 2, sourceProvenance,
  native: {path: nativePath, sha256: hash(nativePath)}, failures};
fs.writeFileSync(path.resolve(reportArg), JSON.stringify(report, null, 2) + '\n');
console.log(JSON.stringify(report));
process.exitCode = report.accepted ? 0 : 1;
