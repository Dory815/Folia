// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-table-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx','core/mml_tree/script_defaults.ixx','util/string.ixx','core/mml_tree/mo_patterns.ixx','core/mml_tree/mo_types.ixx','core/mml_tree/mo_host.ixx','core/mml_tree/operator_dictionary.ixx','core/mml_tree/arithmetic_host.ixx','core/mml_tree/table_host.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','core/mml_tree/mml_node_host.cpp','util/string.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp','core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp'];
const nodeNames=['math','mrow','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom','msubsup','munderover','mfrac','msqrt','mroot','mtable','mtr','mtd'];
modules.push(...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'),'core/mml_tree/mml_nodes/mo.ixx');
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']),...['bindings','lookup','classes','scripts'].map(n=>'core/mml_tree/mml_nodes/mo_'+n+'.cpp'));
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_table.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_table_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_table*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}


const table='core/mml_tree/mml_nodes/mtable.cpp',row='core/mml_tree/mml_nodes/mtr.cpp',cell='core/mml_tree/mml_nodes/mtd.cpp';
const mutation=(name,file,from,to)=>({name,file,edits:[[from,to]]});
const mutations=[
 mutation('indent-iteration-length-snapshotted',table,'for (std::size_t index = 0; index < indentAttributes.size(); ++index)','const auto length=indentAttributes.size();for(std::size_t index=0;index<length;++index)'),
 mutation('indent-tuple-before-attributes-receiver',table,'const auto receiver = host::arithmeticAttributes(*this);\n            const Property tuple = host::get(Property(attributes), name);\n            const Property value = host::get(tuple, "1");','const Property tuple=host::get(Property(attributes),name);const Property value=host::get(tuple,"1");const auto receiver=host::arithmeticAttributes(*this);'),
 mutation('explicit-indent-not-removed',table,'host::arithmeticAttributes(*this)->unset(name);','/* omitted explicit unset */'),
 mutation('forced-indent-not-inherited',table,'receiver->setInherited(name, value);','/* omitted inherited write */'),
 mutation('table-default-display-ignored',table,'displayValue = host::arithmeticAttributes(*this)->getDefault("displaystyle");','displayValue = false;'),
 mutation('table-explicit-display-ignored',table,'Property displayValue = explicitDisplay;','Property displayValue=false;'),
 mutation('table-cramped-prime-always-false',table,'Property(host::truthy(cramped))','Property(false)'),
 mutation('table-rowalign-reset-every-child',table,'rowalignValue = host::get(host::get(Property(attributes), "rowalign"), "1");','rowalignValue="center";'),
 mutation('table-null-receiver-checked-before-tuple',table,'MmlNode* const child = host::tableSlot(rows, index);','MmlNode* const child=host::tableSlot(rows,index);(void)host::tableNode(child);'),
 mutation('table-row-record-copied-per-child',table,'setInheritedAttributes(attributes, display, level,','setInheritedAttributes(host::arithmeticCopy(attributes), display, level,'),
 mutation('table-row-grouping-reset-per-child',table,'const bool isCell = child.isKind("mtd");','const bool isCell=child.isKind("mtd");currentRow=nullptr;'),
 mutation('table-removal-index-not-decremented',table,'--index;','/* no index decrement */'),
 mutation('table-fix-option-branch-inverted',table,'if (!host::truthy(options.get("fixMtables")))','if (host::truthy(options.get("fixMtables")))'),
 mutation('table-error-direct-parent-write-omitted',table,'child.parent = this;','/* direct parent write omitted */'),
 mutation('table-error-cell-short-flag-always-false',table,'ErrorOptions(options), isCell','ErrorOptions(options), false'),
 mutation('table-tex-previous-state-omitted',table,'getPrevClass(previous);','/* previous state omitted */'),
 mutation('table-tex-iteration-length-snapshotted',table,'for (std::size_t index = 0; index < children->size(); ++index)','const auto length=children->size();for(std::size_t index=0;index<length;++index)'),
 mutation('table-tex-previous-passed-to-child',table,'host::tableChild(children, index).setTeXclass(nullptr);','host::tableChild(children,index).setTeXclass(previous);'),
 mutation('row-label-side-not-prepended',row,'columnAlign.push_front(std::move(side));','/* label side omitted */'),
 mutation('row-label-break-top-not-prepended',row,'breakAlign.push_front("top");','/* label break alignment omitted */'),
 mutation('row-column-fallback-center',row,'columnValue = host::get(attributes.get("columnalign"), "1");','columnValue="center";'),
 mutation('row-vertical-fallback-literal-top',row,'verticalAlign = host::get(attributes.get("data-break-align"), "1");','verticalAlign="top";'),
 mutation('row-attributes-copied-per-child',row,'setInheritedAttributes(attributes, display, level, prime)','setInheritedAttributes(host::arithmeticCopy(attributes),display,level,prime)'),
 mutation('row-null-receiver-checked-before-tuple',row,'MmlNode* const child = host::tableSlot(inheritedChildren, index);','MmlNode* const child=host::tableSlot(inheritedChildren,index);(void)host::tableNode(child);'),
 mutation('row-invalid-parent-check-skipped',row,'if (this->parent && !this->parent->isKind("mtable"))','if (false)'),
 mutation('row-invalid-parent-return-omitted',row,'return;','/* early return omitted */'),
 mutation('row-fix-option-branch-inverted',row,'if (!host::truthy(options.get("fixMtables")))','if (host::truthy(options.get("fixMtables")))'),
 mutation('row-tex-return-null',row,'return this;','return nullptr;'),
 mutation('row-tex-previous-state-omitted',row,'getPrevClass(previous);','/* previous state omitted */'),
 mutation('cell-parent-kind-label-rejected',cell,'!this->parent->isKind("mtr")','this->parent->kind()!="mtr"'),
 mutation('cell-invalid-parent-return-omitted',cell,'return;','/* early return omitted */'),
 mutation('cell-invalid-parent-error-not-short',cell,'ErrorOptions(options), true','ErrorOptions(options), false'),
 mutation('cell-tex-previous-state-omitted',cell,'getPrevClass(previous);','/* previous state omitted */'),
 mutation('cell-tex-return-child',cell,'return this;','return &child;'),
 mutation('table-properties-field-merged','core/mml_tree/mml_nodes/mtable_bindings.cpp','properties=PropertyList{{"useHeight",true}};','properties.setOwn("useHeight",true);'),
 mutation('table-texclass-field-missing','core/mml_tree/mml_nodes/mtable_bindings.cpp','initializeTexClass(TEXCLASS.ORD);','/* field phase omitted */'),
 mutation('label-arity-unbounded','core/mml_tree/mml_nodes/mtr_bindings.cpp','MmlMlabeledtr::arity()const{return 1.0;}','MmlMlabeledtr::arity()const{return std::numeric_limits<double>::infinity();}'),
 mutation('cell-linebreak-alignment-wrong','core/mml_tree/mml_nodes/mtd_bindings.cpp','MmlMtd::linebreakAlign()const{return "columnalign";}','MmlMtd::linebreakAlign()const{return "";}'),
 mutation('table-split-coerces-non-string','core/mml_tree/table_host.ixx','if (!raw) typeError("Table alignment trim is not callable");','const std::string converted=debugText(string(value));if(!raw)raw=&converted;')
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Table baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} table mutations detected; working sources and fixtures unchanged.`);
