// Root checks every source member and its reviewed native responsibility.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const {root}=require('./inheritance-reference.cjs');
const inputs=['ts/core/MmlTree/MML.ts','ts/core/MmlTree/MmlFactory.ts','ts/core/MmlTree/MmlNodes/maction.ts'];
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),...inputs],{encoding:'utf8'}));
const file=path.join(root,'docs/math-port/evidence/p18-mml-action-members.json');
const selection=new Set(['selected','isEmbellished','isSpacelike','core','coreMO']);
const bindings=new Set(['defaults','kind','arity']);
function target(f,m){
 if(f.source.endsWith('/MML.ts'))return 'src/math/core/mml_tree/mml.cpp:folia::math::mml::MML';
 if(f.source.endsWith('/MmlFactory.ts'))return 'src/math/core/mml_tree/mml_factory.'+(m.name==='defaultNodes'?'ixx:MmlFactory::defaultNodes':'cpp:MmlFactory::MML');
 return 'src/math/core/mml_tree/mml_nodes/maction_'+(selection.has(m.name)?'selection':bindings.has(m.name)?'bindings':'behavior')+'.cpp:MmlMaction::'+m.name;
}
if(process.argv.includes('--freeze-reviewed')){
 fresh.status='locally_accepted_mml_registry_and_action_not_renderer';
 for(const f of fresh.files)for(const m of f.members){m.target=target(f,m);m.review='root_review_passed';m.evidence='docs/math-port/evidence/p18'+(f.source.endsWith('/maction.ts')?'b-maction':'c-mml-registry')+'-acceptance.md';
  if(f.source.endsWith('/MML.ts'))m.native_adaptation='One const shared binding to an ordered mutable registry; all 43 compiled constructor identities retained.';
  if(f.source.endsWith('/MmlFactory.ts')&&m.name==='defaultNodes')m.native_adaptation='Separately rebindable shared handle initially aliases the registry; inherited this.constructor.defaultNodes is selected by the existing typed injected-default constructor.';
  if(m.name==='selected')m.native_adaptation='Virtual pointer getter in the dense nullable native child-array and raw scalar/record/array host; full NaN/fraction/default behavior retained.';
  if(m.name==='setChildInheritedAttributes')m.native_adaptation='The accepted general Unicode default-lowercase host implements the actual string method; no coercion of non-string receivers.';
 }
 fs.writeFileSync(file,JSON.stringify(fresh,null,2)+'\n');
}
const accepted=JSON.parse(fs.readFileSync(file));assert.deepEqual(accepted.files.map(f=>f.members.length),[1,2,12]);
for(let i=0;i<fresh.files.length;++i){const f=fresh.files[i],a=accepted.files[i];for(const[k,value]of Object.entries(f))if(k!=='members')assert.deepEqual(a[k],value);
 for(let j=0;j<f.members.length;++j){const m=f.members[j],entry=a.members[j];for(const[k,value]of Object.entries(m))if(!['target','review','evidence','native_adaptation'].includes(k))assert.deepEqual(entry[k],value);assert.equal(entry.review,'root_review_passed');assert.equal(entry.target,target(f,m));assert.ok(fs.existsSync(path.join(root,entry.target.split(':')[0])));}
}
console.log(JSON.stringify({source_files:3,members:15,registry_entries:43,status:'reviewed_pinned_inventory_matches'}));
