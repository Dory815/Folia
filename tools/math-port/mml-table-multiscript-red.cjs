// Root preimplementation proof that frozen Source cases reach real native stubs.
'use strict';
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
require('./native-mutation-runner.cjs')({name:'math-port-table-multiscript-red',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_table.cpp',additionalFixtures:['reference_multiscript.cpp','reference_mml_pending_entries.inc'],pattern:'*math_port*',assertions:6006,tests:362,baselineOutcome:'red',mutants:[]});
