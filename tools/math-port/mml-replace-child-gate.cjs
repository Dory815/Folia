'use strict';
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]),units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_node.cpp',source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n'),sig='NodeValue AbstractMmlNode::replaceChild(',begin=source.indexOf(sig),open=source.indexOf('{',begin);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}const body=source.slice(open+1,end-1);
const mutants=[{name:'replace-child-does-nothing',file,edits:[[body,'\n return newChild;\n']]}];
if(!process.argv.includes('--red')){
 const change=(name,from,to)=>{if(body.split(from).length!==2)throw Error('Unique replacement anchor '+name);mutants.push({name,file,edits:[[body,body.replace(from,to)]]});};
 change('replace-child-zero-enters-inferred','nodeArity < 0','nodeArity <= 0');
 change('replace-child-nan-enters-inferred','nodeArity < 0','!(nodeArity >= 0)');
 change('replace-child-reads-second-slot','children->get(0)','children->get(1)');
 change('replace-child-ignores-prototype','children->get(0)','children->hasOwn(0)?children->get(0):NodeValue(Missing{})');
 change('replace-child-swaps-delegated-arguments','->replaceChild(newChild, oldChild);','->replaceChild(oldChild, newChild);');
 change('replace-child-returns-delegated-result','first.requireNode("replaceChild")->replaceChild(newChild, oldChild);\n        return newChild;','return first.requireNode("replaceChild")->replaceChild(newChild, oldChild);');
 change('replace-child-bypasses-child-virtual','first.requireNode("replaceChild")->replaceChild(newChild, oldChild);','static_cast<AbstractMmlNode*>(first.requireNode("replaceChild"))->AbstractMmlNode::replaceChild(newChild, oldChild);');
 change('replace-child-validates-new-too-early','const double nodeArity = arity();','newChild.requireNode("parent");\n    const double nodeArity = arity();');
 change('replace-child-repeats-arity','if (nodeArity < 0)','if (arity() < 0)');
 const eager=body.replace('const double nodeArity = arity();','const Children children=this->childNodes;\n    const double nodeArity = arity();').replace('        const Children children = this->childNodes;\n','');if(eager===body)throw Error('Eager array anchors');mutants.push({name:'replace-child-captures-array-before-arity',file,edits:[[body,eager]]});
 change('replace-child-discards-generic-return','return Generic::replaceChild(newChild, oldChild);','Generic::replaceChild(newChild,oldChild);return oldChild;');
 change('replace-child-omits-call-budget','tree::TreeCallGuard<MmlNode> guard;','');
}
require('./native-mutation-runner.cjs')({name:'math-port-mml-replace-child',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_replace_child.cpp',pattern:'*math_port_replace_child*',assertions:1508,tests:2,mutants,baselineOutcome:process.argv.includes('--red')?'red':'green'});
