'use strict';
const fs=require('node:fs'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const source=JSON.parse(fs.readFileSync(process.argv[2])),native=JSON.parse(fs.readFileSync(process.argv[3]));
assert.equal(crypto.createHash('sha256').update(fs.readFileSync(process.argv[2])).digest('hex'),
 '4fe64e87ccaf386e19f67defd1d940d5332b247151d4d0f41b90274a500f443c');
const casesPath='tools/math-port/math-api-cases.json';
assert.equal(source.corpusSha256,crypto.createHash('sha256').update(fs.readFileSync(casesPath)).digest('hex'));
assert.equal(source.schema,1);assert.equal(native.schema,1);
assert.equal(source.records.length,9);assert.equal(native.records.length,9);
let matched=0;const failures=[];
for(let i=0;i<source.records.length;++i){
 try{assert.deepStrictEqual(native.records[i],source.records[i]);++matched;}
 catch(error){failures.push({name:source.records[i].name,message:error.message});}
}
assert.equal(native.detachedScenes,native.records.filter(x=>x.svg!==null).length);
assert.ok(native.detachedSceneNodes>0);
const summary={total:9,matched,accepted:matched===9,failures};
fs.writeFileSync(process.argv[4]||'build/math-port/math-api-comparison.json',JSON.stringify(summary,null,2)+'\n');
console.log(JSON.stringify({total:9,matched,accepted:summary.accepted}));process.exitCode=summary.accepted?0:1;
