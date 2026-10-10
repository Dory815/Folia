// Root acceptance, isolated source copies; working implementation is never edited.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-class-state-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','core/mml_tree/mml_node_host.cpp'];
const copies=new Map();fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const fixture=fs.readFileSync(path.join(root,'tests/math/reference_class_state.cpp'));
fs.writeFileSync(path.join(work,'reference_class_state.cpp'),fixture);
for(const file of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',file));const source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_class_state_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_class_state.cpp")
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_class_state*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Class state baseline compiles and passes.');
const normal='core/mml_tree/mml_node.cpp',branches='core/mml_tree/mml_branches.cpp',empty='core/mml_tree/mml_empty.cpp';
const mutations=[
  {
    "name": "initial-current-missing",
    "file": "core/mml_tree/mml_node.ixx",
    "edits": [
      [
        "Property texclass_=PropertyNull{},prevClass_=PropertyNull{};",
        "Property texclass_=Missing{},prevClass_=PropertyNull{};"
      ]
    ]
  },
  {
    "name": "initial-previous-missing",
    "file": "core/mml_tree/mml_node.ixx",
    "edits": [
      [
        "Property texclass_=PropertyNull{},prevClass_=PropertyNull{};",
        "Property texclass_=PropertyNull{},prevClass_=Missing{};"
      ]
    ]
  },
  {
    "name": "current-setter-coerces-number",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "texclass_ = std::move(value);",
        "texclass_ = host::number(value);"
      ]
    ]
  },
  {
    "name": "previous-setter-coerces-number",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "prevClass_ = std::move(value);",
        "prevClass_ = host::number(value);"
      ]
    ]
  },
  {
    "name": "class-selection-uses-truthy",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "return !host::nullish(texClass()) ? this : previous;",
        "return host::truthy(texClass()) ? this : previous;"
      ]
    ]
  },
  {
    "name": "previous-read-collapsed",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "!host::nullish(firstPrevious) ? prevClass()",
        "!host::nullish(firstPrevious) ? firstPrevious"
      ]
    ]
  },
  {
    "name": "spacing-current-getter-bypassed",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "const Property rawCurrent = texClass();",
        "const Property rawCurrent = texclass_;"
      ]
    ]
  },
  {
    "name": "none-coerces-number",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "previous == Property(TEXCLASS.NONE) || current == Property(TEXCLASS.NONE)",
        "host::number(previous) == TEXCLASS.NONE || host::number(current) == TEXCLASS.NONE"
      ]
    ]
  },
  {
    "name": "transfer-current-coerces-number",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "setTexClass(coreNode->texClass());",
        "setTexClass(host::number(coreNode->texClass()));"
      ]
    ]
  },
  {
    "name": "core-clears-missing",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "coreNode->setPrevClass(PropertyNull{});",
        "coreNode->setPrevClass(Missing{});"
      ]
    ]
  },
  {
    "name": "core-clear-order-inverted",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "coreNode->setPrevLevel(PropertyNull{});\n        coreNode->setPrevClass(PropertyNull{});",
        "coreNode->setPrevClass(PropertyNull{});\n        coreNode->setPrevLevel(PropertyNull{});"
      ]
    ]
  },
  {
    "name": "current-script-ignored",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "currentScript = host::greaterThanZero(attrs->get(\"scriptlevel\"));",
        "currentScript = false;"
      ]
    ]
  },
  {
    "name": "previous-script-truthy",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "const bool previousScript = host::greaterThanZero(prevLevel());",
        "const bool previousScript = host::truthy(prevLevel());"
      ]
    ]
  },
  {
    "name": "invalid-column-early-return",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "const Property space = host::get(row, columnKey);",
        "const Property space = host::get(row, columnKey); if (space.missing()) return Missing{};"
      ]
    ]
  },
  {
    "name": "numeric-prototype-is-object",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(key==\"__proto__\")return BuiltinProperty::numberPrototype;",
        "if(key==\"__proto__\")return BuiltinProperty::prototype;"
      ]
    ]
  },
  {
    "name": "number-prototype-unboxing-lost",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "if(p==Property(BuiltinProperty::numberPrototype))return 0;",
        "if(p==Property(BuiltinProperty::numberPrototype))return std::numeric_limits<double>::quiet_NaN();"
      ]
    ]
  },
  {
    "name": "number-tostring-arity-lost",
    "file": "core/mml_tree/mml_node_host.ixx",
    "edits": [
      [
        "case BuiltinProperty::numberConstructor:case BuiltinProperty::numberToString:",
        "case BuiltinProperty::numberConstructor:"
      ]
    ]
  },
  {
    "name": "spacing-negative-sign-lost",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "PropertyArray{ 0,-1, 2, 3, 0, 0, 0, 1}",
        "PropertyArray{ 0, 1, 2, 3, 0, 0, 0, 1}"
      ]
    ]
  },
  {
    "name": "previous-level-raw-value-lost",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "setPrevLevel(attrs->get(\"scriptlevel\"));",
        "setPrevLevel(host::number(attrs->get(\"scriptlevel\")));"
      ]
    ]
  },
  {
    "name": "selection-previous-transfer-skipped",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "getPrevClass(previous);\n    return !host::nullish(texClass())",
        "/* skipped */\n    return !host::nullish(texClass())"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7);
const selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}
 try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_class_state.cpp'))),'Working fixture changed');
for(const [file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working implementation changed: '+file);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Class state mutations detected; working sources and fixtures unchanged.`);
