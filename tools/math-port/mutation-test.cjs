// Root acceptance only: mutate isolated copies, never the working source tree.
const fs = require('node:fs');
const path = require('node:path');
const cp = require('node:child_process');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const work = path.join(root, 'build/math-port-mutations');
const nativeFiles = ['util/option_types.ixx', 'core/tree/property.ixx',"util/callable_types.ixx", 'core/mml_tree/attributes.ixx',
  'util/constants.ixx', 'util/bbox.ixx', 'util/object_types.ixx', 'util/prioritized_list.ixx'];
fs.mkdirSync(work, {recursive:true});
const copies = new Map();
for (const relative of nativeFiles) {
  const source = fs.readFileSync(path.join(root, 'src/math', relative), 'utf8');
  const target = path.join(work, 'src', relative);
  fs.mkdirSync(path.dirname(target), {recursive:true});
  fs.writeFileSync(target, source);
  copies.set(relative, {source, target});
}
const slash = p => p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'), `
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_port_mutations LANGUAGES CXX)
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
target_sources(math PUBLIC FILE_SET CXX_MODULES FILES ${nativeFiles.map(f=>'"src/'+f+'"').join(' ')})
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "${slash(path.join(root,'tests/math/reference_primitives.cpp'))}")
set_target_properties(MutationTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(MutationTests PRIVATE "${slash(path.join(root,'tests'))}")
target_link_libraries(MutationTests PRIVATE math Boost::ut_module)
`);
fs.writeFileSync(path.join(work,'build.cmd'), `@echo off\r
setlocal\r
set "VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe"\r
for /f "usebackq delims=" %%I in (\`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath\`) do set "VSINSTALL=%%I"\r
if not defined VSINSTALL exit /b 1\r
call "%VSINSTALL%\\VC\\Auxiliary\\Build\\vcvars64.bat" >nul\r
cmake -Wno-dev -G Ninja -S "${work}" -B "${work}\\bin"\r
if errorlevel 1 exit /b %errorlevel%\r
cmake --build "${work}\\bin" --target MutationTests\r
exit /b %errorlevel%\r
`);
function build(name) {
  const result = cp.spawnSync('cmd.exe', ['/d','/c',path.join(work,'build.cmd')],{encoding:'utf8'});
  fs.writeFileSync(path.join(work,name+'-build.log'), (result.stdout||'')+(result.stderr||''));
  assert.equal(result.status,0,`${name} must compile; inspect build/math-port-mutations/${name}-build.log`);
}
function run(name) {
  const result = cp.spawnSync(path.join(work,'bin/MutationTests.exe'),[],{encoding:'utf8'});
  fs.writeFileSync(path.join(work,name+'-test.log'), (result.stdout||'')+(result.stderr||''));
  return result;
}
build('baseline');
const baseline=run('baseline');
assert.equal(baseline.status,0,'Unmutated primitives must pass');
assert.match(baseline.stdout,/1223 asserts in 3 tests/,'Complete nonzero primitive baseline');
console.log('Baseline compiles and passes frozen reference tests.');
const mutations = [
  {name:'attribute-inheritance-dropped', file:'core/mml_tree/attributes.ixx',
    from:'return inherited_.get(name);', to:'return inherited_.getOwn(name);'},
  {name:'bbox-left-spacing-dropped', file:'util/bbox.ixx',
    from:'cbox.w + cbox.L + cbox.R', to:'cbox.w + cbox.R'},
  {name:'priority-ties-reversed', file:'util/prioritized_list.ixx',
    from:'priority < (*items_)[index - 1]->priority', to:'priority <= (*items_)[index - 1]->priority'},
  {name:'priority-default-split-by-type', file:'util/prioritized_list.ixx',
    from:'inline static double& DEFAULTPRIORITY = detail::prioritized_list_default_priority;',
    to:'inline static double DEFAULTPRIORITY = 5;'}
];
const results=[];
for (const mutation of mutations) {
  const {source,target} = copies.get(mutation.file);
  assert.ok(source.includes(mutation.from),`Mutation anchor missing: ${mutation.name}`);
  try {
    fs.writeFileSync(target,source.replace(mutation.from,mutation.to));
    build(mutation.name);
    const result=run(mutation.name);
    assert.equal(result.status,1,`Mutation must fail assertions, not crash: ${mutation.name}`);
    assert.match(result.stdout||'',/FAILED|failed|FAIL/,'Expected an assertion failure, not an unrelated crash');
    results.push({mutation:mutation.name,compiles:true,test_exit:result.status,detected:true});
    console.log(`Detected ${mutation.name}: compiled, failed frozen assertions.`);
  } finally {fs.writeFileSync(target,source);}
}
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');
console.log('All four native mutations detected. Working-tree native sources were never modified.');
