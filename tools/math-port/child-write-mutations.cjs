// Root detects assignment receiver errors using unchanged actual Source golds.
'use strict';
const file='core/tree/child_array.ixx',mutants=[];
const add=(name,from,to)=>mutants.push({name,file,edits:[[from,to]]});
add('write-receiver-returns-null','Product* requireNodeForWrite(std::string_view member)const {\n        if(auto* p=nodeOrNull())return p;','Product* requireNodeForWrite(std::string_view member)const {\n        if(auto* p=nodeOrNull())return nullptr;');
add('write-uses-read-error','std::string("Cannot set properties of ")','std::string("Cannot read properties of ")');
add('write-operation-text-lost','" (setting \'"','" (reading \'"');
add('write-nullish-tag-lost','const auto message=std::string("Cannot set properties of ")+(kind()==ChildKind::Null?"null":"undefined")','const auto message=std::string("Cannot set properties of ")+"null"');
add('write-error-class-lost','" (setting \'"+std::string(member)+"\')";\n        throw options::Exception("TypeError",','" (setting \'"+std::string(member)+"\')";\n        throw options::Exception("RangeError",');
require('./native-mutation-runner.cjs')({name:'math-port-child-write-mutations',fixture:'reference_child_write.cpp',pattern:'*math_port_child_write*',assertions:7,tests:1,mutants,
 modules:['util/option_types.ixx','util/object_types.ixx','util/callable_types.ixx','core/tree/property.ixx','core/tree/child_array.ixx'],units:[]});
