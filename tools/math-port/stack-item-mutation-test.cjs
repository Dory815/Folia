// Root-only independent compiling faults against pinned original MmlStack gold.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),target=path.join(root,'src/math/input/tex/stack_item.cpp');
const saved=fs.readFileSync(target,'utf8');
const faults=[
 ['push-tagged-values','for(const auto& value:values)receiver->push_back(value);','for(const auto& value:values)if(value.isNode())receiver->push_back(value);'],
 ['first-virtual-size','const double index=Size()-1;','const double index=receiver?static_cast<double>(receiver->size())-1:Size()-1;',0],
 ['first-captured-receiver','return array->getProperty(property);','return stack_item_host::array(nodes_,property)->getProperty(property);'],
 ['first-empty-named-slot','array->setProperty(property,std::move(value));','array->setProperty(property=="-1"?"0":property,std::move(value));'],
 ['last-fixed-zero','return receiver->getProperty("0");','return receiver->getProperty(stack_item_host::key(static_cast<double>(receiver->size())-1));'],
 ['peek-null-default','if(property_host::nullish(count))count=1;','if(count.missing())count=1;'],
 ['peek-fractional-start','const double numericCount=property_host::number(count);','const double numericCount=std::trunc(property_host::number(count));'],
 ['peek-null-receiver-order','const auto array=stack_item_host::array(receiver,"slice");\n    const double size=Size();','const double size=Size();\n    const auto array=stack_item_host::array(receiver,"slice");'],
 ['clear-array-identity','nodes_=std::make_shared<tree::ChildArray<mml::MmlNode>>();','if(nodes_)nodes_->resize(0);else nodes_=std::make_shared<tree::ChildArray<mml::MmlNode>>();'],
 ['tomml-force-row','if(receiver->size()==1&&!property_host::truthy(forceRow))return First();','if(receiver->size()==1)return First();'],
 ['tomml-null-inferred','if(inferred.missing())inferred=true;','if(property_host::nullish(inferred))inferred=true;'],
 ['tomml-children-identity','std::any(receiver),','std::any(receiver->slice()),']
];
function run(label){
 const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_stack_item*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});
 assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');
 fs.writeFileSync(path.join(root,'build','p42-'+label+'.log'),log);
 return{status:r.status,log};
}
const results=[];
try{
 assert.equal(run('mutation-baseline').status,0);
 for(const[id,from,to,index]of faults){
  const parts=saved.split(from);assert.equal(parts.length,index===undefined?2:3,id+' exact anchor');
  const changed=index===undefined?saved.replace(from,to):parts[0]+to+parts[1]+from+parts[2];
  fs.writeFileSync(target,changed);const r=run('fault-'+id);
  const compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log);
  const detected=r.status!==0&&/FAILED/.test(r.log);
  results.push({id,compiling,detected,status:r.status});
  fs.writeFileSync(target,saved);console.log(JSON.stringify(results.at(-1)));
  assert.ok(compiling&&detected,id+' must compile and fail gold assertions');
 }
}finally{fs.writeFileSync(target,saved);}
assert.equal(run('mutation-restored').status,0);
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p42-mml-stack-mutations.json'),JSON.stringify({source_sha256:crypto.createHash('sha256').update(saved).digest('hex'),faults:results},null,2)+'\n');
