// Root independent restoration gate; unchanged original fixtures and byte guards.
'use strict';
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const mutants=[];
function method(file,sig){const source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n'),start=source.indexOf(sig);if(start<0)throw Error(sig);const open=source.indexOf('{',start);let depth=1,end=open+1;for(;depth;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}const original=source.slice(open+1,end-1);return(name,from,to)=>{if(original.split(from).length!==2)throw Error('Unique '+name);mutants.push({name,file,edits:[[original,original.replace(from,to)]]});};}
method('core/mml_tree/mml_nodes/mathchoice.cpp','void MathChoice::inheritAttributes(')('mathchoice-style-selection-omitted','selection = clamped + 1.0;','selection = 0;');
const choice=method('core/mml_tree/mml_nodes/mathchoice.cpp','void MathChoice::inheritAttributes(');
choice('mathchoice-fraction-truncated','host::numericChild(this->childNodes, selection)','host::numericChild(this->childNodes, std::trunc(selection))');
choice('mathchoice-raw-defaults-normalized','tree::TreeCallGuard<MmlNode> guard;','tree::TreeCallGuard<MmlNode> guard; host::inheritanceDefaults(attributes,display,level,prime);');
choice('mathchoice-replacement-result-used','parent.requireNode("replaceChild")->replaceChild(child, NodeValue(this));','child = parent.requireNode("replaceChild")->replaceChild(child, NodeValue(this));');
choice('mathchoice-parent-captured-before-factory','const NodeValue parent = this->getParent();','');
// The previous mutation also needs a declaration at the earlier Source read.
mutants.at(-1).edits.push(['double selection = 0;','const NodeValue parent = this->getParent();\n    double selection = 0;']);
const selection='core/mml_tree/mml_nodes/maction_selection.cpp',behavior='core/mml_tree/mml_nodes/maction_behavior.cpp';
method(selection,'MmlNode* MmlMaction::selected()')('action-fraction-truncated','host::numericChild(childNodes, index)','host::numericChild(childNodes, std::trunc(index))');
method(selection,'MmlNode* MmlMaction::selected()')('action-lower-clamp-zero','host::mathMax(1.0,','host::mathMax(0.0,');
method(selection,'MmlNode* MmlMaction::selected()')('action-factory-fallback-omitted','return this->factory.create("mrow", {});','return nullptr;');
method(selection,'NodeValue MmlMaction::core()')('action-core-wrong-dispatch','child.requireNode("core")->core()','child.requireNode("core")->coreMO()');
method(selection,'NodeValue MmlMaction::coreMO()')('action-coremo-wrong-dispatch','child.requireNode("coreMO")->coreMO()','child.requireNode("coreMO")->core()');
const classes=method(behavior,'NodeValue MmlMaction::setTeXclass(');
classes('action-tooltip-class-omitted','host::numericChild(childNodes, 1).requireNode("setTeXclass")->setTeXclass(PropertyNull{});','');
classes('action-tooltip-previous-propagated','setTeXclass(PropertyNull{});','setTeXclass(previous);');
classes('action-selected-class-update-omitted','updateTeXclass(selectedNode);','');
classes('action-selected-result-discarded','return result;','return this;');
const toggle=method(behavior,'void MmlMaction::nextToggleSelection()');
toggle('action-toggle-parseint-replaced','host::integer(host::string(rawSelection))','host::number(rawSelection)');
toggle('action-toggle-overflow-reset-omitted','if (selection > static_cast<double>(children->size())) selection = 1.0;','');
const inherit=method(behavior,'void MmlMaction::setChildInheritedAttributes(');
inherit('action-tooltip-case-fold-omitted','host::lowerString(actiontype)','host::string(actiontype)');
inherit('action-tooltip-fixed-args-replaced','setInheritedAttributes(attributes, false, 1, false);','setInheritedAttributes(attributes, display, level, prime);');
inherit('action-tooltip-tip-read-before-base-callback','const NodeValue tip = host::numericChild(childNodes, 1);','');
mutants.at(-1).edits.push(['const NodeValue base = host::numericChild(childNodes, 0);','const NodeValue tip = host::numericChild(childNodes, 1);\n    const NodeValue base = host::numericChild(childNodes, 0);']);
method('core/mml_tree/action_host.ixx','inline std::u16string lowerString(')('action-null-string-error-flattened','if(std::holds_alternative<PropertyNull>(value.value))typeError("Cannot read properties of null (reading \'toLowerCase\')");','');
require('./native-mutation-runner.cjs')({name:'math-port-action-choice-consumers',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_maction.cpp',additionalFixtures:['reference_mathchoice.cpp','reference_mml_action_entries.inc','reference_mml_action_errors.cpp'],pattern:'*math_port*',assertions:1364,tests:1346,suiteSummaries:[{assertions:935,tests:934},{assertions:412,tests:410},{assertions:9,tests:1},{assertions:8,tests:1}],mutants});
