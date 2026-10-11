// Root actual full graph and unchanged original raw-class observations.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]),units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const source=fs.readFileSync(path.join(root,'src/math/core/mml_tree/mml_node.cpp'),'utf8').replaceAll('\r\n','\n');
function body(signature){const begin=source.indexOf(signature);if(begin<0)throw Error('Existing method signature');const open=source.indexOf('{',begin);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}return source.slice(open+1,end-1);}
const mutants=[
 {name:'set-class-always-returns-previous',edits:[[body('NodeValue AbstractMmlNode::setTeXclass(NodeValue previous)'), '\n return previous;\n']]},
 {name:'update-class-only-copies-current',edits:[[body('void AbstractMmlNode::updateTeXclass(NodeValue coreNode)'), '\n if(coreNode.isNode())setTexClass(coreNode.requireNode()->texClass());\n']]},
 {name:'previous-class-omits-scriptlevel',edits:[[body('void AbstractMmlNode::getPrevClass(NodeValue previous)'), '\n if(previous.isNode())setPrevClass(previous.requireNode()->texClass());\n']]},
];
function edit(name,from,to){if(!process.argv.includes('--red')){if(source.split(from).length!==2)throw Error('Unique accepted body anchor '+name);mutants.push({name,edits:[[from,to]]});}}
edit('core-clear-order-swapped','core->setPrevLevel(PropertyNull{});\n    core->setPrevClass(PropertyNull{});','core->setPrevClass(PropertyNull{});\n    core->setPrevLevel(PropertyNull{});');
edit('current-selection-uses-truthiness','!host::nullish(texClass())','host::truthy(texClass())');
edit('previous-hook-qualified','    getPrevClass(previous);','    AbstractMmlNode::getPrevClass(previous);');
edit('core-level-prefetched-before-target-setter','    setPrevClass(core->prevClass());\n    setPrevLevel(core->prevLevel());','    auto capturedLevel=core->prevLevel();\n    setPrevClass(core->prevClass());\n    setPrevLevel(capturedLevel);');
edit('previous-level-written-before-class','    setPrevClass(node->texClass());\n    auto previousAttributes = node->attributes();','    auto previousAttributes = node->attributes();\n    if(previousAttributes)setPrevLevel(previousAttributes->get("scriptlevel"));\n    setPrevClass(node->texClass());');
edit('previous-level-coerced-to-number','    setPrevLevel(previousAttributes->get("scriptlevel"));','    setPrevLevel(host::number(previousAttributes->get("scriptlevel")));');
edit('null-attributes-silently-returns','if (!previousAttributes) host::typeError("Cannot read properties of null (reading \'get\')");','if (!previousAttributes) return;');
edit('undefined-current-result-normalized-to-null','return !host::nullish(texClass()) ? NodeValue(this) : previous;','return !host::nullish(texClass()) ? NodeValue(this) : (previous.kind()==tree::ChildKind::Undefined?NodeValue(PropertyNull{}):previous);');
require('./native-mutation-runner.cjs')({name:'math-port-mml-base-class',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_class_state.cpp',additionalFixtures:['reference_mml_base_class_boundary.cpp','reference_mml_base_class_callbacks.cpp'],pattern:'*math_port_class_state*',assertions:13892,tests:13,suiteSummaries:[{assertions:13441,tests:9},{assertions:19,tests:3},{assertions:432,tests:1}],mutants:mutants.map(m=>({...m,file:'core/mml_tree/mml_node.cpp'})),baselineOutcome:process.argv.includes('--red')?'red':'green'});
