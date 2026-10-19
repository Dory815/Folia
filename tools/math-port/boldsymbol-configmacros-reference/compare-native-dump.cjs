'use strict';
const fs=require('node:fs'),assert=require('node:assert/strict');
const gold=JSON.parse(fs.readFileSync(process.argv[2]||'tools/math-port/boldsymbol-configmacros-reference/original.json','utf8'));
const native=JSON.parse(fs.readFileSync(process.argv[3]||'build/math-port/boldsymbol-configmacros-native.json','utf8'));
if(native.bootError){console.error(`Native setup failed before cases: ${JSON.stringify(native.bootError)}`);process.exit(1);}
const rows=new Map(native.cases.map(x=>[x.name,x]));
assert.equal(rows.size,gold.cases.length,'native case count/unique names');
for(const expected of gold.cases){
 const actual=rows.get(expected.name);assert.ok(actual,`missing native case ${expected.name}`);
 try{
  assert.deepStrictEqual(actual.result,expected.result,`${expected.name} full MML result`);
  assert.deepStrictEqual(actual.error,expected.error,`${expected.name} error`);
  assert.deepStrictEqual(actual.maps,expected.maps,`${expected.name} global and active token maps`);
  assert.deepStrictEqual(actual.packageData,expected.packageData,`${expected.name} ParseOptions.packageData`);
 }catch(error){console.error(`Mismatch: ${expected.name}\n${error.message}`);process.exitCode=1;}
}
if(!process.exitCode)console.log(`matched ${gold.cases.length} locked full-MML results and errors`);
