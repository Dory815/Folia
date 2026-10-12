// Root isolated compiling mutations; working sources/expectations are byte guarded.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-function-list-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','util/object_types.ixx','util/callable_types.ixx','util/prioritized_list.ixx','util/promise.ixx','util/function_types.ixx','util/function_list.ixx'],units=['util/promise.cpp',"util/function_value.cpp",'util/function_list.cpp'];
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_function_list.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_function_mutations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/utf-8 /permissive- /Zc:preprocessor /Zc:__cplusplus)
set(BOOST_UT_DISABLE_MODULE OFF CACHE BOOL "" FORCE)
set(BOOST_UT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BOOST_UT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BOOST_UT_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${slash(path.join(root,'third_party/ut'))}" ut)
add_library(math STATIC)
set_target_properties(math PROPERTIES CXX_MODULE_STD ON)
target_sources(math PUBLIC FILE_SET CXX_MODULES FILES ${modules.map(f=>'"src/'+f+'"').join(' ')})
target_sources(math PRIVATE ${units.map(f=>'"src/'+f+'"').join(' ')})
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" reference_function_list.cpp)
set_target_properties(MutationTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(MutationTests PRIVATE "${slash(path.join(root,'tests'))}")
target_link_libraries(MutationTests PRIVATE math Boost::ut_module)
target_link_options(MutationTests PRIVATE /STACK:16777216)
`);
fs.writeFileSync(path.join(work,'build.cmd'),`@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe"
for /f "usebackq delims=" %%I in (\`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath\`) do set "VSINSTALL=%%I"
if not defined VSINSTALL exit /b 1
call "%VSINSTALL%\\VC\\Auxiliary\\Build\\vcvars64.bat" >nul
cmake -Wno-dev -G Ninja -S "${work}" -B "${work}\\bin"
if errorlevel 1 exit /b %errorlevel%
cmake --build "${work}\\bin" --target MutationTests
exit /b %errorlevel%
`.replaceAll('\n','\r\n'));
function build(name){const p=cp.spawnSync('cmd.exe',['/d','/c',path.join(work,'build.cmd')],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(p.stdout||'')+(p.stderr||''));assert.equal(p.status,0,name+' must compile; inspect ignored log');}
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_function*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[];
const add=(name,file,from,to)=>mutations.push({name,file,edits:[[from,to]]});
const body='util/function_list.cpp',types='util/function_types.ixx',priority='util/prioritized_list.ixx',promise='util/promise.cpp';
add('constructor-skipped',body,'if (list) addList(std::move(list));','if (false && list) addList(std::move(list));');
add('constructor-empty-skipped',body,'if (list) addList(std::move(list));','if (list && !list->empty()) addList(std::move(list));');
add('constructor-virtual-bypassed',body,'if (list) addList(std::move(list));','if (list) FunctionList::addList(std::move(list));');
add('definitions-reversed',body,'const FunctionDefinition definition=source.read(index);','const FunctionDefinition definition=source.read(source.length()-1-index);');
add('definitions-growth-ignored',body,'for(std::size_t index=0;index<source.length();++index){','const auto limit=source.length();for(std::size_t index=0;index<std::min(limit,source.length());++index){');
add('direct-default-frozen',body,'this->add(*direct);','this->add(*direct,5);');
add('tuple-priority-ignored',body,'if(priority)this->add(item,*priority);','if(priority)this->add(item);');
add('omitted-priority-zero',body,'else this->add(item);','else this->add(item,0);');
add('sync-false-ignored',body,'if (functions::isFalse(result)) return false;','if (false && functions::isFalse(result)) return false;');
add('iterator-array-copied',priority,'storage_(owner ? owner->items_ : nullptr)','storage_(owner && owner->items_ ? std::make_shared<Storage>(*owner->items_) : nullptr)');
add('null-array-error-category',body,'if (!itemStorage()) functions::typeError(u"FunctionList item array is null");','if (!itemStorage()) throw tree::FactoryError("RangeError",u"FunctionList item array is null");');
add('receiver-priority-overwritten',types,'return body(FunctionReceiver{std::static_pointer_cast<NativeObject>(receiver)},args);','receiver->priority=0;return body(FunctionReceiver{std::static_pointer_cast<NativeObject>(receiver)},args);');
add('argument-slot-rewritten',types,'return body(FunctionReceiver{std::static_pointer_cast<NativeObject>(receiver)},args);','std::vector<std::any> changed(args.begin(),args.end());if(changed.size()>1)changed[1]=Property(999);return body(FunctionReceiver{std::static_pointer_cast<NativeObject>(receiver)},changed);');
add('undefined-stops',types,'return *p==Property(false);','return p->missing() || *p==Property(false);');
add('async-first-item-skipped',body,'frame->index = -1;','frame->index = 0;');
add('async-array-copied',body,'frame->items = itemStorage();','frame->items = itemStorage() ? std::make_shared<PrioritizedList<FunctionValue>::Storage>(*itemStorage()) : nullptr;');
add('promise-false-stops',body,'[frame](const async::Value&) -> async::Value {\n                    resumeAsync(frame);','[frame](const async::Value& value) -> async::Value {\n                    if(functions::isFalse(value)){frame->output->resolve(false);return {};}resumeAsync(frame);');
add('rejection-link-shortened',body,'resumeAsync(frame);\n                    return {};\n                });','resumeAsync(frame);\n                    return {};\n                },[output=frame->output](const async::Value& reason)->async::Value{output->reject(reason);return {};});');
add('async-direct-false-true',body,'frame->output->resolve(std::any(false));','frame->output->resolve(std::any(true));');
add('jobs-inline',promise,'jobs_.push_back(std::move(job));','job();');
add('fulfilled-no-handler-rejects',promise,'if(state==State::fulfilled)reaction.next->resolve(value);','if(state==State::fulfilled)reaction.next->reject(value);');
add('catch-attached-to-fulfillment',promise,'return then({},std::move(rejected));','return then(std::move(rejected),{});');
add('handler-return-ignored',promise,'reaction.next->resolve(handler(value));','handler(value);reaction.next->resolve(Value{});');
add('resolve-first-settlement-ignored',promise,'if(current.resolved)return;','if(current.resolved){}');
add('reject-first-settlement-ignored',promise,'void Promise::reject(Value reason){\n    auto& current=record();\n    if(current.resolved)return;','void Promise::reject(Value reason){\n    auto& current=record();\n    if(current.resolved){}');
add('adoption-job-inline',promise,'queue->post([self,adopted]{\n            try{\n                adopted->then','auto adopt=[self,adopted]{\n            try{\n                adopted->then');
mutations.at(-1).edits.push(['}catch(...){self->settle(State::rejected,currentReason());}\n        });','}catch(...){self->settle(State::rejected,currentReason());}\n        };adopt();']);
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('FunctionList baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const[from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const[f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const[f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} FunctionList mutations detected; working sources and fixtures unchanged.`);
