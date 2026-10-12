// Root independent compiling faults. Agents must be idle before this runs.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),body='src/math/input/tex/stack.cpp',host='src/math/input/tex/stack_host.ixx';
const saved=new Map([body,host].map(file=>[file,fs.readFileSync(path.join(root,file),'utf8')]));
const faults=[
 ['constructor-inner',body,[['{"isInner",std::move(inner)}','{"isInner",false}']]],
 ['env-setter',body,[['void Stack::setEnv(Property value){env_=std::move(value);}','void Stack::setEnv(Property value){}']]],
 ['push-rest-snapshot',body,[['for(const StackEntry& input:arguments){','for(const StackEntry& input:values){']]],
 ['push-global-before-check',body,[['item.global=global;','item.global=PropertyList{};']]],
 ['check-success-truthiness',body,[['if(!succeeded)continue;','if(false)continue;']]],
 ['replacement-tuple-slot-reread',body,[['bool checkedTop=false;','bool checkedTop=false;std::shared_ptr<CheckTuple> originalTuple;'],['const auto tuple=stack_host::tuple(checked);','const auto tuple=stack_host::tuple(checked);originalTuple=tuple;'],['stack_host::spread(topValue)','stack_host::spread(originalTuple->items)']]],
 ['replacement-array-deep-copy',body,[['topValue=tuple->items;','topValue=tuple->items;if(auto*p=std::get_if<CheckList>(&topValue))topValue=std::make_shared<std::vector<StackEntry>>(**p);']]],
 ['null-kind-append',body,[['if(!nullItem){','if(true){']]],
 ['environment-cached-target',body,[['const Property target=item.env();','const Property target=itemEnvironment;']]],
 ['environment-source-before-target',body,[['const Property target=item.env();\n                const Property source=this->env();','const Property source=this->env();\n                const Property target=item.env();']]],
 ['environment-cached-final',body,[['const Property nextEnvironment=item.env();','const Property nextEnvironment=itemEnvironment;']]],
 ['pop-delete-inherited-env',body,[['if(!popped.isOpen())stack_host::deleteEnv(popped);','if(!popped.isOpen())popped.setEnv(Missing{});']]],
 ['pop-removal-after-open',body,[['stack_.pop_back();','/* delayed removal */'],['if(!popped.isOpen())stack_host::deleteEnv(popped);','if(!popped.isOpen())stack_host::deleteEnv(popped);stack_.pop_back();']]],
 ['top-null-instead-of-missing',body,[['return stack_host::index(stack_,lengthForIndex-countForIndex);','auto value=stack_host::index(stack_,lengthForIndex-countForIndex);if(auto*p=std::get_if<Property>(&value);p&&p->missing())return Property(PropertyNull{});return value;']]],
 ['prev-dispatch',body,[['if(property_host::truthy(noPop))','if(!property_host::truthy(noPop))']]],
 ['join-live-length',host,[['for(std::size_t i=0;i<length;++i){','for(std::size_t i=0;i<entries.size();++i){']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_parser_stack*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p43-stack-'+label+'.log'),log);return{status:r.status,log};}
const results=[];try{
 assert.equal(run('mutation-baseline').status,0);
 for(const[id,file,changes]of faults){let changed=saved.get(file);for(const[from,to]of changes){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,file,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved.get(file));console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}
}finally{for(const[file,text]of saved)fs.writeFileSync(path.join(root,file),text);}
assert.equal(run('mutation-restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p43-parser-stack-mutations.json'),JSON.stringify({sources:[...saved].map(([file,text])=>({file,sha256:crypto.createHash('sha256').update(text).digest('hex')})),faults:results},null,2)+'\n');
