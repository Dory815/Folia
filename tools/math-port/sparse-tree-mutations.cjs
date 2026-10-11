// Root sparse binding acceptance; fixed Source golds, isolated compiled mutants.
'use strict';
const node='core/tree/node.ixx',wrapper='core/tree/wrapper.ixx';
const mutants=[
 {name:'walk-length-cached',file:node,edits:[['for (std::size_t index = 0; index < children->size(); ++index) {','const auto frozenLength=children->size();\n        for (std::size_t index = 0; index < frozenLength; ++index) {']]},
 {name:'walk-child-recursion-lost',file:node,edits:[['if (child.isNode()) child.requireNode("walkTree")->walkTree(visit, data);','if (child.isNode()) visit(child.requireNode("walkTree"), data);']]},
 {name:'walk-nullish-receiver-dispatch',file:node,edits:[['if (child.isNode()) child.requireNode("walkTree")->walkTree(visit, data);','child.requireNode("walkTree")->walkTree(visit, data);']]},
 {name:'join-captured-array-lost',file:node,edits:[['const auto children = this->childNodes;\n        if (!children) {\n            type_error("Cannot read properties of null (reading \'join\')");','const auto& children = this->childNodes;\n        if (!children) {\n            type_error("Cannot read properties of null (reading \'join\')");']]},
 {name:'join-length-live',file:node,edits:[['index < length; ++index','index < children->size(); ++index']]},
 {name:'join-nullish-string-conversion',file:node,edits:[['if (child.isNode()) result += child.requireNode("toString")->toString();','result += child.requireNode("toString")->toString();']]},
 {name:'join-circular-result-changed',file:node,edits:[['if (!joinGuard.entered()) return nodeKind + "()";','if (!joinGuard.entered()) return nodeKind + "(cycle)";']]},
 {name:'native-budget-premature',file:'core/tree/node_types.ixx',edits:[['limit=512;','limit=511;']]},
 {name:'copy-properties-root-aliased',file:node,edits:[['copiedProperties.assign(this->properties);','copiedProperties = this->properties;']]},
 {name:'copy-own-proto-property-no-assignment',file:node,edits:[['copiedProperties.assign(this->properties);','for (const auto& key : this->properties.keys()) copiedProperties.setOwn(key,this->properties.getOwn(key));']]},
 {name:'wrapper-absent-children-throws',file:wrapper,edits:[['if (!this->childNodes) return data;','if (!this->childNodes) throw FactoryError("TypeError",u"absent children");']]},
 {name:'wrapper-child-recursion-lost',file:wrapper,edits:[['if (child.isNode()) child.requireNode("walkTree")->walkTree(visit, data);','if (child.isNode()) visit(child.requireNode("walkTree"), data);']]},
 {name:'wrapper-extra-args-forwarded',file:wrapper,edits:[['Product* wrap(Source* source, FactoryArgs = {}) override {\n        return this->factory.wrap(source, FactoryArgs{});','Product* wrap(Source* source, FactoryArgs args = {}) override {\n        return this->factory.wrap(source, args);']]},
];
require('./native-mutation-runner.cjs')({name:'math-port-sparse-tree-mutations',fixture:'reference_tree.cpp',pattern:'*math_port_tree*',assertions:156,tests:18,mutants,
 modules:['util/option_types.ixx','util/object_types.ixx','util/callable_types.ixx','core/tree/property.ixx','core/tree/child_array.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','core/tree/node_factory.ixx','core/tree/wrapper_types.ixx','core/tree/wrapper.ixx','core/tree/wrapper_factory.ixx'],units:[]});
