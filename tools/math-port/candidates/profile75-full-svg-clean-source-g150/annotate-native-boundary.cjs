'use strict';
// Attach audited owner-boundary metadata to a native capture without rebuilding
// or changing any SVG bytes. Assertions tie the claim to the captured executable
// and the registered candidate's explicit Renderer construction sites.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../../../..');
const sourcePath = path.resolve(root, process.argv[2] || 'build/math-port/profile75-full-svg-native-g149.json');
const outputPath = path.resolve(root, process.argv[3] || 'build/math-port/profile75-full-svg-native-boundary-g150.json');
const buildGate = Number(process.argv[4] || 149);
assert.ok(Number.isSafeInteger(buildGate) && buildGate > 0, 'invalid build gate');
const probePath = path.join(root, 'tools/math-port/candidates/profile75-full-svg-g147/profile75-full-svg-native-candidate.cpp');
const digest = bytes => crypto.createHash('sha256').update(bytes).digest('hex');
const hashFile = file => digest(fs.readFileSync(file));
const native = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));
const provenance = JSON.parse(fs.readFileSync(sourcePath + '.provenance.json', 'utf8'));
const probe = fs.readFileSync(probePath, 'utf8');
const manifestPath = path.join(root, `build/math-port/gate-${buildGate}-inputs-before.json`);
const binaryAuditPath = path.join(root, `build/math-port/profile75-full-svg-binary-audit-g${buildGate}.json`);
const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
const capturedProbe = manifest.find(row => path.resolve(root, row.path) === probePath);
assert.ok(capturedProbe, 'native probe is absent from the Gate149 build inputs');
assert.equal(hashFile(probePath), capturedProbe.sha256, 'native probe differs from the compiled Gate149 input');
const binaryAudit = JSON.parse(fs.readFileSync(binaryAuditPath, 'utf8'));
assert.equal(binaryAudit.exitCode, 0);
assert.equal(binaryAudit.unchanged, true);
assert.equal(binaryAudit.before.toLowerCase(), provenance.executableSha256);
assert.equal(binaryAudit.after.toLowerCase(), provenance.executableSha256);
assert.equal(native.renderer, 'folia.math::Renderer');
assert.deepStrictEqual(native.groups, ['continuous', 'fresh-session-per-case']);
assert.equal(native.records.length, 150);
assert.equal(provenance.exitCode, 0);
assert.equal(hashFile(provenance.executable), provenance.executableSha256,
  'captured native executable changed since G149');
assert.ok(probe.includes('if(activeGroup=="continuous")continuous=std::make_unique<Renderer>();'),
  'continuous owner construction changed; boundary must be reviewed');
assert.ok(probe.includes('else {Renderer fresh;records.push_back(renderRecord(row,fresh));}'),
  'per-case Renderer construction changed; boundary must be reviewed');
assert.equal(provenance.corpusSha256, native.corpusSha256);
assert.equal(provenance.caseInventorySha256, native.caseInventorySha256);

const output = {
  ...native,
  runtimeBoundaries: {
    continuous: { isolation: 'shared-owner-across-cases', owner: 'one folia.math::Renderer with one NativeRenderSession/Context' },
    'fresh-session-per-case': { isolation: 'isolated-owner-per-case', owner: 'fresh folia.math::Renderer/NativeRenderSession/Context per record' }
  },
  boundaryEvidence: {
    buildGate,
    nativeCapture: path.relative(root, sourcePath).replaceAll(path.sep, '/'),
    nativeCaptureSha256: hashFile(sourcePath),
    executable: provenance.executable,
    executableSha256: provenance.executableSha256,
    probeSource: path.relative(root, probePath).replaceAll(path.sep, '/'),
    probeSourceSha256: hashFile(probePath),
    buildInputsSha256: hashFile(manifestPath),
    binaryAuditSha256: hashFile(binaryAuditPath)
  }
};
fs.mkdirSync(path.dirname(outputPath), { recursive: true });
fs.writeFileSync(outputPath, JSON.stringify(output, null, 2) + '\n');
process.stdout.write(JSON.stringify({ output: outputPath, records: output.records.length,
  executableSha256: provenance.executableSha256, probeSourceSha256: output.boundaryEvidence.probeSourceSha256 }) + '\n');
