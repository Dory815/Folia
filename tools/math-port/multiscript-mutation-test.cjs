// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-multiscript-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','util/property_host.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx','core/mml_tree/script_defaults.ixx','util/string.ixx','core/mml_tree/mo_patterns.ixx','core/mml_tree/mo_types.ixx','core/mml_tree/mo_host.ixx','core/mml_tree/operator_dictionary.ixx','core/mml_tree/arithmetic_host.ixx','core/mml_tree/table_host.ixx','core/mml_tree/multiscript_host.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','util/property_host.cpp','util/string.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp','core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp'];
const nodeNames=['math','mrow','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom','msubsup','munderover','mfrac','msqrt','mroot','mtable','mtr','mtd','mmultiscripts'];
modules.push(...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'),'core/mml_tree/mml_nodes/mo.ixx');
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']),...['bindings','lookup','classes','scripts'].map(n=>'core/mml_tree/mml_nodes/mo_'+n+'.cpp'));
modules.push('core/mml_tree/operator_class.ixx');
units.push('core/mml_tree/operator_class.cpp');
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_multiscript.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_multiscript_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_multiscript*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}



function mutation(name,file,from,to){return{name,file,edits:[[from,to]]};}
const body='core/mml_tree/mml_nodes/mmultiscripts.cpp',bindings='core/mml_tree/mml_nodes/mmultiscripts_bindings.cpp';
const mutations=[
 mutation('base-display-forced-false',body,'attributes, display, level, prime);','attributes, Property(false), level, prime);'),
 mutation('base-level-incremented',body,'attributes, display, level, prime);','attributes, display, host::add(level,1.0), prime);'),
 mutation('script-loop-start-skips-first',body,'std::size_t i = 1, n = 0','std::size_t i = 2, n = 0'),
 mutation('script-loop-array-snapshotted',body,'bool prescripts = false;\n    for (std::size_t i = 1','const auto capturedChildren=host::multiscriptChildren(*this);\n    bool prescripts = false;\n    for (std::size_t i = 1'),
 mutation('script-loop-length-captured',body,'if (i >= conditionChildren->size()) break;','if (i >= 2) break;'),
 mutation('every-marker-normalized',body,'if (!prescripts) {','if (true) {'),
 mutation('insert-parent-write-omitted',body,'host::multiscriptNode(none).parent = this;','/* parent omitted */'),
 mutation('insert-array-copy',body,'host::multiscriptInsert(spliceChildren, i, none);','host::multiscriptInsert(std::make_shared<std::vector<MmlNode*>>(*spliceChildren), i, none);'),
 mutation('insert-index-increment-omitted',body,'                    ++i;','                    /* increment omitted */'),
 mutation('insert-placeholder-inherited',body,'host::multiscriptNode(none).parent = this;','host::multiscriptNode(none).parent = this; none->setInheritedAttributes(attributes,false,host::add(level,1.0),prime);'),
 mutation('prime-raw-value-coerced',body,'? prime','? Property(true)'),
 mutation('script-prime-alternation-inverted',body,'Property(n % 2 == 0)','Property(n % 2 != 0)'),
 mutation('script-level-numeric-only',body,'host::add(level, Property(1.0))','Property(host::number(level)+1.0)'),
 mutation('append-original-prime-omitted',body,'attributes, Property(false), std::move(childLevel), prime);','attributes, Property(false), std::move(childLevel), Property(true));'),
 mutation('append-inheritance-omitted',body,'appendedChild.setInheritedAttributes(\n            attributes, Property(false), std::move(childLevel), prime);','/* appended inheritance omitted */'),
 mutation('inherit-parity-inverted',body,'parityLength % 2 == (prescripts ? 1U : 0U)','parityLength % 2 != (prescripts ? 1U : 0U)'),
 mutation('verify-fix-always-true',body,'const Property fix = options.get("fixMmultiscripts");','const Property fix = true;'),
 mutation('duplicate-error-not-short',body,'child.mError(message, ErrorOptions(options), true);','child.mError(message, ErrorOptions(options), false);'),
 mutation('duplicate-kind-order-reversed',body,'const std::string childKind = child.kind();\n                const std::string multiscriptKind = kind();','const std::string multiscriptKind = kind();\n                const std::string childKind = child.kind();'),
 mutation('duplicate-options-cloned',body,'child.mError(message, ErrorOptions(options), true);','child.mError(message, ErrorOptions(host::arithmeticCopy(options)), true);'),
 mutation('super-verification-omitted',body,'MmlMsubsup::verifyChildren(std::move(options));','/* superclass omitted */'),
 mutation('marker-null-options-early-return',body,'void MmlMprescripts::verifyTree(std::optional<PropertyList> options) {','void MmlMprescripts::verifyTree(std::optional<PropertyList> options) { if(!options)return;'),
 mutation('marker-base-verification-omitted',body,'AbstractMmlNode::verifyTree(options);','/* superclass omitted */'),
 mutation('marker-logical-parent-used',body,'MmlNode* const currentParent = parent;','MmlNode* const currentParent = Parent();'),
 mutation('marker-parent-error-not-short',body,'mError(message, host::multiscriptErrorOptions(options), true);','mError(message, host::multiscriptErrorOptions(options), false);'),
 mutation('none-null-options-early-return',body,'void MmlNone::verifyTree(std::optional<PropertyList> options) {','void MmlNone::verifyTree(std::optional<PropertyList> options) { if(!options)return;'),
 mutation('none-parent-check-skipped',body,'void MmlNone::verifyTree(std::optional<PropertyList> options) {','void MmlNone::verifyTree(std::optional<PropertyList> options) { AbstractMmlNode::verifyTree(options);return;'),
 mutation('none-arity-one',bindings,'MmlNone::arity()const{return 0.0;}','MmlNone::arity()const{return 1.0;}'),
 mutation('multiscript-arity-three',bindings,'MmlMmultiscripts::arity()const{return 1.0;}','MmlMmultiscripts::arity()const{return 3.0;}'),
 mutation('none-defaults-aliased',bindings,'return MmlNoneDefaults;','return AbstractMmlNode::defaults();'),
 mutation('factory-arguments-changed','core/mml_tree/multiscript_host.ixx','return factory.create(kind, {});','const std::array<std::any,2> args{PropertyList{},std::make_shared<std::vector<MmlNode*>>()};return factory.create(kind,args);'),
 mutation('null-options-coerced','core/mml_tree/multiscript_host.ixx','ErrorOptions(nullptr);','ErrorOptions(PropertyList{});'),
 mutation('error-option-handle-cloned','core/mml_tree/mml_node.ixx','value()const{return value_;}','value()const{if(!value_)return std::nullopt;PropertyList clone;clone.assign(*value_);return clone;}')
];


mutations.find(m=>m.name==='script-loop-array-snapshotted').edits.push(['const Children conditionChildren = host::multiscriptChildren(*this);','const Children conditionChildren = capturedChildren;'],['const Children indexedChildren = host::multiscriptChildren(*this);','const Children indexedChildren = capturedChildren;']);
mutations.push({name:'verify-fix-reread',file:body,edits:[['!host::truthy(fix)','!host::truthy(options.get("fixMmultiscripts"))'],['!host::truthy(fix)','!host::truthy(options.get("fixMmultiscripts"))']]});
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Multiscript baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} multiscript mutations detected; working sources and fixtures unchanged.`);
