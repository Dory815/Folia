'use strict';
// Strictly compares actual Source and native Menclose method traces. Object
// identities are compared by aliasing relationships within each case because
// the Context and JavaScript heaps assign different numeric identifiers.
const fs=require('node:fs'),assert=require('node:assert/strict'),path=require('node:path');
const root=path.resolve(__dirname,'../..');
const source=JSON.parse(fs.readFileSync(process.argv[2]||path.join(root,'build/math-port/menclose-source-observation.json')));
const native=JSON.parse(fs.readFileSync(process.argv[3]||path.join(root,'build/math-port/menclose-native-observation.json')));
assert.equal(source.schema,1);assert.equal(native.schema,1);
assert.equal(source.cases.length,30);assert.equal(native.cases.length,30);
assert.deepEqual(native.htmlContract,source.htmlContract,'CommonOutputJax.html method metadata and live adaptor.node behavior');
assert.deepEqual(native.packageList,source.packageList,'actual Input Jax package list');
assert.deepEqual(native.notationNames,source.notationNames,'ordered Source notation Map');
function normalizedEvents(events){
  const ids=new Map();let next=1;
  const identity=value=>{
    const key=String(value);
    if(!ids.has(key))ids.set(key,next++);
    return ids.get(key);
  };
  const value=v=>{
    if(v&&typeof v==='object'){
      if(v.type==='string')return v.value;
      if(v.type==='array')return {type:'array',id:identity(v.id),length:v.length,
        items:(v.items||[]).map(value)};
      if(v.type==='object'||v.type==='function'){
        const result={type:v.type,id:identity(v.id)};
        if(v.fields)result.fields=Object.fromEntries(Object.entries(v.fields).map(([k,x])=>[k,value(x)]));
        if(v.circular)result.circular=true;
        return result;
      }
      if(v.type==='throws')return {type:'throws',name:v.name,message:v.message};
      if(v.type==='hole')return {type:'hole'};
    }
    return v;
  };
  return events.map(event=>({method:event.method,receiver:identity(event.receiver),
    args:event.args.map(value),result:value(event.result)}));
}
const differences=[];
for(let i=0;i<source.cases.length;i++){
  const expected=source.cases[i],actual=native.cases[i];
  const mismatch={name:expected.name};
  if(actual.name!==expected.name)mismatch.caseName={source:expected.name,native:actual.name};
  if(JSON.stringify(actual.error)!==JSON.stringify(expected.error))
    mismatch.error={source:expected.error,native:actual.error};
  if(!expected.error&&actual.sceneNodeCount<=0)mismatch.completedScene='missing native Scene nodes';
  const sourceEvents=normalizedEvents(expected.events),nativeEvents=normalizedEvents(actual.events);
  if(JSON.stringify(sourceEvents)!==JSON.stringify(nativeEvents)){
    let index=0;
    while(index<sourceEvents.length&&index<nativeEvents.length&&
          JSON.stringify(sourceEvents[index])===JSON.stringify(nativeEvents[index]))index++;
    mismatch.events={sourceCount:sourceEvents.length,nativeCount:nativeEvents.length,
      firstMismatchIndex:index,source:sourceEvents[index]??null,native:nativeEvents[index]??null};
  }
  if(Object.keys(mismatch).length>1)differences.push(mismatch);
}
if(native.contextReleased!==true)differences.push({lifetime:{contextReleased:native.contextReleased}});
if(native.detachedSceneNodeCount<=0)differences.push({lifetime:{detachedSceneNodeCount:native.detachedSceneNodeCount}});
if(differences.length){
  const reportPath=path.join(root,'build/math-port/menclose-comparison-report.json');
  fs.writeFileSync(reportPath,JSON.stringify({casesCompared:source.cases.length,differences},null,2)+'\n');
  process.stderr.write(`Menclose differences in ${differences.length} cases; details: ${reportPath}\n`);
  for(const difference of differences.slice(0,8))process.stderr.write(JSON.stringify(difference)+'\n');
  process.exitCode=1;
}else process.stdout.write(`Menclose observations match for ${source.cases.length} actual conversion cases; Scene lifetime checks passed.\n`);
