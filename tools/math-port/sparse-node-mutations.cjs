// Root-owned six-method Source64 verification; no gold generated from native.
'use strict';
const file='core/tree/node.ixx';
const mutants=[
 {name:'retain-old-destination-array',edits:[['this->childNodes = std::make_shared<ChildArray<Product>>();','// Receiver replacement omitted.']]},
 {name:'skip-nullish-set-children',edits:[['this->appendChild(input->get(index));','if (input->get(index).isNode()) this->appendChild(input->get(index));']]},
 {name:'omit-append-parent',edits:[['receiver->setParent(ChildValue<Product>(static_cast<Product*>(this)));','(void)receiver;']]},
 {name:'check-before-push',edits:[['target->push_back(child);','if (!child.isNode()) child.requireNodeForWrite("parent");\n        target->push_back(child);']]},
 {name:'check-before-replace-write',edits:[['target->set(*index, newChild);','if (!newChild.isNode()) newChild.requireNodeForWrite("parent");\n            target->set(*index, newChild);']]},
 {name:'keep-old-parent-on-self-replace',edits:[['oldReceiver->setParent(ChildValue<Product>(PropertyNull{}));','(void)oldReceiver;']]},
 {name:'omit-remove-splice',edits:[['target->splice(static_cast<std::int64_t>(*index), std::size_t{1});','(void)target;']]},
 {name:'copy-nullish-values',edits:[['if (child.isNode()) {','if (true) {']]},
 {name:'reuse-source-child-in-copy',edits:[['Product* const copiedChild = child.requireNode("copy")->copy();','Product* const copiedChild = child.requireNode("copy");']]}
].map(m=>({file,...m}));
require('./native-mutation-runner.cjs')({name:'math-port-sparse-node-mutations',fixture:'reference_sparse_node.cpp',pattern:'*math_port_sparse_node*',assertions:64,tests:1,mutants,
 modules:['util/option_types.ixx','util/object_types.ixx','util/callable_types.ixx','core/tree/property.ixx','core/tree/child_array.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','core/tree/node_factory.ixx'],units:[]});
