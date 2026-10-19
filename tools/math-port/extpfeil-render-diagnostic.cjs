'use strict';

// Re-run the locked Source profile oracle and compare the extpfeil case with
// the latest recorded native common-layout capture. This is diagnostic only;
// the Source oracle remains the authority for expected behavior.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const { spawnSync } = require('node:child_process');
const assert = require('node:assert/strict');

const root = path.resolve(__dirname, '../..');
const build = path.join(root, 'build/math-port');
const digest = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const readJson = file => JSON.parse(fs.readFileSync(file, 'utf8'));
const sourceRun = spawnSync(process.execPath, [
  path.join(__dirname, 'common-layout-original-oracle.cjs'), '--profile-corpus'
], { cwd: root, encoding: 'utf8', maxBuffer: 16 * 1024 * 1024 });
assert.equal(sourceRun.status, 0, sourceRun.stderr || 'Source profile observer failed');
const source = JSON.parse(sourceRun.stdout);
const sourceCase = source.records.find(record => record.name === 'extpfeil-arrows');
assert.ok(sourceCase, 'Source profile omitted extpfeil-arrows');

const nativePath = path.join(build, 'common-layout-native-g113.json');
const comparisonPath = path.join(build, 'common-layout-comparison-g113.json');
const provenancePath = `${nativePath}.provenance.json`;
const nativeBytes = fs.readFileSync(nativePath);
const native = JSON.parse(nativeBytes.toString('utf8'));
const nativeCase = native.records.find(record => record.name === 'extpfeil-arrows');
const comparison = readJson(comparisonPath);
const provenance = readJson(provenancePath);
assert.ok(nativeCase, 'Gate113 native capture omitted extpfeil-arrows');
assert.equal(digest(nativeBytes), comparison.nativeSha256, 'Gate113 native capture hash changed');
assert.equal(provenance.executableSha256.length, 64, 'Gate113 executable hash is missing');
assert.equal(provenance.exitCode, 0, 'Gate113 native capture did not complete successfully');

function findText(node, kind, text) {
  if (!node) return null;
  if (node.kind === kind && node.text === text) return node;
  for (const child of node.children || []) {
    const found = findText(child, kind, text);
    if (found) return found;
  }
  return null;
}
const sourceArrow = findText(sourceCase.mml, 'mo', '↦');
const nativeArrow = findText(nativeCase.mml, 'mo', '↦');
assert.ok(sourceArrow && nativeArrow, 'Source/native MML arrow text differs');

const placeChars = record => record.calls
  .filter(call => call.method === 'placeChar')
  .map(call => ({ codepoint: call.codepoint, wrapper: call.wrapper, variant: call.variant }));
const sourceGlyphs = placeChars(sourceCase);
const nativeGlyphs = placeChars(nativeCase);
const sourceArrowCall = sourceGlyphs.find(call => call.codepoint === 0x27FC);
const nativeArrowCall = nativeGlyphs.find(call => call.codepoint === 0x21A6);
assert.ok(sourceArrowCall, 'Source did not select the recorded NewCM xmapsto glyph U+27FC');
assert.ok(nativeArrowCall, 'Native did not place the semantic xmapsto glyph U+21A6');

const wrapper = (record, id) => {
  const value = record.wrappers.find(item => item.id === id);
  assert.ok(value, `Missing wrapper ${id}`);
  return { id, kind: value.kind, width: value.bbox.w, scale: value.bbox.scale };
};
const result = {
  schema: 1,
  purpose: 'Diagnose extpfeil source semantics versus common/font rendering.',
  sourceVersion: source.version,
  sourceScopeHash: source.scopeHash,
  input: sourceCase.tex,
  sourceMmlArrow: { text: sourceArrow.text, attributes: sourceArrow.attributes },
  nativeMmlArrow: { text: nativeArrow.text, attributes: nativeArrow.attributes },
  sourcePlaceChars: sourceGlyphs,
  nativePlaceChars: nativeGlyphs,
  selectedGlyphs: {
    source: `U+${sourceArrowCall.codepoint.toString(16).toUpperCase()}`,
    native: `U+${nativeArrowCall.codepoint.toString(16).toUpperCase()}`
  },
  wrappers: {
    sourceMo: wrapper(sourceCase, sourceArrowCall.wrapper + 4),
    nativeMo: wrapper(nativeCase, nativeArrowCall.wrapper + 4),
    sourceMpadded: wrapper(sourceCase, 15),
    nativeMpadded: wrapper(nativeCase, 15)
  },
  sourceDynamicLoads: sourceCase.loads,
  nativeCapture: {
    file: path.relative(root, nativePath).replaceAll(path.sep, '/'),
    sha256: digest(nativeBytes),
    executableSha256: provenance.executableSha256,
    startedUtc: provenance.startedUtc,
    finishedUtc: provenance.finishedUtc,
    pid: provenance.pid,
    exitCode: provenance.exitCode
  },
  sourceWarnings: sourceRun.stderr.trim().split(/\r?\n/).filter(Boolean)
};
const outputPath = path.join(build, 'extpfeil-render-diagnostic.json');
fs.writeFileSync(outputPath, `${JSON.stringify(result, null, 2)}\n`);
process.stdout.write(`${JSON.stringify({ output: path.relative(root, outputPath), result }, null, 2)}\n`);
