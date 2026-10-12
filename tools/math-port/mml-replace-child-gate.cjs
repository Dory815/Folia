'use strict';
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]),units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_node.cpp',source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n'),sig='NodeValue AbstractMmlNode::replaceChild(',begin=source.indexOf(sig),open=source.indexOf('{',begin);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}const body=source.slice(open+1,end-1);
const mutants=[{name:'replace-child-does-nothing',file,edits:[[body,'\n return newChild;\n']]}];
require('./native-mutation-runner.cjs')({name:'math-port-mml-replace-child',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_replace_child.cpp',pattern:'*math_port_replace_child*',assertions:1504,tests:1,mutants,baselineOutcome:process.argv.includes('--red')?'red':'green'});
