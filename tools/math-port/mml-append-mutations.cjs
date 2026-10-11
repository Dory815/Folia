// Root append policy acceptance uses original Source cases frozen before implementation.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_node.cpp',mutants=[
 {name:'infinite-inferred-for-of-visits-holes',file,edits:[['originalChildren->forEach([&](NodeValue node, std::size_t, tree::ChildArray<MmlNode>&) {\n                Generic::appendChild(node);\n            });','for(std::size_t i=0;i<originalChildren->size();++i)Generic::appendChild(originalChildren->get(i));']]},
 {name:'skip-present-nullish-inferred-children',file,edits:[['Generic::appendChild(node);','if(node.isNode())Generic::appendChild(node);']]},
 {name:'inferred-parent-array-rebind-not-captured',file,edits:[['originalChildren->forEach([&](NodeValue node, std::size_t, tree::ChildArray<MmlNode>&) {\n                Generic::appendChild(node);\n            });','const auto len=originalChildren->size();for(std::size_t i=0;i<len;++i){const auto live=input->childNodes;if(live->has(i))Generic::appendChild(live->get(i));}']]},
 {name:'return-target-from-infinite-inferred-append',file,edits:[['            return child;\n        }\n\n        MmlNode* const original = input;','            return NodeValue(this);\n        }\n\n        MmlNode* const original = input;']]},
 {name:'finite-inferred-flattened-as-original',file,edits:[['MmlNode* const replacement = factory.create("mrow", {});','MmlNode* const replacement = original;']]},
];
require('./native-mutation-runner.cjs')({name:'math-port-mml-append-mutations',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_append.cpp',pattern:'*math_port_mml_append*',assertions:62,tests:1,mutants});
