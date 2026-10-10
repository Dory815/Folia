// Root acceptance, isolated source copies; working implementation is never edited.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-leaf-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp'];
const nodeNames=['mrow','math','mi','mn','mtext','ms','mspace','mstyle','merror','mphantom'];
modules.push('core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx',...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'));
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']));
const copies=new Map();fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const fixture=fs.readFileSync(path.join(root,'tests/math/reference_leaf.cpp'));
fs.writeFileSync(path.join(work,'reference_leaf.cpp'),fixture);
for(const file of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',file));const source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_leaf_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_leaf.cpp")
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_leaf*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Concrete MML baseline compiles and passes.');
const file=n=>'core/mml_tree/mml_nodes/'+n+'.cpp',host='core/mml_tree/leaf_host.ixx';
const mutations=[
 {name:'mi-surrogate-prefix-lost',file:host,edits:[['s.front()<=0xdbff&&match(1)','s.front()<=0xdbff&&false&&match(1)']]},
 {name:'mi-combining-range-extended',file:host,edits:[['c<=0x1abe','c<=0x1aff']]},
 {name:'whitespace-NEL-admitted',file:'util/js_numeric_input.ixx',edits:[['case 0x0020: case 0x00A0: case 0x1680:','case 0x0085: case 0x0020: case 0x00A0: case 0x1680:']]},
 {name:'mi-operator-virtual-setter-bypassed',file:file('mi'),section:'MmlNode* MmlMi::setTeXclass',edits:[['setTexClass(static_cast<double>(TEXCLASS.OP));','initializeTexClass(static_cast<double>(TEXCLASS.OP));']]},
 {name:'mi-incoming-variant-ignored',file:file('mi'),edits:[['!host::truthy(attributes.get("mathvariant"))','true']]},
 {name:'mi-autoOP-null-taken-as-missing',file:file('mi'),edits:[['getProperty("autoOP").missing()','host::nullish(getProperty("autoOP"))']]},
 {name:'row-delimiter-presence-taken-as-truth',file:file('mrow'),edits:[['!host::nullish(getProperty("open")) || !host::nullish(getProperty("close"))','host::truthy(getProperty("open")) || host::truthy(getProperty("close"))']]},
 {name:'row-class-iterator-retargeted',file:file('mrow'),section:'MmlNode* MmlMrow::setTeXclass',edits:[['\n    const auto children = this->childNodes;','\n    const auto& children = this->childNodes;']]},
 {name:'row-last-search-array-captured',file:file('mrow'),edits:[['while (--index >= 0) {\n        const auto children = this->childNodes;','const auto frozenChildren=this->childNodes; while (--index >= 0) {\n        const auto children = frozenChildren;']]},
 {name:'row-core-array-read-before-predicate',file:file('mrow'),section:'MmlNode* MmlMrow::core()',edits:[['if (!isEmbellished() || !core_) return this;\n    const auto children = this->childNodes;','const auto children = this->childNodes; if (!isEmbellished() || !core_) return this;']]},
 {name:'inferred-join-live-length',file:file('mrow'),section:'std::string MmlInferredMrow::toString',edits:[['i < length; ++i','i < children->size(); ++i']]},
 {name:'inferred-join-retargeted-array',file:file('mrow'),section:'std::string MmlInferredMrow::toString',edits:[['const auto children = this->childNodes;','const auto& children = this->childNodes;']]},
 {name:'mspace-linebreak-truthiness',file:file('mspace'),edits:[['!nodeAttributes->hasExplicit("linebreak")','!host::truthy(nodeAttributes->getExplicit("linebreak"))']]},
 {name:'mspace-width-not-trimmed',file:file('mspace'),edits:[['host::trim(width)','std::u16string_view(width)']]},
 {name:'mspace-indenting-newline-lost',file:file('mspace'),edits:[['linebreak == Property(std::string("indentingnewline"))','false']]},
 {name:'mspace-break-constraint-ignored-for-newline',file:file('mspace'),edits:[['if (!canBreak()) return false;','/* constraint lost */']]},
 {name:'mstyle-entry-argument-defaults-lost',file:file('mstyle'),edits:[['host::inheritanceDefaults(display, level, prime);','/* defaults lost before partial writes */']]},
 {name:'mstyle-relative-level-numeric-only',file:file('mstyle'),edits:[['host::add(level, Property(host::integer(scriptText)))','Property(host::number(level)+host::integer(scriptText))']]},
 {name:'mstyle-script-prime-reset-lost',file:file('mstyle'),edits:[['prime = false;','/* prime reset lost */']]},
 {name:'mstyle-display-truthiness',file:file('mstyle'),edits:[['display = displaystyle == Property(true);','display = host::truthy(displaystyle);']]},
 {name:'mstyle-cramped-coerced-to-bool',file:file('mstyle'),edits:[['if (!host::nullish(cramped)) prime = cramped;','if (!host::nullish(cramped)) prime = host::truthy(cramped);']]},
 {name:'mstyle-all-child-dispatch',file:file('mstyle'),section:'void MmlMstyle::setChildInheritedAttributes',edits:[['children->front()->setInheritedAttributes(std::move(attributes), std::move(display),','for(auto* child:*children) child->setInheritedAttributes(attributes, display,'],['std::move(level), std::move(prime));','level, prime);']]},
 {name:'math-defaults-read-from-current-registry',file:file('math'),edits:[['constructorDefaults().get("scriptlevel")','std::any_cast<std::shared_ptr<MmlClassInfo>>(factory.getNodeClass(kind())->staticData)->getDefaults()->get("scriptlevel")']]},
 {name:'mi-field-phase-uses-virtual-setter',file:file('mi'),section:'void MmlMi::initializeClassFields',edits:[['initializeTexClass(static_cast<double>(TEXCLASS.ORD));','setTexClass(static_cast<double>(TEXCLASS.ORD));']]},
 {name:'class-field-phase-precedes-super',file:'core/mml_tree/class_binding.ixx',edits:[['node->initializeMml(args);\n            node->initializeClassFields();','node->initializeClassFields();\n            node->initializeMml(args);']]},
 {name:'math-null-options-repaired-to-empty-map',file:file('math'),edits:[['ErrorOptions(nullptr)','ErrorOptions{}']]},
 {name:'inferred-defaults-alias-lost',file:'core/mml_tree/mml_nodes/mrow_bindings.cpp',edits:[['return MmlMrow::defaults();','static PropertyList copied;static bool done=false;if(!done){copied.assign(MmlMrow::defaults());done=true;}return copied;']]},
 {name:'native-budget-premature-limit',file:'core/tree/node_types.ixx',edits:[['limit=512;','limit=511;']]},
 {name:'math-display-attributes-getter-collapsed',file:file('math'),edits:[['attrs = attributes();\n            if (!attrs) host::typeError("Cannot read display from null attributes");','/* third virtual read lost */']]}
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');
for(const m of selected){const {source,target}=copies.get(m.file);const start=m.section?source.indexOf(m.section):0;assert.ok(start>=0,'Missing section '+m.name);let changed=source.slice(start);for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}changed=source.slice(0,start)+changed;
 try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_leaf.cpp'))),'Working fixture changed');
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working source changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} concrete MML mutations detected; working sources and fixtures unchanged.`);
