// Root faults in the actual ordinary-array host; one shared build/core target.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='src/math/util/option_array_iteration.cpp',saved=fs.readFileSync(path.join(root,file),'utf8'),split=saved.indexOf('void Context::arrayForEach');assert.ok(split>0);
const head=saved.slice(0,split),tail=saved.slice(split),gold=['tools/math-port/option-array-map-oracle.cjs','tests/math/reference_option_array_map.cpp','docs/math-port/evidence/p56-option-array-map-source.json'].map(file=>({file,bytes:fs.readFileSync(path.join(root,file))}));
const constructorGet='const Value constructorValue=get(Value(object),Key(u"constructor"));';
const faults=[
 ['length-read-live',[['index<length','index<arrayLength(object)']]],
 ['holes-visited',[['if(!has(receiver,key))continue;','if(false)continue;']]],
 ['inherited-slots-ignored',[['has(receiver,key)','hasOwn(receiver,key)']]],
 ['result-starts-empty-and-loses-trailing-holes',[['const ObjectRef result=array(length);','const ObjectRef result=array();']]],
 ['constructor-get-bypassed',[[constructorGet,'const Value constructorValue=Undefined{};']]],
 ['constructor-get-before-callback-validation',[['const auto* callbackObject=',''+constructorGet+'\n    const auto* callbackObject='],[constructorGet+'\n    ObjectRef intrinsicArray=','ObjectRef intrinsicArray=']]],
 ['constructor-get-before-length-capture',[['const std::uint32_t length=arrayLength(object);',constructorGet+'\n    const std::uint32_t length=arrayLength(object);'],[constructorGet+'\n    ObjectRef intrinsicArray=','ObjectRef intrinsicArray=']]],
 ['callback-arity-two',[['const std::array<Value,3> arguments{\n            value,Value(static_cast<double>(index)),receiver','const std::array<Value,2> arguments{\n            value,Value(static_cast<double>(index))']]],
 ['callback-index-boolean',[['value,Value(static_cast<double>(index)),receiver','value,Value(true),receiver']]],
 ['callback-this-discarded',[['call(*callbackObject,thisArgument,arguments)','call(*callbackObject,Undefined{},arguments)']]],
 ['callback-array-identity-replaced',[['value,Value(static_cast<double>(index)),receiver','value,Value(static_cast<double>(index)),Value(array())']]],
 ['result-set-dispatches-prototype-setter',[['define(result,key,Descriptor::data(mapped));','set(result,key,mapped);']]],
 ['callback-return-discarded',[['define(result,key,Descriptor::data(mapped));','define(result,key,Descriptor::data(Value(0)));']]],
 ['result-slots-collapsed-to-zero',[['define(result,key,Descriptor::data(mapped));','define(result,Key(u"0"),Descriptor::data(mapped));']]],
 ['iteration-stops-after-one',[['define(result,key,Descriptor::data(mapped));','define(result,key,Descriptor::data(mapped));\n        break;']]],
 ['custom-constructor-silently-defaulted',[['throw Exception("Unimplemented",u"Custom Array species requires a native constructor binding");','(void)intrinsicArray;']]],
 ['invalid-callback-returns-empty-array',[['throw Exception("TypeError",u"Array map callback must be callable");','return array(length);']]],
 ['result-data-not-writable',[['Descriptor::data(mapped)','Descriptor::data(mapped,false)']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_option_array_map*'],{cwd:root,encoding:'utf8',timeout:180000,maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p56-array-map-'+label+'.log'),log);return{status:r.status,log};}
const results=[];
try{const baseline=run('mutation-baseline');assert.equal(baseline.status,0);assert.match(baseline.log,/43 asserts in 42 tests/);
 for(const[id,edits]of faults){let changed=head;for(const[from,to]of edits){assert.equal(changed.split(from).length,2,id+' exact map-body anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed+tail);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status===1&&/FAILED/.test(r.log);results.push({id,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved);console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail assertions');}
}finally{fs.writeFileSync(path.join(root,file),saved);}
assert.equal(run('mutation-restored').status,0);
for(const{file,bytes}of gold)assert.ok(bytes.equals(fs.readFileSync(path.join(root,file))),file+' unchanged');
fs.writeFileSync(path.join(root,'docs/math-port/evidence/p56-option-array-map-mutations.json'),JSON.stringify({sources:[{file,sha256:crypto.createHash('sha256').update(saved).digest('hex')}],baseline:{original_observations:41,native_scope_assertions:2,assertions:43,tests:42},faults:results,gold_unchanged:true,forEach_unchanged_by_faults:true},null,2)+'\n');
