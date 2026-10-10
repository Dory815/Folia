// Root acceptance, isolated source copies; working implementation is never edited.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-input-host-mutations');
const modules=['util/utf16_types.ixx','util/utf16.ixx','util/js_numeric_input.ixx'];
const copies=new Map();fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const fixture=fs.readFileSync(path.join(root,'tests/math/reference_input_host.cpp'));
fs.writeFileSync(path.join(work,'reference_input_host.cpp'),fixture);
for(const file of modules){const original=fs.readFileSync(path.join(root,'src/math',file));const source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_input_host_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_input_host.cpp")
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_input_host*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Input host baseline compiles and passes.');
const numeric='util/js_numeric_input.ixx',utf16='util/utf16.ixx';
const mutations=[
 {name:'number-signed-zero-lost',file:numeric,edits:[['return std::copysign(0.0, negative ? -1.0 : 1.0);','return 0.0;']]},
 {name:'number-trailing-whitespace-rejected',file:numeric,edits:[['while (!text.empty() && is_space(text.back())) text.remove_suffix(1);','/* trailing whitespace lost */']]},
 {name:'number-signed-radix-accepted',file:numeric,edits:[['if (!had_sign && !text.empty() && text.front() == u\'0\' && text.size() >= 2) {','text = unsigned_text; if (!text.empty() && text.front() == u\'0\' && text.size() >= 2) {']]},
 {name:'number-integer-invalid-suffix-accepted',file:numeric,edits:[['if (digit < 0 || static_cast<unsigned>(digit) >= radix)\n            return std::numeric_limits<double>::quiet_NaN();','if (digit < 0 || static_cast<unsigned>(digit) >= radix) break;']]},
 {name:'integer-rounding-sticky-bit-lost',file:numeric,edits:[['const bool sticky = any_low_bits(shift - 1);','const bool sticky = false;']]},
 {name:'parseInt-negative-sign-lost',file:numeric,edits:[['return negative ? std::copysign(result, -1.0) : result;','return result;']]},
 {name:'number-decimal-trailing-junk-accepted',file:numeric,edits:[['if (i != text.size()) return false;','/* incomplete decimal accepted */'],['if (converted.ec == std::errc{} && converted.ptr == ascii.data() + ascii.size())','if (converted.ec == std::errc{})']]},
 {name:'number-decimal-range-sign-reversed',file:numeric,edits:[['if (decimal_magnitude_is_negative(exponent_negative, exponent_magnitude,','if (!decimal_magnitude_is_negative(exponent_negative, exponent_magnitude,']]},
 {name:'utf16-surrogate-pair-not-combined',file:utf16,edits:[['if (second >= 0xDC00 && second <= 0xDFFF) {','if (false && second >= 0xDC00 && second <= 0xDFFF) {']]},
 {name:'utf16-iteration-steps-one-unit',file:utf16,edits:[['i += *point > 0xFFFF ? 2 : 1;','i += 1;']]},
 {name:'utf8-low-surrogate-byte-offset-shifted',file:utf16,edits:[['result.text.push_back(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));\n        result.byteOffsets.push_back(start);','result.text.push_back(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));\n        result.byteOffsets.push_back(start+2);']]},
 {name:'utf8-surrogate-encoding-admitted',file:utf16,edits:[['(lead == 0xED && next > 0x9F)','(false && lead == 0xED && next > 0x9F)']]},
 {name:'utf8-lone-surrogate-replacement-lost',file:utf16,edits:[['append_utf8(0xFFFD); // TextEncoder replacement for lone high surrogate.','append_utf8(first); // incorrect raw surrogate output']]},
 {name:'utf8-BOM-dropped',file:utf16,edits:[['append_utf16(cp, start);','if(cp != 0xFEFF) append_utf16(cp, start);']]}
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7);
const results=[],selected=only?mutations.filter(m=>m.name===only):process.argv.includes('--numeric')?mutations.filter(m=>m.file===numeric):mutations;
assert.ok(selected.length,'Unknown mutation');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}
 try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must produce assertion failure, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_input_host.cpp'))),'Working fixture changed');
for(const [file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working implementation changed: '+file);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':process.argv.includes('--numeric')?'results-numeric.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} input host mutations detected; working sources and fixtures unchanged.`);
