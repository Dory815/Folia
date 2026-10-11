// Root full actual implementation graph; no substituted declarations or expected values.
'use strict';
const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8'),source=fs.readFileSync(path.join(root,'src/math/core/mml_tree/mml_node.cpp'),'utf8').replaceAll('\r\n','\n');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]),units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
function body(signature){const begin=source.indexOf(signature);if(begin<0)throw Error('Existing signature');const open=source.indexOf('{',begin);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}return source.slice(open+1,end-1);}
const mutants=[{name:'logical-parent-is-raw-parent',edits:[[body('NodeValue AbstractMmlNode::Parent() const'),'\n return getParent();\n']]},{name:'child-position-always-null',edits:[[body('std::optional<std::size_t> AbstractMmlNode::childPosition()'),'\n return {};\n']]}];
if(!process.argv.includes('--red')){
 const p=body('NodeValue AbstractMmlNode::Parent() const'),c=body('std::optional<std::size_t> AbstractMmlNode::childPosition()');
 const change=(name,original,from,to)=>{if(original.split(from).length!==2)throw Error('Unique body anchor '+name);mutants.push({name,edits:[[original,original.replace(from,to)]]});};
 change('logical-parent-advances-raw',p,'parent = node->Parent();','parent = node->getParent();');
 change('logical-parent-undefined-normalized',p,'return parent;','return parent.kind()==tree::ChildKind::Undefined?NodeValue(PropertyNull{}):parent;');
 change('logical-parent-nullish-flag-test',p,'!host::truthy(node->notParent())','host::nullish(node->notParent())');
 change('position-advances-logical',c,'parent = node->getParent();','parent = node->Parent();');
 change('position-searches-this-only',c,'if (!child.isNode()) child = NodeValue(this);','child = NodeValue(this);');
 change('position-skips-prototype',c,'if (children->get(index) == child) return index;','if (children->hasOwn(index) && children->get(index) == child) return index;');
 change('position-null-array-swallowed',c,'if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");','if (!children) return {};');
 if(c.split('NodeValue parent = getParent();').length!==2||c.split('const auto children = node->childNodes;').length!==2)throw Error('Unique stale receiver anchors');
 mutants.push({name:'position-captures-array-before-flag',edits:[[c,c.replace('NodeValue parent = getParent();','NodeValue parent = getParent();\n const auto initialParent = parent;\n const auto initialChildren = parent.isNode()?parent.requireNode()->childNodes:decltype(childNodes){};').replace('const auto children = node->childNodes;','const auto children = parent==initialParent?initialChildren:node->childNodes;')]]});
}
require('./native-mutation-runner.cjs')({name:'math-port-mml-parent-position',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_parent_position.cpp',additionalFixtures:['reference_mml_parent_position_rebind.cpp'],pattern:'*math_port_parent_position*',assertions:12960,tests:2,suiteSummaries:[{assertions:10800,tests:1},{assertions:2160,tests:1}],mutants:mutants.map(m=>({...m,file:'core/mml_tree/mml_node.cpp'})),baselineOutcome:process.argv.includes('--red')?'red':'green'});
