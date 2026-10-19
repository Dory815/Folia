'use strict';
// Exercises comparer success/failure and SVG-byte hashing with synthetic data.
// This is a utility test only; it does not assert native parity.
const fs=require('node:fs');
const path=require('node:path');
const cp=require('node:child_process');
const assert=require('node:assert/strict');

const root=path.resolve(__dirname,'../../../..');
const sourcePath=path.join(__dirname,'bussproofs-public-render-source.json');
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const comparer=path.join(__dirname,'compare-bussproofs-public-render.cjs');
const scratch=path.join(root,'build/math-port/bussproofs-comparer-tool-test');
fs.mkdirSync(scratch,{recursive:true});
const input=path.join(scratch,'synthetic-native.json');
const output=path.join(scratch,'comparison.json');
const invoke=()=>cp.spawnSync(process.execPath,[comparer,sourcePath,input,output],
  {cwd:root,encoding:'utf8'});
const native={schema:1,profile:'mathjax-full',records:source.records.map(record=>({...record,sceneNodes:1}))};
fs.writeFileSync(input,JSON.stringify(native));
const positive=invoke();
assert.equal(positive.status,0,positive.stderr||positive.stdout);
assert.equal(JSON.parse(fs.readFileSync(output,'utf8')).matched,3);

native.records[1].svg+='<!-- mismatch -->';
fs.writeFileSync(input,JSON.stringify(native));
const negative=invoke();
assert.equal(negative.status,1,negative.stderr||negative.stdout);
const failed=JSON.parse(fs.readFileSync(output,'utf8'));
assert.equal(failed.matched,2);
assert.equal(failed.failures.length,1);
assert.equal(failed.failures[0].name,'ordinary-after-proof');
assert.match(failed.failures[0].sourceSvgSha256,/^[0-9a-f]{64}$/);
assert.match(failed.failures[0].nativeSvgSha256,/^[0-9a-f]{64}$/);
console.log(JSON.stringify({positiveMatched:3,negativeMatched:2,negativeFailure:failed.failures[0].name,
  parityClaim:false}));
