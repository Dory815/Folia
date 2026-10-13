'use strict';
// Root-owned independent whole-record comparison of actual Source/native hosts.
const fs=require('node:fs');
const path=require('node:path');
const assert=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const root=path.resolve(__dirname,'../..');
const sourcePath=path.resolve(process.argv[2]||path.join(root,'build/math-port/output-jax-original.json'));
const nativePath=path.resolve(process.argv[3]||path.join(root,'build/math-port/output-jax-native.json'));
const source=JSON.parse(fs.readFileSync(sourcePath));
const fresh=JSON.parse(execFileSync(process.execPath,
  [path.join(root,'tools/math-port/output-jax-original-oracle.cjs')],{encoding:'utf8'}));
assert.deepStrictEqual(fresh,source,'original behavior/source hashes differ from frozen baseline');
assert.equal(source.records.length,14,'unexpected original fixture coverage');
const native=JSON.parse(fs.readFileSync(nativePath));
assert.deepStrictEqual(Object.keys(native).sort(),Object.keys(source).sort(),'native top-level schema differs');
for(const key of ['version','sourceFile','sourceHash','cjsHash'])
  assert.deepStrictEqual(native[key],source[key],`native ${key} differs`);
assert.deepStrictEqual(native.records.map(row=>row.name),source.records.map(row=>row.name),
  'native row coverage/order differs');
const failures=[];
for(let i=0;i<source.records.length;i++){
  try{assert.deepStrictEqual(native.records[i],source.records[i],source.records[i].name);}
  catch(error){
    failures.push(source.records[i].name);
    process.stderr.write(error.message.split('\n').slice(0,26).join('\n')+'\n');
  }
}
process.stdout.write(JSON.stringify({total:14,matched:14-failures.length,failures})+'\n');
if(failures.length)process.exitCode=1;
