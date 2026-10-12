// Root acceptance through the existing core build; no duplicate build tree.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/util/string_map.ixx',saved=fs.readFileSync(path.join(root,file),'utf8');
const gold=['tests/math/reference_string_map.cpp','docs/math-port/evidence/p53-string-map-source.json'].map(file=>({file,bytes:fs.readFileSync(path.join(root,file))}));
const faults=[
 ['replacement-duplicates-key', [['entry.second=std::move(value);return;','break;']]],
 ['iterator-completion-revives', [['if(done_)return std::nullopt;','if(false && done_)return std::nullopt;']]],
 ['iterator-snapshots-storage', [['return KeyIterator(storage_);','return KeyIterator(std::make_shared<Storage>(*storage_));']]],
 ['existing-get-reported-missing', [['return entry.second;','return std::nullopt;']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_string_map*'],{cwd:root,encoding:'utf8',timeout:180000,maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p53-string-map-'+label+'.log'),log);return {status:r.status,log};}
const results=[];
try{
 const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/12 asserts in 12 tests/);
 for(const[id,edits]of faults){let changed=saved;for(const[from,to]of edits){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id);const compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status===1&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}
}finally{fs.writeFileSync(path.join(root,file),saved);}
assert.equal(run('mutation-restored').status,0);
for(const {file,bytes}of gold)assert.ok(bytes.equals(fs.readFileSync(path.join(root,file))),file+' unchanged');
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p53-string-map-mutations.json'),JSON.stringify({source:{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')},baseline:{assertions:12,tests:12},faults:results,gold_unchanged:true},null,2)+'\n');
