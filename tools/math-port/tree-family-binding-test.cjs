// Checked native family boundary; its error variant is separately tested.
'use strict';
require('./native-mutation-runner.cjs')({name:'math-port-tree-family-binding',fixture:'native_tree_binding.cpp',pattern:'*math_native_tree_binding*',assertions:3,tests:1,
 mutants:[{name:'native-family-error-misclassified',file:'core/tree/node.ixx',edits:[['type_error("Factory product does not implement this Node property storage");','throw FactoryError("RangeError",u"Factory product does not implement this Node property storage");']]}],
 modules:['util/option_types.ixx','util/object_types.ixx','util/callable_types.ixx','core/tree/property.ixx','core/tree/child_array.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','core/tree/node_factory.ixx'],units:[]});
