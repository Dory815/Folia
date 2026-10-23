// Legacy object-protocol return gate, pending migration; not current native acceptance.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);
const methods={zero:['core/mml_tree/mml_branches.cpp','NodeValue child_zero('],required:['core/mml_tree/mml_branches.cpp','MmlNode& required_child_zero('],layout:['core/mml_tree/mml_branches.cpp','NodeValue AbstractMmlLayoutNode::coreMO('],base:['core/mml_tree/mml_branches.cpp','NodeValue AbstractMmlBaseNode::coreMO('],core:['core/mml_tree/mml_nodes/mrow.cpp','NodeValue MmlMrow::core('],coreMO:['core/mml_tree/mml_nodes/mrow.cpp','NodeValue MmlMrow::coreMO('],style:['core/mml_tree/mml_nodes/mstyle.cpp','Property MmlMstyle::notParent(']},mutants=[];
function edit(name,id,changes){const[file,signature]=methods[id],source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n'),start=source.indexOf(signature),end=source.indexOf('\n}\n',start)+2;assert.ok(start>=0&&end>start);const body=source.slice(start,end);let changed=body;for(const[from,to]of changes){assert.equal(changed.split(from).length-1,1,name+' anchor');changed=changed.replace(from,to);}mutants.push({name,file,edits:[[body,changed]]});}
edit('raw-zero-undefined-normalized-to-null','zero',[['return children->get(0);','return NodeValue(children->get(0).nodeOrNull());']]);
edit('raw-zero-null-converted-to-undefined','zero',[['return children->get(0);','auto v=children->get(0); return v.kind()==tree::ChildKind::Null?NodeValue(Missing{}):v;']]);
edit('raw-zero-ignore-prototype','zero',[['return children->get(0);','return children->hasOwn(0)?children->get(0):NodeValue(Missing{});']]);
edit('raw-zero-wrong-null-array-member','zero',[["reading '0'","reading 'coreMO'"]]);
edit('required-zero-ignore-prototype','required',[['return *children->get(0).requireNode(member);','return *(children->hasOwn(0)?children->get(0):NodeValue(Missing{})).requireNode(member);']]);
edit('required-zero-wrong-member','required',[['requireNode(member)','requireNode("0")']]);
for(const id of ['layout','base']){
 edit(id+'-return-child-instead-of-virtual-result',id,[['return child_zero(*this).requireNode("coreMO")->coreMO();','return child_zero(*this);']]);
 edit(id+'-normalize-virtual-undefined',id,[['return child_zero(*this).requireNode("coreMO")->coreMO();','return NodeValue(child_zero(*this).requireNode("coreMO")->coreMO().nodeOrNull());']]);
 edit(id+'-skip-nullish-member-error',id,[['return child_zero(*this).requireNode("coreMO")->coreMO();','auto child=child_zero(*this); return child.isNode()?child.requireNode("coreMO")->coreMO():child;']]);
}
for(const id of ['core','coreMO']){
 edit(id+'-bypass-virtual-getter',id,[['this->isEmbellished()','MmlMrow::isEmbellished()']]);
 edit(id+'-capture-cache-before-getter',id,[['if (!this->isEmbellished())','const auto savedCore=core_; if (!this->isEmbellished())'],['if (!core_) return this;','if (!savedCore) return this;'],['const std::size_t index = *core_;','const std::size_t index = *savedCore;']]);
 edit(id+'-capture-array-before-getter',id,[['if (!this->isEmbellished())','const auto savedChildren=this->childNodes; if (!this->isEmbellished())'],['const auto children = this->childNodes;','const auto children = savedChildren;']]);
 edit(id+'-zero-cache-treated-absent',id,[['if (!core_) return this;','if (!core_ || *core_==0) return this;']]);
 edit(id+'-wrong-dynamic-null-index',id,[['std::to_string(index)','std::string("0")']]);
 edit(id+'-ignore-getter-false-result',id,[['if (!this->isEmbellished()) return this;','(void)this->isEmbellished();']]);
}
edit('coreMO-return-child-not-call','coreMO',[['return child.requireNode("coreMO")->coreMO();','return child;']]);
edit('coreMO-normalize-virtual-undefined','coreMO',[['return child.requireNode("coreMO")->coreMO();','return NodeValue(child.requireNode("coreMO")->coreMO().nodeOrNull());']]);
edit('coreMO-skip-nullish-error','coreMO',[['return child.requireNode("coreMO")->coreMO();','return child.isNode()?child.requireNode("coreMO")->coreMO():child;']]);
edit('style-falsey-result-bool','style',[['return first.kind() == tree::ChildKind::Null ? Property(PropertyNull{}) : Property(Missing{});','return Property(false);']]);
edit('style-own-length-instead-of-child','style',[['grandchildren->size() == 1','currentArray->size() == 1']]);
edit('style-accept-two-child-length','style',[['grandchildren->size() == 1','grandchildren->size() <= 2']]);
edit('style-ignore-prototype','style',[['const NodeValue first = firstArray->get(0);','const NodeValue first = firstArray->hasOwn(0)?firstArray->get(0):NodeValue(Missing{});']]);
edit('style-null-inner-length-error-wrong','style',[["reading 'length'","reading 'childNodes'"]]);
require('./native-mutation-runner.cjs')({name:'math-port-mml-return-mutations',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_returns.cpp',additionalFixtures:['reference_mml_return_probes.cpp'],pattern:'*math_port_mml_returns*',assertions:187,tests:2,suiteSummaries:[{assertions:32,tests:1},{assertions:155,tests:1}],mutants});
