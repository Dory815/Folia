// Root acceptance, isolated source copies; working implementation is never edited.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-mml-node-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp'];
const copies=new Map();fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const fixture=fs.readFileSync(path.join(root,'tests/math/reference_mml_node.cpp'));
fs.writeFileSync(path.join(work,'reference_mml_node.cpp'),fixture);
for(const file of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',file));const source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_mml_node_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_mml_node.cpp")
set_target_properties(MutationTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(MutationTests PRIVATE "${slash(path.join(root,'tests'))}")
target_link_libraries(MutationTests PRIVATE math Boost::ut_module)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_mml_node*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('MML node baseline compiles and passes.');
const normal='core/mml_tree/mml_node.cpp',branches='core/mml_tree/mml_branches.cpp',empty='core/mml_tree/mml_empty.cpp';
const mutations=[
 {name:'spacing-signs-lost',file:normal,edits:[['{{ 0,-1, 2, 3, 0, 0, 0, 1}}','{{ 0, 1, 2, 3, 0, 0, 0, 1}}']]},
 {name:'spacing-virtual-class-bypassed',file:normal,edits:[['const double current = texClass().value_or(0);','const double current = texclass_.value_or(0);']]},
 {name:'spacing-invalid-column-short-circuits',file:normal,edits:[['const auto column = class_index(normalized, TEXSPACE[*row].size());','const auto column = class_index(normalized, TEXSPACE[*row].size()); if (!column) return Missing{};']]},
 {name:'copy-id-preserved-by-default',file:normal,edits:[['if (name != "id" || keepIds)', 'if (true)']]},
 {name:'copy-keepIds-forwarded-to-children',file:normal,edits:[['copied->appendChild(child->copy());','copied->appendChild(child->copy(keepIds));']]},
 {name:'copy-attributes-discarded',file:normal,edits:[['target->set(name, sourceAttributes.getOwn(name));','/* copied attribute lost */']]},
 {name:'inferred-conversion-alias-lost',file:normal,edits:[['child->setAttributes(original->attributes());','/* converted attributes lost */']]},
 {name:'empty-array-retargeted-during-iteration',file:normal,edits:[['const auto children = this->childNodes;','const auto& children = this->childNodes;']]},
 {name:'inherit-stop-copy-lost',file:normal,edits:[['AttributeList updated;\n            updated.assign(inherited);','AttributeList updated=inherited;']]},
 {name:'inherit-null-treated-as-missing',file:normal,edits:[['if (attrs->getExplicit("displaystyle").missing())','if (attrs->getExplicit("displaystyle").missing() || std::holds_alternative<PropertyNull>(attrs->getExplicit("displaystyle").value))']]},
 {name:'inherit-raw-child-level-converted',file:normal,edits:[['child->setInheritedAttributes(inherited, display, level, prime);','child->setInheritedAttributes(inherited, display, host::number(level), prime);']]},
 {name:'verify-variants-ignored',file:normal,edits:[['if (host::truthy(variant) && !property_set_contains(variant) &&','if (false && host::truthy(variant) && !property_set_contains(variant) &&']]},
 {name:'layout-virtual-core-used-for-class-update',file:branches,edits:[['updateTeXclass(child_zero(*this));','updateTeXclass(core());']]},
 {name:'token-inheritance-includes-text-empty',file:branches,edits:[['if (auto* const child = dynamic_cast<AbstractMmlNode*>((*children)[index])) {','if (auto* const child = (*children)[index]) {']]},
 {name:'base-script-array-retargeted',file:branches,edits:[['for (MmlNode* const child : scripts) {','for (std::size_t i=1;i<childNodes->size();++i) { MmlNode* const child=(*childNodes)[i];']]},
 {name:'text-debug-calls-virtual-getter',file:empty,edits:[['host::debugText(text_)','host::debugText(getText())']]},
 {name:'xml-copy-retains-adaptor',file:empty,edits:[['return result->setXML(clone);','return result->setXML(clone,adaptor_);']]},
 {name:'empty-class-becomes-ord',file:empty,edits:[['AbstractMmlEmptyNode::texClass() const { return TEXCLASS.NONE; }','AbstractMmlEmptyNode::texClass() const { return TEXCLASS.ORD; }']]},
 {name:'native-budget-premature-limit',file:'core/tree/node_types.ixx',edits:[['limit=512;','limit=511;']]},
 {name:'token-walk-includes-text-empty',file:branches,edits:[['if (auto* const child = dynamic_cast<AbstractMmlNode*>((*children)[index])) {\n            child->walkTree(visit, data);','if (auto* const child = (*children)[index]) {\n            child->walkTree(visit, data);']]}
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7);
const selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}
 try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_mml_node.cpp'))),'Working fixture changed');
for(const [file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working implementation changed: '+file);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} MML node mutations detected; working sources and fixtures unchanged.`);
