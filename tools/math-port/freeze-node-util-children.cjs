'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23i-node-util-child-source.json')));assert.equal(source.cases.length,163);
const rows=source.cases.map(({input:i,expected})=>' {'+[i.op,i.shape,i.target??'',i.child??'',i.receiver??'',i.newReceiver??'',i.prototype??'',i.mode??'',i.parentMode??''].map(JSON.stringify).join(',')+','+(i.position??0)+','+Boolean(i.sourceNull)+','+Boolean(i.targetNull)+','+JSON.stringify(expected)+'},').join('\n');
const data=`// Root actual registered MML graph; expected values execute original NodeUtil.
#include "support/folia_test.hpp"
import folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
namespace {
using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct ParentHook:MmlMi{using MmlMi::MmlMi;std::function<void(NodeValue)> after;void setParent(NodeValue p)override{MmlMi::setParent(p);if(after)after(p);}};
struct Observed:MmlFactory{Observed(){setNodeClass("mi",classBinding<ParentHook>());}};
struct Receiver:tex::NodeUtil{std::function<void(std::size_t,NodeValue)> before,after;void setChild(NodeValue n,std::size_t p,NodeValue c)override{if(before)before(p,c);tex::NodeUtil::setChild(n,p,c);if(after)after(p,c);}};
struct Case{std::string op,shape,target,child,receiver,newReceiver,prototype,mode,parentMode;std::size_t position;bool sourceNull,targetNull;std::string expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
std::string run(const Case& c){Observed f;std::map<std::string,MmlNode*> nodes;for(auto label:{"a","b","c","old","target"})nodes[label]=f.create(std::string_view("mi"),tree::FactoryArgs{});std::vector<std::string> trace;
 const auto tag=[&](NodeValue v){if(v.kind()==tree::ChildKind::Undefined)return std::string("U");if(v.kind()==tree::ChildKind::Null)return std::string("N");for(const auto&[k,n]:nodes)if(n==v.nodeOrNull())return k;return std::string("UNBOUND");};
 const auto value=[&](std::string_view v){return v=="U"?NodeValue(Missing{}):v=="N"?NodeValue(PropertyNull{}):NodeValue(nodes.at(std::string(v)));};
 const auto shape=[&](std::string_view s){auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(s.size());for(std::size_t i=0;i<s.size();++i)if(s[i]!='H')a->set(i,value(s.substr(i,1)));return a;};
 nodes["old"]->childNodes=c.sourceNull?Children{}:shape(c.shape);nodes["target"]->childNodes=c.targetNull?Children{}:shape(c.target);const auto old=nodes["old"]->childNodes,target=nodes["target"]->childNodes;if(!c.prototype.empty())old->setPrototype(shape(c.prototype));
 const auto action=[&](std::string_view mode){if(mode=="source-rebind")nodes["old"]->childNodes=shape("b");if(mode=="source-grow")old->push_back(nodes["b"]);if(mode=="source-truncate")old->resize(1);if(mode=="source-delete")old->erase(2);if(mode=="source-prototype")old->setPrototype(shape("bbb"));if(mode=="target-rebind")nodes["target"]->childNodes=shape("b");if(mode=="target-null")nodes["target"]->childNodes={};if(mode=="throw")throw Error("Error",u"boom");};
 if(!c.parentMode.empty())dynamic_cast<ParentHook*>(nodes["a"])->after=[&](NodeValue v){trace.push_back("parent:a:"+tag(v));action(c.parentMode);};
 Receiver u;u.before=[&](std::size_t p,NodeValue v){trace.push_back("set:"+std::to_string(p)+':'+tag(v));};u.after=[&](std::size_t p,NodeValue){if(p==0)action(c.mode);};
 std::string result="THREW",error="ok";try{auto source=c.receiver.empty()?NodeValue(nodes["old"]):value(c.receiver),newNode=c.newReceiver.empty()?NodeValue(nodes["target"]):value(c.newReceiver);if(c.op=="get")result=tag(u.getChildAt(source,c.position));else if(c.op=="set"){u.setChild(c.receiver.empty()?newNode:source,c.position,value(c.child));result="U";}else{u.copyChildren(source,newNode);result="U";}}catch(const Error&e){error=e.name+'|'+ascii(e.message);}
 const auto array=[&](const Children&a){if(!a)return std::string("N");std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;};
 std::string out=error+'|'+result+'|';for(auto label:{"a","b","c","old","target"}){if(std::string_view(label)!="a")out+=';';auto*n=nodes[label];out+=std::string(label)+':'+tag(n->getParent())+':'+array(n->childNodes);}out+='|'+array(old)+'|'+array(target)+'|'+(old==nodes["old"]->childNodes?'1':'0')+(target==nodes["target"]->childNodes?'1':'0')+'|';for(std::size_t i=0;i<trace.size();++i){if(i)out+=',';out+=trace[i];}return out;
}
suite node_util_children=[] {"math_port_node_util_children_source"_test=[] {const Case cases[]={
${rows}
 };for(const auto&c:cases)expect(eq(run(c),c.expected))<<c.op<<c.shape<<c.position<<c.mode<<c.parentMode;};};
}
`;
const out=path.join(root,'tests/math/reference_node_util_children.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}console.log(source.cases.length+' actual original NodeUtil observations bound');
