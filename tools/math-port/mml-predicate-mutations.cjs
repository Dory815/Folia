// Root isolated full-registry Source-bound comparisons and discriminating mutations.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');
const modules=[...cmake.match(/target_sources\(folia_math PUBLIC FILE_SET CXX_MODULES FILES([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.ixx)/g)].map(x=>x[1]);
const units=[...cmake.match(/target_sources\(folia_math PRIVATE([\s\S]*?)\)/)[1].matchAll(/src\/math\/(\S+\.cpp)/g)].map(x=>x[1]);const mutants=[];
const methods=[['base','core/mml_tree/mml_node.cpp','AbstractMmlNode::isEmpty'],['token','core/mml_tree/mml_branches.cpp','AbstractMmlTokenNode::isEmpty'],['space','core/mml_tree/mml_nodes/mrow.cpp','MmlMrow::isSpacelike'],['emb','core/mml_tree/mml_nodes/mrow.cpp','MmlMrow::isEmbellished']];
function edit(name,id,from,to){const[_,file,signature]=methods.find(m=>m[0]===id),source=fs.readFileSync(path.join(root,'src/math',file),'utf8').replaceAll('\r\n','\n'),start=source.indexOf('bool '+signature+'() const {'),end=source.indexOf('\n}\n',start)+2;assert.ok(start>=0&&end>start);const body=source.slice(start,end);assert.equal(body.split(from).length-1,1,name+' anchor');mutants.push({name,file,edits:[[body,body.replace(from,to)]]});}
for(const[id]of methods){
 edit(id+'-snapshot-length',id,'for (std::size_t i = 0; i < children->size(); ++i)','const auto length=children->size(); for (std::size_t i = 0; i < length; ++i)');
 edit(id+'-read-rebound-array',id,'children->get(i)','this->childNodes->get(i)');
 edit(id+'-null-array-accepted',id,'if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");','if (!children) return true;');
 edit(id+'-wrong-null-iterator-message',id,'if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");','if (!children) throw Error("TypeError",u"Object is not iterable.");');
}
edit('base-nullish-dereference','base','child.nodeOrNull()','child.requireNode("isEmpty")');
edit('base-omit-child-predicate','base','node && !node->isEmpty()','node && false');
edit('token-use-child-isEmpty','token','if (!text || !text->getText().empty()) return false;','if (!node || !node->isEmpty()) return false;');
edit('token-reject-TextNode-subclass','token','if (!text || !text->getText().empty()) return false;','if (!text || typeid(*text)!=typeid(TextNode) || !text->getText().empty()) return false;');
edit('token-ignore-nullish','token','MmlNode* const node = child.nodeOrNull();','if(!child.isNode())continue; MmlNode* const node = child.nodeOrNull();');
edit('base-ignore-prototype','base','const NodeValue child = children->get(i);','const NodeValue child = children->hasOwn(i)?children->get(i):NodeValue(Missing{});');
edit('token-ignore-prototype','token','const NodeValue child = children->get(i);','const NodeValue child = children->hasOwn(i)?children->get(i):NodeValue(Missing{});');
edit('space-skip-nullish','space','if (!value.requireNode("isSpacelike")->isSpacelike()) return false;','if(value.isNode() && !value.requireNode("isSpacelike")->isSpacelike()) return false;');
edit('space-ignore-prototype','space','const NodeValue value = children->get(i);','const NodeValue value = children->hasOwn(i)?children->get(i):NodeValue(Missing{});');
edit('emb-reset-stale-core','emb','bool embellished = false;','core_.reset(); bool embellished = false;');
edit('emb-omit-core-write','emb','core_ = i;','(void)i;');
edit('emb-core-index-off-by-one','emb','core_ = i;','core_ = i+1;');
edit('emb-accept-second-core','emb','if (embellished) return false;','if (embellished) return true;');
edit('emb-read-space-before-embellished','emb','if (child->isEmbellished()) {','if (!child->isSpacelike()) return false; if (child->isEmbellished()) {');
edit('emb-ignore-nonspace','emb','} else if (!child->isSpacelike()) {','} else if (false) {');
edit('emb-dereference-nullish','emb','MmlNode* const child = value.nodeOrNull();','MmlNode* const child = value.requireNode("isEmbellished");');
edit('emb-ignore-prototype','emb','const NodeValue value = children->get(i);','const NodeValue value = children->hasOwn(i)?children->get(i):NodeValue(Missing{});');
require('./native-mutation-runner.cjs')({name:'math-port-mml-predicate-mutations',modules,units,supportFiles:['util/option_storage.hpp'],fixture:'reference_mml_predicates.cpp',additionalFixtures:['reference_mml_predicate_probes.cpp','reference_mml_predicate_rebind.cpp'],pattern:'*math_port_mml_predicates*',assertions:193,tests:3,suiteSummaries:[{assertions:75,tests:1},{assertions:111,tests:1},{assertions:7,tests:1}],mutants});
