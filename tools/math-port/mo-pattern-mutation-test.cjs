// Root acceptance, isolated source copies; working implementation is never edited.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),work=path.join(root,'build/math-port-mo-pattern-mutations');
const modules=['util/option_types.ixx','core/tree/property.ixx',"util/callable_types.ixx","util/object_types.ixx",'core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','core/mml_tree/attributes.ixx','util/property_host.ixx','core/mml_tree/mml_node_host.ixx','core/mml_tree/mml_node.ixx','core/mml_tree/leaf_host.ixx','core/mml_tree/mo_patterns.ixx'];
const units=['core/mml_tree/mml_bindings.cpp','core/mml_tree/mml_node.cpp','core/mml_tree/mml_branches.cpp','core/mml_tree/mml_empty.cpp','util/property_host.cpp','core/mml_tree/mo_pattern_data.cpp','core/mml_tree/mo_patterns.cpp'];
const copies=new Map();fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));
process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const fixture=fs.readFileSync(path.join(root,'tests/math/reference_mo_patterns.cpp'));
fs.writeFileSync(path.join(work,'reference_mo_patterns.cpp'),fixture);
for(const file of [...modules,...units]){const original=fs.readFileSync(path.join(root,'src/math',file));const source=original.toString().replaceAll('\r\n','\n'),target=path.join(work,'src',file);fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,source);copies.set(file,{original,source,target});}
const slash=p=>p.replaceAll('\\','/');
fs.writeFileSync(path.join(work,'CMakeLists.txt'),`
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
cmake_minimum_required(VERSION 4.0)
project(math_mo_pattern_mutations LANGUAGES CXX)
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
add_executable(MutationTests "${slash(path.join(root,'tests/main.cpp'))}" "reference_mo_patterns.cpp")
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
function run(name){const p=cp.spawnSync(path.join(work,'bin/MutationTests.exe'),['*math_port_mo_pattern*'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(p.stdout||'')+(p.stderr||''));return p;}
build('baseline');assert.equal(run('baseline').status,0,'Baseline must pass');console.log('Mo pattern baseline compiles and passes.');
const mutations=[
  {
    "name": "minimum-omitted",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "if (text.size() < spec.minimum) return false;",
        "/* minimum omitted */"
      ]
    ]
  },
  {
    "name": "minimum-inclusive-rejected",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "text.size() < spec.minimum",
        "text.size() <= spec.minimum"
      ]
    ]
  },
  {
    "name": "maximum-omitted",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "if (spec.maximum && text.size() > *spec.maximum) return false;",
        "/* maximum omitted */"
      ]
    ]
  },
  {
    "name": "maximum-boundary-rejected",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "text.size() > *spec.maximum",
        "text.size() >= *spec.maximum"
      ]
    ]
  },
  {
    "name": "class-lower-endpoint-excluded",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "range.first <= unit",
        "range.first < unit"
      ]
    ]
  },
  {
    "name": "class-upper-endpoint-excluded",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "unit <= range.last",
        "unit < range.last"
      ]
    ]
  },
  {
    "name": "range-union-became-intersection",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "std::any_of(spec.units.begin()",
        "std::all_of(spec.units.begin()"
      ]
    ]
  },
  {
    "name": "only-first-unit-checked",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "if (!allowed) return false;",
        "if (!allowed) return false; return true;"
      ]
    ]
  },
  {
    "name": "only-last-unit-checked",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "for (const char16_t unit : text) {",
        "for (const char16_t unit : (text.empty()?text:text.substr(text.size()-1))) {"
      ]
    ]
  },
  {
    "name": "invalid-middle-unit-accepted",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "if (!allowed) return false;",
        "if (!allowed) continue;"
      ]
    ]
  },
  {
    "name": "unit-truncated-to-byte",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "for (const char16_t unit : text) {",
        "for (char16_t unit : text) { unit=static_cast<unsigned char>(unit);"
      ]
    ]
  },
  {
    "name": "final-lf-relaxed",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "if (text.size() < spec.minimum) return false;",
        "if(!text.empty() && text.back()==u'\\n')text.remove_suffix(1);\n    if (text.size() < spec.minimum) return false;"
      ]
    ]
  },
  {
    "name": "leading-lf-relaxed",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "if (text.size() < spec.minimum) return false;",
        "if(!text.empty() && text.front()==u'\\n')text.remove_prefix(1);\n    if (text.size() < spec.minimum) return false;"
      ]
    ]
  },
  {
    "name": "nul-truncates-input",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "if (text.size() < spec.minimum) return false;",
        "text=text.substr(0,text.find(u'\\0'));\n    if (text.size() < spec.minimum) return false;"
      ]
    ]
  },
  {
    "name": "all-kinds-use-pseudoscripts",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "const MoPatternSpec& spec = moPatternSpec(kind);",
        "const MoPatternSpec& spec = moPatternSpec(MoPattern::pseudoScripts);"
      ]
    ]
  },
  {
    "name": "callback-creation-defers-validation",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "(void)moPatternSpec(kind);",
        "/* validation deferred */"
      ]
    ]
  },
  {
    "name": "callback-uses-primes",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "return matchMoPattern(kind, text);",
        "return matchMoPattern(MoPattern::primes, text);"
      ]
    ]
  },
  {
    "name": "callback-inverts-result",
    "file": "core/mml_tree/mo_patterns.cpp",
    "edits": [
      [
        "return matchMoPattern(kind, text);",
        "return !matchMoPattern(kind, text);"
      ]
    ]
  },
  {
    "name": "invalid-enum-repaired-to-first",
    "file": "core/mml_tree/mo_pattern_data.cpp",
    "edits": [
      [
        "if(index>=specs.size())host::typeError(\"Invalid native MmlMo pattern\");",
        "if(index>=specs.size())return specs[0];"
      ]
    ]
  },
  {
    "name": "pseudo-class-endpoint-dropped",
    "file": "core/mml_tree/mo_pattern_data.cpp",
    "edits": [
      [
        "{8216,8223}",
        "{8216,8222}"
      ]
    ]
  },
  {
    "name": "operator-minimum-one",
    "file": "core/mml_tree/mo_pattern_data.cpp",
    "edits": [
      [
        "\"\",ranges2,2,std::nullopt",
        "\"\",ranges2,1,std::nullopt"
      ]
    ]
  },
  {
    "name": "width-accents-repeat",
    "file": "core/mml_tree/mo_pattern_data.cpp",
    "edits": [
      [
        "\"\",ranges4,1,1",
        "\"\",ranges4,1,std::nullopt"
      ]
    ]
  }
];
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7);
const selected=only?mutations.filter(m=>m.name===only):mutations,results=[];
assert.ok(selected.length,'Unknown mutation');
for(const m of selected){const {source,target}=copies.get(m.file);let changed=source;for(const [from,to]of m.edits){assert.ok(changed.includes(from),'Missing anchor '+m.name);changed=changed.replace(from,to);}
 try{fs.writeFileSync(target,changed);build(m.name);const p=run(m.name);assert.equal(p.status,1,'Mutation must fail assertions, not crash: '+m.name);assert.match(p.stdout||'',/FAILED|failed|FAIL/);results.push({mutation:m.name,compiles:true,test_exit:p.status,detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,source);}
}
assert.ok(fixture.equals(fs.readFileSync(path.join(root,'tests/math/reference_mo_patterns.cpp'))),'Working fixture changed');
for(const [file,{original}]of copies)assert.ok(original.equals(fs.readFileSync(path.join(root,'src/math',file))),'Working implementation changed: '+file);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify(results,null,2)+'\n');
console.log(`All ${selected.length} Mo pattern mutations detected; working sources and fixtures unchanged.`);
