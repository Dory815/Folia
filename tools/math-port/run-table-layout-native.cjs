'use strict';
// Sends the focused table inputs to the existing real native document runner.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const {spawnSync}=require('node:child_process');
const root=path.resolve(__dirname,'../..');
const executable=process.argv[2];
if(!executable)throw Error('Usage: run-table-layout-native.cjs <native-executable> [output-json]');
const casesPath=path.join(__dirname,'table-layout-cases.json');
const cases=JSON.parse(fs.readFileSync(casesPath,'utf8')).cases;
const scope=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/scope.json'),'utf8'));
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const b64=value=>Buffer.from(value,'utf8').toString('base64');
const input='@packages\t'+scope.profile.tex_packages.map(b64).join('\t')+'\n'+cases.map(item=>[
  b64(item.name),b64(item.tex),item.display?'1':'0',String(item.width??1280),
  String(item.em??16),String(item.ex??8)
].join('\t')).join('\n')+'\n';
const provenance={executable:path.resolve(executable),executableSha256:digest(executable),
  corpusSha256:digest(casesPath),startedUtc:new Date().toISOString()};
const result=spawnSync(executable,[],{cwd:root,input,encoding:'utf8',maxBuffer:256*1024*1024,
  windowsHide:true,timeout:120000});
Object.assign(provenance,{finishedUtc:new Date().toISOString(),pid:result.pid??null,
  exitCode:result.status,signal:result.signal,error:result.error?.message??null});
const output=path.resolve(root,process.argv[3]||'build/math-port/table-layout-native-current.json');
fs.mkdirSync(path.dirname(output),{recursive:true});
fs.writeFileSync(output+'.provenance.json',JSON.stringify(provenance,null,2)+'\n');
fs.writeFileSync(output+'.stderr',result.stderr||'');
if(result.status!==0||result.error){
  process.stderr.write(JSON.stringify(provenance)+'\n'+(result.stderr||''));
  process.exitCode=1;return;
}
const native=JSON.parse(result.stdout);
if(!Array.isArray(native.records)||native.records.length!==cases.length)
  throw Error(`Expected ${cases.length} native records`);
for(let i=0;i<cases.length;i++){
  if(native.records[i].name!==cases[i].name||native.records[i].tex!==cases[i].tex)
    throw Error(`Native row ${i} does not match the supplied case`);
}
native.tableCorpusSha256=provenance.corpusSha256;
fs.writeFileSync(output,JSON.stringify(native,null,2)+'\n');
process.stdout.write(JSON.stringify({output,caseCount:cases.length,provenance})+'\n');
