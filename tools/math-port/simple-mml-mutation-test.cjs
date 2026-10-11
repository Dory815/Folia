// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-simple-mml-mutations');
const modules=["util/option_types.ixx","core/tree/property.ixx","util/callable_types.ixx","util/object_types.ixx","core/tree/factory_types.ixx","core/tree/factory.ixx","core/tree/node_factory.ixx","core/tree/node_types.ixx","core/tree/node.ixx","util/js_number.ixx","util/js_numeric_input.ixx","util/utf16_types.ixx","util/utf16.ixx","core/mml_tree/attributes.ixx","util/property_host.ixx","core/mml_tree/mml_node_host.ixx","core/mml_tree/mml_node.ixx","core/mml_tree/class_binding.ixx","core/mml_tree/leaf_host.ixx","core/mml_tree/leaf_defaults.ixx","core/mml_tree/arithmetic_host.ixx","core/mml_tree/mml_nodes/menclose.ixx","core/mml_tree/mml_nodes/mpadded.ixx","core/mml_tree/mml_nodes/maligngroup.ixx","core/mml_tree/mml_nodes/malignmark.ixx","core/mml_tree/mml_nodes/mglyph.ixx","core/mml_tree/mml_nodes/math.ixx","core/mml_tree/mml_nodes/mrow.ixx","core/mml_tree/mml_nodes/mi.ixx","core/mml_tree/mml_nodes/merror.ixx","core/mml_tree/mml_nodes/mtext.ixx"];
const units=["core/mml_tree/mml_bindings.cpp","core/mml_tree/mml_node.cpp","core/mml_tree/mml_branches.cpp","core/mml_tree/mml_empty.cpp","util/property_host.cpp","core/mml_tree/mml_nodes/menclose_bindings.cpp","core/mml_tree/mml_nodes/menclose.cpp","core/mml_tree/mml_nodes/mpadded_bindings.cpp","core/mml_tree/mml_nodes/mpadded.cpp","core/mml_tree/mml_nodes/maligngroup_bindings.cpp","core/mml_tree/mml_nodes/maligngroup.cpp","core/mml_tree/mml_nodes/malignmark_bindings.cpp","core/mml_tree/mml_nodes/mglyph_bindings.cpp","core/mml_tree/mml_nodes/mglyph.cpp","core/mml_tree/mml_nodes/math_bindings.cpp","core/mml_tree/mml_nodes/math.cpp","core/mml_tree/mml_nodes/mrow_bindings.cpp","core/mml_tree/mml_nodes/mrow.cpp","core/mml_tree/mml_nodes/mi_bindings.cpp","core/mml_tree/mml_nodes/mi.cpp","core/mml_tree/mml_nodes/merror_bindings.cpp","core/mml_tree/mml_nodes/merror.cpp","core/mml_tree/mml_nodes/mtext_bindings.cpp","core/mml_tree/mml_nodes/mtext.cpp"];
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_simple_mml.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_simple_mml_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_simple_mml*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "enclose-original-prev-lost",
    "file": "core/mml_tree/mml_nodes/menclose.cpp",
    "edits": [
      [
        "setTeXclass(previous)",
        "setTeXclass(nullptr)"
      ]
    ]
  },
  {
    "name": "enclose-update-omitted",
    "file": "core/mml_tree/mml_nodes/menclose.cpp",
    "edits": [
      [
        "updateTeXclass(currentChild);",
        "(void)currentChild;"
      ]
    ]
  },
  {
    "name": "enclose-stale-update",
    "file": "core/mml_tree/mml_nodes/menclose.cpp",
    "edits": [
      [
        "updateTeXclass(currentChild);",
        "updateTeXclass(result);"
      ]
    ]
  },
  {
    "name": "enclose-return-self",
    "file": "core/mml_tree/mml_nodes/menclose.cpp",
    "edits": [
      [
        "return result;",
        "return this;"
      ]
    ]
  },
  {
    "name": "enclose-empty-second-read-errors",
    "file": "core/mml_tree/mml_nodes/menclose.cpp",
    "edits": [
      [
        "? nullptr",
        "? &host::arithmeticChild(*this, 0)"
      ]
    ]
  },
  {
    "name": "padding-vbox-truthiness-lost",
    "file": "core/mml_tree/mml_nodes/mpadded.cpp",
    "edits": [
      [
        "!host::truthy(getProperty(\"vbox\"))",
        "getProperty(\"vbox\") != Property(true)"
      ]
    ]
  },
  {
    "name": "padding-super-branch-lost",
    "file": "core/mml_tree/mml_nodes/mpadded.cpp",
    "edits": [
      [
        "return AbstractMmlLayoutNode::setTeXclass(previous);",
        "return this;"
      ]
    ]
  },
  {
    "name": "padding-prev-omitted",
    "file": "core/mml_tree/mml_nodes/mpadded.cpp",
    "edits": [
      [
        "getPrevClass(previous);",
        "(void)previous;"
      ]
    ]
  },
  {
    "name": "padding-ord-lost",
    "file": "core/mml_tree/mml_nodes/mpadded.cpp",
    "edits": [
      [
        "setTexClass(TEXCLASS.ORD);",
        "setTexClass(TEXCLASS.INNER);"
      ]
    ]
  },
  {
    "name": "padding-child-prev-wrong",
    "file": "core/mml_tree/mml_nodes/mpadded.cpp",
    "edits": [
      [
        "setTeXclass(nullptr);",
        "setTeXclass(previous);"
      ]
    ]
  },
  {
    "name": "padding-return-null",
    "file": "core/mml_tree/mml_nodes/mpadded.cpp",
    "edits": [
      [
        "return this;",
        "return nullptr;"
      ]
    ]
  },
  {
    "name": "padding-class-before-prev",
    "file": "core/mml_tree/mml_nodes/mpadded.cpp",
    "edits": [
      [
        "getPrevClass(previous);\n    setTexClass(TEXCLASS.ORD);",
        "setTexClass(TEXCLASS.ORD);\n    getPrevClass(previous);"
      ]
    ]
  },
  {
    "name": "glyph-condition-and",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "fontfamily == empty || index == empty",
        "fontfamily == empty && index == empty"
      ]
    ]
  },
  {
    "name": "glyph-condition-or",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "src == empty &&",
        "src == empty ||"
      ]
    ]
  },
  {
    "name": "glyph-src-truthiness",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "src == empty",
        "!host::truthy(src)"
      ]
    ]
  },
  {
    "name": "glyph-family-truthiness",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "fontfamily == empty",
        "!host::truthy(fontfamily)"
      ]
    ]
  },
  {
    "name": "glyph-index-truthiness",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "index == empty",
        "!host::truthy(index)"
      ]
    ]
  },
  {
    "name": "glyph-src-family-swapped",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "const Property src = values.get(\"src\");",
        "const Property src = values.get(\"fontfamily\");"
      ]
    ]
  },
  {
    "name": "glyph-error-not-short",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "ErrorOptions(options), true",
        "ErrorOptions(options), false"
      ]
    ]
  },
  {
    "name": "glyph-error-options-cloned",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "import folia.math.core.mml_tree.leaf_host;",
        "import folia.math.core.mml_tree.leaf_host;\nimport folia.math.core.mml_tree.arithmetic_host;"
      ],
      [
        "ErrorOptions(options), true",
        "ErrorOptions(host::arithmeticCopy(options)), true"
      ]
    ]
  },
  {
    "name": "glyph-message-altered",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "mglyph must have either src or fontfamily and index attributes",
        "invalid glyph"
      ]
    ]
  },
  {
    "name": "glyph-super-omitted",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "AbstractMmlTokenNode::verifyAttributes(options);",
        "(void)options;"
      ]
    ]
  },
  {
    "name": "glyph-super-extra-invalid",
    "file": "core/mml_tree/mml_nodes/mglyph.cpp",
    "edits": [
      [
        "ErrorOptions(options), true);",
        "ErrorOptions(options), true);\n        AbstractMmlTokenNode::verifyAttributes(options);"
      ]
    ]
  },
  {
    "name": "group-merge-omitted",
    "file": "core/mml_tree/mml_nodes/maligngroup.cpp",
    "edits": [
      [
        "addInheritedAttributes(attributes, allAttributes)",
        "attributes"
      ]
    ]
  },
  {
    "name": "group-wrong-merge-args",
    "file": "core/mml_tree/mml_nodes/maligngroup.cpp",
    "edits": [
      [
        "addInheritedAttributes(attributes, allAttributes)",
        "addInheritedAttributes(allAttributes, attributes)"
      ]
    ]
  },
  {
    "name": "group-wrong-merge-result",
    "file": "core/mml_tree/mml_nodes/maligngroup.cpp",
    "edits": [
      [
        "inherited, display, level, prime",
        "attributes, display, level, prime"
      ]
    ]
  },
  {
    "name": "group-display-coerced",
    "file": "core/mml_tree/mml_nodes/maligngroup.cpp",
    "edits": [
      [
        "inherited, display, level, prime",
        "inherited, Property(true), level, prime"
      ]
    ]
  },
  {
    "name": "group-level-coerced",
    "file": "core/mml_tree/mml_nodes/maligngroup.cpp",
    "edits": [
      [
        "inherited, display, level, prime",
        "inherited, display, Property(0), prime"
      ]
    ]
  },
  {
    "name": "group-prime-lost",
    "file": "core/mml_tree/mml_nodes/maligngroup.cpp",
    "edits": [
      [
        "inherited, display, level, prime",
        "inherited, display, level, Property(false)"
      ]
    ]
  },
  {
    "name": "menclose-default-altered",
    "file": "core/mml_tree/mml_nodes/menclose_bindings.cpp",
    "edits": [
      [
        "value.setOwn(\"notation\",\"longdiv\");",
        "value.setOwn(\"notation\",\"box\");"
      ]
    ]
  },
  {
    "name": "mpadded-default-altered",
    "file": "core/mml_tree/mml_nodes/mpadded_bindings.cpp",
    "edits": [
      [
        "value.setOwn(\"width\",\"\");",
        "value.setOwn(\"width\",\"0\");"
      ]
    ]
  },
  {
    "name": "maligngroup-default-altered",
    "file": "core/mml_tree/mml_nodes/maligngroup_bindings.cpp",
    "edits": [
      [
        "value.setOwn(\"groupalign\",\"_inherit_\");",
        "value.setOwn(\"groupalign\",\"left\");"
      ]
    ]
  },
  {
    "name": "malignmark-default-altered",
    "file": "core/mml_tree/mml_nodes/malignmark_bindings.cpp",
    "edits": [
      [
        "value.setOwn(\"edge\",\"left\");",
        "value.setOwn(\"edge\",\"right\");"
      ]
    ]
  },
  {
    "name": "mglyph-default-altered",
    "file": "core/mml_tree/mml_nodes/mglyph_bindings.cpp",
    "edits": [
      [
        "value.setOwn(\"src\",\"\");",
        "value.setOwn(\"src\",\"image\");"
      ]
    ]
  },
  {
    "name": "mark-arity-not-zero",
    "file": "core/mml_tree/mml_nodes/malignmark_bindings.cpp",
    "edits": [
      [
        "return 0.0;",
        "return 1.0;"
      ]
    ]
  },
  {
    "name": "mark-space-false",
    "file": "core/mml_tree/mml_nodes/malignmark_bindings.cpp",
    "edits": [
      [
        "return true;",
        "return false;"
      ]
    ]
  },
  {
    "name": "glyph-ord-lost",
    "file": "core/mml_tree/mml_nodes/mglyph_bindings.cpp",
    "edits": [
      [
        "initializeTexClass(TEXCLASS.ORD);",
        "initializeTexClass(TEXCLASS.INNER);"
      ]
    ]
  },
  {
    "name": "menclose-defaults-alias-base",
    "file": "core/mml_tree/mml_nodes/menclose_bindings.cpp",
    "edits": [
      [
        "return MmlMencloseDefaults;",
        "return AbstractMmlNode::defaults();"
      ]
    ]
  },
  {
    "name": "mpadded-defaults-alias-base",
    "file": "core/mml_tree/mml_nodes/mpadded_bindings.cpp",
    "edits": [
      [
        "return MmlMpaddedDefaults;",
        "return AbstractMmlLayoutNode::defaults();"
      ]
    ]
  },
  {
    "name": "maligngroup-defaults-alias-base",
    "file": "core/mml_tree/mml_nodes/maligngroup_bindings.cpp",
    "edits": [
      [
        "return MmlMaligngroupDefaults;",
        "return AbstractMmlLayoutNode::defaults();"
      ]
    ]
  },
  {
    "name": "malignmark-defaults-alias-base",
    "file": "core/mml_tree/mml_nodes/malignmark_bindings.cpp",
    "edits": [
      [
        "return MmlMalignmarkDefaults;",
        "return AbstractMmlNode::defaults();"
      ]
    ]
  },
  {
    "name": "mglyph-defaults-alias-base",
    "file": "core/mml_tree/mml_nodes/mglyph_bindings.cpp",
    "edits": [
      [
        "return MmlMglyphDefaults;",
        "return AbstractMmlTokenNode::defaults();"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Simple MML baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Simple MML mutations detected; working sources and fixtures unchanged.`);
