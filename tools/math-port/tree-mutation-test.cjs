// Root acceptance: build isolated copies, never mutate the working sources.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-tree-mutations');
const modules=['core/tree/property.ixx',"util/callable_types.ixx","util/object_types.ixx",'util/option_types.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/wrapper_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','core/tree/wrapper_types.ixx','core/tree/wrapper.ixx'];
const copies=new Map();fs.mkdirSync(work,{recursive:true});
// Reject overlapping runs: sharing Ninja output would invalidate evidence.
const lock=path.join(work,'run.lock');
const lockFd=fs.openSync(lock,'wx');fs.writeFileSync(lockFd,String(process.pid));
process.on('exit',()=>{fs.closeSync(lockFd);fs.unlinkSync(lock);});
const fixtureSource=fs.readFileSync(path.join(root,'tests/math/reference_tree.cpp'));
fs.writeFileSync(path.join(work,'reference_tree.cpp'),fixtureSource);
for(const file of modules){const source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n');const target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_tree_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_tree.cpp")
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
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Tree baseline compiles and passes.');
const node='core/tree/node.ixx',wrapper='core/tree/wrapper.ixx';
const mutations=[
 {name:'constructor-virtual-setter-bypassed',file:node,from:'this->setProperty(name, inputProperties->getOwn(name));',to:'properties.set(name,inputProperties->getOwn(name));'},
 {name:'setChildren-length-cached',file:node,from:'for (std::size_t index = 0; index < children->size(); ++index)',to:'const auto frozen_length=children->size(); for (std::size_t index = 0; index < frozen_length; ++index)'},
 {name:'live-properties-handle-lost',file:node,from:'return properties;',to:'return PropertyList{};'},
 {name:'append-null-partial-push-lost',file:node,from:'this->childNodes->push_back(child);',to:'if (!child) type_error("null"); this->childNodes->push_back(child);'},
 {name:'replace-return-old',file:node,from:'return newChild;',to:'return oldChild;'},
 {name:'remove-foreign-parent-retained',file:node,from:'child->parent = nullptr;',to:'if(child->parent==static_cast<Product*>(this)) child->parent = nullptr;'},
 {name:'copy-spread-instead-runtime-assignment',file:node,from:'result->properties.assign(properties);',to:'for(const auto& key:properties.keys())result->properties.setOwn(key,properties.getOwn(key));'},
 {name:'copy-current-kind-lost',file:node,from:'this->factory.create(this->kind(), {});',to:'this->factory.create("a", {});'},
 {name:'walk-child-virtual-recursion-lost',file:node,from:'if (child) child->walkTree(visit, data);',to:'if (child) visit(child, data);'},
 {name:'join-captured-array-lost',file:node,from:'const std::string kindName = this->kind();\n        const auto children = this->childNodes;',to:'const std::string kindName = this->kind();\n        const auto& children = this->childNodes;'},
 {name:'join-length-made-live',file:node,from:'index < count; ++index',to:'index < children->size(); ++index'},
 {name:'join-cycle-suppression-lost',file:node,from:'if (!joinGuard.entered()) return kindName + "()";',to:'if (false && !joinGuard.entered()) return kindName + "()";'},
 {name:'native-budget-premature-limit',file:'core/tree/node_types.ixx',from:'limit=512;',to:'limit=511;'},
 {name:'wrapper-forwards-dropped-extras',file:wrapper,from:'Product* wrap(Source* source, FactoryArgs = {}) override {\n        return this->factory.wrap(source, FactoryArgs{});',to:'Product* wrap(Source* source, FactoryArgs args = {}) override {\n        return this->factory.wrap(source, args);'},
 {name:'wrapper-absent-children-throws',file:wrapper,from:'if (!this->childNodes) return data;',to:'if (!this->childNodes) throw FactoryError("TypeError",u"absent children");'}
];
const results=[];
for(const m of mutations){const {source,target}=copies.get(m.file);assert.ok(source.includes(m.from),'Missing anchor '+m.name);
 try{fs.writeFileSync(target,source.replace(m.from,m.to));build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must produce assertion failure, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');
assert.ok(fixtureSource.equals(fs.readFileSync(path.join(root,'tests/math/reference_tree.cpp'))),'Working fixture changed during acceptance run');
console.log('All fifteen tree mutations detected; working sources unchanged.');
