'use strict';
// Feeds the locked 75-case JSON corpus to the public Renderer probe without
// copying case strings into C++ or rewriting captured renderer fields.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const {spawnSync}=require('node:child_process');
const assert=require('node:assert/strict');

const root=path.resolve(__dirname,'../../../..');
const executable=process.argv[2];
if(!executable)throw Error('Usage: node run-profile75-native.cjs <native-executable> [output-json]');
const output=path.resolve(root,process.argv[3]||'build/math-port/profile75-full-svg-native.json');
const corpusPath=path.join(root,'tools/math-port/common-layout-reference/profile-cases.json');
const corpus=JSON.parse(fs.readFileSync(corpusPath,'utf8'));
const lockPath=path.join(root,'tools/math-port/common-layout-reference/profile-source-lock.json');
const lock=JSON.parse(fs.readFileSync(lockPath,'utf8'));
const inputCapturePath=path.join(root,'build/math-port/common-layout-profile-root-locked.json');
const inputCapture=JSON.parse(fs.readFileSync(inputCapturePath,'utf8'));
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
assert.equal(digest(corpusPath),lock.corpusSha256,'locked profile case corpus changed');
assert.equal(digest(inputCapturePath),'9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740',
  'locked 75-case input capture changed');
assert.equal(corpus.cases.length,57);assert.equal(inputCapture.records.length,75);
const cases=inputCapture.records.map(({name,tex,display,em,ex,width})=>({name,tex,display,em,ex,width}));
assert.ok(cases.every(item=>item.ex===item.em*.5),'public Renderer has no configurable ex outside the locked profile');
assert.deepStrictEqual(cases.slice(-corpus.cases.length).map(({name,tex,display,width})=>({
  name,tex,...(display?{display:true}:{}),...(width!==1280?{width}:{}),
})),corpus.cases.map(({name,tex,display,width})=>({
  name,tex,...(display?{display:true}:{}),...(width!==undefined?{width}:{}),
})), '75-case input inventory does not include the locked 57-case profile suffix');
assert.deepStrictEqual(inputCapture.configuredPackages,JSON.parse(fs.readFileSync(
  path.join(root,'docs/math-port/scope.json'),'utf8')).profile.tex_packages);
const b64=value=>Buffer.from(value,'utf8').toString('base64');
const groups=['continuous','fresh-session-per-case'];
const input=groups.map(group=>'@group\t'+b64(group)+'\n'+cases.map((item,index)=>[
  index,b64(item.name),b64(item.tex),item.display?'1':'0',item.em??16,item.width??1280
].join('\t')).join('\n')+'\n').join('');
const provenance={executable:path.resolve(executable),executableSha256:digest(executable),
  corpusSha256:digest(corpusPath),caseInventorySha256:digest(inputCapturePath),
  startedUtc:new Date().toISOString()};
const result=spawnSync(path.resolve(executable),[],{cwd:root,input,encoding:'utf8',
  maxBuffer:512*1024*1024,windowsHide:true,timeout:300000});
Object.assign(provenance,{finishedUtc:new Date().toISOString(),pid:result.pid??null,
  exitCode:result.status,signal:result.signal,error:result.error?.message??null});
fs.mkdirSync(path.dirname(output),{recursive:true});
fs.writeFileSync(output+'.provenance.json',JSON.stringify(provenance,null,2)+'\n');
fs.writeFileSync(output+'.stdout',result.stdout||'');fs.writeFileSync(output+'.stderr',result.stderr||'');
if(result.status!==0||result.error){process.stderr.write(JSON.stringify(provenance)+'\n'+(result.stderr||''));process.exitCode=1;return;}
const observation=JSON.parse(result.stdout);
assert.deepStrictEqual(observation.groups,groups);
assert.equal(observation.records.length,150,'expected two independently labeled 75-case groups');
for(let index=0;index<observation.records.length;++index){
  const groupIndex=Math.floor(index/75),caseIndex=index%75;
  const group=groups[groupIndex],source=cases[caseIndex],row=observation.records[index];
  assert.equal(row.group,group,`record ${index} group`);assert.equal(row.index,caseIndex);
  assert.equal(row.name,source.name);assert.equal(row.tex,source.tex);
  assert.equal(row.display,source.display??false);assert.equal(row.em,source.em??16);
  assert.equal(row.ex,source.ex??(source.em??16)*.5);assert.equal(row.width,source.width??1280);
}
const native={schema:1,renderer:observation.renderer,groups:observation.groups,
  corpusSha256:digest(corpusPath),caseInventorySha256:digest(inputCapturePath),records:observation.records};
fs.writeFileSync(output,JSON.stringify(native,null,2)+'\n');
process.stdout.write(JSON.stringify({output,groups,caseCount:cases.length,records:native.records.length,
  svgRecords:native.records.filter(row=>row.svg!==null).length,
  errors:native.records.filter(row=>row.error!==null).length,
  mmlErrors:native.records.filter(row=>row.mmlError).length,provenance})+'\n');
