// Root full declaration and implementation graph with original class algorithm observations.
'use strict';const fs=require('node:fs'),path=require('node:path');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8'),source=fs.readFileSync(path.join(root,'src/math/core/mml_tree/mml_branches.cpp'),'utf8').replaceAll('\r\n','\n');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]),units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
function body(signature){const begin=source.indexOf(signature);if(begin<0)throw Error('Existing signature');const open=source.indexOf('{',begin);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}return source.slice(open+1,end-1);}
const mutants=['Layout','Base'].map(kind=>({name:kind.toLowerCase()+'-returns-previous-without-propagation',file:'core/mml_tree/mml_branches.cpp',edits:[[body('NodeValue AbstractMml'+kind+'Node::setTeXclass(NodeValue previous)'),'\n return previous;\n']]}));
if(!process.argv.includes('--red')){
 const l=body('NodeValue AbstractMmlLayoutNode::setTeXclass(NodeValue previous)'),b=body('NodeValue AbstractMmlBaseNode::setTeXclass(NodeValue previous)');
 const change=(name,original,from,to)=>{if(original.split(from).length!==2)throw Error('Unique body anchor '+name);mutants.push({name,file:'core/mml_tree/mml_branches.cpp',edits:[[original,original.replace(from,to)]]});};
 change('layout-retains-old-core',l,'previous = required_child_zero(*this, "setTeXclass").setTeXclass(previous);\n    updateTeXclass(child_zero(*this));','const auto old=child_zero(*this);\n previous=old.requireNode("setTeXclass")->setTeXclass(previous);\n updateTeXclass(old);');
 change('layout-nullish-return-normalized',l,'return previous;','return previous.isNode()?previous:NodeValue(this);');
 change('base-class-written-before-previous',b,'getPrevClass(previous);\n    setTexClass(static_cast<double>(TEXCLASS.ORD));','setTexClass(static_cast<double>(TEXCLASS.ORD));\n getPrevClass(previous);');
 change('base-short-circuit-order-reversed',b,'isEmbellished() || baseNode->isKind("mi")','baseNode->isKind("mi") || isEmbellished()');
 change('base-updates-captured-base',b,'updateTeXclass(core());','updateTeXclass(base);');
 change('base-non-embellished-gets-prev',b,'baseNode->setTeXclass(NodeValue(PropertyNull{}));','baseNode->setTeXclass(previous);');
 change('base-texatom-copy-skipped',b,'setTexClass(baseNode->texClass());','setTexClass(static_cast<double>(TEXCLASS.ORD));');
 change('base-scripts-follow-original-array',b,'const auto tail = currentChildren->slice(1);','const auto tail = currentChildren;');
 change('base-nullish-script-dereferenced',b,'if (child.isNode()) {','if (true) {');
 change('base-result-always-this',b,'return result.isNode() ? result : NodeValue(this);','return NodeValue(this);');
}
require('./native-mutation-runner.cjs')({name:'math-port-mml-container-class',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_container_class.cpp',pattern:'*math_port_container_class*',assertions:6480,tests:1,mutants,baselineOutcome:process.argv.includes('--red')?'red':'green'});
