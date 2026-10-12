// Root real stub baseline for the original Source operator/script corpus.
'use strict';const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
require('./native-mutation-runner.cjs')({name:'math-port-operator-script-red',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mo.cpp',additionalFixtures:['reference_mo_order.cpp','reference_script.cpp','reference_atom_semantics.cpp','reference_mml_operator_entries.inc'],pattern:'*math_port*',assertions:32694,tests:157,baselineOutcome:'red',mutants:[]});
