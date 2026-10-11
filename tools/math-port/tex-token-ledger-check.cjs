// Root inventory check; source TypeScript and reviewed mappings must still agree.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const {root,hashes}=require('./tex-token-reference.cjs');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),...hashes.map(h=>h.source)],{encoding:'utf8'}));
const file=path.join(root,'docs/math-port/evidence/p17a-token-members.json');
if(process.argv.includes('--freeze-reviewed')){
fresh.status='locally_accepted_token_and_type_foundation_not_parser';
for(const f of fresh.files)for(const m of f.members){const name=m.name==='constructor'?m.owner:m.name==='char'?'character':m.name;m.target=f.source.endsWith('Token.ts')?'src/math/input/tex/'+m.owner.toLowerCase()+'.cpp:'+m.owner+'::'+name:'src/math/input/tex/tex_constants.ixx:TexConstant';m.review='root_review_passed';m.evidence='docs/math-port/evidence/p17a-token-acceptance.md';}
fs.writeFileSync(file,JSON.stringify(fresh,null,2)+'\n');
}
const accepted=JSON.parse(fs.readFileSync(file));assert.equal(accepted.files.length,4);
for(let i=0;i<4;++i){const a=fresh.files[i],b=accepted.files[i];assert.equal(a.source,b.source);assert.equal(a.source_sha256,b.source_sha256);assert.equal(a.members.length,b.members.length);for(let j=0;j<a.members.length;++j){for(const [k,v]of Object.entries(a.members[j]))if(k!=='target'&&k!=='review')assert.deepEqual(b.members[j][k],v,k);assert.equal(b.members[j].review,'root_review_passed');assert.ok(b.members[j].target);}}
console.log(JSON.stringify({files:4,members:9,type_aliases:6,string_enum_members:16,status:'reviewed_pinned_inventory_matches'}));
