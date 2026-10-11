// Root constructor acceptance only; append inference/copy still have separate gates.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_node.cpp',mutants=[
 {name:'omit-inferred-parent-write',file,edits:[['inferredChild.requireNodeForWrite("parent")->setParent(\n            NodeValue(static_cast<MmlNode*>(this)));','(void)inferredChild;']]},
 {name:'skip-empty-virtual-setChildren',file,edits:[['this->setChildren(std::move(inputChildren));','if(inputChildren && inputChildren->size())this->setChildren(std::move(inputChildren));']]},
 {name:'attributes-before-virtual-setChildren',file,edits:[['this->setChildren(std::move(inputChildren));','// mutated: delay child hook'],['this->setAttributes(std::move(newAttributes));','this->setAttributes(std::move(newAttributes)); this->setChildren(std::move(inputChildren));']]},
 {name:'discard-explicit-attribute-input',file,edits:[['const PropertyList assignedAttributes = host::objectAssign(inputAttributes);','const PropertyList assignedAttributes{};']]},
 {name:'convert-null-children-to-empty',file,edits:[['inputChildren = *value;','inputChildren = *value ? *value : std::make_shared<tree::ChildArray<MmlNode>>();']]},
 {name:'replace-undefined-child-error-with-generic-write',file,edits:[['MmlNode* const input = child.requireNode("isInferred");','if(!child.isNode())return Generic::appendChild(child); MmlNode* const input = child.requireNode("isInferred");']]},
];
require('./native-mutation-runner.cjs')({name:'math-port-mml-constructor-mutations',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_constructor.cpp',pattern:'*math_port_mml_constructor*',assertions:91,tests:2,mutants});
