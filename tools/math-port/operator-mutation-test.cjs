// Root acceptance, isolated source copies; working implementation is never edited.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-operator-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','util/property_host.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp'];
const nodeNames=['mrow','math'];
modules.push('core/mml_tree/class_binding.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/leaf_defaults.ixx',...nodeNames.map(n=>'core/mml_tree/mml_nodes/'+n+'.ixx'));
units.push(...nodeNames.flatMap(n=>['core/mml_tree/mml_nodes/'+n+'_bindings.cpp','core/mml_tree/mml_nodes/'+n+'.cpp']));
modules.push('core/mml_tree/operator_dictionary.ixx'); units.push('core/mml_tree/operator_dictionary_data.cpp','core/mml_tree/operator_dictionary.cpp','util/property_host.cpp');
const copies=new Map();fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const fixture=fs.readFileSync(path.join(root,'tests/math/reference_operator.cpp'));
fs.writeFileSync(path.join(work,'reference_operator.cpp'),fixture);
for(const file of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',file));const source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_operator_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_operator.cpp")
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
function run(name,pattern='*math_port_operator*'){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),[pattern],{encoding:'utf8',timeout:300000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('P11 baseline compiles and passes.');
const dictionary='core/mml_tree/operator_dictionary.cpp',data='core/mml_tree/operator_dictionary_data.cpp',property='core/tree/property.ixx';
const mutations=[
 {name:'OPDEF-null-default-repaired',file:dictionary,edits:[['if (texClass.missing())','if (texClass.missing()||std::holds_alternative<PropertyNull>(texClass.value))']]},
 {name:'OPDEF-wrong-default-class',file:dictionary,edits:[['texClass = static_cast<double>(TEXCLASS.BIN)','texClass = static_cast<double>(TEXCLASS.REL)']]},
 {name:'OPDEF-default-props-object',file:dictionary,edits:[['if (properties.missing()) properties = PropertyNull{};','if (properties.missing()) properties = PropertyList{};']]},
 {name:'OPDEF-class-coerced',file:dictionary,edits:[['std::move(texClass), std::move(properties)','Property(host::number(texClass)), std::move(properties)']]},
 {name:'OPDEF-property-identity-cloned',file:dictionary,edits:[['if (properties.missing()) properties = PropertyNull{};','if (properties.missing()) properties = PropertyNull{};if(auto record=PropertyList::object(properties)){PropertyList copy;copy.assign(*record);properties=copy;}']]},
 {name:'lookup-prefix-before-infix',file:dictionary,edits:[['Property infix = host::get(Property(OPTABLE()), "infix");','Property infix = host::get(Property(OPTABLE()), "prefix");'],['Property prefix = host::get(Property(OPTABLE()), "prefix");','Property prefix = host::get(Property(OPTABLE()), "infix");']]},
 {name:'lookup-own-only',file:dictionary,edits:[['Property definition = host::get(infix, key);','Property definition = PropertyList::record(infix).value().getOwn(key);'],['definition = host::get(prefix, key);','definition = PropertyList::record(prefix).value().getOwn(key);'],['definition = host::get(postfix, key);','definition = PropertyList::record(postfix).value().getOwn(key);']]},
 {name:'lookup-falsey-treated-as-hit',file:dictionary,edits:[['if (!host::truthy(definition))','if (definition.missing())'],['if (!host::truthy(definition))','if (definition.missing())'],['if (host::truthy(definition))','if (!definition.missing())']]},
 {name:'lookup-first-unit-key',file:dictionary,edits:[['const std::string key = host::debugText(text);','const std::string key = host::debugText(text.substr(0,1));']]},
 {name:'lookup-eager-null-postfix-read',file:dictionary,edits:[['Property infix = host::get(Property(OPTABLE()), "infix");','auto eager=host::get(host::get(Property(OPTABLE()), "postfix"),key);Property infix = host::get(Property(OPTABLE()), "infix");']]},
 {name:'range-first-unit-not-codepoint',file:dictionary,edits:[['const auto first = utf16::codePointAt(text, 0);','const std::optional<char32_t> first = text.empty()?std::nullopt:std::optional<char32_t>(text[0]);']]},
 {name:'range-upper-exclusive',file:dictionary,edits:[['pointValue <= upperValue','pointValue < upperValue']]},
 {name:'range-lower-exclusive',file:dictionary,edits:[['pointValue >= lowerValue','pointValue > lowerValue']]},
 {name:'range-continue-through-gap',file:dictionary,edits:[['            break;','            continue;']]},
 {name:'range-matching-row-cloned',file:dictionary,edits:[['if (pointValue >= lowerValue) return row;','if (pointValue >= lowerValue) return PropertyArray::array(row).value().spread();']]},
 {name:'fallback-wrong-class',file:dictionary,edits:[['Property(static_cast<double>(TEXCLASS.REL))','Property(static_cast<double>(TEXCLASS.ORD))']]},
 {name:'data-pristine-startup-write-baked',file:data,edits:[['PropertyList& MO(){return state().mo;}',"PropertyList& MO(){auto& result=state().mo;PropertyList::object(PropertyArray::array(h::get(result,\"CLOSE\")).value().get(3)).value().setOwn(\"linebreakstyle\",\"after\");return result;}"]]},
 {name:'data-named-definition-alias-lost',file:data,edits:[['PropertyList& MO(){return state().mo;}',"PropertyList& MO(){auto& result=state().mo;auto a=PropertyArray::array(h::get(result,\"REL\")).value();result.setOwn(\"REL\",a.spread());return result;}"]]},
 {name:'array-own-undefined-as-hole',file:property,edits:[['storage_->arrayPresent[i]=true;','storage_->arrayPresent[i]=!storage_->arrayValues[i].missing();']]},
 {name:'array-spread-preserves-holes',file:property,edits:[['for(std::size_t i=0;i<size();++i)out.push(get(i));','out.resize(size());for(std::size_t i=0;i<size();++i)if(has(i))out.set(i,get(i));']]},
 {name:'array-prototype-parent-self',file:'util/property_host.ixx',edits:[['return key=="__proto__"?Property(BuiltinProperty::prototype):get(Property(PropertyArray{}),key);','return get(Property(PropertyArray{}),key);']]}
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}
 const pattern=m.name.startsWith('OPDEF')?'*OPDEF_missing*':m.name.startsWith('data-')?'*entire_pristine_identity_graph*':m.name.startsWith('array-')?'*native_array_host*':['range-upper-exclusive','range-lower-exclusive'].includes(m.name)?'*every_range_boundary*':['range-continue-through-gap','range-matching-row-cloned','lookup-eager-null-postfix-read'].includes(m.name)?'*malformed_tables*':['lookup-prefix-before-infix','lookup-falsey-treated-as-hit'].includes(m.name)?'*table_priority*':'*inherited_names*';
 try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name,pattern);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true,pattern});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_operator.cpp'))),'Working fixture changed');
for(const [f,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',f))),'Working source changed: '+f);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} P11 mutations detected; working sources and fixtures unchanged.`);
