// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-html-node-mutations');
const modules=["util/option_types.ixx","core/tree/property.ixx","util/callable_types.ixx","util/object_types.ixx","core/tree/factory_types.ixx","core/tree/factory.ixx","core/tree/node_factory.ixx","core/tree/node_types.ixx","core/tree/node.ixx","util/js_number.ixx","util/js_numeric_input.ixx","util/utf16_types.ixx","util/utf16.ixx","core/mml_tree/attributes.ixx","util/property_host.ixx","core/mml_tree/mml_node_host.ixx","core/mml_tree/mml_node.ixx","core/mml_tree/class_binding.ixx","core/mml_tree/leaf_host.ixx","core/mml_tree/leaf_defaults.ixx","core/mml_tree/html_adaptor.ixx","core/mml_tree/mml_nodes/html_node.ixx","core/mml_tree/mml_nodes/math.ixx","core/mml_tree/mml_nodes/mi.ixx","core/mml_tree/mml_nodes/mrow.ixx"];
const units=["core/mml_tree/mml_bindings.cpp","core/mml_tree/mml_node.cpp","core/mml_tree/mml_branches.cpp","core/mml_tree/mml_empty.cpp","util/property_host.cpp","core/mml_tree/mml_nodes/html_node_bindings.cpp","core/mml_tree/mml_nodes/html_node.cpp","core/mml_tree/mml_nodes/math_bindings.cpp","core/mml_tree/mml_nodes/math.cpp","core/mml_tree/mml_nodes/mi_bindings.cpp","core/mml_tree/mml_nodes/mi.cpp","core/mml_tree/mml_nodes/mrow_bindings.cpp","core/mml_tree/mml_nodes/mrow.cpp"];
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_html_node.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_html_node_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_html_node*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutations=[
  {
    "name": "semantic-kind-is-xml",
    "file": "core/mml_tree/mml_nodes/html_node_bindings.cpp",
    "edits": [
      [
        "return \"html\";",
        "return \"XML\";"
      ]
    ]
  },
  {
    "name": "html-getter-bypasses-virtual",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "return getXML();",
        "return xml_;"
      ]
    ]
  },
  {
    "name": "attribute-name-altered",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "\"data-mjx-hdw\"",
        "\"hdw\""
      ]
    ]
  },
  {
    "name": "attribute-attempt-omitted",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "(void)htmlAdaptor(adaptor).getAttribute(html, \"data-mjx-hdw\");",
        "(void)0;"
      ]
    ]
  },
  {
    "name": "attribute-payload-is-null",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "getAttribute(html, \"data-mjx-hdw\")",
        "getAttribute(nullptr, \"data-mjx-hdw\")"
      ]
    ]
  },
  {
    "name": "catch-only-native-error",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "catch (...)",
        "catch (const Error&)"
      ]
    ]
  },
  {
    "name": "wrapper-kind-is-div",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        ".node(\"span\",",
        ".node(\"div\","
      ]
    ]
  },
  {
    "name": "wrapper-return-ignored",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "html = fallbackAdaptor.node",
        "(void)fallbackAdaptor.node"
      ]
    ]
  },
  {
    "name": "wrapper-payload-is-null",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "std::vector<XmlRef> children{html};",
        "std::vector<XmlRef> children{nullptr};"
      ]
    ]
  },
  {
    "name": "wrapper-attributes-reused",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "node(\"span\", PropertyList{},",
        "node(\"span\", ([] {static PropertyList attrs;return attrs;})(),"
      ]
    ]
  },
  {
    "name": "setter-adaptor-omitted",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "return setXML(std::move(html), adaptor);",
        "return setXML(std::move(html));"
      ]
    ]
  },
  {
    "name": "setter-virtual-dispatch-bypassed",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "return setXML(std::move(html), adaptor);",
        "return XMLNode::setXML(std::move(html), adaptor);"
      ]
    ]
  },
  {
    "name": "setter-return-is-this",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "return setXML(std::move(html), adaptor);",
        "setXML(std::move(html), adaptor);return this;"
      ]
    ]
  },
  {
    "name": "null-wrapper-return-rejected",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "return setXML(std::move(html), adaptor);",
        "if(!html)host::typeError(\"Null wrapper\");return setXML(std::move(html), adaptor);"
      ]
    ]
  },
  {
    "name": "html-serialization-is-xml",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        ".outerHTML(xml_)",
        ".serializeXML(xml_)"
      ]
    ]
  },
  {
    "name": "text-content-is-empty",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "return htmlAdaptor(adaptor_).textContent(xml_);",
        "return {};"
      ]
    ]
  },
  {
    "name": "host-kind-is-semantic-kind",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "const std::string htmlKind = htmlAdaptor(adaptor_).kind(xml_);",
        "const std::string htmlKind = kind();"
      ]
    ]
  },
  {
    "name": "host-kind-read-twice",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "const std::string htmlKind = htmlAdaptor(adaptor_).kind(xml_);",
        "const std::string htmlKind = htmlAdaptor(adaptor_).kind(xml_);(void)htmlAdaptor(adaptor_).kind(xml_);"
      ]
    ]
  },
  {
    "name": "verification-token-test-inverted",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "!currentParent->isToken()",
        "currentParent->isToken()"
      ]
    ]
  },
  {
    "name": "verification-short-flag-false",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "htmlErrorOptions(options), true",
        "htmlErrorOptions(options), false"
      ]
    ]
  },
  {
    "name": "verification-message-altered",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "HTML can only be a child of a token element",
        "HTML must be token"
      ]
    ]
  },
  {
    "name": "verification-options-replaced",
    "file": "core/mml_tree/mml_nodes/html_node.cpp",
    "edits": [
      [
        "ErrorOptions(*options)",
        "ErrorOptions(PropertyList{})"
      ]
    ]
  },
  {
    "name": "text-binding-not-virtual",
    "file": "core/mml_tree/mml_nodes/html_node.ixx",
    "edits": [
      [
        "return textContent();",
        "return HtmlNode::textContent();"
      ]
    ]
  },
  {
    "name": "text-binding-absent",
    "file": "core/mml_tree/mml_nodes/html_node.ixx",
    "edits": [
      [
        "TextContentBinding::textContent=[this] {return textContent();};",
        "TextContentBinding::textContent=std::nullopt;"
      ]
    ]
  },
  {
    "name": "copy-adaptor-repaired",
    "file": "core/mml_tree/mml_empty.cpp",
    "edits": [
      [
        "return result->setXML(clone);",
        "return result->setXML(clone,adaptor_);"
      ]
    ]
  },
  {
    "name": "copy-factory-after-clone",
    "file": "core/mml_tree/mml_empty.cpp",
    "edits": [
      [
        "MmlNode* const created = factory.create(kind(), {});\n    if (!adaptor_) host::typeError(\"Cannot read properties of null (reading 'clone')\");\n    XmlRef const clone = adaptor_->clone(xml_);",
        "if (!adaptor_) host::typeError(\"Cannot read properties of null (reading 'clone')\");\n    XmlRef const clone = adaptor_->clone(xml_);\n    MmlNode* const created = factory.create(kind(), {});"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('HTML baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} HTML mutations detected; working sources and fixtures unchanged.`);
