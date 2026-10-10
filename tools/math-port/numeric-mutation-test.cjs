// Root acceptance only: mutate isolated copies, never the working source tree.
const fs = require('node:fs');
const path = require('node:path');
const cp = require('node:child_process');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const work = path.join(root, 'build/math-port-numeric-mutations');
const nativeFiles = ['core/tree/property.ixx','util/constants.ixx','util/js_number.ixx','util/lengths.ixx'];
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "${slash(path.join(root,'tests/math/reference_numeric.cpp'))}")
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
  assert.equal(result.status,0,`${name} must compile; inspect build/math-port-numeric-mutations/${name}-build.log`);
}
function run(name) {
  const result = cp.spawnSync(path.join(work,'bin/MutationTests.exe'),[],{encoding:'utf8'});
  fs.writeFileSync(path.join(work,name+'-test.log'), (result.stdout||'')+(result.stderr||''));
  return result;
}
build('baseline');
assert.equal(run('baseline').status,0,'Unmutated numeric prerequisites must pass');
console.log('Baseline compiles and passes frozen reference tests.');
const mutations = [
  {name:'shortest-notation-threshold',file:'util/js_number.ixx',from:'scientific_exponent >= -6',to:'scientific_exponent >= -5'},
  {name:'fixed-half-rounding-dropped',file:'util/js_number.ixx',from:'denominator_power != 0 && integer.bit(denominator_power - 1)',to:'false'},
  {name:'absolute-unit-scale-dropped',file:'util/lengths.ixx',from:'(m * factor) / em / scale',to:'(m * factor) / em'},
  {name:'em-threshold-expanded',file:'util/lengths.ixx',from:'std::abs(m) < 0.001',to:'std::abs(m) < 0.01'},
  {name:'percent-reference-size-dropped',file:'util/lengths.ixx',from:'(m / 100) * size',to:'(m / 100)'},
  {name:'numeric-input-prefix-bypassed',file:'util/lengths.ixx',from:'return length2em(js_number::toString(length), size, scale, em);',to:'return Property(length * size);'}
];
const results=[];
for (const mutation of mutations) {
  const {source,target} = copies.get(mutation.file);
  assert.ok(source.includes(mutation.from),`Mutation anchor missing: ${mutation.name}`);
  try {
    fs.writeFileSync(target,source.replace(mutation.from,mutation.to));
    build(mutation.name);
    const result=run(mutation.name);
    assert.notEqual(result.status,0,`Oracle failed to detect ${mutation.name}`);
    assert.match(result.stdout||'',/FAILED|failed|FAIL/,'Expected an assertion failure, not an unrelated crash');
    results.push({mutation:mutation.name,compiles:true,test_exit:result.status,detected:true});
    console.log(`Detected ${mutation.name}: compiled, failed frozen assertions.`);
  } finally {fs.writeFileSync(target,source);}
}
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');
console.log('All six native numeric mutations detected. Working-tree native sources were never modified.');
