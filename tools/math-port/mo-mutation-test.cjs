// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-mo-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx','core/mml_tree/script_defaults.ixx','util/string.ixx','core/mml_tree/mo_patterns.ixx','core/mml_tree/mo_types.ixx','core/mml_tree/mo_host.ixx','core/mml_tree/operator_dictionary.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','core/mml_tree/mml_node_host.cpp','util/string.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp','core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp'];
const nodeNames=['math','mrow','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom','msubsup','munderover'];
modules.push(...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'),'core/mml_tree/mml_nodes/mo.ixx');
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']),...['bindings','lookup','classes','scripts'].map(n=>'core/mml_tree/mml_nodes/mo_'+n+'.cpp'));
modules.push('core/mml_tree/operator_class.ixx');
units.push('core/mml_tree/operator_class.cpp');
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_mo.cpp','reference_mo_order.cpp','reference_mo_values.cpp','reference_mo_patterns.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_mo_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_mo*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const lookup='core/mml_tree/mml_nodes/mo_lookup.cpp',classes='core/mml_tree/mml_nodes/mo_classes.cpp',operatorClass='core/mml_tree/operator_class.cpp',scripts='core/mml_tree/mml_nodes/mo_scripts.cpp';
const mutation=(name,file,from,to)=>({name,file,edits:[[from,to]]});
const mutations=[
 mutation('undefined-treated-as-null',lookup,'_texClass == Property(PropertyNull{})','host::nullish(_texClass)'),
 mutation('lazy-getter-caches',lookup,'return host::get(definition, "2");','return _texClass = host::get(definition, "2");'),
 mutation('spacing-requires-both',lookup,'if (firstAttributes->isSet("lspace")) return true;','if (!firstAttributes->isSet("lspace")) return false;'),
 mutation('no-dict-marker-omitted',lookup,'setProperty("noDictDef", true);','/* omitted source miss marker */'),
 mutation('operator-class-condition-inverted',lookup,'if (texClassProperty.missing())','if (!texClassProperty.missing())'),
 mutation('spacing-uses-module-not-constructor',lookup,'host::moStatic<Property>(services.spacing)','Property(operator_dict::MMLSPACING())'),
 mutation('lookup-hit-copied',lookup,'if (host::truthy(definition)) return definition;','if(host::truthy(definition)){if(auto a=PropertyArray::array(definition))return Property(PropertyArray{a->get(0),a->get(1),a->get(2),a->get(3)});return definition;}'),
 mutation('left-spacing-denominator',lookup,'lspace = host::number(left) / 18.0;','lspace = host::number(left) / 12.0;'),
 mutation('explicit-form-filter-inverted',lookup,'if (!(name == form)) result.push(name);','if (name == form) result.push(name);'),
 mutation('sparse-filter-ignores-inherited',lookup,'if (!formRecord.has(key)) continue;','if (!formRecord.hasOwn(key)) continue;'),
 mutation('prefix-position-is-postfix',lookup,'return formOrder("prefix", "infix", "postfix");','return formOrder("postfix", "infix", "prefix");'),
 mutation('attribute-receiver-after-definition',lookup,'const auto currentAttributes = host::moAttributes(*this);\n        const Property currentProperties = host::get(definition, "3");\n        const Property value = host::get(currentProperties, name);','const Property currentProperties = host::get(definition, "3");\n        const Property value = host::get(currentProperties, name);\n        const auto currentAttributes = host::moAttributes(*this);'),
 mutation('coreparent-compares-kind-label',classes,'!host::moInstance(parent, math)','parent->kind() != "math"'),
 mutation('explicit-parent-accent-returned',classes,'if (value.missing()) {','if (!value.missing()) accent=value;\n            if (value.missing()) {'),
 mutation('autoop-not-demoted',operatorClass,'prev->setTexClass(TEXCLASS.ORD);\n            prevClassValue = TEXCLASS.ORD;','/* no previous class write */\n            prevClassValue = TEXCLASS.ORD;'),
 mutation('binary-after-open-not-demoted',operatorClass,'prevClassValue == Property(TEXCLASS.OPEN) ||','false ||'),
 mutation('binary-last-child-not-demoted',operatorClass,'if (!children.empty() && children.back() == child) receiver.setTexClass(TEXCLASS.ORD);','/* omitted final child rule */'),
 mutation('binary-previous-chain-order',operatorClass,'receiver.setPrevClass(TEXCLASS.ORD);\n        prev->setTexClass(TEXCLASS.ORD);','prev->setTexClass(TEXCLASS.ORD);\n        receiver.setPrevClass(TEXCLASS.ORD);'),
 mutation('coretext-singleton-not-unwrapped',classes,'parent->childNodes->size() == 1;','parent->childNodes->size() == 2;'),
 mutation('parent-read-after-accent-property',scripts,'MmlNode* const parent = Parent();\n    if (!getProperty("mathaccent").missing()) return;','if (!getProperty("mathaccent").missing()) return;\n    MmlNode* const parent = Parent();'),
 mutation('child-snapshot-read-after-base-callback',scripts,'MmlNode* const under = nodes->size() > 1 ? (*nodes)[1] : nullptr;\n    MmlNode* const over = nodes->size() > 2 ? (*nodes)[2] : nullptr;\n    if (!base) host::typeError("Cannot read base embellishment");\n    if (base->isEmbellished() && base->coreMO() == this) return;','if (!base) host::typeError("Cannot read base embellishment");\n    if (base->isEmbellished() && base->coreMO() == this) return;\n    MmlNode* const under = nodes->size() > 1 ? (*nodes)[1] : nullptr;\n    MmlNode* const over = nodes->size() > 2 ? (*nodes)[2] : nullptr;'),
 mutation('source-isover-bug-repaired',scripts,'isOver = under->coreMO() == this;','isOver = over->coreMO() == this;'),
 mutation('pseudo-in-msup-is-true',scripts,'!(parent->isKind("msubsup") && !parent->isKind("msub"))','true'),
 mutation('inheritance-check-order-swapped',scripts,'checkPseudoScripts(mo);\n    checkPrimes(mo);','checkPrimes(mo);\n    checkPseudoScripts(mo);'),
 mutation('optional-empty-uses-default-text',scripts,'const std::u16string defaultText = text ? std::u16string{} : getText();','if(text&&text->empty())text.reset();\n    const std::u16string defaultText = text ? std::u16string{} : getText();'),
 mutation('prime-remap-key-shifted',scripts,'static_cast<std::uint32_t>(codepoint)','static_cast<std::uint32_t>(codepoint)+1'),
 mutation('class-fields-initialized-before-super','core/mml_tree/mml_nodes/mo.ixx','Property _texClass=Missing{},lspace=Missing{},rspace=Missing{};','Property _texClass=PropertyNull{},lspace=5.0/18.0,rspace=5.0/18.0;'),
 mutation('captured-static-table-snapshotted','core/mml_tree/mml_nodes/mo.ixx','s->table=[] {return T::operatorTable();};','const auto captured=T::operatorTable();s->table=[captured] {return captured;};'),
 mutation('function-poison-accessor-missing','core/mml_tree/mml_node_host.ixx','if(key=="caller"||key=="arguments")typeError("Restricted function property");','/* restricted accessor omitted */'),
 mutation('function-prototype-is-object-prototype','core/mml_tree/mml_node_host.ixx','?BuiltinProperty::prototype:BuiltinProperty::functionPrototype;','?BuiltinProperty::prototype:BuiltinProperty::prototype;'),
 mutation('function-call-identity-is-apply','core/mml_tree/mml_node_host.ixx','if(key=="call")return BuiltinProperty::functionCall;','if(key=="call")return BuiltinProperty::functionApply;'),
 mutation('string-property-keys-omitted','core/mml_tree/mo_host.ixx','for(std::size_t i=0;i<text(*s).size();++i)','for(std::size_t i=0;i<0;++i)'),
 mutation('string-property-keys-count-utf8-bytes','core/mml_tree/mo_host.ixx','for(std::size_t i=0;i<text(*s).size();++i)','for(std::size_t i=0;i<s->size();++i)')
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Mo baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Mo mutations detected; working sources and fixtures unchanged.`);
