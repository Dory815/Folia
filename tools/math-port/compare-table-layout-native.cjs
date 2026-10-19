'use strict';
// Exact field comparison; no field omission, normalization, or numeric tolerance.
const fs=require('node:fs');
const path=require('node:path');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const sourcePath=path.resolve(root,process.argv[2]||'build/math-port/table-layout-source.json');
const nativePath=path.resolve(root,process.argv[3]||'build/math-port/table-layout-native-current.json');
const summaryPath=path.resolve(root,process.argv[4]||'build/math-port/table-layout-comparison.json');
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
const failures=[];
function firstDiff(want,got,at='$'){
  if(Object.is(want,got))return null;
  if(typeof want!==typeof got||want===null||got===null)
    return {path:at,source:want,native:got};
  if(Array.isArray(want)||Array.isArray(got)){
    if(!Array.isArray(want)||!Array.isArray(got)||want.length!==got.length)
      return {path:at+'.length',source:Array.isArray(want)?want.length:typeof want,
        native:Array.isArray(got)?got.length:typeof got};
    for(let i=0;i<want.length;i++){
      const diff=firstDiff(want[i],got[i],`${at}[${i}]`);if(diff)return diff;
    }
    return null;
  }
  if(typeof want==='object'){
    const a=Object.keys(want).sort(),b=Object.keys(got).sort();
    if(JSON.stringify(a)!==JSON.stringify(b))return {path:at+'.keys',source:a,native:b};
    for(const key of a){const diff=firstDiff(want[key],got[key],at+'.'+key);if(diff)return diff;}
    return null;
  }
  return {path:at,source:want,native:got};
}
assert.equal(source.schema,1);
assert.equal(source.corpusSha256,native.tableCorpusSha256);
const a=source.records||[],b=native.records||[];
if(a.length!==b.length)failures.push({path:'records.length',source:a.length,native:b.length});
let matched=0;
for(let i=0;i<a.length;i++){
  const diff=firstDiff(a[i],b[i],`records[${i}](${a[i].name})`);
  if(diff)failures.push(diff);else matched++;
}
const summary={source:sourcePath,native:nativePath,total:a.length,matched,
  accepted:failures.length===0&&matched===a.length,failures};
fs.writeFileSync(summaryPath,JSON.stringify(summary,null,2)+'\n');
process.stdout.write(JSON.stringify({total:a.length,matched,accepted:summary.accepted,
  failures:failures.length,firstFailure:failures[0]||null,summary:summaryPath})+'\n');
process.exitCode=summary.accepted?0:1;
