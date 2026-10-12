// Root semantic acceptance; implementation/review agents must be idle.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/input/tex/find_tex.cpp',saved=fs.readFileSync(path.join(root,file),'utf8');
const faults=[
 ['shorter-opener-sorted-first',[['string_util::sortLength(left,right)<0','string_util::sortLength(left,right)>0']]],
 ['empty-environment-capture-treated-as-absent',[['if (environment && options::truthy(this->env_))','if (environment && !environment->empty() && options::truthy(this->env_))']]],
 ['close-delimiter-inside-braces-accepted',[['Value(candidate)==close && braces==0','Value(candidate)==close']]],
 ['display-tuple-value-discarded',[['to_property(display));','Property(false));']]],
 ['escape-display-default-replaced-with-false',[['Property(static_cast<double>(start->index)),Property(end));','Property(static_cast<double>(start->index)),Property(end),Property(false));']]],
 ['scanner-skips-next-adjacent-opener',[['regexp_.setLastIndex(this->start_,item_end_n(match));','regexp_.setLastIndex(this->start_,item_end_n(match)+1);']]],
 ['duplicate-delimiter-keeps-first-definition',[['optionsContext().set(end_receiver,key,Value(end_tuple));','if (!optionsContext().hasOwn(end_receiver,key)) optionsContext().set(end_receiver,key,Value(end_tuple));']]],
 ['tuple-iterator-ignores-current-length',[['if (index_>=length)','if (false && index_>=length)']]],
 ['looked-up-forEach-bypassed',[['(void)optionsContext().call(*method_ref,delimiters,arguments);','optionsContext().arrayForEach(std::get<ObjectRef>(delimiters.data),Value(callback));']]],
 ['input-values-copied-before-first-scan',[['const double limit=primitive_number(length);','const double limit=primitive_number(length);\n    const auto snapshot=optionsContext().arrayValues(std::get<ObjectRef>(input.data));'],['const Value text=optionsContext().get(input,index_key(static_cast<std::uint64_t>(i)));','const Value text=snapshot.at(static_cast<std::size_t>(i));']]]
];
function run(label){
 const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_find_tex*'],{cwd:root,encoding:'utf8',timeout:120000,maxBuffer:64*1024*1024});assert.ifError(r.error);
 const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p52-find-tex-'+label+'.log'),log);return {status:r.status,log};
}
const results=[];
try{
 const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/77 asserts in 77 tests/);
 for(const[id,edits]of faults){
  let changed=saved;for(const[from,to]of edits){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}
  fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id);
  const compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status===1&&/FAILED/.test(r.log);
  results.push({id,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved);console.log(JSON.stringify(results.at(-1)));
  assert.ok(compiling&&detected,id+' must compile and fail original assertions');
 }
}finally{fs.writeFileSync(path.join(root,file),saved);}
assert.equal(run('mutation-restored').status,0);
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p52-find-tex-mutations.json'),JSON.stringify({sources:[{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')}],baseline:{assertions:77,tests:77},faults:results},null,2)+'\n');
