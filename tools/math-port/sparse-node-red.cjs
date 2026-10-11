// Root closed-module compilation of the concrete generic Node red freeze.
'use strict';
require('./native-mutation-runner.cjs')({name:'math-port-sparse-node-red',fixture:'reference_sparse_node.cpp',pattern:'*math_port_sparse_node*',assertions:64,tests:1,mutants:[],baselineOutcome:'red',
 modules:['util/option_types.ixx','util/object_types.ixx','util/callable_types.ixx','core/tree/property.ixx','core/tree/child_array.ixx','core/tree/factory_types.ixx','core/tree/factory.ixx','core/tree/node_types.ixx','core/tree/node.ixx','core/tree/node_factory.ixx'],units:[]});
