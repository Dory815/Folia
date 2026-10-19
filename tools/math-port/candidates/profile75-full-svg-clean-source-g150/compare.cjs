'use strict';
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');

const sourcePath=path.resolve(process.argv[2]);
const nativePath=path.resolve(process.argv[3]);
const destination=path.resolve(process.argv[4]||'build/math-port/profile75-full-svg-comparison.json');
const root=path.resolve(__dirname,'../../../..');
const corpusPath=path.join(root,'tools/math-port/common-layout-reference/profile-cases.json');
const lockPath=path.join(root,'tools/math-port/common-layout-reference/profile-source-lock.json');
const inputCapturePath=path.join(root,'build/math-port/common-layout-profile-root-locked.json');
const dependencyLockPath=path.join(root,'tools/mathjax-bundle/package-lock.json');
const scopePath=path.join(root,'docs/math-port/scope.json');
const scope=JSON.parse(fs.readFileSync(scopePath,'utf8'));
const profilePath=path.join(root,scope.profile.file);
const corpus=JSON.parse(fs.readFileSync(corpusPath,'utf8'));
const lock=JSON.parse(fs.readFileSync(lockPath,'utf8'));
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
const digest=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
const hashFile=file=>digest(fs.readFileSync(file));
const groups=['continuous','fresh-session-per-case'];

