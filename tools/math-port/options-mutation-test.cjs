// Root acceptance: build isolated native copies against unchanged JS fixtures.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-options-mutations');
const modules=['util/option_types.ixx','util/options.ixx'];
const units=['util/option_object.cpp','util/option_array.cpp'];
const copies=new Map();
fs.mkdirSync(work,{recursive:true});
for(const file of [...modules,...units,'util/option_storage.hpp']){
  const source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n');
  const target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{source,target});
}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_options_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "${slash(path.join(root,'tests/math/reference_options.cpp'))}")
set_target_properties(MutationTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(MutationTests PRIVATE "${slash(path.join(root,'tests'))}")
target_link_libraries(MutationTests PRIVATE math Boost::ut_module)
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
function build(name){
  const p=cp.spawnSync('cmd.exe',['/d','/c',path.join(work,'build.cmd')],{encoding:'utf8'});
  fs.writeFileSync(path.join(work,name+'-build.log'),(p.stdout||'')+(p.stderr||''));
  assert.equal(p.status,0,name+' must compile; inspect ignored build log');
}
function run(name){
  const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),[],{encoding:'utf8',timeout:15000});
  fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;
}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Options baseline compiles and passes.');
const mutations=[
  {name:'nonenumerable-symbol-dropped',file:'util/option_object.cpp',from:'result.push_back(*symbol_ref);',to:'if (descriptor(object_value,key)->enumerable) result.push_back(*symbol_ref);'},
  {name:'inherited-getter-wrong-receiver',file:'util/option_object.cpp',from:'return call(*property->getter, object_value, {});',to:'return call(*property->getter, current, {});'},
  {name:'truncation-writable-rollback-dropped',file:'util/option_object.cpp',from:'if (!new_descriptor.writable) length->second.writable = false;',to:'/* dropped writable rollback */'},
  {name:'spread-length-cached',file:'util/option_array.cpp',from:'for (std::uint64_t index = 0; index < arrayLength(object); ++index)',to:'const auto frozen_length=arrayLength(object); for (std::uint64_t index = 0; index < frozen_length; ++index)'},
  {name:'indexof-samevalue',file:'util/option_array.cpp',from:'get(Value(object), key) == needle',to:'sameValue(get(Value(object), key), needle)'},
  {name:'indexof-inherited-index-dropped',file:'util/option_array.cpp',from:'has(Value(object), key) && get(Value(object), key) == needle',to:'hasOwn(Value(object), key) && get(Value(object), key) == needle'},
  {name:'constructor-second-read-dropped',file:'util/options.ixx',from:'return context.get(value, constructor_key) == Value(context.intrinsics().expandableConstructor);',to:'return false;'},
  {name:'remainder-returned-last',file:'util/options.ixx',from:'results.insert(results.begin(), std::move(options));',to:'results.push_back(std::move(options));'}
];
const results=[];
for(const mutation of mutations){
  const {source,target}=copies.get(mutation.file);assert.ok(source.includes(mutation.from),'Missing anchor '+mutation.name);
  try{fs.writeFileSync(target,source.replace(mutation.from,mutation.to));build(mutation.name);const p=run(mutation.name);
    assert.equal(p.status,1,'Mutation must fail tests with an assertion exit, not a crash: '+mutation.name);
    assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:mutation.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+mutation.name);
  }finally{fs.writeFileSync(target,source);}
}
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');
console.log('All eight native option mutations detected; working sources unchanged.');
