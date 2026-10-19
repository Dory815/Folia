'use strict';
// Drives the native observer with the exact pinned 75-case input
// sequence. The executable emits only observations; this runner does not
// synthesize result fields or layout data. Source provenance fields are copied
// from the locked oracle, while configuredPackages reports the native session's
// actual current package set.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const {spawnSync}=require('node:child_process');
const root=path.resolve(__dirname,'../..');
const executable=process.argv[2];
const output=path.resolve(root,process.argv[3]||'build/math-port/common-layout-native-current.json');
if(!executable)throw Error('Usage: run-common-layout-native.cjs <native-executable> [output-json]');
const sourcePath=path.join(root,'build/math-port/common-layout-profile-root-locked.json');
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const pinned='9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740';
if(digest(sourcePath)!==pinned)throw Error('Pinned Source oracle provenance changed');
if(source.records.length!==75)throw Error('Expected the locked 75-case profile');
const b64=value=>Buffer.from(value,'utf8').toString('base64');
const input='@packages\t'+source.configuredPackages.map(b64).join('\t')+'\n'+source.records.map(row=>[
  b64(row.name),b64(row.tex),row.display?'1':'0',String(row.width),String(row.em),String(row.ex)
].join('\t')).join('\n')+'\n';
const provenance={executable:path.resolve(executable),executableSha256:digest(executable),
  sourceSha256:pinned,startedUtc:new Date().toISOString()};
const result=spawnSync(executable,[],{cwd:root,input,encoding:'utf8',maxBuffer:256*1024*1024,
  windowsHide:true,timeout:120000});
Object.assign(provenance,{finishedUtc:new Date().toISOString(),pid:result.pid??null,
  exitCode:result.status,signal:result.signal,error:result.error?.message??null});
fs.mkdirSync(path.dirname(output),{recursive:true});
fs.writeFileSync(output+'.provenance.json',JSON.stringify(provenance,null,2)+'\n');
fs.writeFileSync(output+'.stdout',result.stdout||'');
fs.writeFileSync(output+'.stderr',result.stderr||'');
if(result.status!==0||result.error){
  process.stderr.write(JSON.stringify(provenance)+'\n'+(result.stderr||''));
  process.exitCode=1;
  return;
}
const stdout=result.stdout;
const observed=JSON.parse(stdout);
const records=observed.records;
if(!Array.isArray(observed.configuredPackages)||
   observed.configuredPackages.some(value=>typeof value!=='string'))
  throw Error('Native executable did not report its actual TeX options.packages');
for(const field of ['registeredPackages','configurationWarnings'])
  if(!Array.isArray(observed[field])||observed[field].some(value=>typeof value!=='string'))
    throw Error(`Native executable did not report actual ${field}`);
if(!Array.isArray(records)||records.length!==75)throw Error('Native executable did not emit 75 actual records');
for(let i=0;i<records.length;++i){
  if(records[i].name!==source.records[i].name||records[i].tex!==source.records[i].tex)
    throw Error(`Native record ${i} does not correspond to the supplied input`);
  // This is a fixture label from the locked corpus, not a claim that the native
  // runtime loaded or executed those package handlers.
  records[i].intendedFeatures=source.records[i].intendedFeatures||[];
}
const native={schema:source.schema,version:source.version,scopeHash:source.scopeHash,
  sourceLocks:source.sourceLocks,cjsLocks:source.cjsLocks,dependencies:source.dependencies,
  corpusSha256:source.corpusSha256,configuredPackages:observed.configuredPackages,
  registeredPackages:observed.registeredPackages,
  configurationWarnings:observed.configurationWarnings,records};
fs.mkdirSync(path.dirname(output),{recursive:true});
fs.writeFileSync(output,JSON.stringify(native,null,2)+'\n');
process.stdout.write(JSON.stringify({output,records:records.length,
  provenance,
  actualNativeConfiguredPackages:native.configuredPackages,
  actualRegisteredPackages:native.registeredPackages,
  actualConfigurationWarnings:native.configurationWarnings,
  successfulLayouts:records.filter(row=>row.layoutRoot!==null).length,
  errors:records.filter(row=>row.error!==null).length})+'\n');
