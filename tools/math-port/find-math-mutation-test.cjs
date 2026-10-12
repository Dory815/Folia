// Root compiling semantic faults against the actual Source FindMath constructor.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/core/find_math.cpp',saved=fs.readFileSync(path.join(root,file),'utf8');
const faults=[
 ['constructor-class-not-captured',[['context_.get(klass,options::Key(u"OPTIONS"))','context_.get(context_.get(options::Value(identity_),options::Key(u"constructor")),options::Key(u"OPTIONS"))']]],
 ['defaults-used-as-instance-options',[['options::defaultOptions(context_,destination,defaults)','classOptions']]],
 ['user-merge-skipped',[['options::userOptions(context_,defaulted,userDefinitions)','defaulted']]],
 ['options-published-before-merge',[['const options::Value initialized=options::userOptions','options_=destination;\n    const options::Value initialized=options::userOptions']]],
 ['default-merge-skipped',[['options::defaultOptions(context_,destination,defaults)','destination']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_find_math*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p49b-find-math-'+label+'.log'),log);return{status:r.status,log};}
const results=[];
try{
 const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/11 asserts in 11 tests/);
 for(const[id,edits]of faults){let changed=saved;for(const[from,to]of edits){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}
}finally{fs.writeFileSync(path.join(root,file),saved);}
assert.equal(run('mutation-restored').status,0);
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p49b-find-math-mutations.json'),JSON.stringify({sources:[{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')}],faults:results},null,2)+'\n');
