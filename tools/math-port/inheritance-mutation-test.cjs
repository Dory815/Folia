// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-inheritance-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx','core/mml_tree/script_defaults.ixx','util/string.ixx','core/mml_tree/mo_patterns.ixx','core/mml_tree/mo_types.ixx','core/mml_tree/mo_host.ixx','core/mml_tree/operator_dictionary.ixx','core/mml_tree/arithmetic_host.ixx','core/mml_tree/table_host.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','core/mml_tree/mml_node_host.cpp','util/string.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp','core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp'];
const nodeNames=['math','mrow','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom','msubsup','munderover','mfrac','msqrt','mroot','mtable','mtr','mtd'];
modules.push(...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'),'core/mml_tree/mml_nodes/mo.ixx');
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']),...['bindings','lookup','classes','scripts'].map(n=>'core/mml_tree/mml_nodes/mo_'+n+'.cpp'));
modules.push('core/mml_tree/operator_class.ixx');
units.push('core/mml_tree/operator_class.cpp');
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_inheritance_boundary.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_inheritance_mutations LANGUAGES CXX)
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
target_link_options(MutationTests PRIVATE /STACK:16777216 /INCREMENTAL:NO)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_raw_inheritance*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}


const mutations=[
  {
    "name": "missing-attributes-not-defaulted",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(attributes.missing())attributes=PropertyList{};",
        "/* Missing stays raw */"
      ]
    ]
  },
  {
    "name": "null-attributes-defaulted",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(attributes.missing())",
        "if(attributes.missing()||std::holds_alternative<PropertyNull>(attributes.value))"
      ]
    ]
  },
  {
    "name": "default-display-true",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(display.missing())display=false;",
        "if(display.missing())display=true;"
      ]
    ]
  },
  {
    "name": "default-level-one",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(level.missing())level=0;",
        "if(level.missing())level=1;"
      ]
    ]
  },
  {
    "name": "default-prime-true",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(prime.missing())prime=false;",
        "if(prime.missing())prime=true;"
      ]
    ]
  },
  {
    "name": "shared-default-record",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(attributes.missing())attributes=PropertyList{};",
        "if(attributes.missing()){static Property shared=PropertyList{};attributes=shared;}"
      ]
    ]
  },
  {
    "name": "base-clones-argument",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "host::inheritanceDefaults(inherited,display,level,prime);",
        "host::inheritanceDefaults(inherited,display,level,prime);inherited=host::objectAssign(inherited);"
      ]
    ]
  },
  {
    "name": "table-defaults-before-indent",
    "file": "core/mml_tree/mml_nodes/mtable.cpp",
    "edits": [
      [
        "for (std::size_t index = 0; index < indentAttributes.size(); ++index)",
        "host::inheritanceDefaults(attributes,display,level,prime);for(std::size_t index=0;index<indentAttributes.size();++index)"
      ]
    ]
  },
  {
    "name": "style-defaults-after-first-writes",
    "file": "core/mml_tree/mml_nodes/mstyle.cpp",
    "edits": [
      [
        "host::inheritanceDefaults(attributes, display, level, prime);",
        "/* entry defaults omitted */"
      ]
    ]
  },
  {
    "name": "tuple-string-uses-code-units",
    "file": "core/mml_tree/mml_bindings.cpp",
    "edits": [
      [
        "if(units[start]>=0xD800",
        "if(false&&units[start]>=0xD800"
      ]
    ]
  },
  {
    "name": "tuple-noniterable-rejected",
    "file": "core/mml_tree/mml_bindings.cpp",
    "edits": [
      [
        "return {host::get(p,\"0\"),host::get(p,\"1\")};",
        "host::typeError(\"Noniterable rejected\");"
      ]
    ]
  },
  {
    "name": "spread-proto-is-data",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "result.set(key,get(input,key))",
        "result.setOwn(key,get(input,key))"
      ]
    ]
  },
  {
    "name": "add-display-style-not-filtered",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "if (name != \"displaystyle\" && name != \"scriptlevel\" && name != \"style\")",
        "if(true)"
      ]
    ]
  },
  {
    "name": "noinherit-kind-read-eager",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "if (host::nullish(from)) return false;",
        "if(host::nullish(from)){(void)node.kind();return false;}"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Raw inheritance baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} raw inheritance mutations detected; working sources and fixtures unchanged.`);
