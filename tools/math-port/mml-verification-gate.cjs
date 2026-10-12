'use strict';
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]),units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const file='core/mml_tree/mml_node.cpp',source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n');
function body(sig){const begin=source.indexOf(sig),open=source.indexOf('{',begin);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}return source.slice(open+1,end-1);}
const tree=body('void AbstractMmlNode::verifyTree('),children=body('void AbstractMmlNode::verifyChildren(');
const mutants=[{name:'verify-tree-is-noop',file,edits:[[tree,'\n return;\n']]},{name:'verify-children-is-noop',file,edits:[[children,'\n return;\n']]}];
if(!process.argv.includes('--red')){
 const change=(name,original,from,to)=>{if(original.split(from).length!==2)throw Error('Unique verification anchor '+name);mutants.push({name,file,edits:[[original,original.replace(from,to)]]});};
 const t=(name,from,to)=>change(name,tree,from,to),c=(name,from,to)=>change(name,children,from,to);
 t('omitted-options-become-object','options = PropertyNull{}','options = PropertyList{}');
 t('null-options-continue','if (std::holds_alternative<PropertyNull>(options.value)) return;','');
 t('falsey-options-skip','if (std::holds_alternative<PropertyNull>(options.value)) return;','if (!host::truthy(options)) return;');
 t('attributes-skipped','verifyAttributes(options);','');
 t('attributes-after-arity','verifyAttributes(options);\n    const double nodeArity = arity();','const double nodeArity = arity();\n    verifyAttributes(options);');
 t('arity-read-twice','const double nodeArity = arity();','const double nodeArity = arity(); (void)arity();');
 t('check-arity-requires-boolean','host::truthy(host::get(options, "checkArity"))','host::get(options, "checkArity") == Property(true)');
 t('zero-arity-ignored','nodeArity >= 0','nodeArity > 0');
 t('infinity-checked','!(std::isinf(nodeArity) && nodeArity > 0)','true');
 t('unary-requires-one-child','wrongCount = children->size() == 0;','wrongCount = children->size() != 1;');
 t('arity-error-skipped','mError("Wrong number of children for \\"" + kind() + "\\" node", options, true);','');
 t('arity-error-uses-long-message','options, true);','options, false);');
 t('arity-error-stops-children','options, true);','options, true); return;');
 t('children-use-empty-options','verifyChildren(options);','verifyChildren(PropertyList{});');
 t('tree-call-budget-omitted','tree::TreeCallGuard<MmlNode> guard;','');
 c('children-call-budget-omitted','tree::TreeCallGuard<MmlNode> guard;','');
 c('null-array-swallowed','if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");','if (!children) return;');
 c('children-length-frozen','for (std::size_t index = 0; index < children->size(); ++index)','const auto length=children->size();\n    for (std::size_t index = 0; index < length; ++index)');
 c('children-normalize-undefined','->verifyTree(options);','->verifyTree(options.missing()?Property(PropertyNull{}):options);');
 c('children-skip-prototypes','children->get(index)','(children->hasOwn(index)?children->get(index):NodeValue(Missing{}))');
 c('children-skip-alternate-slots','++index)','index+=2)');
 const reload=children.replace('index < children->size()','index < (this->childNodes?this->childNodes->size():0)').replace('children->get(index)','this->childNodes->get(index)');
 mutants.push({name:'children-reread-rebound-array',file,edits:[[children,reload]]});
}
require('./native-mutation-runner.cjs')({name:'math-port-mml-verification',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_verification.cpp',pattern:'*math_port_verification_*',assertions:3756,tests:2,mutants,baselineOutcome:process.argv.includes('--red')?'red':'green'});
