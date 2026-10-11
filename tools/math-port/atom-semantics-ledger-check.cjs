// Root acceptance inventory must retain every pinned AST member and final binding.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'atom-semantics-member-ledger.cjs')],{encoding:'utf8'}));
const accepted=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p15bc-members.json'),'utf8'));
assert.equal(accepted.files.length,fresh.files.length);
let members=0,bindings=0;
function compare(actual,expected){
  for(const [key,value]of Object.entries(expected))if(key!=='target'&&key!=='review')assert.deepEqual(actual[key],value,key);
  assert.equal(actual.review,'root_review_passed');
  assert.ok(typeof actual.target==='string'&&actual.target.length);
  assert.equal(actual.evidence,'docs/math-port/evidence/p15bc-atom-semantics-acceptance.md');
}
for(let i=0;i<fresh.files.length;++i){
  const f=fresh.files[i],a=accepted.files[i];
  assert.equal(a.source,f.source);assert.equal(a.source_sha256,f.source_sha256);
  assert.equal(a.members.length,f.members.length);
  f.members.forEach((m,j)=>{compare(a.members[j],m);++members;});
  assert.equal((a.module_bindings||[]).length,(f.module_bindings||[]).length);
  (f.module_bindings||[]).forEach((m,j)=>{compare(a.module_bindings[j],m);++bindings;});
}
console.log(JSON.stringify({members,module_bindings:bindings,source_files:fresh.files.length,status:'root_reviewed_inventory_matches_pinned_source'}));
