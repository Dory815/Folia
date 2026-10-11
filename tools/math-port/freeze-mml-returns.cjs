// Root binds the pre-existing original Return32 to registered native classes.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23b-sparse-mml-return-source.json')));assert.equal(source.cases.length,32);
function array(a){return a.length+':'+a.own.map(Number).join('')+':'+a.values.join(',');}
function fingerprint(c){return(c.error?c.error.name+'|'+c.error.message:'ok')+'|'+c.result+'|'+Object.entries(c.nodes).map(([label,n])=>label+':'+n.parent+':'+array(n.children)).join(';')+'|'+Object.entries(c.captured).map(([k,a])=>k+'='+array(a)).join(';')+'|'+c.trace.join(',');}
const rows=source.cases.map(c=>{const m=c.name.match(/^(mstyle|mfrac|mrow)-(core|coreMO|notParent)-(getter-rebind-|flag-length-)?([HUNa])$/);assert.ok(m);return '        {'+[c.name,m[1],m[2],m[4]].map(JSON.stringify).join(',')+','+(m[3]==='getter-rebind-')+','+(m[3]==='flag-length-')+','+JSON.stringify(fingerprint(c))+'},';}).join('\n');
const data=`// Root actual Return32 labeled-node observer; factory still creates original inferred children.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mrow;
namespace {
using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct RowHook:MmlMrow{using MmlMrow::MmlMrow;std::function<void(bool)> after;bool isEmbellished()const override{const auto v=MmlMrow::isEmbellished();if(after)after(v);return v;}};
struct Case{std::string name,kind,member,state;bool rebind,secondSlot;std::string expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
std::string run(const Case& c){MmlFactory f;f.setNodeClass("mrow",classBinding<RowHook>());std::vector<std::pair<std::string,MmlNode*>> nodes;for(const auto& [label,kind]:std::array<std::pair<const char*,const char*>,4>{{{"a","mi"},{"b","mo"},{"c","mi"},{"target",c.kind.c_str()}}})nodes.push_back({label,f.create(std::string_view(kind),tree::FactoryArgs{})});auto* node=nodes.back().second;
 const auto tag=[&](NodeValue v){if(v.kind()==tree::ChildKind::Undefined)return std::string("U");if(v.kind()==tree::ChildKind::Null)return std::string("N");for(const auto& [label,n]:nodes)if(n==v.nodeOrNull())return label;return std::string("UNBOUND");};
 const auto shape=[&](std::string_view s){auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(s.size());for(std::size_t i=0;i<s.size();++i){auto v=s[i];if(v=='H')continue;if(v=='U')a->set(i,Missing{});else if(v=='N')a->set(i,PropertyNull{});else a->set(i,nodes[v-'a'].second);}return a;};
 const auto array=[&](const Children& a){std::string own,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';if(i)values+=',';values+=tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+values;};
 node->childNodes=shape(c.rebind?"b":c.state+(c.secondSlot?"b":""));Children original;std::vector<std::string> trace;if(c.rebind){original=node->childNodes;dynamic_cast<RowHook*>(node)->after=[&](bool v){trace.push_back(v?"embellished:true":"embellished:false");node->childNodes=shape(c.state);};}
 std::string error="ok",result="THREW";try{if(c.member=="core")result=tag(node->core());else if(c.member=="coreMO")result=tag(node->coreMO());else{const auto value=node->notParent();if(value.missing())result="U";else if(std::holds_alternative<PropertyNull>(value.value))result="N";else if(auto* v=std::get_if<bool>(&value.value))result=*v?"true":"false";else result="UNBOUND";}}catch(const Error& e){error=e.name+'|'+ascii(e.message);}
 std::string out=error+'|'+result+'|';for(std::size_t i=0;i<nodes.size();++i){if(i)out+=';';out+=nodes[i].first+':'+tag(nodes[i].second->getParent())+':'+array(nodes[i].second->childNodes);}out+='|';if(original)out+="original="+array(original);out+='|';for(std::size_t i=0;i<trace.size();++i){if(i)out+=',';out+=trace[i];}return out;
}
suite mml_returns_source=[] {"math_port_mml_returns_actual_source"_test=[] {const Case cases[]={
${rows}
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.name;};};
}
`;
const file=path.join(root,'tests/math/reference_mml_returns.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(file));fs.writeFileSync(file,data);}console.log('32 original raw-return/getter-rebind cases frozen');
