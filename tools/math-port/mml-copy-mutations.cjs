// Root acceptance of actual MML copy, independent sparse/metadata/boundary Source gold.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_node.cpp',mutants=[
 {name:'mml-copy-skip-falsey-slots',file,edits:[['destination->push_back(NodeValue(PropertyNull{}));','(void)destination;']]},
 {name:'mml-copy-preserve-undefined-not-null',file,edits:[['destination->push_back(NodeValue(PropertyNull{}));','destination->push_back(child);']]},
 {name:'mml-copy-null-via-virtual-append',file,edits:[['destination->push_back(NodeValue(PropertyNull{}));','normalNode->appendChild(NodeValue(PropertyNull{}));']]},
 {name:'mml-copy-forward-root-id-to-child',file,edits:[['child.requireNode("copy")->copy();','child.requireNode("copy")->copy(keepIds);']]},
 {name:'mml-copy-copy-all-root-ids',file,edits:[['if (name != "id" || keepIds) {','if (true) {']]},
 {name:'mml-copy-drop-retained-root-id',file,edits:[['if (name != "id" || keepIds) {','if (name != "id") {']]},
 {name:'mml-copy-attribute-layer-alias',file,edits:[['    // Preserve the source\'s condition reads before capturing its iteration','    if(this->attributes())normalNode->setAttributes(this->attributes());\n    // Preserve the source\'s condition reads before capturing its iteration']]},
 {name:'mml-copy-entire-property-store-alias',file,edits:[['normalNode->properties = std::move(copiedProperties);','normalNode->properties = this->properties;']]},
 {name:'mml-copy-mutate-factory-retained-property-store',file,edits:[['normalNode->properties = std::move(copiedProperties);','normalNode->properties.assign(copiedProperties);']]},
 {name:'mml-copy-omit-singleton-inference-get',file,edits:[['if (children->size() == 1) {','if (false && children->size() == 1) {']]},
 {name:'mml-copy-snapshot-loop-length',file,edits:[['for (std::size_t index = 0; index < children->size(); ++index) {','const auto capturedLength=children->size(); for (std::size_t index = 0; index < capturedLength; ++index) {']]},
 {name:'mml-copy-read-rebound-source-array',file,edits:[['for (std::size_t index = 0; index < children->size(); ++index) {\n            const NodeValue child = children->get(index);','for (std::size_t index = 0; index < children->size(); ++index) {\n            const NodeValue child = this->childNodes->get(index);']]},
 {name:'mml-copy-cache-destination-array-before-callback',file,edits:[['for (std::size_t index = 0; index < children->size(); ++index) {','const auto frozenDestination=normalNode->childNodes; for (std::size_t index = 0; index < children->size(); ++index) {'],['const Children destination = normalNode->childNodes;','const Children destination = frozenDestination;']]},
 {name:'mml-copy-deep-clone-nested-property',file,edits:[['normalNode->properties = std::move(copiedProperties);','if(auto nested=PropertyList::object(copiedProperties.getOwn("nested"))){PropertyList record;record.assign(*nested);copiedProperties.setOwn("nested",record);}normalNode->properties = std::move(copiedProperties);']]},
 {name:'mml-copy-copy-inherited-attrs-as-explicit',file,edits:[['const std::vector<std::string> names = attributes.keys();','for(const auto& key:sourceAttributes->getInheritedNames())normalNode->attributes()->set(key,sourceAttributes->getInherited(key));const std::vector<std::string> names = attributes.keys();']]},
 {name:'mml-copy-wrong-null-iterator-message',file,edits:[['if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");','if (!children) host::typeError("object is not iterable (cannot read property Symbol.iterator)");']]},
 {name:'mml-copy-null-factory-treated-as-foreign-storage',file,edits:[['if (!node) host::typeError("Cannot set properties of null (setting \'properties\')");','// mutated: omit nullable receiver check']]},
];
require('./native-mutation-runner.cjs')({name:'math-port-mml-copy-mutations',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_copy.cpp',additionalFixtures:['reference_mml_copy_metadata.cpp','reference_mml_copy_boundary.cpp'],pattern:'*math_port_mml_copy*',assertions:146,tests:3,suiteSummaries:[{assertions:108,tests:1},{assertions:32,tests:1},{assertions:6,tests:1}],mutants});