assert.equal(corpus.schema,1);assert.equal(corpus.cases.length,57);
const inputCapture=JSON.parse(fs.readFileSync(inputCapturePath,'utf8'));
assert.equal(inputCapture.records.length,75);
const cases=inputCapture.records.map(({name,tex,display,em,ex,width})=>({name,tex,display,em,ex,width}));
assert.ok(cases.every(item=>item.ex===item.em*.5));
assert.equal(source.schema,1);assert.equal(native.schema,1);
assert.equal(source.source.version,'4.1.3');
assert.equal(source.source.commit,'fb987178c2d279a99b0db5ea02e751691703955e');
assert.equal(source.source.scopeSha256,hashFile(scopePath));
assert.equal(source.source.profileSha256,scope.profile.sha256);
assert.equal(source.source.profileSha256,hashFile(profilePath));
assert.equal(source.source.sourceLockSha256,hashFile(lockPath));
assert.equal(source.source.corpusSha256,hashFile(corpusPath));
assert.equal(source.source.caseInventorySha256,'9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740');
assert.equal(source.source.caseInventorySha256,hashFile(inputCapturePath));
assert.equal(source.source.dependencyLockSha256,scope.dependency_lock_sha256);
assert.equal(hashFile(dependencyLockPath),source.source.dependencyLockSha256);
assert.deepStrictEqual(source.source.dependencies,lock.dependencies);
for(const dependency of lock.dependencies)
  assert.equal(hashFile(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax',dependency.file)),
    dependency.sha256,dependency.file);
assert.deepStrictEqual(source.source.configuredPackages,scope.profile.tex_packages);
assert.deepStrictEqual(source.groups,groups);assert.deepStrictEqual(native.groups,groups);
const originalSourcePath=path.join(root,'build/math-port/profile75-full-svg-source-g149.json');
const nativeGate=native.boundaryEvidence.buildGate??149;
assert.ok(Number.isSafeInteger(nativeGate)&&nativeGate>0,'invalid native build gate');
const originalNativePath=path.join(root,`build/math-port/profile75-full-svg-native-g${nativeGate}.json`);
const sourceWorkerPath=path.join(__dirname,'worker.cjs');
const nativeProbePath=path.join(root,'tools/math-port/candidates/profile75-full-svg-g147/profile75-full-svg-native-candidate.cpp');
const sourceCaptureMethod=source.source.captureMethod;
assert.equal(sourceCaptureMethod.continuousInput,'build/math-port/profile75-full-svg-source-g149.json');
assert.equal(sourceCaptureMethod.cleanProcesses,75);
assert.equal(sourceCaptureMethod.freshWorker,'tools/math-port/candidates/profile75-full-svg-clean-source-g150/worker.cjs');
assert.equal(sourceCaptureMethod.freshWorkerSha256,hashFile(sourceWorkerPath));
assert.equal(sourceCaptureMethod.originalG149Sha256,hashFile(originalSourcePath));
const originalSource=JSON.parse(fs.readFileSync(originalSourcePath,'utf8'));
assert.deepStrictEqual(source.records.slice(0,75),originalSource.records.slice(0,75),
  'continuous observations must remain the untouched G149 records');
assert.equal(native.boundaryEvidence.nativeCapture,`build/math-port/profile75-full-svg-native-g${nativeGate}.json`);
assert.equal(native.boundaryEvidence.nativeCaptureSha256,hashFile(originalNativePath));
assert.equal(native.boundaryEvidence.probeSource,'tools/math-port/candidates/profile75-full-svg-g147/profile75-full-svg-native-candidate.cpp');
assert.equal(native.boundaryEvidence.probeSourceSha256,hashFile(nativeProbePath));
const nativeProvenance=JSON.parse(fs.readFileSync(originalNativePath+'.provenance.json','utf8'));
assert.equal(native.boundaryEvidence.executableSha256,nativeProvenance.executableSha256);
assert.equal(hashFile(nativeProvenance.executable),nativeProvenance.executableSha256);
assert.equal(nativeProvenance.exitCode,0);
if(native.boundaryEvidence.buildGate!==undefined){
  const manifestPath=path.join(root,`build/math-port/gate-${nativeGate}-inputs-before.json`);
  const binaryAuditPath=path.join(root,`build/math-port/profile75-full-svg-binary-audit-g${nativeGate}.json`);
  assert.equal(native.boundaryEvidence.buildInputsSha256,hashFile(manifestPath));
  assert.equal(native.boundaryEvidence.binaryAuditSha256,hashFile(binaryAuditPath));
}
const expectedSourceBoundaries={
  continuous:{isolation:'shared-owner-across-cases',owner:'one MathJax module realm and one HTMLMathDocument'},
  'fresh-session-per-case':{isolation:'isolated-owner-per-case',owner:'one clean process/module realm, TeX/SVG jax pair, and HTMLMathDocument per record'},
};
const expectedNativeBoundaries={
  continuous:{isolation:'shared-owner-across-cases',owner:'one folia.math::Renderer with one NativeRenderSession/Context'},
  'fresh-session-per-case':{isolation:'isolated-owner-per-case',owner:'fresh folia.math::Renderer/NativeRenderSession/Context per record'},
};
assert.deepStrictEqual(source.source.runtimeBoundaries,expectedSourceBoundaries,
  'Source capture does not prove the declared module/process boundaries');
assert.deepStrictEqual(native.runtimeBoundaries,expectedNativeBoundaries,
  'native capture does not prove the declared Renderer/Context boundaries');
assert.equal(source.source.runtimeBoundaries.continuous.isolation,
  native.runtimeBoundaries.continuous.isolation,'continuous groups do not share state across cases');
assert.equal(source.source.runtimeBoundaries['fresh-session-per-case'].isolation,
  native.runtimeBoundaries['fresh-session-per-case'].isolation,'fresh groups are not independently isolated per case');
assert.equal(native.renderer,'folia.math::Renderer');
assert.equal(source.source.corpusSha256,hashFile(corpusPath));assert.equal(native.corpusSha256,hashFile(corpusPath));
assert.equal(native.caseInventorySha256,hashFile(inputCapturePath));
assert.equal(source.records.length,150);assert.equal(native.records.length,150);

const matched=[];const failures=[];
for(let position=0;position<150;++position){
  const groupIndex=Math.floor(position/75),index=position%75;
  const group=groups[groupIndex],item=cases[index];
  const src=source.records[position],nat=native.records[position];
  const label=`${group}:${index}:${item.name}`;
  try{
    for(const row of [src,nat]){
      assert.equal(row.group,group,`${label}: group`);assert.equal(row.index,index,`${label}: index`);
      assert.equal(row.name,item.name,`${label}: name`);assert.equal(row.tex,item.tex,`${label}: TeX`);
      assert.equal(row.display,item.display??false,`${label}: display`);
      assert.equal(row.em,item.em??16,`${label}: em`);assert.equal(row.ex,item.ex??(item.em??16)*.5,`${label}: ex`);
      assert.equal(row.width,item.width??1280,`${label}: width`);
      assert.ok(row.svg===null||typeof row.svg==='string',`${label}: SVG type`);
      assert.equal(typeof row.mmlError,'boolean',`${label}: MML-error flag`);
      assert.ok(row.error===null||(row.error&&typeof row.error.name==='string'&&
        typeof row.error.message==='string'),`${label}: error shape`);
    }
    assert.deepStrictEqual(nat.error,src.error,`${label}: exception name/message`);
    assert.equal(nat.mmlError,src.mmlError,`${label}: MML error`);
    if(src.svg===null)assert.equal(nat.svg,null,`${label}: absent SVG`);
    else {
      assert.ok(nat.svg!==null,`${label}: native SVG missing`);
      assert.ok(Buffer.from(nat.svg,'utf8').equals(Buffer.from(src.svg,'utf8')),
        `${label}: complete SVG bytes differ`);
      assert.match(nat.svg,/<svg\b/,`${label}: native SVG element`);
      assert.ok(nat.sceneNodes>0,`${label}: rendered SVG has no completed Scene nodes`);
    }
    if(src.svg===null)assert.equal(nat.sceneNodes,0,`${label}: scene should be absent after throw`);
    matched.push({group,index,name:item.name,svgSha256:src.svg===null?null:digest(Buffer.from(src.svg,'utf8')),
      error:src.error,mmlError:src.mmlError});
  }catch(error){
    failures.push({group,index,name:item.name,message:error.message,
      sourceSvgSha256:src.svg===null?null:digest(Buffer.from(src.svg,'utf8')),
      nativeSvgSha256:nat.svg===null?null:digest(Buffer.from(nat.svg,'utf8'))});
  }
}
const summary={sourceCommit:source.source.commit,casesPerGroup:75,total:150,
  matched:matched.length,accepted:matched.length===150,matchedCases:matched,failures,
  sourceInputSha256:hashFile(sourcePath),nativeInputSha256:hashFile(nativePath)};
fs.mkdirSync(path.dirname(destination),{recursive:true});
fs.writeFileSync(destination,JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({casesPerGroup:75,total:150,matched:summary.matched,
  accepted:summary.accepted,failures}));
process.exitCode=summary.accepted?0:1;
