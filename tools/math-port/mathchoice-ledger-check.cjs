// Root checks all five MathChoice members against the pinned TypeScript AST.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const {root}=require('./inheritance-reference.cjs');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),'ts/core/MmlTree/MmlNodes/mathchoice.ts'],{encoding:'utf8'}));
const file=path.join(root,'docs/math-port/evidence/p17c-mathchoice-members.json');
if(process.argv.includes('--freeze-reviewed')){
    fresh.status='locally_accepted_mathchoice_semantics_not_renderer';
    for(const f of fresh.files)for(const m of f.members){
        const body=m.name==='setInheritedAttributes';
        m.target='src/math/core/mml_tree/mml_nodes/mathchoice'+(body?'.cpp:MathChoice::inheritAttributes':'_bindings.cpp:MathChoice::'+m.name);
        m.review='root_review_passed';m.evidence='docs/math-port/evidence/p17c-mathchoice-acceptance.md';
        if(body)m.native_adaptation='The single raw public facade dispatches here without defaults; qualified source body identity is preserved.';
    }
    fs.writeFileSync(file,JSON.stringify(fresh,null,2)+'\n');
}
const accepted=JSON.parse(fs.readFileSync(file));assert.equal(accepted.files.length,1);assert.equal(accepted.files[0].members.length,5);
for(const[k,value]of Object.entries(fresh.files[0]))if(k!=='members')assert.deepEqual(accepted.files[0][k],value);
for(let i=0;i<5;i++){const source=fresh.files[0].members[i],entry=accepted.files[0].members[i];for(const[k,value]of Object.entries(source))if(!['target','review','evidence','native_adaptation'].includes(k))assert.deepEqual(entry[k],value);assert.equal(entry.review,'root_review_passed');assert.ok(entry.target);}
console.log(JSON.stringify({source_files:1,members:5,status:'reviewed_pinned_inventory_matches'}));
