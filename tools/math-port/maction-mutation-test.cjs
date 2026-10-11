// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-maction-mutations');
const manifest=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...manifest.matchAll(/src\/math\/([^\s()]+\.ixx)/g)].map(x=>x[1]);
const units=[...manifest.matchAll(/src\/math\/([^\s()]+\.cpp)/g)].map(x=>x[1]);
assert.equal(new Set(modules).size,modules.length);assert.equal(new Set(units).size,units.length);
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});

const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_maction.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units,"util/option_storage.hpp"]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_maction_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_maction*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "wrong-default-action",
    "file": "core/mml_tree/mml_nodes/maction_bindings.cpp",
    "edits": [
      [
        "result.setOwn(\"actiontype\",\"toggle\")",
        "result.setOwn(\"actiontype\",\"statusline\")"
      ]
    ]
  },
  {
    "name": "wrong-default-selection",
    "file": "core/mml_tree/mml_nodes/maction_bindings.cpp",
    "edits": [
      [
        "result.setOwn(\"selection\",1)",
        "result.setOwn(\"selection\",2)"
      ]
    ]
  },
  {
    "name": "wrong-kind",
    "file": "core/mml_tree/mml_nodes/maction_bindings.cpp",
    "edits": [
      [
        "return \"maction\"",
        "return \"action\""
      ]
    ]
  },
  {
    "name": "wrong-arity",
    "file": "core/mml_tree/mml_nodes/maction_bindings.cpp",
    "edits": [
      [
        "return 1",
        "return 2"
      ]
    ]
  },
  {
    "name": "selection-offset-omitted",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "std::max(1.0, minimum) - 1.0",
        "std::max(1.0, minimum)"
      ]
    ]
  },
  {
    "name": "selection-length-clamp-omitted",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "std::min(childLength, selectedNumber)",
        "selectedNumber"
      ]
    ]
  },
  {
    "name": "fractional-selection-truncated",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "const double selectedNumber = host::number(selection);",
        "const double selectedNumber = std::trunc(host::number(selection));"
      ]
    ]
  },
  {
    "name": "selection-nan-becomes-zero",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "const double selectedNumber = host::number(selection);",
        "const double selectedNumber = std::isnan(host::number(selection)) ? 0.0 : host::number(selection);"
      ]
    ]
  },
  {
    "name": "fallback-extra-argument",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "this->factory.create(\"mrow\", {})",
        "this->factory.create(\"mrow\", std::array<std::any,1>{PropertyList{}})"
      ]
    ]
  },
  {
    "name": "fallback-omitted",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "if (!child) child = this->factory.create(\"mrow\", {});",
        "/* fallback omitted */"
      ]
    ]
  },
  {
    "name": "embellished-projection-swapped",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "return child->isEmbellished();",
        "return child->isSpacelike();"
      ]
    ]
  },
  {
    "name": "spacelike-projection-swapped",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "return child->isSpacelike();",
        "return child->isEmbellished();"
      ]
    ]
  },
  {
    "name": "core-projection-swapped",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "return child->core();",
        "return child->coreMO();"
      ]
    ]
  },
  {
    "name": "coremo-projection-swapped",
    "file": "core/mml_tree/mml_nodes/maction_selection.cpp",
    "edits": [
      [
        "return child->coreMO();",
        "return child->core();"
      ]
    ]
  },
  {
    "name": "base-attribute-verification-omitted",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "AbstractMmlNode::verifyAttributes(std::move(options));",
        "/* base verification omitted */"
      ]
    ]
  },
  {
    "name": "non-toggle-selection-retained",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "host::arithmeticAttributes(*this)->unset(\"selection\");",
        "/* selection unset omitted */"
      ]
    ]
  },
  {
    "name": "tooltip-class-casefolded",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "actiontype == Property(std::string(\"tooltip\"))",
        "host::lowerString(actiontype) == u\"tooltip\""
      ]
    ]
  },
  {
    "name": "tooltip-class-dispatch-omitted",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "(*currentChildren)[1]->setTeXclass(nullptr);",
        "/* tip class omitted */"
      ]
    ]
  },
  {
    "name": "selected-before-tip-callback",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "    const Property actiontype = host::arithmeticAttributes(*this)->get(\"actiontype\");\n    if (actiontype == Property(std::string(\"tooltip\")))",
        "    MmlNode* const selectedNode = selected();\n    const Property actiontype = host::arithmeticAttributes(*this)->get(\"actiontype\");\n    if (actiontype == Property(std::string(\"tooltip\")))"
      ],
      [
        "    MmlNode* const selectedNode = selected();\n    if (!selectedNode)",
        "    if (!selectedNode)"
      ]
    ]
  },
  {
    "name": "class-update-omitted",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "updateTeXclass(selectedNode);",
        "/* update omitted */"
      ]
    ]
  },
  {
    "name": "class-updates-returned-node",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "updateTeXclass(selectedNode);",
        "updateTeXclass(result);"
      ]
    ]
  },
  {
    "name": "class-returns-selection",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "return result;",
        "return selectedNode;"
      ]
    ]
  },
  {
    "name": "toggle-number-coercion",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "host::integer(host::string(rawSelection))",
        "host::number(rawSelection)"
      ]
    ]
  },
  {
    "name": "toggle-increment-omitted",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "parsed + 1.0",
        "parsed"
      ]
    ]
  },
  {
    "name": "toggle-nan-repaired",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "double selection = parsed + 1.0;",
        "double selection = std::isnan(parsed) ? 1.0 : parsed + 1.0;"
      ]
    ]
  },
  {
    "name": "toggle-wraps-equal-length",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "selection > static_cast<double>(children->size())",
        "selection >= static_cast<double>(children->size())"
      ]
    ]
  },
  {
    "name": "toggle-skips-live-attribute-read",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "host::arithmeticAttributes(*this)->set(\"selection\", Property(selection));",
        "AbstractMmlNode::attributes()->set(\"selection\", Property(selection));"
      ]
    ]
  },
  {
    "name": "hook-string-coercion-instead-of-method",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "host::lowerString(actiontype)",
        "host::string(actiontype)"
      ]
    ]
  },
  {
    "name": "hook-base-propagation-omitted",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "AbstractMmlNode::setChildInheritedAttributes(\n            std::move(attributes), std::move(display), std::move(level), std::move(prime));",
        "/* base propagation omitted */"
      ]
    ]
  },
  {
    "name": "hook-first-prime-replaced",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "first->setInheritedAttributes(attributes, display, level, prime);",
        "first->setInheritedAttributes(attributes, display, level, false);"
      ]
    ]
  },
  {
    "name": "hook-second-uses-original-style",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "attributes, Property(false), Property(1.0), Property(false)",
        "attributes, display, level, prime"
      ]
    ]
  },
  {
    "name": "hook-second-array-stale",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "const Children secondChildren = childNodes;",
        "const Children secondChildren = firstChildren;"
      ]
    ]
  },
  {
    "name": "hook-null-array-silently-skipped",
    "file": "core/mml_tree/mml_nodes/maction_behavior.cpp",
    "edits": [
      [
        "if (!firstChildren) host::typeError(\"Cannot read child 0 from null maction children\");",
        "if (!firstChildren) return;"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Maction baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Maction mutations detected; working sources and fixtures unchanged.`);
