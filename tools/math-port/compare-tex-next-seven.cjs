'use strict';
// Exact comparison of pinned Source output against native parser output.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const nativePath=process.argv[2];
if(!nativePath)throw Error('Usage: compare-tex-next-seven.cjs <native-json> [summary-json] [source-json]');
const sourcePath=path.resolve(root,process.argv[4]||'build/math-port/tex-next-seven-original.json');
const summaryPath=path.resolve(root,process.argv[3]||'build/math-port/tex-next-seven-comparison.json');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const scope=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/scope.json'),'utf8'));
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
assert.equal(source.cases.length,23,'Source oracle case count changed');
assert.equal(source.scopeSha256,digest(path.join(root,'docs/math-port/scope.json')),'Source scope changed');
const nativeFile=path.resolve(root,nativePath),native=JSON.parse(fs.readFileSync(nativeFile,'utf8'));
function diff(a,b,at='$'){
 if(Object.is(a,b))return null;
 if(typeof a!==typeof b||a===null||b===null)return{path:at,source:a,native:b};
 if(Array.isArray(a)||Array.isArray(b)){
  if(!Array.isArray(a)||!Array.isArray(b)||a.length!==b.length)return{path:at+'.length',source:Array.isArray(a)?a.length:typeof a,native:Array.isArray(b)?b.length:typeof b};
  for(let i=0;i<a.length;i++){const d=diff(a[i],b[i],`${at}[${i}]`);if(d)return d;}return null;
 }
 if(typeof a==='object'){
  const ak=Object.keys(a).sort(),bk=Object.keys(b).sort();
  if(JSON.stringify(ak)!==JSON.stringify(bk))return{path:at+'.keys',source:ak,native:bk};
  for(const k of ak){const d=diff(a[k],b[k],`${at}.${k}`);if(d)return d;}return null;
 }
 return{path:at,source:a,native:b};
}
const expected=source.cases,actual=native.cases||[],failures=[];
if(actual.length!==expected.length)failures.push({path:'cases.length',source:expected.length,native:actual.length});
let matched=0;
for(let i=0;i<expected.length;i++){
 const d=diff(expected[i],actual[i],`cases[${i}](${expected[i].name})`);
 if(d)failures.push(d);else matched++;
}
const summary={source:sourcePath,sourceSha256:digest(sourcePath),sourceLocks:scope.files.length,native:nativeFile,nativeSha256:digest(nativeFile),total:expected.length,matched,accepted:matched===expected.length&&failures.length===0,failures};
fs.mkdirSync(path.dirname(summaryPath),{recursive:true});fs.writeFileSync(summaryPath,JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({total:expected.length,matched,accepted:summary.accepted,firstFailure:failures[0]||null,summary:summaryPath}));
process.exitCode=summary.accepted?0:1;
