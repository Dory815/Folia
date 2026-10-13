'use strict';
// Root-owned independent check of the actual production RegExp observer.
const fs=require('node:fs');
const path=require('node:path');
const assert=require('node:assert/strict');
const crypto=require('node:crypto');
const {spawnSync}=require('node:child_process');
const {isDeepStrictEqual}=require('node:util');
const root=path.resolve(__dirname,'../..');
const label=process.argv[2];
assert.match(label||'',/^[a-zA-Z0-9-]+$/,'supply a checkpoint label');
const hash=file=>crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const sourceFile=path.join(root,'build/math-port/unicode-regexp-original-root.json');
// Reproduce the independently pinned original instead of depending on an
// ignored result left behind by an earlier session. Do not replace the lock
// with a candidate-generated expectation when the original host changes.
const original=spawnSync(process.execPath,[path.join(__dirname,'unicode-regexp-original-oracle.cjs')],
  {cwd:root,encoding:'utf8',maxBuffer:16*1024*1024});
assert.ifError(original.error);
assert.equal(original.status,0,original.stderr);
const originalHash=crypto.createHash('sha256').update(original.stdout).digest('hex');
assert.equal(originalHash,'e6c01910ecb588816a0b1c99f7746b7a9b21bd8abeb1cd84f93a3b7163e5d634',
  'original observation changed; review source/host provenance before accepting a new lock');
fs.mkdirSync(path.dirname(sourceFile),{recursive:true});
fs.writeFileSync(sourceFile,original.stdout);
const source=JSON.parse(fs.readFileSync(sourceFile));
const executable=path.join(root,'build/core/FoliaMathUnicodeRegexpDump.exe');
const destination=path.join(root,'build/math-port',`unicode-checkpoint-${label}`);
fs.mkdirSync(destination,{recursive:true});
const run=spawnSync(executable,[],{cwd:root,encoding:'utf8',maxBuffer:16*1024*1024});
fs.writeFileSync(path.join(destination,'native.json'),run.stdout||'');
fs.writeFileSync(path.join(destination,'native.stderr'),run.stderr||'');
const summary={executable,executableSha256:hash(executable),exitCode:run.status,
  source:path.relative(root,sourceFile),sourceSha256:hash(sourceFile),
  sourceHost:{node:source.node,v8:source.v8,unicode:source.unicode},
  total:source.records.length,matched:0,failures:[]};
if(run.error||run.status!==0){
  summary.executionError=run.error?.message||`native exit ${run.status}`;
}else{
  const native=JSON.parse(run.stdout);
  assert.equal(native.records.length,source.records.length,'native row count');
  assert.equal(new Set(native.records.map(row=>row.name)).size,native.records.length,'duplicate native names');
  assert.deepEqual(native.records.map(row=>row.name),source.records.map(row=>row.name),'whole corpus in original order');
  for(let i=0;i<source.records.length;i++){
    if(isDeepStrictEqual(native.records[i],source.records[i]))summary.matched++;
    else summary.failures.push({name:source.records[i].name,source:source.records[i],native:native.records[i]});
  }
}
fs.writeFileSync(path.join(destination,'summary.json'),JSON.stringify(summary,null,2)+'\n');
process.stdout.write(JSON.stringify({checkpoint:label,exitCode:summary.exitCode,
  matched:summary.matched,total:summary.total,
  failedNames:summary.failures.map(row=>row.name),
  executionError:summary.executionError,
  summary:path.relative(root,path.join(destination,'summary.json'))},null,2)+'\n');
if(summary.executionError||summary.failures.length)process.exitCode=1;
