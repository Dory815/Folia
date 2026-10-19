'use strict';
// Strict acceptance of complete observations from the actual native pipeline.
// No normalization, numeric tolerance, omitted fields or case filtering.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const nativePath=process.argv[2];
if(!nativePath)throw Error('Usage: compare-common-layout-profile.cjs <native-json> [summary-json] [source-json]');
const sourcePath=path.resolve(root,process.argv[4]||'build/math-port/common-layout-profile-root-locked.json');
const summaryPath=path.resolve(root,process.argv[3]||'build/math-port/common-layout-profile-comparison.json');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const pinnedSourceSha256='9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740';
assert.equal(digest(sourcePath),pinnedSourceSha256,'Pinned whole Source observation changed');
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const nativeFile=path.resolve(root,nativePath);
const native=JSON.parse(fs.readFileSync(nativeFile,'utf8'));
const failures=[];
function firstDiff(want,got,at='$') {
  if(Object.is(want,got))return null;
  if(typeof want!==typeof got||want===null||got===null)
    return {path:at,source:want,native:got};
  if(Array.isArray(want)||Array.isArray(got)) {
    if(!Array.isArray(want)||!Array.isArray(got)||want.length!==got.length)
      return {path:at+'.length',source:Array.isArray(want)?want.length:typeof want,
        native:Array.isArray(got)?got.length:typeof got};
    for(let i=0;i<want.length;++i) {
      const diff=firstDiff(want[i],got[i],`${at}[${i}]`);
      if(diff)return diff;
    }
    return null;
  }
  if(typeof want==='object') {
    const sourceKeys=Object.keys(want).sort(),nativeKeys=Object.keys(got).sort();
    if(JSON.stringify(sourceKeys)!==JSON.stringify(nativeKeys))
      return {path:at+'.keys',source:sourceKeys,native:nativeKeys};
    for(const key of sourceKeys) {
      const diff=firstDiff(want[key],got[key],at+'.'+key);
      if(diff)return diff;
    }
    return null;
  }
  return {path:at,source:want,native:got};
}
assert.equal(source.records.length,75);
for(const key of ['schema','version','scopeHash','corpusSha256','configuredPackages']) {
  const diff=firstDiff(source[key],native[key],key);
  if(diff)failures.push(diff);
}
// Requested TeX options do not prove that its package handlers were installed.
// Source has all 38 constructors registered and no configuration warnings.
// Native reports these actual Context registry observations independently.
const registeredDiff=firstDiff(source.configuredPackages,native.registeredPackages,'registeredPackages');
if(registeredDiff)failures.push(registeredDiff);
const warningDiff=firstDiff([],native.configurationWarnings,'configurationWarnings');
if(warningDiff)failures.push(warningDiff);
const actual=Array.isArray(native.records)?native.records:[];
if(actual.length!==source.records.length)
  failures.push({path:'records.length',source:source.records.length,native:actual.length});
let matched=0;
for(let i=0;i<source.records.length;++i) {
  const expected=source.records[i];
  const diff=firstDiff(expected,actual[i],`records[${i}](${expected.name})`);
  if(diff)failures.push(diff);
  else ++matched;
}
const summary={source:sourcePath,sourceSha256:pinnedSourceSha256,native:nativeFile,
  nativeSha256:digest(nativeFile),total:75,matched,
  sourceSvgResults:source.records.filter(x=>x.svg!==null).length,
  sourceErrorResults:source.records.filter(x=>x.error!==null).length,
  nativeSvgResults:actual.filter(x=>x?.svg!=null).length,
  nativeErrorResults:actual.filter(x=>x?.error!=null).length,
  accepted:failures.length===0&&matched===75,failures};
fs.writeFileSync(summaryPath,JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({total:75,matched,accepted:summary.accepted,failures:failures.length,
  firstFailure:failures[0]||null,summary:summaryPath}));
process.exitCode=summary.accepted?0:1;
