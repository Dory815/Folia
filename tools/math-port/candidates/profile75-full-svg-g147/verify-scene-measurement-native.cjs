'use strict';
// Runs the existing locked 75-case feed/comparison and checks the additional
// scene-only measurement counters emitted by the paired NativeRenderSession probe.
const fs=require('node:fs');
const path=require('node:path');
const {spawnSync}=require('node:child_process');
const assert=require('node:assert/strict');

const root=path.resolve(__dirname,'../../../..');
const rendererExecutable=process.argv[2];
const sessionProbeExecutable=process.argv[3];
const gate=Number(process.argv[4]);
if(!rendererExecutable||!sessionProbeExecutable||!Number.isSafeInteger(gate)||gate<1)
  throw Error('Usage: node verify-scene-measurement-native.cjs <public-renderer-exe> <session-probe-exe> <build-gate> [output-json]');
const output=path.resolve(root,process.argv[5]||'build/math-port/profile75-scene-measurement-native.json');
const runner=path.join(__dirname,'run-profile75-native.cjs');
const publicCapture=path.join(root,`build/math-port/profile75-full-svg-native-g${gate}.json`);
const sessionCapture=path.join(root,`build/math-port/profile75-scene-measurement-session-g${gate}.json`);
const publicRun=spawnSync(process.execPath,[runner,path.resolve(rendererExecutable),
  path.relative(root,publicCapture)],{
  cwd:root,encoding:'utf8',windowsHide:true,timeout:360000,maxBuffer:512*1024*1024,
});
if(publicRun.status!==0||publicRun.error){
  process.stderr.write(publicRun.stdout||'');process.stderr.write(publicRun.stderr||'');
  process.exit(publicRun.status??1);
}
const annotate=path.join(__dirname,'annotate-scene-measurement-renderer-boundary.cjs');
const annotated=spawnSync(process.execPath,[annotate,path.relative(root,publicCapture),
  `build/math-port/profile75-full-svg-native-boundary-g${gate}.json`,String(gate)],{
  cwd:root,encoding:'utf8',windowsHide:true,timeout:30000,maxBuffer:16*1024*1024,
});
if(annotated.status!==0||annotated.error){
  process.stderr.write(annotated.stdout||'');process.stderr.write(annotated.stderr||'');
  process.exit(annotated.status??1);
}
const compare=path.join(root,'tools/math-port/candidates/profile75-full-svg-clean-source-g150/compare.cjs');
const cleanSource=path.join(root,'build/math-port/profile75-full-svg-source-clean-g150-root.json');
const comparison=path.join(root,`build/math-port/profile75-full-svg-comparison-g${gate}-scene-measurement.json`);
const compared=spawnSync(process.execPath,[compare,cleanSource,
  `build/math-port/profile75-full-svg-native-boundary-g${gate}.json`,
  path.relative(root,comparison)],{
  cwd:root,encoding:'utf8',windowsHide:true,timeout:360000,maxBuffer:512*1024*1024,
});
if(compared.status!==0||compared.error){
  process.stderr.write(compared.stdout||'');process.stderr.write(compared.stderr||'');
  process.exit(compared.status??1);
}
const comparisonResult=JSON.parse(fs.readFileSync(comparison,'utf8'));
assert.equal(comparisonResult.matched,150,'clean Source comparator must match every public Renderer SVG record');

const sessionRun=spawnSync(process.execPath,[runner,path.resolve(sessionProbeExecutable),
  path.relative(root,sessionCapture)],{
  cwd:root,encoding:'utf8',windowsHide:true,timeout:360000,maxBuffer:512*1024*1024,
});
if(sessionRun.status!==0||sessionRun.error){
  process.stderr.write(sessionRun.stdout||'');process.stderr.write(sessionRun.stderr||'');
  process.exit(sessionRun.status??1);
}
const capture=JSON.parse(fs.readFileSync(sessionCapture,'utf8'));
assert.equal(capture.records.length,150,'two 75-case groups are required');
for(const row of capture.records){
  assert.equal(row.sceneOnlyMaterializeCalls,0,
    `${row.group} case ${row.index} entered Serializer::materialize`);
  assert.equal(row.sceneOnlySerializeXmlCalls,0,
    `${row.group} case ${row.index} entered LiteAdaptor::serializeXML`);
  if(row.error===null){
    assert.equal(row.sceneEqual,true,`${row.group} case ${row.index} changed the produced Scene`);
    assert.ok(row.svgMaterializeCalls>0,
      `${row.group} case ${row.index} normal SVG consumer did not materialize`);
    assert.ok(row.svgSerializeXmlCalls>0,
      `${row.group} case ${row.index} normal SVG output did not serialize XML`);
  }else{
    assert.deepEqual(row.error.scene,row.error.svg,
      `${row.group} case ${row.index} produced different Scene-only and SVG route errors`);
  }
}
fs.copyFileSync(sessionCapture,output);
process.stdout.write(JSON.stringify({publicRendererComparison:comparison,matchedPublicSvgRecords:comparisonResult.matched,
  sessionProbeOutput:sessionCapture,output,records:capture.records.length,
  groups:capture.groups,sceneOnlyMaterializeCalls:0,sceneOnlySerializeXmlCalls:0,
  matchedScenes:capture.records.filter(row=>row.error===null&&row.sceneEqual).length})+'\n');
