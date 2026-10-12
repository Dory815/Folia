// Root compiling faults for identity/lifetime plumbing; no body agents active.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/util/native_identities.ixx',saved=fs.readFileSync(path.join(root,file),'utf8');
const faults=[
 ['foreign-context-validation-skipped',[['context_.kind(object); // Reject foreign-context handles before aliasing.','// Context validation omitted.']]],
 ['canonical-registration-skipped',[['objects_.emplace(std::move(identity),object);','// Canonical registration omitted.']]],
 ['canonical-alias-replaced',[['if(auto existing=find(identity))return *existing;','if(auto existing=find(identity);false)return *existing;',2]]],
 ['noncanonical-payload-accepted',[['if(!canonical||*canonical!=object)throw','if(false)throw']]],
 ['first-raw-binding-lifetime-not-required',[['if constexpr(std::is_pointer_v<Identity>)if(!lifetime)','if constexpr(std::is_pointer_v<Identity>)if(false)']]],
 ['graph-lifetime-not-retained',[['NativeBindingPayload<Identity>{identity,std::move(lifetime)}','NativeBindingPayload<Identity>{identity,{}}']]],
 ['clear-keeps-payload-owner',[['if(bound&&bound->identity==identity)payload.reset();','if(false)payload.reset();']]],
 ['clear-keeps-canonical-index',[['objects_.clear();','// Canonical index not cleared.']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_native_identity*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p49a-native-identity-'+label+'.log'),log);return{status:r.status,log};}
const results=[];try{const b=run('mutation-baseline');assert.equal(b.status,0);assert.match(b.log,/26 asserts in 6 tests/);assert.match(b.log,/9 asserts in 9 tests/);for(const[id,edits]of faults){let changed=saved;for(const[from,to,count=1]of edits){assert.equal(changed.split(from).length,count+1,id+' exact anchor');changed=changed.replaceAll(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail assertions');}}finally{fs.writeFileSync(path.join(root,file),saved);}assert.equal(run('mutation-restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p49a-native-identities-mutations.json'),JSON.stringify({sources:[{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')}],faults:results},null,2)+'\n');
