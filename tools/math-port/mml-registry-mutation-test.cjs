// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-mml-registry-mutations');
const manifest=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...manifest.matchAll(/src\/math\/([^\s()]+\.ixx)/g)].map(x=>x[1]);
const units=[...manifest.matchAll(/src\/math\/([^\s()]+\.cpp)/g)].map(x=>x[1]);
assert.equal(new Set(modules).size,modules.length);assert.equal(new Set(units).size,units.length);
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});

const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_mml_registry.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units,"util/option_storage.hpp"]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_mml_registry_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_mml_registry*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "omitted-action-class",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "        FOLIA_MML_BIND(\"maction\", MmlMaction);",
        ""
      ]
    ]
  },
  {
    "name": "wrong-inferred-class",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "FOLIA_MML_BIND(\"inferredMrow\", MmlInferredMrow)",
        "FOLIA_MML_BIND(\"inferredMrow\", MmlMrow)"
      ]
    ]
  },
  {
    "name": "wrong-subscript-class",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "FOLIA_MML_BIND(\"msub\", MmlMsub)",
        "FOLIA_MML_BIND(\"msub\", MmlMsubsup)"
      ]
    ]
  },
  {
    "name": "wrong-annotation-class",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "FOLIA_MML_BIND(\"annotation-xml\", MmlAnnotationXML)",
        "FOLIA_MML_BIND(\"annotation-xml\", MmlAnnotation)"
      ]
    ]
  },
  {
    "name": "wrong-html-class",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "FOLIA_MML_BIND(\"html\", HtmlNode)",
        "FOLIA_MML_BIND(\"html\", XMLNode)"
      ]
    ]
  },
  {
    "name": "wrong-texatom-key",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "FOLIA_MML_BIND(\"TeXAtom\", MmlTeXAtom)",
        "FOLIA_MML_BIND(\"texatom\", MmlTeXAtom)"
      ]
    ]
  },
  {
    "name": "wrong-registry-order",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "FOLIA_MML_BIND(\"mi\", MmlMi);\n        FOLIA_MML_BIND(\"mn\", MmlMn);",
        "FOLIA_MML_BIND(\"mn\", MmlMn);\n        FOLIA_MML_BIND(\"mi\", MmlMi);"
      ]
    ]
  },
  {
    "name": "replaced-global-handle",
    "file": "core/mml_tree/mml.cpp",
    "edits": [
      [
        "static const auto registry",
        "static auto registry"
      ],
      [
        "    return registry;",
        "    registry=std::make_shared<tree::FactoryClassMap<MmlNode>>(*registry);\n    return registry;"
      ]
    ]
  },
  {
    "name": "broken-default-alias",
    "file": "core/mml_tree/mml_factory.ixx",
    "edits": [
      [
        "defaultNodes=folia::math::mml::MML()",
        "defaultNodes=std::make_shared<tree::FactoryClassMap<MmlNode>>(*folia::math::mml::MML())"
      ]
    ]
  },
  {
    "name": "copied-creator-view",
    "file": "core/mml_tree/mml_factory.cpp",
    "edits": [
      [
        "return creatorView();",
        "return std::make_shared<tree::FactoryCreatorObject<MmlNode>>(*creatorView());"
      ]
    ]
  },
  {
    "name": "ignored-explicit-map",
    "file": "core/mml_tree/mml_factory.cpp",
    "edits": [
      [
        "Base(std::move(nodes), selectedClassDefaults(nodes, classDefaults))",
        "Base(std::nullopt, selectedClassDefaults(std::nullopt, classDefaults))"
      ]
    ]
  },
  {
    "name": "ignored-injected-defaults",
    "file": "core/mml_tree/mml_factory.cpp",
    "edits": [
      [
        "if (nodes || injectedDefaults) return injectedDefaults;",
        "if (nodes || injectedDefaults) return nullptr;"
      ]
    ]
  },
  {
    "name": "silent-null-default-fallback",
    "file": "core/mml_tree/mml_factory.cpp",
    "edits": [
      [
        "throw tree::FactoryError(\"TypeError\", u\"MmlFactory.defaultNodes is null\");",
        "return &tree::AbstractFactory<MmlNode>::defaultNodes;"
      ]
    ]
  },
  {
    "name": "empty-map-replaced-by-defaults",
    "file": "core/mml_tree/mml_factory.cpp",
    "edits": [
      [
        "Base(std::move(nodes), selectedClassDefaults(nodes, classDefaults))",
        "Base(nodes && nodes->empty() ? std::nullopt : std::move(nodes), selectedClassDefaults(nodes && nodes->empty() ? std::nullopt : nodes, classDefaults))"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('MML registry baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} MML registry mutations detected; working sources and fixtures unchanged.`);
