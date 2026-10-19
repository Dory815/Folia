// Root-owned faults in the actual shared math library. No extra build tree.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process');
const assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/parser-host-faults');
const plan=JSON.parse(fs.readFileSync(path.join(__dirname,'parser-host-fault-plan.json'),'utf8'));
const hash=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
const originals=new Map(Object.entries(plan.baselineSha256).map(([file,sha])=>{
 const absolute=path.resolve(root,file);assert.ok(absolute.startsWith(root+path.sep));
 const bytes=fs.readFileSync(absolute);assert.equal(hash(bytes),sha.toLowerCase(),file+' baseline');
 return [file,{absolute,bytes,text:bytes.toString('utf8')}];
}));
// Find a complete method body without treating quoted/comment braces as C++.
function methodRange(text,method){
 const name=method.split(' ')[0],start=text.indexOf(name+'(');assert.ok(start>=0,method);
 assert.equal(text.indexOf(name+'(',start+1),-1,method+' must identify one definition');
 let i=text.indexOf('{',start),depth=0,state='code';const begin=i;
 for(;i<text.length;i++){
  const c=text[i],next=text[i+1];
  if(state==='line'){if(c==='\n')state='code';continue;}
  if(state==='comment'){if(c==='*'&&next==='/'){state='code';i++;}continue;}
  if(state==='string'||state==='char'){
   if(c==='\\'){i++;continue;}
   if(c===(state==='string'?'"':"'"))state='code';continue;
  }
  if(c==='/'&&next==='/'){state='line';i++;continue;}
  if(c==='/'&&next==='*'){state='comment';i++;continue;}
  if(c==='"'){state='string';continue;}if(c==="'"){state='char';continue;}
  if(c==='{')depth++;if(c==='}'&&!--depth)return [begin,i+1];
 }
 throw new Error('Unterminated method '+method);
}
const mutations=plan.rows.map(row=>{
 const original=originals.get(row.source);assert.ok(original,row.source);
 const [begin,end]=methodRange(original.text,row.method),body=original.text.slice(begin,end);
 // A plan may use LF anchors in a CRLF file; retain all unrelated bytes.
 const anchor=row.anchor.replaceAll('\n',original.text.includes('\r\n')?'\r\n':'\n');
 const replacement=row.replacement.replaceAll('\n',original.text.includes('\r\n')?'\r\n':'\n');
 assert.equal(body.split(anchor).length,2,row.id+' exact method anchor');
 assert.equal(row.restoreSha256.toLowerCase(),hash(original.bytes));
 const candidate=['parser-array-host-reference.cpp','parser-iterator-reference.cpp'].map(file=>fs.readFileSync(path.join(__dirname,file),'utf8')).join('\n');
 assert.ok(candidate.includes('"'+row.nativeCase+'"_test'),row.id+' named native witness');
 return {...row,changed:original.text.slice(0,begin)+body.replace(anchor,replacement)+original.text.slice(end)};
});
if(process.argv.includes('--preflight')){console.log('Validated '+mutations.length+' method-scoped anchors, source hashes and native witnesses.');process.exit(0);}
fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
const gold=['parser-host-fault-plan.json','parser-array-host-reference.cpp','parser-iterator-reference.cpp'].map(file=>({file:path.join(__dirname,file),bytes:fs.readFileSync(path.join(__dirname,file))}));
function build(file,label){
 const batch=path.join(work,'build.cmd');
 // Ninja creates the response file consumed by the real archive command.
 // Replaying `ninja -t commands` alone would omit that transient dependency.
 fs.writeFileSync(batch,'@echo off\r\ncall "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat" >nul\r\nif errorlevel 1 exit /b %errorlevel%\r\ncd /d "'+path.join(root,'build/core')+'"\r\nninja -d keeprsp FoliaMathHostTests.exe\r\nexit /b %errorlevel%\r\n');
 const result=cp.spawnSync('cmd.exe',['/d','/c',batch],{cwd:root,encoding:'utf8',timeout:60000,maxBuffer:16*1024*1024});
 fs.writeFileSync(path.join(work,label+'-build.log'),(result.stdout||'')+(result.stderr||''));
 assert.ifError(result.error);assert.equal(result.status,0,label+' must compile and link');
}
function run(pattern,label){
 const result=cp.spawnSync(path.join(root,'build/core/FoliaMathHostTests.exe'),pattern?[pattern]:[],{encoding:'utf8',timeout:30000,maxBuffer:16*1024*1024});
 fs.writeFileSync(path.join(work,label+'-test.log'),(result.stdout||'')+(result.stderr||''));
 assert.ifError(result.error);return result;
}
const rows=[];
try{
 const baseline=run(null,'baseline');assert.equal(baseline.status,0,'Baseline must pass');
 assert.match(baseline.stdout,/[1-9]\d* asserts in [1-9]\d* tests/);
 for(const row of mutations){
  const original=originals.get(row.source);
  try{
   fs.writeFileSync(original.absolute,row.changed);build(row.source,row.id);
   const result=run('*'+row.nativeCase+'*',row.id);
   assert.equal(result.status,1,row.id+' must fail assertions rather than crash');
   assert.match(result.stdout,/FAILED|failed|FAIL/);
   rows.push({id:row.id,source:row.source,nativeCase:row.nativeCase,compiles:true,testExit:result.status,detected:true});
  }finally{
   fs.writeFileSync(original.absolute,original.bytes);
   assert.equal(hash(fs.readFileSync(original.absolute)),hash(original.bytes));
   build(row.source,row.id+'-restored');
  }
  assert.equal(run('*'+row.nativeCase+'*',row.id+'-restored').status,0,row.id+' restored witness');
  fs.writeFileSync(path.join(work,'progress.json'),JSON.stringify({completed:rows.length,total:mutations.length,rows},null,2)+'\n');
  console.log('Detected and restored '+row.id+' ('+rows.length+'/'+mutations.length+').');
 }
 assert.equal(run(null,'restored-full').status,0,'Restored full host suite');
 for(const {file,bytes} of gold)assert.ok(bytes.equals(fs.readFileSync(file)),file+' unchanged');
 fs.writeFileSync(path.join(work,'results.json'),JSON.stringify({schema:'folia.math.parser-host-native-faults.v1',baseline:plan.baselineSha256,rows,goldUnchanged:true,restoredFullExit:0},null,2)+'\n');
}finally{
 for(const {absolute,bytes} of originals.values())fs.writeFileSync(absolute,bytes);
 fs.closeSync(fd);fs.unlinkSync(lock);
}
