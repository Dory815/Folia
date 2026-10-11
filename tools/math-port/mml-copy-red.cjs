// Root: actual complete registry implementation graph and real copy entry point.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const metadata=process.argv.includes('--metadata');
require('./native-mutation-runner.cjs')({name:metadata?'math-port-mml-copy-metadata-red':'math-port-mml-copy-red',modules,units,supportFiles:['util/option_storage.hpp'],fixture:metadata?'reference_mml_copy_metadata.cpp':'reference_mml_copy.cpp',pattern:'*math_port_mml_copy*',mutants:[],baselineOutcome:'red'});
