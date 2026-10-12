// Root real-stub evidence over the complete current module graph.
'use strict';
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
require('./native-mutation-runner.cjs')({name:'math-port-action-choice-red',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_maction.cpp',additionalFixtures:['reference_mathchoice.cpp','reference_simple_mml.cpp','reference_mml_action_entries.inc'],pattern:'*math_port*',assertions:0,tests:0,suiteSummaries:[],mutants:[],baselineOutcome:'red'});
