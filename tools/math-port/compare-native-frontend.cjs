'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),{execFileSync}=require('node:child_process');
const root=path.resolve(__dirname,'../..');
execFileSync(process.execPath,[path.join(root,'tools/math-port/textmacros-native-tree-oracle.cjs'),'--check'],{stdio:'inherit'});
const sourcePath=path.join(root,'docs/math-port/evidence/textmacros-native-tree-source.json');
const nativePath=path.resolve(process.argv[2]||path.join(root,'build/math-port/native-frontend.json'));
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
assert.equal(native.schema,source.schema,'native dump schema differs');
const stable=value=>Array.isArray(value)?value.map(stable):value&&typeof value==='object'
  ?Object.fromEntries(Object.keys(value).sort().map(key=>[key,stable(value[key])])):value;
const expectedKeys=Object.keys(source.cases).sort(),actualKeys=Object.keys(native.cases).sort();
assert.deepStrictEqual(actualKeys,expectedKeys,'native case coverage differs from locked Source');
function firstDifference(expected,actual,at='$'){
  if(Object.is(expected,actual))return null;
  if(!expected||!actual||typeof expected!=='object'||typeof actual!=='object')return{at,expected,actual};
  const ek=Object.keys(expected).sort(),ak=Object.keys(actual).sort();
  if(JSON.stringify(ek)!==JSON.stringify(ak))return{at,expectedKeys:ek,actualKeys:ak};
  for(const key of ek){const mismatch=firstDifference(expected[key],actual[key],`${at}.${key}`);if(mismatch)return mismatch;}
  return null;
}
for(const key of expectedKeys){
  const mismatch=firstDifference(stable(source.cases[key]),stable(native.cases[key]));
  if(mismatch){
    process.stderr.write(`${key}: ${JSON.stringify(mismatch,null,2)}\n`);
    process.exitCode=1;break;
  }
}
if(!process.exitCode)console.log(JSON.stringify({matched:expectedKeys.length,source:'pinned MathJax CJS',native:nativePath}));
