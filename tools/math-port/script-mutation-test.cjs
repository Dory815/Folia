// Root acceptance, isolated source copies; working implementation is never edited.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-script-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp'];
const nodeNames=['mrow','math','msubsup','munderover'];
modules.push('core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx',...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'));
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']));
modules.push('util/string.ixx','core/mml_tree/script_defaults.ixx'); units.push('util/string.cpp');
const copies=new Map();fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const fixture=fs.readFileSync(path.join(root,'tests/math/reference_script.cpp'));
fs.writeFileSync(path.join(work,'reference_script.cpp'),fixture);
for(const file of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',file));const source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_script_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_script.cpp")
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_script*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('P10 baseline compiles and passes.');
const str='util/string.cpp',sub='core/mml_tree/mml_nodes/msubsup.cpp',under='core/mml_tree/mml_nodes/munderover.cpp';
const mutations=[
 {name:'sort-length-ascending',file:str,edits:[['static_cast<double>(b.size()) - static_cast<double>(a.size())','static_cast<double>(a.size()) - static_cast<double>(b.size())']]},
 {name:'quote-slash-too',file:str,edits:[['u"^$(){}.+*?-|[]:\\\\"','u"/^$(){}.+*?-|[]:\\\\"']]},
 {name:'unicode-chars-unit-iteration',file:str,edits:[['const auto points = folia::math::utf16::codePoints(text);','const std::vector<char32_t> points(text.begin(),text.end());']]},
 {name:'unicode-string-surrogates-rejected',file:str,section:'std::u16string unicodeString',edits:[['if (!std::isfinite(point) ||','if ((point>=0xD800&&point<=0xDFFF) || !std::isfinite(point) ||']]},
 {name:'unicode-string-fractions-truncated',file:str,section:'std::u16string unicodeString',edits:[[' ||\n            std::trunc(point) != point','']]},
 {name:'percent-NEL-admitted',file:str,section:'bool isPercent',edits:[['folia::math::js_input::isWhitespace(text.back())','(text.back()==0x0085||folia::math::js_input::isWhitespace(text.back()))']]},
 {name:'split-empty-array',file:str,edits:[['if (text.empty()) return {std::u16string{}};','if (text.empty()) return {};']]},
 {name:'unicode-bare-three-digits',file:str,edits:[['text.substr(after_u, 4)','text.substr(after_u, 3)'],['i = after_u + 4;','i = after_u + 3;']]},
 {name:'unicode-generic-escape-accepted',file:str,edits:[['invalid_code_point(std::numeric_limits<double>::quiet_NaN());','result.push_back(text[i+1]); i+=2;']]},
 {name:'unicode-replacement-rescanned',file:str,section:'std::u16string replaceUnicode',edits:[['    return result;\n}\n\nstd::u16string toEntity','    if(result==text)return result;return replaceUnicode(result);\n}\n\nstd::u16string toEntity']]},
 {name:'entity-lowercase-hex',file:str,edits:[['u"0123456789ABCDEF"','u"0123456789abcdef"']]},
 {name:'error-number-format-native',file:str,edits:[['folia::math::js_number::toString(point)','std::to_string(point)']]},
 {name:'sub-level-number-addition',file:sub,edits:[['host::add(level, Property(1.0))','Property(host::number(level)+1)']]},
 {name:'sub-array-retargeted',file:sub,edits:[['const auto nodes = this->childNodes;','const auto& nodes = this->childNodes;']]},
 {name:'sub-prime-bool-coerced',file:sub,edits:[['subPrime = prime;','subPrime = Property(host::truthy(prime));']]},
 {name:'sup-uses-sub-one',file:sub,edits:[['supPrime = Property(subIndex == 2);','supPrime = Property(subIndex == 1);']]},
 {name:'under-over-getter-eager',file:under,edits:[['Property basePrime = prime;\n    if (!host::truthy(prime))\n        basePrime = Property(child_at_or_null(nodes, over()) != nullptr);','auto eagerOver=over(); Property basePrime=prime; if(!host::truthy(prime))basePrime=Property(child_at_or_null(nodes,eagerOver)!=nullptr);']]},
 {name:'under-force-ignored',file:under,edits:[['force = host::truthy(coreAttributes->get("movablelimits"));','force = false;']]},
 {name:'under-level-string-plus',file:under,edits:[['Property(host::number(level) + 1)','Property(host::debugText(host::string(level)+u"1"))']]},
 {name:'under-constructor-current-registry',file:under,edits:[['const auto accents = constructorAccents();','const auto accents=std::any_cast<std::shared_ptr<MmlClassInfo>>(factory.getNodeClass(kind())->staticData)->accentsBinding();']]},
 {name:'under-captured-accents-copied',file:under,edits:[['const auto accents = constructorAccents();','const auto accents=std::make_shared<std::vector<Property>>(*constructorAccents());']]},
 {name:'under-accent-current-slot-cached',file:under,edits:[['const Property underAccentForInheritance = accent_at(accents, 1);','const Property underAccentForInheritance = underAccent;']]},
 {name:'under-accent-attributes-getter-collapsed',file:under,section:'void MmlMunderover::setInheritedAccent',edits:[['currentAttributes = attributes();\n    if (!currentAttributes) host::typeError("Cannot set inherited accent on null attributes");','/* fresh getter lost */']]},
 {name:'under-accent-NaN-equal',file:under,edits:[['if (value == defaultValue) return;','if (host::sameValueZero(value,defaultValue)) return;']]},
 {name:'under-reinherit-display-forced-false',file:under,edits:[['node->setInheritedAttributes(std::move(emptyAttributes), std::move(display),','node->setInheritedAttributes(std::move(emptyAttributes), Property(false),']]},
 {name:'under-prime-raw-lost',file:under,edits:[['if (host::truthy(left)) return left;','if (host::truthy(left)) return Property(true);']]},
 {name:'munder-accent-alias-lost',file:'core/mml_tree/mml_nodes/munderover.ixx',edits:[['class MmlMunder:public MmlMunderover {','class MmlMunder:public MmlMunderover { public: static std::shared_ptr<std::vector<Property>>& accents(){static auto copied=std::make_shared<std::vector<Property>>(*MmlMunderover::accents());return copied;}']]},
 {name:'native-script-budget-premature',file:'core/tree/node_types.ixx',edits:[['limit=512;','limit=511;']]}
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');
for(const m of selected){const {source,target}=copies.get(m.file);const start=m.section?source.indexOf(m.section):0;assert.ok(start>=0,'Missing section '+m.name);let changed=source.slice(start);for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}changed=source.slice(0,start)+changed;
 try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_script.cpp'))),'Working fixture changed');
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working source changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} P10 mutations detected; working sources and fixtures unchanged.`);
