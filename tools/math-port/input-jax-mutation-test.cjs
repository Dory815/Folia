// Root independent compiling faults; run only with body agents idle.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),ctor='src/math/core/input_jax_constructor.cpp',body='src/math/core/input_jax.cpp',host='src/math/core/input_jax_host.ixx';
const saved=new Map([ctor,body,host].map(file=>[file,fs.readFileSync(path.join(root,file),'utf8')]));
const faults=[
 ['constructor-class-not-captured',ctor,[['context.get(klass,options::Key(u"OPTIONS"))','context.get(context.get(options::Value(identity_),options::Key(u"constructor")),options::Key(u"OPTIONS"))']]],
 ['class-options-used-as-instance-options',ctor,[['options::defaultOptions(context,destination,defaults)','classOptions']]],
 ['user-merge-skipped',ctor,[['options::userOptions(context,defaulted,userDefinitions)','defaulted']]],
 ['options-published-before-merge-completes',ctor,[['const options::Value classOptions=context.get(klass,options::Key(u"OPTIONS"));','const options::Value classOptions=context.get(klass,options::Key(u"OPTIONS"));\n    options=destination;']]],
 ['post-filter-list-reuses-pre-list',ctor,[['postFilters=makeOptionFunctionList(bindings_,post);','postFilters=preFilters;']]],
 ['name-returned-as-static-generic',body,[['return context.get(constructor,options::Key(u"NAME"));','return options::Value(u"generic");']]],
 ['processStrings-false',body,[['processStrings()const{return true;}','processStrings()const{return false;}']]],
 ['adaptor-identity-discarded',body,[['adaptor=std::move(value);','adaptor=PropertyNull{};']]],
 ['factory-identity-discarded',body,[['mmlFactory=std::move(value);','mmlFactory=PropertyNull{};']]],
 ['reset-clears-option-state',body,[['void AbstractInputJax::reset(FunctionArgs){}','void AbstractInputJax::reset(FunctionArgs){options=options::Undefined{};}']]],
 ['findMath-array-reused',body,[['{return std::make_shared<std::vector<ProtoMathItem>>();}','{static auto result=std::make_shared<std::vector<ProtoMathItem>>();return result;}']]],
 ['filter-argument-record-copied',body,[['std::array<std::any,1> arguments{InputJaxFilterDataRef(record)};','std::array<std::any,1> arguments{std::make_shared<InputJaxFilterData>(*record)};']]],
 ['filter-original-data-returned',body,[['record->data=std::move(value);','const auto original=value;\n    record->data=std::move(value);'],['return record->data;','return original;']]],
 ['filter-exceptions-swallowed',body,[['filters.execute(arguments);','try{filters.execute(arguments);}catch(...){}']]],
 ['original-class-static-name-wrong',host,[['options::Key(u"NAME"),u"generic"','options::Key(u"NAME"),u"wrong"']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_input_jax*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p48-input-jax-'+label+'.log'),log);return{status:r.status,log};}
const results=[];try{const b=run('mutation-baseline');assert.equal(b.status,0);assert.match(b.log,/30 asserts in 30 tests/);for(const[id,file,edits]of faults){let changed=saved.get(file);for(const[from,to]of edits){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,file,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved.get(file));console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}}finally{for(const[file,bytes]of saved)fs.writeFileSync(path.join(root,file),bytes);}assert.equal(run('mutation-restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p48-input-jax-mutations.json'),JSON.stringify({sources:[...saved].map(([file,bytes])=>({file,sha256:crypto.createHash('sha256').update(bytes).digest('hex')})),faults:results},null,2)+'\n');
