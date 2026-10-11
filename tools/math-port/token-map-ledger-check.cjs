// Root complete pinned Source inventory; metadata alone never passes acceptance.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),file=path.join(root,'docs/math-port/evidence/p22b-token-map-members.json');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),'ts/input/tex/TokenMap.ts'],{encoding:'utf8'}));
assert.equal(fresh.files.length,1);const source=fresh.files[0];assert.equal(source.members.length,23);
assert.equal(source.members.filter(m=>m.kind==='initialized_field').length,1);
function target(m){
 if(m.owner==='<module>')return 'src/math/input/tex/token_map.cpp:parseResult';
 if(m.kind==='initialized_field')return 'src/math/input/tex/token_map.cpp:AbstractParseMap::initializeSource [map_ field initialization]';
 const method=m.kind==='constructor'?'initializeSource':m.name;
 return 'src/math/input/tex/token_map.cpp:'+m.owner+'::'+method+(m.kind==='get'?' [getter]':m.kind==='set'?' [setter]':'');
}
if(process.argv.includes('--freeze-reviewed')){
 assert.ok(fs.existsSync(path.join(root,'docs/math-port/evidence/p22b-token-map-acceptance.md')),'Root acceptance evidence required');
 fresh.status='locally_accepted_complete_token_map_under_explicit_native_host_profile_not_engine_parser_or_renderer';
 source.native_adaptation='One final shared allocation before original base-to-derived initialization/registration; optional ordered SameValueZero Map; canonical Property/Token/Macro/callable owners and virtual getters; controlled identity-bearing RegExpContext dependency. Regex engine and complete configured parser/Common/SVG/Folia remain unaccepted.';
 for(const m of source.members){m.target=target(m);m.review='root_review_passed';m.evidence='docs/math-port/evidence/p22b-token-map-acceptance.md';}
 fs.writeFileSync(file,JSON.stringify(fresh,null,2)+'\n');
}
const accepted=JSON.parse(fs.readFileSync(file));assert.equal(accepted.files.length,1);const a=accepted.files[0];
assert.equal(a.source,source.source);assert.equal(a.source_sha256,source.source_sha256);assert.equal(a.members.length,23);
for(let i=0;i<23;i++){const entry=a.members[i],m=source.members[i];for(const[k,v]of Object.entries(m))if(!['target','review','evidence'].includes(k))assert.deepEqual(entry[k],v);assert.equal(entry.review,'root_review_passed');assert.equal(entry.target,target(m));assert.ok(fs.existsSync(path.join(root,entry.target.split(':')[0])));assert.ok(fs.existsSync(path.join(root,entry.evidence)));}
console.log(JSON.stringify({source_files:1,executable_members:22,initialized_fields:1,status:'reviewed_pinned_inventory_matches'}));
