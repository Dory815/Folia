// Root revalidates unchanged original factory expectations with the sole sparse array binding.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
require('./native-mutation-runner.cjs')({name:'math-port-factory-sparse-binding',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_factory.cpp',pattern:'*math_port_factory*',assertions:116,tests:10,mutants:[{name:'forwarded-properties-identity-discarded',file:'core/tree/node_factory.ixx',edits:[['std::any(std::move(properties)), std::any(std::move(children))','std::any(PropertyList{}), std::any(std::move(children))']]}]});
