// Legacy metadata object-protocol gate, pending migration; not a current native gate.
'use strict';
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_node.cpp',mutants=[
 {name:'mml-copy-forward-root-id-to-child',file,edits:[['child.requireNode("copy")->copy();','child.requireNode("copy")->copy(keepIds);']]},
 {name:'mml-copy-copy-all-root-ids',file,edits:[['if (name != "id" || keepIds) {','if (true) {']]},
 {name:'mml-copy-drop-retained-root-id',file,edits:[['if (name != "id" || keepIds) {','if (name != "id") {']]},
 {name:'mml-copy-attribute-layer-alias',file,edits:[['    // Preserve the source\'s condition reads before capturing its iteration','    if(this->attributes())normalNode->setAttributes(this->attributes());\n    // Preserve the source\'s condition reads before capturing its iteration']]},
 {name:'mml-copy-entire-property-store-alias',file,edits:[['normalNode->properties = std::move(copiedProperties);','normalNode->properties = this->properties;']]},
 {name:'mml-copy-deep-clone-nested-property',file,edits:[['normalNode->properties = std::move(copiedProperties);','if(auto nested=PropertyList::object(copiedProperties.getOwn("nested"))){PropertyList record;record.assign(*nested);copiedProperties.setOwn("nested",record);}normalNode->properties = std::move(copiedProperties);']]},
 {name:'mml-copy-copy-inherited-attrs-as-explicit',file,edits:[['const std::vector<std::string> names = attributes.keys();','for(const auto& key:sourceAttributes->getInheritedNames())normalNode->attributes()->set(key,sourceAttributes->getInherited(key));const std::vector<std::string> names = attributes.keys();']]},
];
require(path.join(root,'tools/math-port/native-mutation-runner.cjs'))({name:'math-port-mml-copy-metadata-mutations',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_copy_metadata.cpp',pattern:'*math_port_mml_copy_metadata*',assertions:32,tests:1,suiteSummaries:[{assertions:32,tests:1}],mutants});
