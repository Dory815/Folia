'use strict';
// Attach auditable shared/fresh Renderer boundaries to the standard capture.
// The clean-G150 comparer requires this metadata before checking every SVG byte.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../../../..');
const capturePath=path.resolve(root,process.argv[2]);
const outputPath=path.resolve(root,process.argv[3]);
const gate=Number(process.argv[4]);
if(!process.argv[2]||!process.argv[3]||!Number.isSafeInteger(gate)||gate<1)
  throw Error('Usage: node annotate-scene-measurement-renderer-boundary.cjs <native-capture> <output> <build-gate>');
const digest=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
const hashFile=file=>digest(fs.readFileSync(file));
const probePath=path.join(root,'tools/math-port/candidates/profile75-full-svg-g147/profile75-full-svg-native-candidate.cpp');
const manifestPath=path.join(root,`build/math-port/gate-${gate}-inputs-before.json`);
const auditPath=path.join(root,`build/math-port/profile75-full-svg-binary-audit-g${gate}.json`);
const native=JSON.parse(fs.readFileSync(capturePath,'utf8'));
const provenance=JSON.parse(fs.readFileSync(capturePath+'.provenance.json','utf8'));
const manifest=JSON.parse(fs.readFileSync(manifestPath,'utf8'));
const capturedProbe=manifest.find(row=>path.resolve(root,row.path)===probePath);
assert.ok(capturedProbe,'public Renderer profile probe is absent from build inputs');
assert.equal(hashFile(probePath),capturedProbe.sha256,'profile probe differs from compiled input');
const audit=JSON.parse(fs.readFileSync(auditPath,'utf8'));
assert.equal(audit.exitCode,0);assert.equal(audit.unchanged,true);
assert.equal(audit.before.toLowerCase(),provenance.executableSha256);
assert.equal(audit.after.toLowerCase(),provenance.executableSha256);
assert.equal(native.renderer,'folia.math::Renderer');
assert.deepStrictEqual(native.groups,['continuous','fresh-session-per-case']);
assert.equal(native.records.length,150);assert.equal(provenance.exitCode,0);
assert.equal(hashFile(provenance.executable),provenance.executableSha256,
  'public Renderer executable changed after capture');
const output={...native,
  runtimeBoundaries:{
    continuous:{isolation:'shared-owner-across-cases',owner:'one folia.math::Renderer with one NativeRenderSession/Context'},
    'fresh-session-per-case':{isolation:'isolated-owner-per-case',owner:'fresh folia.math::Renderer/NativeRenderSession/Context per record'},
  },
  boundaryEvidence:{
    buildGate:gate,
    nativeCapture:path.relative(root,capturePath).replaceAll(path.sep,'/'),
    nativeCaptureSha256:hashFile(capturePath),
    executable:provenance.executable,
    executableSha256:provenance.executableSha256,
    probeSource:path.relative(root,probePath).replaceAll(path.sep,'/'),
    probeSourceSha256:hashFile(probePath),
    buildInputsSha256:hashFile(manifestPath),
    binaryAuditSha256:hashFile(auditPath),
  },
};
fs.mkdirSync(path.dirname(outputPath),{recursive:true});
fs.writeFileSync(outputPath,JSON.stringify(output,null,2)+'\n');
process.stdout.write(JSON.stringify({output:outputPath,records:output.records.length,
  executableSha256:provenance.executableSha256,probeSourceSha256:output.boundaryEvidence.probeSourceSha256})+'\n');
