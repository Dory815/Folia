// Root ordinary-array host faults; only after the body agent completes.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/util/option_array_iteration.cpp',saved=fs.readFileSync(path.join(root,file),'utf8');
const faults=[
 ['length-read-live-instead-of-captured',[['index < length','index < arrayLength(array)']]],
 ['holes-not-skipped',[['if (!has(receiver,key)) continue;','if (false) continue;']]],
 ['inherited-slots-ignored',[['has(receiver,key)','hasOwn(receiver,key)']]],
 ['future-values-snapshotted',[['const Value receiver(array);','const Value receiver(array);\n    const auto snapshot=arrayValues(array);'],['const Value value = get(receiver,key);','const Value value = snapshot.at(static_cast<std::size_t>(index));']]],
 ['invalid-empty-array-callback-not-rejected',[['throw Exception("TypeError",u"Array forEach callback must be callable");','return;']]],
 ['callback-this-argument-discarded',[['call(*callback_object,thisArgument,arguments)','call(*callback_object,Undefined{},arguments)']]],
 ['callback-array-identity-replaced',[['value, Value(static_cast<double>(index)), receiver','value, Value(static_cast<double>(index)), Value(this->array())']]],
 ['iteration-stops-after-first-callback',[['(void)call(*callback_object,thisArgument,arguments);','(void)call(*callback_object,thisArgument,arguments);\n        break;']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_option_array_for_each*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p51a-option-array-'+label+'.log'),log);return{status:r.status,log};}
const results=[];try{const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/22 asserts in 22 tests/);for(const[id,edits]of faults){let changed=saved;for(const[from,to]of edits){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}}finally{fs.writeFileSync(path.join(root,file),saved);}assert.equal(run('mutation-restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p51a-option-array-iteration-mutations.json'),JSON.stringify({sources:[{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')}],faults:results},null,2)+'\n');
