'use strict';
// Strict, field-by-field comparison of pinned Source and native package records.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const nativeArg=process.argv[2];if(!nativeArg)throw Error('Usage: compare-tex-next-six.cjs <native-json> [summary-json] [source-json]');
const sourceFile=path.resolve(root,process.argv[4]||'build/math-port/tex-next-six-original.json');
const summaryFile=path.resolve(root,process.argv[3]||'build/math-port/tex-next-six-comparison.json');
const scope=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/scope.json'),'utf8'));
const source=JSON.parse(fs.readFileSync(sourceFile,'utf8')),nativeFile=path.resolve(root,nativeArg),native=JSON.parse(fs.readFileSync(nativeFile,'utf8'));
assert.equal(source.scopeSha256,digest(path.join(root,'docs/math-port/scope.json')),'Source oracle scope hash differs');
function firstDiff(a,b,path='$'){
 if(Object.is(a,b))return null;
 if(typeof a!==typeof b||a===null||b===null)return{path,source:a,native:b};
 if(Array.isArray(a)||Array.isArray(b)){
  if(!Array.isArray(a)||!Array.isArray(b)||a.length!==b.length)return{path:path+'.length',source:Array.isArray(a)?a.length:typeof a,native:Array.isArray(b)?b.length:typeof b};
  for(let i=0;i<a.length;i++){const d=firstDiff(a[i],b[i],`${path}[${i}]`);if(d)return d;}return null;
 }
 if(typeof a==='object'){
  const ak=Object.keys(a).sort(),bk=Object.keys(b).sort();if(JSON.stringify(ak)!==JSON.stringify(bk))return{path:path+'.keys',source:ak,native:bk};
  for(const k of ak){const d=firstDiff(a[k],b[k],`${path}.${k}`);if(d)return d;}return null;
 }
 return{path,source:a,native:b};
}
const expected=source.cases,actual=native.cases||[],failures=[];
const sourceKeys=Object.keys(source).sort(),nativeKeys=Object.keys(native).sort();
if(JSON.stringify(sourceKeys)!==JSON.stringify(nativeKeys))failures.push({path:'$.[keys]',source:sourceKeys,native:nativeKeys});
for(const key of ['schema','scopeSha256']){
 const d=firstDiff(source[key],native[key],`$.${key}`);if(d)failures.push(d);
}
if(actual.length!==expected.length)failures.push({path:'cases.length',source:expected.length,native:actual.length});
let matched=0;for(let i=0;i<expected.length;i++){
 const at=`cases[${i}](${expected[i].name})`,d=firstDiff(expected[i],actual[i],at);if(d)failures.push(d);else matched++;
 // The canonical Textcomp jax is shared by packageData and TeX. Its own-key
 // order determines where the persistent WeakSet marks the shared handlers
 // alias as a cycle, so compare this order directly instead of treating keys
 // as an unordered record here.
 const sourceJax=expected[i].packageData?.entries?.[0]?.[1]?.jax;
 const nativeJax=actual[i]?.packageData?.entries?.[0]?.[1]?.jax;
 if(sourceJax&&nativeJax&&JSON.stringify(Object.keys(sourceJax))!==JSON.stringify(Object.keys(nativeJax)))
  failures.push({path:`${at}.packageData.entries[0][1].jax.[keyOrder]`,source:Object.keys(sourceJax),native:Object.keys(nativeJax)});
}
const summary={source:sourceFile,sourceSha256:digest(sourceFile),sourceLocks:scope.files.length,native:nativeFile,nativeSha256:digest(nativeFile),total:expected.length,matched,accepted:matched===expected.length&&failures.length===0,failures};
fs.mkdirSync(path.dirname(summaryFile),{recursive:true});fs.writeFileSync(summaryFile,JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({total:expected.length,matched,accepted:summary.accepted,firstFailure:failures[0]||null,summary:summaryFile}));process.exitCode=summary.accepted?0:1;
