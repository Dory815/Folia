'use strict';
// Strict whole-record comparison for the locked TagFormat and SetOptions packets.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../../..'),digest=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const nativePath=process.argv[2];if(!nativePath)throw new Error('Usage: node tools/math-port/tag-setoptions-reference/compare.cjs <native-json> [source-json] [summary-json]');
const sourcePath=path.resolve(root,process.argv[3]||'build/math-port/tag-setoptions-original.json');
const summaryPath=path.resolve(root,process.argv[4]||'build/math-port/tag-setoptions-comparison.json');
const lockPath=path.join(__dirname,'source-lock.json'),lock=JSON.parse(fs.readFileSync(lockPath,'utf8'));
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8')),native=JSON.parse(fs.readFileSync(path.resolve(root,nativePath),'utf8'));
const expectedOriginalSha256='03e4f1e640234845e2b9854744ea3a8f2c336f66b46649914f5a60692aa744f5';
assert.equal(digest(path.join(root,'docs/math-port/scope.json')),lock.scopeSha256,'scope.json differs from the locked Source profile');
assert.equal(source.scopeSha256,lock.scopeSha256,'Source observations use another scope profile');
assert.equal(source.sourceLockSha256,digest(lockPath),'Source observation lock hash differs');
assert.equal(native.sourceLockSha256,digest(lockPath),'native observations use another Source lock');
assert.equal(digest(sourcePath),expectedOriginalSha256,'full Source observation output differs from its pinned hash');
if(lock.oracleOutputSha256)assert.equal(digest(sourcePath),lock.oracleOutputSha256,'Source observation output hash differs from its lock');
for(const entry of lock.sources){assert.equal(digest(path.join(root,'.reference/MathJax-src',entry.file)),entry.sha256,`${entry.file} differs from its Source lock`);if(entry.cjsFile)assert.equal(digest(path.join(root,entry.cjsFile)),entry.cjsSha256,`${entry.cjsFile} differs from executable Source lock`);}
function difference(expected,actual,at='$'){
 if(Object.is(expected,actual))return null;
 if(typeof expected!==typeof actual||expected===null||actual===null)return{path:at,source:expected,native:actual};
 if(Array.isArray(expected)||Array.isArray(actual)){
  if(!Array.isArray(expected)||!Array.isArray(actual)||expected.length!==actual.length)return{path:`${at}.length`,source:Array.isArray(expected)?expected.length:typeof expected,native:Array.isArray(actual)?actual.length:typeof actual};
  for(let i=0;i<expected.length;i++){const found=difference(expected[i],actual[i],`${at}[${i}]`);if(found)return found;}return null;
 }
 if(typeof expected==='object'){
  const ek=Object.keys(expected),ak=Object.keys(actual);if(JSON.stringify(ek)!==JSON.stringify(ak))return{path:`${at}.keys`,source:ek,native:ak};
  for(const key of ek){const found=difference(expected[key],actual[key],`${at}.${key}`);if(found)return found;}return null;
 }
 return{path:at,source:expected,native:actual};
}
const expected=source.cases,actual=Array.isArray(native.cases)?native.cases:[],failures=[];
const wholeOutputDifference=difference(source,native,'$');if(wholeOutputDifference)failures.push(wholeOutputDifference);
let matched=0;for(let i=0;i<expected.length;i++)if(!difference(expected[i],actual[i],`cases[${i}](${expected[i].name})`))matched++;
const summary={source:path.relative(root,sourcePath),sourceSha256:digest(sourcePath),sourceLockSha256:digest(lockPath),native:path.relative(root,path.resolve(root,nativePath)),nativeSha256:digest(path.resolve(root,nativePath)),sourceFiles:lock.sources.length,total:expected.length,matched,accepted:matched===expected.length&&failures.length===0,failures};
fs.mkdirSync(path.dirname(summaryPath),{recursive:true});fs.writeFileSync(summaryPath,JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({total:summary.total,matched,accepted:summary.accepted,firstFailure:failures[0]||null,summary:path.relative(root,summaryPath)}));
process.exitCode=summary.accepted?0:1;
