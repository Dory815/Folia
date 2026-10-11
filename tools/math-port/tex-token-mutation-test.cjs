// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-tex-token-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','util/property_host.ixx','core/mml_tree/mml_node_host.ixx','input/tex/token.ixx','input/tex/types.ixx','input/tex/macro.ixx','input/tex/handler_types.ixx','input/tex/tex_constants.ixx'];
modules.push('util/object_types.ixx','util/callable_types.ixx','util/prioritized_list.ixx','util/promise.ixx','util/function_types.ixx');
const units=['util/property_host.cpp','util/promise.cpp',"util/function_value.cpp",'input/tex/token.cpp','input/tex/macro.cpp','input/tex/parse_callable.cpp'];
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});

const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_tex_token.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_tex_token_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_tex*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "token-constructor-token",
    "file": "input/tex/token.cpp",
    "edits": [
      [
        "token_(std::move(token))",
        "token_(Missing{})"
      ]
    ]
  },
  {
    "name": "token-constructor-char",
    "file": "input/tex/token.cpp",
    "edits": [
      [
        "character_(std::move(character))",
        "character_(Missing{})"
      ]
    ]
  },
  {
    "name": "token-constructor-attrs",
    "file": "input/tex/token.cpp",
    "edits": [
      [
        "attributes_(std::move(attributes))",
        "attributes_(Missing{})"
      ]
    ]
  },
  {
    "name": "token-token-getter",
    "file": "input/tex/token.cpp",
    "edits": [
      [
        "return token_;",
        "return Missing{};"
      ]
    ]
  },
  {
    "name": "token-char-getter",
    "file": "input/tex/token.cpp",
    "edits": [
      [
        "return character_;",
        "return token_;"
      ]
    ]
  },
  {
    "name": "token-attrs-getter",
    "file": "input/tex/token.cpp",
    "edits": [
      [
        "return attributes_;",
        "return Missing{};"
      ]
    ]
  },
  {
    "name": "macro-constructor-func",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "func_(std::move(func))",
        "func_(Property(Missing{}))"
      ]
    ]
  },
  {
    "name": "macro-default-missing",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "Property(PropertyArray{})",
        "Property(PropertyNull{})"
      ]
    ]
  },
  {
    "name": "macro-token-getter",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "return token_;",
        "return Missing{};"
      ]
    ]
  },
  {
    "name": "macro-func-getter",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "return func_;",
        "return Property(Missing{});"
      ]
    ]
  },
  {
    "name": "macro-args-getter",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "return args_;",
        "return PropertyArray{};"
      ]
    ]
  },
  {
    "name": "macro-default-shared",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "Property(PropertyArray{})",
        "([]{static Property a=PropertyArray{};return a;})()"
      ]
    ]
  },
  {
    "name": "macro-null-default",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "args.missing()",
        "(args.missing() || std::holds_alternative<PropertyNull>(args.value))"
      ]
    ]
  },
  {
    "name": "macro-cloned-args",
    "file": "input/tex/macro.cpp",
    "edits": [
      [
        "std::move(args))",
        "([](Property v){if(auto a=std::get_if<std::shared_ptr<PropertyStorage>>(&v.value);a&&*a)*a=std::make_shared<PropertyStorage>(**a);return v;})(args))"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('TeX token baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} TeX token mutations detected; working sources and fixtures unchanged.`);
