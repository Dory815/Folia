// Root acceptance: build isolated copies, never mutate the working sources.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-factory-mutations');
const modules=['core/tree/property.ixx',"util/callable_types.ixx","util/object_types.ixx",'util/option_types.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/wrapper_factory.ixx'];
const copies=new Map();fs.mkdirSync(work,{recursive:true});
// Reject overlapping runs: sharing Ninja output would invalidate evidence.
const lock=path.join(work,'run.lock');
const lockFd=fs.openSync(lock,'wx');fs.writeFileSync(lockFd,String(process.pid));
process.on('exit',()=>{fs.closeSync(lockFd);fs.unlinkSync(lock);});
for(const file of modules){const source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n');const target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_factory_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "${slash(path.join(root,'tests/math/reference_factory.cpp'))}")
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
function build(name){const p=cp.spawnSync('cmd.exe',['/d','/c',path.join(work,'build.cmd')],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(p.stdout||'')+(p.stderr||''));assert.equal(p.status,0,name+' must compile; inspect ignored log');}
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),[],{encoding:'utf8',timeout:15000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Factory baseline compiles and passes.');
const factory='core/tree/factory.ixx';
const mutations=[
 {name:'generic-fallback-dropped',file:factory,from:'const auto fallback = creator(defaultKind);',to:'const auto fallback = creator("absent-never-registered");'},
 {name:'direct-dispatch-uses-fallback',file:factory,from:'if (requested.kind != CreatorLookupKind::creator) type_error("Factory creator is not callable");',to:'if (requested.kind != CreatorLookupKind::creator) return AbstractFactory<Product>::create(kind,args);'},
 {name:'captured-class-reread-from-map',file:factory,from:'[this, type = std::move(type)](FactoryArgs args) -> Product* {',to:'[this, key = kind](FactoryArgs args) -> Product* { auto type = getNodeClass(key);'},
 {name:'map-order-reversed',file:factory,from:'return kinds;',to:'std::reverse(kinds.begin(),kinds.end()); return kinds;'},
 {name:'prototype-creator-deleted',file:factory,from:'if (found != nodeMap_.end()) nodeMap_.erase(found);',to:'if (found != nodeMap_.end()) nodeMap_.erase(found); if(kind=="__proto__"){node_->prototypeCreator.reset();node_->prototypeAssigned=false;}'},
 {name:'function-prototype-falls-back',file:factory,from:'if (node_->prototypeAssigned && isFunctionPrototypeMember(kind))',to:'if (false && node_->prototypeAssigned && isFunctionPrototypeMember(kind))'},
 {name:'node-properties-identity-dropped',file:'core/tree/node_factory.ixx',from:'std::any(std::move(properties)), std::any(std::move(children))',to:'std::any(PropertyList{}), std::any(std::move(children))'},
 {name:'wrapper-kind-read-twice',file:'core/tree/wrapper_factory.ixx',from:'const std::string kind = node->kind();',to:'const std::string kind = node->kind(); (void)node->kind();'}
];
const results=[];
for(const m of mutations){const {source,target}=copies.get(m.file);assert.ok(source.includes(m.from),'Missing anchor '+m.name);
 try{fs.writeFileSync(target,source.replace(m.from,m.to));build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must produce assertion failure, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');
console.log('All eight factory mutations detected; working sources unchanged.');
