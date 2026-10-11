// Root isolated acceptance. Mutants must compile and fail unchanged assertions.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-arithmetic-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx','core/mml_tree/script_defaults.ixx','util/string.ixx','core/mml_tree/mo_patterns.ixx','core/mml_tree/mo_types.ixx','core/mml_tree/mo_host.ixx','core/mml_tree/operator_dictionary.ixx','core/mml_tree/arithmetic_host.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','core/mml_tree/mml_node_host.cpp','util/string.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp','core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp'];
const nodeNames=['math','mrow','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom','msubsup','munderover','mfrac','msqrt','mroot'];
modules.push(...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'),'core/mml_tree/mml_nodes/mo.ixx');
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']),...['bindings','lookup','classes','scripts'].map(n=>'core/mml_tree/mml_nodes/mo_'+n+'.cpp'));
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(target,source){if(!fs.existsSync(target)||!fs.readFileSync(target).equals(Buffer.from(source)))fs.writeFileSync(target,source);}
for(const f of ['reference_arithmetic.cpp','reference_arithmetic_order.cpp']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_arithmetic_mutations LANGUAGES CXX)
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_arithmetic*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}

const frac='core/mml_tree/mml_nodes/mfrac.cpp',sqrt='core/mml_tree/mml_nodes/msqrt.cpp',radical='core/mml_tree/mml_nodes/mroot.cpp';
const mutation=(name,file,from,to)=>({name,file,edits:[[from,to]]});
const mutations=[
 mutation('fraction-increment-skipped',frac,'if (!host::truthy(display) || host::number(level) > 0)','if (false)'),
 mutation('fraction-increment-unconditional',frac,'if (!host::truthy(display) || host::number(level) > 0)','if (true)'),
 mutation('fraction-increment-string-concat',frac,'level = Property(host::number(level) + 1.0);','level = host::add(level, Property(1));'),
 mutation('fraction-displayscript-condition-and',frac,'!host::truthy(display) || host::number(level) > 0','!host::truthy(display) && host::number(level) > 0'),
 mutation('fraction-previous-state-omitted',frac,'getPrevClass(previous);','/* omitted */'),
 mutation('fraction-class-normalized',frac,'getPrevClass(previous);','getPrevClass(previous);setTexClass(TEXCLASS.ORD);'),
 mutation('fraction-iteration-original-length',frac,'for (std::size_t index = 0; index < children->size(); ++index)','const auto length=children->size();for (std::size_t index = 0; index < length; ++index)'),
 mutation('fraction-iterator-follows-rebinding',frac,'const Children children = childNodes;','const Children& children = childNodes;'),
 mutation('fraction-iteration-truncated-to-arity',frac,'index < children->size()','index < std::min<std::size_t>(2,children->size())'),
 mutation('fraction-child-previous-propagated',frac,'child->setTeXclass(nullptr);','child->setTeXclass(previous);'),
 mutation('fraction-return-null',frac,'return this;','return nullptr;'),
 mutation('fraction-copy-is-input-alias',frac,'host::arithmeticCopy(attributes)','attributes'),
 mutation('fraction-denominator-key-repaired',frac,'{"denalign", denalign}','{"denomalign", denalign}'),
 mutation('fraction-numerator-prime-forced',frac,'Property(false), level, prime','Property(false), level, Property(true)'),
 mutation('fraction-denominator-prime-original',frac,'level, Property(true)','level, prime'),
 mutation('fraction-display-original',frac,'Property(false), level, prime','display, level, prime'),
 mutation('fraction-indent-last-zero',frac,'{"indentshiftlast", "indentshift"}','{"indentshiftlast", "0"}'),
 mutation('fraction-alignment-receiver-snapshotted',frac,'const Property numalign = host::arithmeticAttributes(*this)->get("numalign");\n    const Property denalign = host::arithmeticAttributes(*this)->get("denomalign");','const auto captured=host::arithmeticAttributes(*this);const Property numalign=captured->get("numalign");const Property denalign=captured->get("denomalign");'),
 mutation('sqrt-previous-state-omitted',sqrt,'getPrevClass(previous);','/* omitted */'),
 mutation('sqrt-prime-original',sqrt,'Property(true));','prime);'),
 mutation('sqrt-attributes-copied',sqrt,'std::move(attributes)','host::arithmeticCopy(attributes)'),
 mutation('sqrt-return-child',sqrt,'return this;','return &child;'),
 mutation('root-level-numeric-add',radical,'host::add(level, Property(2))','Property(host::number(level)+2.0)'),
 mutation('root-base-prime-original',radical,'base.setInheritedAttributes(attributes, display, level, Property(true));','base.setInheritedAttributes(attributes, display, level, prime);'),
 mutation('root-index-prime-forced',radical,'indexLevel, prime','indexLevel, Property(true)'),
 mutation('root-index-display-original',radical,'Property(false), indexLevel','display, indexLevel'),
 mutation('root-index-attributes-copied',radical,'index.setInheritedAttributes(attributes,','index.setInheritedAttributes(host::arithmeticCopy(attributes),'),
 mutation('root-index-receiver-eager',radical,'MmlNode& base = host::arithmeticChild(*this, 0);\n    base.setInheritedAttributes(attributes, display, level, Property(true));\n\n    // JS evaluates the child-1 receiver before evaluating \`level + 2\`.\n    MmlNode& index = host::arithmeticChild(*this, 1);','MmlNode& index = host::arithmeticChild(*this, 1);MmlNode& base=host::arithmeticChild(*this,0);base.setInheritedAttributes(attributes,display,level,Property(true));'),
 mutation('root-tex-index-receiver-eager',radical,'MmlNode& base = host::arithmeticChild(*this, 0);\n    base.setTeXclass(nullptr);\n\n    // Resolve child 1 after child 0\'s virtual call; that callback may have\n    // replaced the source childNodes array.\n    MmlNode& index = host::arithmeticChild(*this, 1);','MmlNode& index=host::arithmeticChild(*this,1);MmlNode& base=host::arithmeticChild(*this,0);base.setTeXclass(nullptr);'),
 mutation('root-previous-state-omitted',radical,'getPrevClass(previous);','/* omitted */'),
 mutation('root-class-field-missing','core/mml_tree/mml_nodes/mroot_bindings.cpp','initializeTexClass(TEXCLASS.ORD);','/* no class initialization */'),
 mutation('sqrt-class-field-missing','core/mml_tree/mml_nodes/msqrt_bindings.cpp','initializeTexClass(TEXCLASS.ORD);','/* no class initialization */'),
 mutation('fraction-default-bevelled-true','core/mml_tree/mml_nodes/mfrac_bindings.cpp','value.setOwn("bevelled",false);','value.setOwn("bevelled",true);'),
 mutation('source-copy-modern-spread','core/mml_tree/arithmetic_host.ixx','copy.assign(input);','for(const auto&key:input.keys())copy.setOwn(key,input.getOwn(key));')
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Arithmetic baseline compiles and passes.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const [f,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',f))),'Working fixture changed: '+f);
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working implementation changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} arithmetic mutations detected; working sources and fixtures unchanged.`);
