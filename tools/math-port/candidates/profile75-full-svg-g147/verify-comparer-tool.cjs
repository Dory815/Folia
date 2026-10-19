'use strict';
// Synthetic records test only comparer success/error paths; this is not a
// native-render parity result.
const fs=require('node:fs');
const path=require('node:path');
const cp=require('node:child_process');
const assert=require('node:assert/strict');

const root=path.resolve(__dirname,'../../../..');
const sourcePath=path.join(__dirname,'profile75-full-svg-source.json');
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const comparer=path.join(__dirname,'compare-profile75-full-svg.cjs');
const scratch=path.join(root,'build/math-port/profile75-full-svg-comparer-test');
fs.mkdirSync(scratch,{recursive:true});
const nativePath=path.join(scratch,'synthetic-native.json');
const comparisonPath=path.join(scratch,'comparison.json');
const invoke=()=>cp.spawnSync(process.execPath,[comparer,sourcePath,nativePath,comparisonPath],
  {cwd:root,encoding:'utf8',maxBuffer:512*1024*1024});
const native={schema:1,renderer:'folia.math::Renderer',groups:source.groups,
  corpusSha256:source.source.corpusSha256,caseInventorySha256:source.source.caseInventorySha256,
  records:source.records.map(row=>({...row,sceneNodes:row.svg===null?0:1}))};
fs.writeFileSync(nativePath,JSON.stringify(native));
const positive=invoke();
assert.equal(positive.status,0,positive.stderr||positive.stdout);
assert.equal(JSON.parse(fs.readFileSync(comparisonPath,'utf8')).matched,150);

const index=native.records.findIndex(row=>row.svg!==null);
assert.notEqual(index,-1);
native.records[index].svg+='<!-- deliberate test mismatch -->';
fs.writeFileSync(nativePath,JSON.stringify(native));
const negative=invoke();
assert.equal(negative.status,1,negative.stderr||negative.stdout);
const failed=JSON.parse(fs.readFileSync(comparisonPath,'utf8'));
assert.equal(failed.matched,149);
assert.equal(failed.failures.length,1);
assert.match(failed.failures[0].sourceSvgSha256,/^[0-9a-f]{64}$/);
assert.match(failed.failures[0].nativeSvgSha256,/^[0-9a-f]{64}$/);
console.log(JSON.stringify({positiveMatched:150,negativeMatched:149,
  mismatchCase:failed.failures[0].name,parityClaim:false}));
