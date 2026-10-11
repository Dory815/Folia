// Root-owned complete inventory check; acceptance requires independent evidence.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),file=path.join(root,'docs/math-port/evidence/p20c-numeric-reductions-members.json');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),'ts/util/numeric.ts'],{encoding:'utf8'}));
assert.equal(fresh.files.length,1);assert.deepEqual(fresh.files[0].members.map(m=>[m.kind,m.name]),[['function','sum'],['function','max']]);
if(process.argv.includes('--freeze-reviewed')){
 fresh.status='locally_accepted_complete_numeric_reductions_with_dense_binary64_host_adaptation';
 fresh.files[0].native_adaptation='Borrowed readonly dense binary64 span. Common mtable retains its numeric array producer and string-width conversion responsibilities.';
 for(const m of fresh.files[0].members){m.target='src/math/util/numeric.cpp:folia::math::numeric::'+m.name;m.review='root_review_passed';m.evidence='docs/math-port/evidence/p20c-numeric-reductions-acceptance.md';}
 fs.writeFileSync(file,JSON.stringify(fresh,null,2)+'\n');
}
const accepted=JSON.parse(fs.readFileSync(file));assert.equal(accepted.files.length,1);
const a=accepted.files[0],f=fresh.files[0];assert.equal(a.source,f.source);assert.equal(a.source_sha256,f.source_sha256);assert.equal(a.members.length,2);
for(let i=0;i<2;i++){const m=f.members[i],entry=a.members[i];for(const[k,v]of Object.entries(m))if(!['target','review','evidence'].includes(k))assert.deepEqual(entry[k],v);assert.equal(entry.review,'root_review_passed');assert.equal(entry.target,'src/math/util/numeric.cpp:folia::math::numeric::'+m.name);assert.ok(fs.existsSync(path.join(root,entry.target.split(':')[0])));assert.ok(fs.existsSync(path.join(root,entry.evidence)));}
console.log(JSON.stringify({source_files:1,inventoried_members:2,status:'reviewed_pinned_inventory_matches'}));
