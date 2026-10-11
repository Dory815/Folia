// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-case-host-mutations');
const modules=['util/utf16_types.ixx','util/utf16.ixx','util/js_case.ixx'];
const units=['util/js_case.cpp','util/js_case_data.cpp'];
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});

const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_case_host.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_case_host_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" ${[...fixtures.keys()].map(f=>'"'+f+'"').join(' ')})
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_case_host*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "mapping-ignored",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "const auto mapped=detail::lowerMapping(point);",
        "const std::span<const char32_t> mapped;"
      ]
    ]
  },
  {
    "name": "supplementary-encoding-offset",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "point-=0x10000;",
        "/* offset omitted */"
      ]
    ]
  },
  {
    "name": "lone-surrogate-replaced",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "if(point<=0xFFFF){",
        "if(point>=0xD800&&point<=0xDFFF)point=0xFFFD;if(point<=0xFFFF){"
      ]
    ]
  },
  {
    "name": "sigma-always-normal",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "point==0x03A3&&finalSigma(points,i)",
        "false&&point==0x03A3&&finalSigma(points,i)"
      ]
    ]
  },
  {
    "name": "sigma-preceding-case-ignored",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "if(!before)return false;",
        "/* preceding cased requirement omitted */"
      ]
    ]
  },
  {
    "name": "sigma-before-ignorable-not-skipped",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "if(flags&2)continue;",
        "/* case-ignorable not skipped */"
      ]
    ]
  },
  {
    "name": "sigma-after-ignorable-not-skipped",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "const auto flags=detail::caseProperties(points[j]);\n        if(flags&2)continue;",
        "const auto flags=detail::caseProperties(points[j]);"
      ]
    ]
  },
  {
    "name": "sigma-following-case-ignored",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "for(std::size_t j=index+1;j<points.size();++j)",
        "for(std::size_t j=points.size();j<points.size();++j)"
      ]
    ]
  },
  {
    "name": "sigma-cased-ignorable-wrong-priority",
    "file": "util/js_case.cpp",
    "edits": [
      [
        "const auto flags=detail::caseProperties(points[j]);\n        if(flags&2)continue;",
        "const auto flags=detail::caseProperties(points[j]);if(flags&1)return false;\n        if(flags&2)continue;"
      ]
    ]
  },
  {
    "name": "lowercase-expansion-truncated",
    "file": "util/js_case_data.cpp",
    "edits": [
      [
        "{304,{105,775,0},2}",
        "{304,{105,775,0},1}"
      ]
    ]
  },
  {
    "name": "supplementary-mapping-omitted",
    "file": "util/js_case_data.cpp",
    "edits": [
      [
        "{66560,{66600,0,0},1}",
        "{66560,{66560,0,0},1}"
      ]
    ]
  },
  {
    "name": "ascii-cased-property-omitted",
    "file": "util/js_case_data.cpp",
    "edits": [
      [
        "{65,90,1}",
        "{65,90,0}"
      ]
    ]
  },
  {
    "name": "mapping-search-equality-skipped",
    "file": "util/js_case_data.cpp",
    "edits": [
      [
        "mappings[mid].point<point",
        "mappings[mid].point<=point"
      ]
    ]
  },
  {
    "name": "property-range-first-exclusive",
    "file": "util/js_case_data.cpp",
    "edits": [
      [
        "ranges[low].first<=point",
        "ranges[low].first<point"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Case host baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Case host mutations detected; working sources and fixtures unchanged.`);
