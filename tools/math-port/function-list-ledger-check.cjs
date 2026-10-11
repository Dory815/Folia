// Root complete source-member coverage; native Promise host is not a MathJax file.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),file=path.join(root,'docs/math-port/evidence/p19a-function-list-members.json');
const fresh=JSON.parse(cp.execFileSync(process.execPath,[path.join(__dirname,'member-ledger.cjs'),'ts/util/FunctionList.ts'],{encoding:'utf8'}));
const target=m=>'src/math/util/function_list.cpp:FunctionList::'+(m.name==='constructor'?'initializeFunctions':m.name);
if(process.argv.includes('--freeze-reviewed')){
 fresh.status='locally_accepted_complete_function_list_not_tex_or_renderer';
 for(const m of fresh.files[0].members){m.target=target(m);m.review='root_review_passed';m.evidence='docs/math-port/evidence/p19a-function-list-acceptance.md';
  if(m.name==='constructor')m.native_adaptation='Post-allocation initializeFunctions preserves virtual addList dispatch; typed vector/tuple handles and numeric priorities follow the source TS interface.';
  if(m.name==='execute')m.native_adaptation='Inherited native iterator captures original shared array and retains actual record identity; raw scalar/record results stop only on exact false.';
  if(m.name==='asyncExecute')m.native_adaptation='Explicit session-local native Promise/job realm, captured shared array and args, same then(execute).catch(fail) chain and FIFO microtask phases; no JS interpreter. Native raw shared reasons use ThrownValue; ordinary C++ exceptions retain captured exception_ptr payload/propagation identity.';
 }
 fs.writeFileSync(file,JSON.stringify(fresh,null,2)+'\n');
}
const accepted=JSON.parse(fs.readFileSync(file));assert.equal(fresh.files.length,1);assert.equal(fresh.files[0].members.length,4);
const f=fresh.files[0],a=accepted.files[0];for(const[k,v]of Object.entries(f))if(k!=='members')assert.deepEqual(a[k],v);
for(let i=0;i<f.members.length;++i){const m=f.members[i],entry=a.members[i];for(const[k,v]of Object.entries(m))if(!['target','review','evidence','native_adaptation'].includes(k))assert.deepEqual(entry[k],v);assert.equal(entry.review,'root_review_passed');assert.equal(entry.target,target(m));assert.ok(fs.existsSync(path.join(root,entry.target.split(':')[0])));assert.ok(fs.existsSync(path.join(root,entry.evidence)));}
console.log(JSON.stringify({source_files:1,members:4,status:'reviewed_pinned_inventory_matches'}));
