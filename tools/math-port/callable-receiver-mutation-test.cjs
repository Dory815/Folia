// Root isolated host mutants: compile and fail assertions, never working files.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-callable-receiver-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','util/object_types.ixx','util/callable_types.ixx','util/prioritized_list.ixx','util/promise.ixx','util/function_types.ixx','util/function_list.ixx','input/tex/token.ixx','input/tex/types.ixx','input/tex/macro.ixx'];
const units=['util/promise.cpp',"util/function_value.cpp",'util/function_list.cpp','input/tex/token.cpp','input/tex/macro.cpp','input/tex/parse_callable.cpp'];
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixture=fs.readFileSync(path.join(root,'tests/math/reference_callable_receiver.cpp'));
function writeChanged(target,bytes){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(bytes)))fs.writeFileSync(target,bytes);}
writeChanged(path.join(work,'reference.cpp'),fixture);
for(const file of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',file)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_callable_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" reference.cpp)
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
function build(name){const p=cp.spawnSync('cmd.exe',['/d','/c',path.join(work,'build.cmd')],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(p.stdout||'')+(p.stderr||''));assert.equal(p.status,0,name+' must compile');}
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_callable*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutants=[],bridge='input/tex/parse_callable.cpp',types='util/function_types.ixx';
const add=(name,file,from,to)=>mutants.push({name,file,edits:[[from,to]]});
const call='return body(FunctionReceiver{std::static_pointer_cast<NativeObject>(receiver)},args);';
add('record-receiver-erased',types,call,'return body(FunctionReceiver{Missing{}},args);');
add('record-receiver-copied',types,call,'return body(FunctionReceiver{std::make_shared<FunctionListItem>(*receiver)},args);');
add('typed-receiver-erased',bridge,'return flatten_result(body(receiver, parser->get(), *atom, rest));','return flatten_result(body(FunctionReceiver{Missing{}}, parser->get(), *atom, rest));');
add('null-receiver-accepted',bridge,'if (!*object) invalid_callable(u"ParseMethod receiver is null");','if (!*object) return;');
add('typed-false-boxed',bridge,'return std::any(value);','return std::any(folia::math::tex::ParseResult{value});');
add('typed-result-erased',bridge,'return std::any(value);','return std::any(folia::math::Property(folia::math::Missing{}));');
add('wrong-protocol-error-category',bridge,'FactoryError("TypeError", std::move(message))','FactoryError("RangeError", std::move(message))');
add('undefined-result-rejected',bridge,'if (!result.has_value()) return folia::math::Property(folia::math::Missing{});','if (!result.has_value()) invalid_callable(u"Undefined result rejected");');
add('raw-bool-result-accepted',bridge,'if (!result.has_value()) return folia::math::Property(folia::math::Missing{});','if (const auto* b=std::any_cast<bool>(&result)) return folia::math::Property(*b);if (!result.has_value()) return folia::math::Property(folia::math::Missing{});');
add('rest-property-rewritten',bridge,'for (const auto& value : rest) slots.push_back(value);','for (const auto& value : rest) {if(std::any_cast<folia::math::Property>(&value))slots.emplace_back(folia::math::Property(999));else slots.push_back(value);}');
add('callable-exceptions-swallowed',bridge,'const std::any result = body(receiver, FunctionArgs(slots));','std::any result;try{result=body(receiver,FunctionArgs(slots));}catch(...){result=folia::math::Property(false);}');
add('closure-state-copied','util/callable_types.ixx','const auto active=callback_;','const auto active=callback_?std::make_shared<Callback>(*callback_):nullptr;');
const only=process.argv.find(x=>x.startsWith('--only='))?.slice(7),selected=only?mutants.filter(x=>x.name===only):mutants,results=[];assert.ok(selected.length,'Unknown mutation');
build('baseline');const baseline=run('baseline');assert.equal(baseline.status,0,'Baseline must pass');assert.match(baseline.stdout,/79 asserts in 9 tests/,'Nonzero complete baseline');console.log('Callable baseline: 79 assertions pass.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const[from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const[file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working implementation changed: '+file);
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_callable_receiver.cpp'))),'Working fixture changed');
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');console.log(`All ${selected.length} callable mutations detected; working files unchanged.`);
