// Root actual complete registry implementation graph and actual predicate entry points.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const probes=process.argv.includes('--probes');
require('./native-mutation-runner.cjs')({name:probes?'math-port-mml-predicate-probes-red':'math-port-mml-predicate-red',modules,units,supportFiles:['util/option_storage.hpp'],fixture:probes?'reference_mml_predicate_probes.cpp':'reference_mml_predicates.cpp',pattern:'*math_port_mml_predicates*',mutants:[],baselineOutcome:'red'});
