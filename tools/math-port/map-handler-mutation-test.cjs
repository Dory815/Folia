// Root isolated native mutations; compile failures/crashes are not detections.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-map-handler-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','util/object_types.ixx','util/callable_types.ixx','util/prioritized_list.ixx','util/promise.ixx','util/function_types.ixx','util/function_list.ixx','util/method_config.ixx','util/utf16_types.ixx','util/utf16.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/property_host.ixx','input/tex/token.ixx','input/tex/types.ixx','input/tex/token_map_types.ixx','input/tex/map_handler.ixx'];
const units=['util/promise.cpp',"util/function_value.cpp",'util/function_list.cpp','util/property_host.cpp','input/tex/token.cpp','input/tex/parse_callable.cpp','input/tex/map_handler.cpp'];
fs.mkdirSync(work,{recursive:true});const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const copies=new Map(),fixtures=new Map();
function writeChanged(file,bytes){if(!fs.existsSync(file)||!fs.readFileSync(file).equals(Buffer.from(bytes)))fs.writeFileSync(file,bytes);}
for(const f of ['reference_map_handler.cpp','map_handler_source.inc']){const original=fs.readFileSync(path.join(root,'tests/math',f));fixtures.set(f,original);writeChanged(path.join(work,f),original);}
for(const f of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',f)),source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',f);fs.mkdirSync(path.dirname(target),{recursive:true});writeChanged(target,source);copies.set(f,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_map_handler_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" reference_map_handler.cpp)
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
function build(name){const p=cp.spawnSync('cmd.exe',['/d','/c',path.join(work,'build.cmd')],{encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,name+'-build.log'),(p.stdout||'')+(p.stderr||''));assert.equal(p.status,0,name+' must compile');}
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_map_handler*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
const mutants=[],file='input/tex/map_handler.cpp';
const add=(name,from,to,f=file)=>mutants.push({name,file:f,edits:[[from,to]]});
add('registry-nan-strict-equality','if (sameValueZero(key, name)) return map;','if (strictEqual(key, name)) return map;');
add('register-name-read-twice','const Property name = mapName(map);','const Property name = mapName(map);(void)mapName(map);');
add('add-original-order','snapshot.get(remaining - 1)','snapshot.get(snapshot.size() - remaining)');
add('missing-map-continue','warn(std::move(message));\n            return;','warn(std::move(message));\n            continue;');
add('priority-lost','configuration_.add(map, priority);','configuration_.add(map, 5);');
add('fallback-priority-lost','fallback_.add(std::move(fallback), priority);','fallback_.add(std::move(fallback), 5);');
add('map-removal-omitted','if (map) configuration_.remove(map);','if (map) (void)map;');
add('fallback-removal-omitted','if (truthy(fallback)) fallback_.remove(fallback);','if (truthy(fallback)) (void)fallback;');
add('parse-contains-precheck','ParseResult result = map->parse(input);','if(!map->contains(input.token))continue;ParseResult result = map->parse(input);');
add('fallback-marker-ignored','if (fallbackIdentity(result)) break;','if (false && fallbackIdentity(result)) break;');
add('fallback-symbol-only','return strictEqual(result, SubHandler::FALLBACK);','auto a=std::get_if<ParseSymbolRef>(&result);auto b=std::get_if<ParseSymbolRef>(&SubHandler::FALLBACK);return a&&b&&*a==*b;');
add('fallback-nan-equals','return strictEqual(a, b);','return sameValueZero(a, b);');
add('null-result-variant-only','if (jsNull(left) || jsNull(right)) return jsNull(left) && jsNull(right);','if (jsNull(left) || jsNull(right)) return left.index()==right.index() && jsNull(left) && jsNull(right);');
add('parse-live-list-snapshot','for (auto current = configuration_.begin(), finish = configuration_.end(); current != finish; ++current) {\n        const auto record = current.record();\n        if (!record) property_host::typeError("Null token map configuration record");\n        const TokenMapRef map = record->item;\n        if (!map) property_host::typeError("Null token map configuration item");\n        ParseResult result = map->parse(input);','std::vector<TokenMapRef> captured;for(auto&r:configuration_)captured.push_back(r.item);for(const auto&map:captured){ParseResult result=map->parse(input);');
add('empty-fallback-safe-default','if (snapshot.empty()) functions::typeError(u"Cannot read properties of undefined (reading \'item\')");','if (snapshot.empty()) return Property(Missing{});');
add('fallback-result-returned','(void)functions::invoke(snapshot.front(), arguments);','return invokeParseMethod(snapshot.front()->item,FunctionReceiver{snapshot.front()},input.parser,atom);');
add('lookup-false-normalized','if (map) return map->lookup(token);','if (map) return Property(PropertyNull{});');
add('character-null-truthiness','return !std::holds_alternative<PropertyNull>(characterOf(map->lookup(token)).value);','return property_host::truthy(characterOf(map->lookup(token)));');
add('character-family-ignored','if (!dynamic_cast<CharacterMapIdentity*>(map.get())) return true;','if (!dynamic_cast<CharacterMapIdentity*>(map.get())) return false;');
add('retrieve-nan-map-equality','if (strictEqual(mapName(map), name)) return map;','if (sameValueZero(mapName(map), name)) return map;');
add('join-nullish-stringified','if (property_host::nullish(names[i])) continue;','if (false && property_host::nullish(names[i])) continue;');
add('warning-prefix-lost','std::u16string output = u"TexParser Warning: ";','std::u16string output;');
add('remove-handler-erased','handler->remove(maps, fallback);','handler->remove(maps, fallback);set(name,std::make_shared<SubHandler>());');
add('keys-order-reversed','return (*storage_)[index_++].first;','return (*storage_)[storage_->size()-1-index_++].first;');
add('keys-exhaustion-revived','if (done_) return std::nullopt;','if (false && done_) return std::nullopt;');
add('keys-retention-lost','KeyIterator keys(){return KeyIterator(map_);}','KeyIterator keys(){return KeyIterator(std::make_shared<MapStorage>());}','input/tex/map_handler.ixx');
add('fallback-own-missing-not-shadowing','if(name==key)return value;','if(name==key&&!(std::holds_alternative<Property>(value)&&std::get<Property>(value).missing()))return value;','util/method_config.ixx');
add('remove-array-snapshot','for (std::size_t index = 0; index < input->size(); ++index) {\n        const Property name = input->get(index);','auto captured=input->spread();for(std::size_t index=0;index<captured.size();++index){const Property name=captured.get(index);');
add('default-warning-stderr','std::cout << property_host::debugText(output)','std::cerr << property_host::debugText(output)');
const only=process.argv.find(x=>x.startsWith('--only='))?.slice(7),selected=only?mutants.filter(x=>x.name===only):mutants,results=[];assert.ok(selected.length,'Unknown mutation');
build('baseline');const baseline=run('baseline');assert.equal(baseline.status,0,'Baseline must pass');assert.match(baseline.stdout,/198 asserts in 22 tests/,'Complete nonzero baseline');console.log('MapHandler baseline: 198 assertions pass.');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const[from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}}
for(const[file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working implementation changed: '+file);
for(const[file,original]of fixtures)assert.ok(original.equals(fs.readFileSync(path.join(root,'tests/math',file))),'Working fixture changed: '+file);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');console.log(`All ${selected.length} MapHandler mutations detected; working files unchanged.`);
