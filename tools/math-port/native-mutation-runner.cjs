// Root-owned isolated C++23 mutation runner. Expected values never change.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
module.exports=function runPacket({name,modules,units,fixture,pattern,assertions,tests,mutants,baselineOutcome='green'}){
 assert.ok(['green','red','compile'].includes(baselineOutcome));
 const root=path.resolve(__dirname,'../..'),work=path.join(root,'build',name);
 fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');
 fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
 const copies=new Map(),originalFixture=fs.readFileSync(path.join(root,'tests/math',fixture));
 function writeChanged(target,bytes){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(bytes)))fs.writeFileSync(target,bytes);}
 writeChanged(path.join(work,'reference.cpp'),originalFixture);
 for(const file of [...modules,...units]){
  const original=fs.readFileSync(path.join(root,'src/math',file)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);
  fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(file,{original,source,target});
 }
 const slash=p=>p.replaceAll('\\','/');
 writeChanged(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(root_native_mutations LANGUAGES CXX)
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
${baselineOutcome==='compile'?`target_sources(math PRIVATE reference.cpp)
target_include_directories(math PRIVATE "${slash(path.join(root,'tests'))}")
target_link_libraries(math PRIVATE Boost::ut_module)`:''}
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" reference.cpp)
set_target_properties(MutationTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(MutationTests PRIVATE "${slash(path.join(root,'tests'))}")
target_link_libraries(MutationTests PRIVATE math Boost::ut_module)
target_link_options(MutationTests PRIVATE /STACK:16777216)
`);
 writeChanged(path.join(work,'build.cmd'),`@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe"
for /f "usebackq delims=" %%I in (\`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath\`) do set "VSINSTALL=%%I"
if not defined VSINSTALL exit /b 1
call "%VSINSTALL%\\VC\\Auxiliary\\Build\\vcvars64.bat" >nul
cmake -Wno-dev -G Ninja -S "${work}" -B "${work}\\bin"
if errorlevel 1 exit /b %errorlevel%
cmake --build "${work}\\bin" --target ${baselineOutcome==='compile'?'math':'MutationTests'}
exit /b %errorlevel%
`.replaceAll('\n','\r\n'));
 function build(label){const p=cp.spawnSync('cmd.exe',['/d','/c',path.join(work,'build.cmd')],{encoding:'utf8'});fs.writeFileSync(path.join(work,label+'-build.log'),(p.stdout||'')+(p.stderr||''));assert.equal(p.status,0,label+' must compile');}
 function test(label){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),[pattern],{encoding:'utf8',timeout:30000,maxBuffer:64*1024*1024});fs.writeFileSync(path.join(work,label+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
 const only=process.argv.find(x=>x.startsWith('--only='))?.slice(7),selected=only?mutants.filter(m=>m.name===only):mutants,results=[];
 assert.ok(selected.length||baselineOutcome==='red'||baselineOutcome==='compile','Unknown mutation');build('baseline');
 if(baselineOutcome==='compile'){
  for(const[file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working source changed: '+file);
  assert.ok(originalFixture.equals(fs.readFileSync(path.join(root,'tests/math',fixture))),'Working fixture changed');
  fs.writeFileSync(path.join(work,'compile-result.json'),JSON.stringify({compiles:true,modules:modules.length,fixture_compiled:true,semantic_tests_executed:false,working_files_unchanged:true},null,2)+'\n');console.log('Concrete declaration graph and static contracts compile; no semantic test executed.');return;
 }
 const baseline=test('baseline');
 if(baselineOutcome==='red'){
  assert.equal(baseline.status,1,'Red baseline must fail assertions, not crash');assert.match(baseline.stdout||'',/FAILED|failed|FAIL/);
  assert.ok((baseline.stdout||'').includes('Unimplemented'),'Real frozen stubs must execute');
  fs.writeFileSync(path.join(work,'red-result.json'),JSON.stringify({compiles:true,test_exit:baseline.status,real_stubs_execute:true},null,2)+'\n');console.log('Real red baseline compiled and failed through frozen stubs.');return;
 }
 assert.equal(baseline.status,0,'Baseline must pass');
 assert.ok(baseline.stdout.includes(`${assertions} asserts in ${tests} tests`),'Nonzero complete baseline');console.log(`Baseline: ${assertions} assertions in ${tests} tests pass.`);
 for(const m of selected){
  const {source,target}=copies.get(m.file);let changed=source;
  for(const[from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}
  assert.notEqual(changed,source,'Mutation must change implementation');
  try{fs.writeFileSync(target,changed);build(m.name);const p=test(m.name);assert.equal(p.status,1,'Must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}
  finally{fs.writeFileSync(target,source);}
 }
 for(const[file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working source changed: '+file);
 assert.ok(originalFixture.equals(fs.readFileSync(path.join(root,'tests/math',fixture))),'Working fixture changed');
 fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify({baseline:{assertions,tests},mutants:results,working_files_unchanged:true},null,2)+'\n');
 console.log(`All ${selected.length} compiling mutations detected; working files unchanged.`);
};
