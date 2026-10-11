// Explicit production manifest graph; static binding checks only, not red/green.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const block=cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1];
const modules=[...block.matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);assert.ok(modules.includes('core/mml_tree/mml_node.ixx'));assert.ok(modules.includes('core/tree/child_array.ixx'));assert.ok(modules.length>80);
require('./native-mutation-runner.cjs')({name:'math-port-mml-declarations',modules,units:[],fixture:'reference_mml_declarations.cpp',mutants:[],baselineOutcome:'compile'});
