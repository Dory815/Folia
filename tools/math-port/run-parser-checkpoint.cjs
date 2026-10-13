'use strict';
// Root-owned orchestration: observe real executables, preserve CLI contracts,
// compare every emitted case against independently frozen original results.
const fs=require('node:fs');
const path=require('node:path');
const assert=require('node:assert/strict');
const {isDeepStrictEqual}=require('node:util');
const {spawnSync}=require('node:child_process');
const crypto=require('node:crypto');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const root=path.resolve(__dirname,'../..');
const label=process.argv[2];
assert.match(label||'',/^[a-zA-Z0-9-]+$/,'supply a checkpoint label');
const destination=path.join(root,'build/math-port',`parser-checkpoint-${label}`);
fs.mkdirSync(destination,{recursive:true});
const specs=[
  {name:'base',exe:'FoliaMathBaseMethodsDump',file:true,source:'build/math-port/base-methods-original.json',fields:['packages'],base:true},
  {name:'frontend',exe:'FoliaMathFrontendDump',file:true,source:'docs/math-port/evidence/textmacros-native-tree-source.json',tree:true},
  {name:'bold',exe:'FoliaMathBoldsymbolConfigmacrosDump',file:false,source:'tools/math-port/boldsymbol-configmacros-reference/original.json',fields:['name','result','error','maps','packageData']},
  {name:'ams',exe:'FoliaMathAmsNewcommandDump',file:true,source:'tools/math-port/ams-newcommand-reference/ams-newcommand-source.json',fields:['result','error','dynamicMaps','packageData']},
  {name:'ams-boundary',exe:'FoliaMathAmsNewcommandBoundaryDump',file:true,source:'tools/math-port/ams-newcommand-reference/ams-newcommand-boundary-source.json',fields:['result','error','dynamicMaps','packageData']},
];
const results=[];
// Explicit selection permits comparing only freshly linked programs after a
// partially failed build. The summary never implies the omitted targets ran.
const requested=process.argv.slice(3);
assert.equal(new Set(requested).size,requested.length,'duplicate selection');
for(const name of requested)assert.ok(specs.some(spec=>spec.name===name),`unknown selection ${name}`);
for(const spec of specs.filter(spec=>!requested.length||requested.includes(spec.name))){
  const executable=path.join(root,'build/core',spec.exe+'.exe');
  const nativePath=path.join(destination,spec.name+'.json');
  const run=spawnSync(executable,spec.file?[nativePath]:[],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});
  fs.writeFileSync(path.join(destination,spec.name+'.stderr'),run.stderr||'');
  if(!spec.file)fs.writeFileSync(nativePath,run.stdout||'');
  else fs.writeFileSync(path.join(destination,spec.name+'.stdout'),run.stdout||'');
  const record={name:spec.name,executable,executableSha256:digest(executable),
    exitCode:run.status,source:spec.source,sourceSha256:digest(path.join(root,spec.source))};
  if(run.error||run.status!==0){record.executionError=run.error?.message||run.stderr;results.push(record);continue;}
  const source=JSON.parse(fs.readFileSync(path.join(root,spec.source),'utf8'));
  const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
  if(spec.tree)assert.equal(native.schema,source.schema);
  const rows=Array.isArray(source.cases)?source.cases:Object.entries(source.cases).map(([name,value])=>({name,value}));
  const actuals=Array.isArray(native.cases)?Object.fromEntries(native.cases.map(row=>[row.name,row])):native.cases;
  assert.deepStrictEqual(Object.keys(actuals).sort(),rows.map(row=>row.name).sort(),`${spec.name} coverage differs`);
  if(spec.base)assert.deepStrictEqual(native.methodNames,source.methodNames);
  const failures=[];
  for(const row of rows){
    const actual=actuals[row.name];
    let fields=spec.fields;
    if(spec.tree)fields=['value'];
    if(spec.base)fields=['packages',...(row.error?['error']:['error','tree','parser'])];
    const differences=fields.filter(field=>{
      if(spec.tree)return !isDeepStrictEqual(actual,row.value);
      if(spec.base&&(field==='tree'||field==='parser'))return !isDeepStrictEqual(actual.result?.[field],row.result?.[field]);
      return !isDeepStrictEqual(actual[field],row[field]);
    });
    if(differences.length)failures.push({name:row.name,fields:differences,error:actual.error||null});
  }
  Object.assign(record,{total:rows.length,matched:rows.length-failures.length,failures});
  results.push(record);
}
fs.writeFileSync(path.join(destination,'summary.json'),JSON.stringify(results,null,2)+'\n');
process.stdout.write(JSON.stringify(results)+'\n');
if(results.some(row=>row.executionError||row.failures.length))process.exitCode=1;
