// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-atom-semantics-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','util/property_host.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx','core/mml_tree/script_defaults.ixx','util/string.ixx','core/mml_tree/mo_patterns.ixx','core/mml_tree/mo_types.ixx','core/mml_tree/mo_host.ixx','core/mml_tree/operator_dictionary.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','util/property_host.cpp','util/string.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp','core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp'];
const nodeNames=['math','mrow','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom','msubsup','munderover','tex_atom','semantics'];
modules.push(...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'),'core/mml_tree/mml_nodes/mo.ixx');
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']),...['bindings','lookup','classes','scripts'].map(n=>'core/mml_tree/mml_nodes/mo_'+n+'.cpp'));
modules.push('core/mml_tree/operator_class.ixx');
modules.push('core/mml_tree/arithmetic_host.ixx');
units.push('core/mml_tree/operator_class.cpp');
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_atom_semantics.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_atom_semantics_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_atom_semantics*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "atom-child-previous-forwarded",
    "file": "core/mml_tree/mml_nodes/tex_atom.cpp",
    "edits": [
      [
        "setTeXclass(nullptr)",
        "setTeXclass(previous)"
      ]
    ]
  },
  {
    "name": "atom-child-return-forwarded",
    "file": "core/mml_tree/mml_nodes/tex_atom.cpp",
    "edits": [
      [
        "host::arithmeticChild(*this, 0).setTeXclass(nullptr);",
        "return host::arithmeticChild(*this, 0).setTeXclass(nullptr);"
      ]
    ]
  },
  {
    "name": "atom-child-call-omitted",
    "file": "core/mml_tree/mml_nodes/tex_atom.cpp",
    "edits": [
      [
        "host::arithmeticChild(*this, 0).setTeXclass(nullptr);",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "atom-virtual-adjustment-bypassed",
    "file": "core/mml_tree/mml_nodes/tex_atom.cpp",
    "edits": [
      [
        "return adjustTeXclass(previous);",
        "return adjustOperatorTeXclass(*this, previous);"
      ]
    ]
  },
  {
    "name": "atom-overwritten-stub-restored",
    "file": "core/mml_tree/mml_nodes/tex_atom.cpp",
    "edits": [
      [
        "return adjustOperatorTeXclass(*this, previous);",
        "return this;"
      ]
    ]
  },
  {
    "name": "atom-delegate-extra-guard",
    "file": "core/mml_tree/mml_nodes/tex_atom.cpp",
    "edits": [
      [
        "return adjustOperatorTeXclass(*this, previous);",
        "tree::TreeCallGuard<MmlNode> extra; return adjustOperatorTeXclass(*this, previous);"
      ]
    ]
  },
  {
    "name": "mo-delegate-extra-guard",
    "file": "core/mml_tree/mml_nodes/mo_classes.cpp",
    "edits": [
      [
        "return adjustOperatorTeXclass(*this, prev);",
        "tree::TreeCallGuard<MmlNode> extra; return adjustOperatorTeXclass(*this, prev);"
      ]
    ]
  },
  {
    "name": "atom-field-ord-omitted",
    "file": "core/mml_tree/mml_nodes/tex_atom_bindings.cpp",
    "edits": [
      [
        "initializeTexClass(TEXCLASS.ORD);",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "atom-constructor-virtual-getter-lost",
    "file": "core/mml_tree/mml_nodes/tex_atom_bindings.cpp",
    "edits": [
      [
        "const Property value=texClass();",
        "const Property value=TEXCLASS.ORD;"
      ]
    ]
  },
  {
    "name": "atom-constructor-property-write-lost",
    "file": "core/mml_tree/mml_nodes/tex_atom_bindings.cpp",
    "edits": [
      [
        "setProperty(\"texClass\",value);",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "atom-defaults-base-alias",
    "file": "core/mml_tree/mml_nodes/tex_atom_bindings.cpp",
    "edits": [
      [
        "return MmlTeXAtomDefaults;",
        "return AbstractMmlBaseNode::defaults();"
      ]
    ]
  },
  {
    "name": "semantics-logical-parent-lost",
    "file": "core/mml_tree/mml_nodes/semantics_bindings.cpp",
    "edits": [
      [
        "bool MmlSemantics::notParent()const{return true;}",
        "bool MmlSemantics::notParent()const{return false;}"
      ]
    ]
  },
  {
    "name": "semantics-arity-two",
    "file": "core/mml_tree/mml_nodes/semantics_bindings.cpp",
    "edits": [
      [
        "double MmlSemantics::arity()const{return 1.0;}",
        "double MmlSemantics::arity()const{return 2.0;}"
      ]
    ]
  },
  {
    "name": "annotation-properties-merged",
    "file": "core/mml_tree/mml_nodes/semantics_bindings.cpp",
    "edits": [
      [
        "properties=PropertyList{{\"isChars\",true}};",
        "properties.setOwn(\"isChars\",true);"
      ]
    ]
  },
  {
    "name": "annotation-is-chars-false",
    "file": "core/mml_tree/mml_nodes/semantics_bindings.cpp",
    "edits": [
      [
        "{\"isChars\",true}",
        "{\"isChars\",false}"
      ]
    ]
  },
  {
    "name": "annotation-defaults-xml-alias",
    "file": "core/mml_tree/mml_nodes/semantics_bindings.cpp",
    "edits": [
      [
        "return MmlAnnotationDefaults;",
        "return MmlAnnotationXML::defaults();"
      ]
    ]
  },
  {
    "name": "annotation-empty-hook-inherits",
    "file": "core/mml_tree/mml_nodes/semantics.cpp",
    "edits": [
      [
        "AttributeList, Property, Property, Property) {}",
        "AttributeList attrs, Property display, Property level, Property prime) { AbstractMmlNode::setChildInheritedAttributes(attrs,display,level,prime); }"
      ]
    ]
  },
  {
    "name": "constructor-previous-virtual-write-bypassed",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "setPrevClass(PropertyNull{});",
        "AbstractMmlNode::setPrevClass(PropertyNull{});"
      ]
    ]
  },
  {
    "name": "constructor-level-virtual-write-bypassed",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "setPrevLevel(PropertyNull{});",
        "AbstractMmlNode::setPrevLevel(PropertyNull{});"
      ]
    ]
  },
  {
    "name": "constructor-write-order-swapped",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "setPrevClass(PropertyNull{});\n    setPrevLevel(PropertyNull{});",
        "setPrevLevel(PropertyNull{});\n    setPrevClass(PropertyNull{});"
      ]
    ]
  },
  {
    "name": "constructor-tex-field-before-callbacks",
    "file": "core/mml_tree/mml_node.cpp",
    "edits": [
      [
        "setPrevClass(PropertyNull{});\n    setPrevLevel(PropertyNull{});\n    initializeTexClass(PropertyNull{});",
        "initializeTexClass(PropertyNull{});\n    setPrevClass(PropertyNull{});\n    setPrevLevel(PropertyNull{});"
      ]
    ]
  },
  {
    "name": "pre-constructor-tex-null",
    "file": "core/mml_tree/mml_node.ixx",
    "edits": [
      [
        "Property texclass_=Missing{},prevClass_=Missing{};",
        "Property texclass_=PropertyNull{},prevClass_=Missing{};"
      ]
    ]
  },
  {
    "name": "pre-constructor-previous-null",
    "file": "core/mml_tree/mml_node.ixx",
    "edits": [
      [
        "Property texclass_=Missing{},prevClass_=Missing{};",
        "Property texclass_=Missing{},prevClass_=PropertyNull{};"
      ]
    ]
  },
  {
    "name": "pre-constructor-level-null",
    "file": "core/mml_tree/mml_node.ixx",
    "edits": [
      [
        "Property prevLevel_=Missing{};",
        "Property prevLevel_=PropertyNull{};"
      ]
    ]
  },
  {
    "name": "shared-direct-parent-replaced",
    "file": "core/mml_tree/operator_class.cpp",
    "edits": [
      [
        "MmlNode* parent = receiver.parent;",
        "MmlNode* parent = receiver.Parent();"
      ]
    ]
  },
  {
    "name": "shared-level-uses-explicit",
    "file": "core/mml_tree/operator_class.cpp",
    "edits": [
      [
        "attributes->getInherited(\"scriptlevel\")",
        "attributes->getExplicit(\"scriptlevel\")"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Atom/semantics baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Atom/semantics mutations detected; working sources and fixtures unchanged.`);
