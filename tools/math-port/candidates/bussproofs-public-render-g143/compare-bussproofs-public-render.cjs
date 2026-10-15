'use strict';
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');

const sourcePath=path.resolve(process.argv[2]);
const nativePath=path.resolve(process.argv[3]);
const destination=path.resolve(process.argv[4]||'build/math-port/bussproofs-public-render-comparison.json');
const digest=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
const hashFile=file=>digest(fs.readFileSync(file));
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
const root=path.resolve(__dirname,'../../../../');
const scope=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/scope.json'),'utf8'));
const lockPath=path.join(root,'docs/math-port/evidence/bussproofs-source-lock.json');
const bussproofsLock=JSON.parse(fs.readFileSync(lockPath,'utf8'));
const expected=[
  ['unary-proof',String.raw`\begin{prooftree}\AxiomC{A}\UnaryInfC{B}\end{prooftree}`],
  ['ordinary-after-proof','x+1'],
  ['binary-proof-after-formula',String.raw`\begin{prooftree}\AxiomC{P}\AxiomC{Q}\BinaryInfC{C}\end{prooftree}`]
];
assert.equal(source.schema,1);assert.equal(native.schema,1);
assert.equal(source.source.version,'4.1.3');
assert.equal(source.source.commit,'fb987178c2d279a99b0db5ea02e751691703955e');
assert.equal(source.source.scopeSha256,'9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1');
assert.equal(source.source.profileSha256,scope.profile.sha256);
assert.equal(source.source.dependencyLockSha256,scope.dependency_lock_sha256);
assert.equal(source.source.bussproofsLockSha256,hashFile(lockPath));
assert.deepStrictEqual(source.source.bussproofsCjs,
  Object.fromEntries(bussproofsLock.files.map(file=>[file.cjs,file.cjsSha256])));
assert.equal(source.records.length,expected.length);assert.equal(native.records.length,expected.length);
assert.equal(native.profile,'mathjax-full');
assert.equal(source.corpusSha256,hashFile(path.join(__dirname,'bussproofs-public-render-cases.json')));
const matched=[];const failures=[];
for(let index=0;index<expected.length;++index){
  const [name,tex]=expected[index];
  const src=source.records[index],nat=native.records[index];
  try{
    assert.equal(src.name,name);assert.equal(nat.name,name);
    assert.equal(src.tex,tex);assert.equal(nat.tex,tex);
    assert.equal(src.mmlError,false,`${name}: Source serialized an MML error`);
    assert.equal(nat.mmlError,false,`${name}: native serialized an MML error`);
    assert.equal(src.error,null,`${name}: Source threw`);
    assert.equal(nat.error,null,`${name}: native threw`);
    assert.match(src.svg,/<svg\b/);assert.match(nat.svg,/<svg\b/);
    assert.ok(nat.sceneNodes>0,`${name}: native completed no Scene nodes`);
    assert.deepStrictEqual({name:nat.name,tex:nat.tex,svg:nat.svg,mmlError:nat.mmlError,error:nat.error},
      {name:src.name,tex:src.tex,svg:src.svg,mmlError:src.mmlError,error:src.error});
    matched.push({name,svgSha256:digest(Buffer.from(src.svg))});
  }catch(error){failures.push({name,message:error.message,
    sourceSvgSha256:src.svg===null?null:digest(Buffer.from(src.svg)),
    nativeSvgSha256:nat.svg===null?null:digest(Buffer.from(nat.svg))});}
}
const summary={sourceCommit:source.source.commit,total:expected.length,matched:matched.length,
  accepted:matched.length===expected.length,matchedCases:matched,failures,
  sourceInputSha256:hashFile(sourcePath),nativeInputSha256:hashFile(nativePath)};
fs.mkdirSync(path.dirname(destination),{recursive:true});
fs.writeFileSync(destination,JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({total:summary.total,matched:summary.matched,accepted:summary.accepted,failures}));
process.exitCode=summary.accepted?0:1;
