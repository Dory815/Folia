// Root independent compiling faults; only runs when agents have stopped writing.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..');
const body='src/math/input/tex/base_item.cpp',bindings='src/math/input/tex/base_item_bindings.cpp';
const paths=[body,bindings],saved=new Map(paths.map(p=>[p,fs.readFileSync(path.join(root,p),'utf8')]));
const faults=[
 ['name-virtual-property',body,'return this->getProperty("name");','return base_item_host::properties(properties_).get("name");'],
 ['properties-own-lookup',body,'return base_item_host::properties(properties_).get(key);','auto p=base_item_host::properties(properties_);return p.hasOwn(key)?p.get(key):StackProperty(Missing{});'],
 ['properties-target-identity',body,'auto target=base_item_host::properties(properties_);','auto target=StackPropertyList{};'],
 ['over-clear-before-set',body,'item.setProperty("num",StackProperty(number));\n        this->Clear();','this->Clear();\n        item.setProperty("num",StackProperty(number));'],
 ['over-row-inference',body,'const mml::NodeValue number=this->toMml(false);','const mml::NodeValue number=this->toMml(true);'],
 ['cell-linebreak-fail',body,'if(stack_property_host::truthy(linebreak))return fail;','if(stack_property_host::truthy(linebreak))return success;'],
 ['close-second-errors-read',body,'const Property errors=this->getErrors(errorKind);','const Property errors=initialErrors;'],
 ['final-first-value',body,'const mml::NodeValue first=item.First();','const mml::NodeValue first=item.Pop();'],
 ['shared-fail-tuple',body,'return fail;','return std::make_shared<CheckTuple>(*fail);',2],
 ['environment-live-target',body,'const Property current=this->env();','const Property current=environment;'],
 ['class-errors-truthiness',body,'if(property_host::truthy(value))return value;','if(!value.missing())return value;'],
 ['class-errors-binding',body,'const Property currentErrors=currentClass->errorsBinding();','const Property currentErrors=BaseItem::baseErrors();'],
 ['latex-closing-brace',body,'if(tex!=u"}"){','if(!tex.empty()){'],
 ['latex-attributes-reread',body,'const auto secondAttributes=base_item_host::attributes(node);','const auto secondAttributes=firstAttributes;'],
 ['latex-first-write',body,'Property(property_host::debugText(tex)));','Property("lost"));',0],
 ['constructor-open-environment',bindings,'if(isOpen())env_=PropertyList{};','if(isOpen())env_=Missing{};'],
 ['constructor-environment-order',bindings,'if(isOpen())env_=PropertyList{};','env_=PropertyList{};if(isOpen())env_=PropertyList{};']
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_base_item*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p42b-'+label+'.log'),log);return{status:r.status,log};}
const results=[];try{
 assert.equal(run('mutation-baseline').status,0);
 for(const[id,file,from,to,index]of faults){
  const text=saved.get(file),parts=text.split(from);assert.equal(parts.length,index===undefined?2:index===2?3:3,id+' exact anchor');
  const occurrence=index===2?1:index;
  const changed=occurrence===undefined?text.replace(from,to):parts.slice(0,occurrence+1).join(from)+to+parts.slice(occurrence+1).join(from);
  fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);
  results.push({id,file,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),text);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');
 }
}finally{for(const[file,text]of saved)fs.writeFileSync(path.join(root,file),text);}
assert.equal(run('mutation-restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p42b-base-item-mutations.json'),JSON.stringify({sources:paths.map(file=>({file,sha256:crypto.createHash('sha256').update(saved.get(file)).digest('hex')})),faults:results},null,2)+'\n');
