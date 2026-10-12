// Root actual current graph, original observations and deliberate faults.
'use strict';const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]),units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_branches.cpp',source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n'),sig='std::any AbstractMmlTokenNode::walkTree(',begin=source.indexOf(sig),open=source.indexOf('{',begin);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}const body=source.slice(open+1,end-1);
const mutants=[{name:'token-visit-only',file,edits:[[body,'\n tree::TreeCallGuard<MmlNode> guard; visit(this,data); return data;\n']]}];
if(!process.argv.includes('--red')){
 const change=(name,from,to)=>{if(body.split(from).length!==2)throw Error('Unique token body anchor '+name);mutants.push({name,file,edits:[[body,body.replace(from,to)]]});};
 change('token-captures-before-visit','visit(this, data);\n\n    const Children children = this->childNodes;','const Children children = this->childNodes;\n    visit(this, data);');
 change('token-freezes-length','for (std::size_t index = 0; index < children->size(); ++index)','const auto length=children->size();\n    for (std::size_t index = 0; index < length; ++index)');
 const reread=body.replace('index < children->size()','index < (this->childNodes?this->childNodes->size():0)').replace('children->get(index)','this->childNodes->get(index)');if(reread===body)throw Error('Reread anchors');mutants.push({name:'token-rereads-rebound-array',file,edits:[[body,reread]]});
 change('token-visits-text','if (auto* const mmlNode = node ? dynamic_cast<AbstractMmlNode*>(node) : nullptr) {\n            mmlNode->walkTree(visit, data);','if (node) {\n            node->walkTree(visit, data);');
 change('token-returns-child-data','mmlNode->walkTree(visit, data);','return mmlNode->walkTree(visit, data);');
 change('token-skips-prototypes','const NodeValue child = children->get(index);','const NodeValue child = children->hasOwn(index)?children->get(index):NodeValue(Missing{});');
 change('token-skips-receiver','visit(this, data);','');
 change('token-null-array-swallowed','host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");','return data;');
 change('token-skips-alternate-slots','++index)', 'index+=2)');
 change('token-shared-depth-guard-omitted','tree::TreeCallGuard<MmlNode> guard;','');
}
require('./native-mutation-runner.cjs')({name:'math-port-mml-token-walk',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_token_walk.cpp',pattern:'*math_port_token_walk*',assertions:784,tests:2,suiteSummaries:[{assertions:784,tests:2}],mutants,baselineOutcome:process.argv.includes('--red')?'red':'green'});
