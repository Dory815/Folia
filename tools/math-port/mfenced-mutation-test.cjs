// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-mfenced-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx',"util/callable_types.ixx","util/object_types.ixx",'core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','util/property_host.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx','core/mml_tree/script_defaults.ixx','util/string.ixx','core/mml_tree/mo_patterns.ixx','core/mml_tree/mo_types.ixx','core/mml_tree/mo_host.ixx','core/mml_tree/operator_dictionary.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','util/property_host.cpp','util/string.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp','core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp'];
const nodeNames=['math','mrow','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom','msubsup','munderover','tex_atom'];
modules.push(...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'),'core/mml_tree/mml_nodes/mo.ixx');
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']),...['bindings','lookup','classes','scripts'].map(n=>'core/mml_tree/mml_nodes/mo_'+n+'.cpp'));
modules.push('core/mml_tree/operator_class.ixx');
modules.push('core/mml_tree/arithmetic_host.ixx');
units.push('core/mml_tree/operator_class.cpp');
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
modules.push('core/mml_tree/mml_nodes/mfenced.ixx');units.push('core/mml_tree/mml_nodes/mfenced_bindings.cpp','core/mml_tree/mml_nodes/mfenced.cpp');
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_mfenced.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_mfenced_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_mfenced*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "prev-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "getPrevClass(previous);",
        "(void)previous;"
      ]
    ]
  },
  {
    "name": "open-hook-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "if (open) previous = open->setTeXclass(previous);",
        "if (open) previous = open;"
      ]
    ]
  },
  {
    "name": "first-chain-lost",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "previous = first->setTeXclass(previous);",
        "previous = first->setTeXclass(nullptr);"
      ]
    ]
  },
  {
    "name": "loop-last-child-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "index < count;",
        "index + 1 < count;"
      ]
    ]
  },
  {
    "name": "separator-class-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "if (separator) previous = separator->setTeXclass(previous);",
        "if (separator) previous = separator;"
      ]
    ]
  },
  {
    "name": "close-hook-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "if (close) previous = close->setTeXclass(previous);",
        "if (close) previous = close;"
      ]
    ]
  },
  {
    "name": "update-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "updateTeXclass(first);",
        "(void)first;"
      ]
    ]
  },
  {
    "name": "return-self",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "return previous;",
        "return this;"
      ]
    ]
  },
  {
    "name": "inherit-fake-creation-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "addFakeNodes();",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "inherit-order-reversed",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "fakeChildren{open, close}",
        "fakeChildren{close, open}"
      ]
    ]
  },
  {
    "name": "inherit-separators-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "fakeChildren.insert(fakeChildren.end(), separatorSnapshot->begin(), separatorSnapshot->end());",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "inherit-raw-display-lost",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "child->setInheritedAttributes(attributes, display, level, prime);",
        "child->setInheritedAttributes(attributes, false, level, prime);"
      ]
    ]
  },
  {
    "name": "inherit-real-children-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "AbstractMmlNode::setChildInheritedAttributes(\n        std::move(attributes), std::move(display), std::move(level), std::move(prime));",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "ascii-cr-preserved",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        " || c == u'\\r'",
        ""
      ]
    ]
  },
  {
    "name": "nbsp-removed",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "c == u' '",
        "c == u' ' || c == 0x00a0"
      ]
    ]
  },
  {
    "name": "padding-first-unit",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "separatorText.push_back(separatorText.back());",
        "separatorText.push_back(separatorText.front());"
      ]
    ]
  },
  {
    "name": "separator-index-skips-lost",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "if (!child) continue;",
        "if (!child) {++separatorIndex;continue;}"
      ]
    ]
  },
  {
    "name": "push-receiver-rebound",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "pushReceiver->push_back(fake);",
        "separators->push_back(fake);"
      ]
    ]
  },
  {
    "name": "separator-virtual-defaults-injected",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "fakeNode(separatorTextValue, Missing{}, Missing{})",
        "fakeNode(separatorTextValue, PropertyList{}, PropertyNull{})"
      ]
    ]
  },
  {
    "name": "text-return-discarded",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "textChildren->push_back(textResult);",
        "textChildren->push_back(textProduct);"
      ]
    ]
  },
  {
    "name": "text-value-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "textNode->setText(std::move(character))",
        "textNode->setText(u\"\")"
      ]
    ]
  },
  {
    "name": "null-tex-default-lost",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "if (texClass.missing()) texClass = PropertyNull{};",
        "if (texClass.missing()) texClass = 0;"
      ]
    ]
  },
  {
    "name": "explicit-tex-null-defaulted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "if (texClass.missing()) texClass = PropertyNull{};",
        "if (texClass.missing() || std::holds_alternative<PropertyNull>(texClass.value)) texClass = 0;"
      ]
    ]
  },
  {
    "name": "parent-before-tex",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "node->setTexClass(std::move(texClass));\n    node->parent = this;",
        "node->parent = this;\n    node->setTexClass(std::move(texClass));"
      ]
    ]
  },
  {
    "name": "parent-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced.cpp",
    "edits": [
      [
        "node->parent = this;",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "field-inner-class-lost",
    "file": "core/mml_tree/mml_nodes/mfenced_bindings.cpp",
    "edits": [
      [
        "initializeTexClass(TEXCLASS.INNER);",
        "initializeTexClass(TEXCLASS.ORD);"
      ]
    ]
  },
  {
    "name": "field-open-reset-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced_bindings.cpp",
    "edits": [
      [
        "open=nullptr;",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "field-close-reset-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced_bindings.cpp",
    "edits": [
      [
        "close=nullptr;",
        "/* omitted */"
      ]
    ]
  },
  {
    "name": "field-separators-reset-omitted",
    "file": "core/mml_tree/mml_nodes/mfenced_bindings.cpp",
    "edits": [
      [
        "separators=std::make_shared<std::vector<MmlNode*>>();",
        "if (!separators) separators=std::make_shared<std::vector<MmlNode*>>();"
      ]
    ]
  },
  {
    "name": "defaults-alias-base",
    "file": "core/mml_tree/mml_nodes/mfenced_bindings.cpp",
    "edits": [
      [
        "return MfencedDefaults;",
        "return AbstractMmlNode::defaults();"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Mfenced baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Mfenced mutations detected; working sources and fixtures unchanged.`);
