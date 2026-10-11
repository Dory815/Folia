// Root observer binding. Gold strings are read only from original Source output.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),data=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23f-constructor-source.json'),'utf8'));
const rows=data.cases.map(c=>'{'+[JSON.stringify(c.input.kind),JSON.stringify(c.input.properties),JSON.stringify(c.input.children),String(!!c.input.trace),JSON.stringify(c.expected)].join(',')+'}').join(',\n');
const text=`// Root-generated actual MML constructor/registry expectations; no native gold.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_node_host;
namespace m=folia::math::mml;namespace tr=folia::math::tree;namespace fm=folia::math;namespace h=m::host;using namespace boost::ut;namespace {
struct Factory:m::MmlFactory{
 std::vector<m::MmlNode*> nodes,adopted;std::vector<std::string> trace;
 m::MmlNode* create(std::string_view kind,tr::FactoryArgs args)override{auto* n=m::MmlFactory::create(kind,args);registerNode(n);return n;}
 m::MmlNode* adopt(std::unique_ptr<m::MmlNode> node)override{auto* n=m::MmlFactory::adopt(std::move(node));adopted.push_back(n);return n;}
 void registerNode(m::MmlNode* node){if(std::find(nodes.begin(),nodes.end(),node)==nodes.end())nodes.push_back(node);}
 std::string tag(m::NodeValue v){if(v.kind()==tr::ChildKind::Undefined)return "U";if(v.kind()==tr::ChildKind::Null)return "N";auto* n=v.requireNode();registerNode(n);return "n"+std::to_string(std::find(nodes.begin(),nodes.end(),n)-nodes.begin());}
 m::MmlNode* seed(){return create("mi",{});}
};
struct Trace:m::MmlMi{using MmlMi::MmlMi;void setChildren(m::Children c)override{dynamic_cast<Factory&>(factory).trace.push_back(std::string("children:attrs=")+(attributes()?"true":"false"));MmlMi::setChildren(std::move(c));}};
std::string scalar(const fm::Property& p){if(p.missing())return "U";if(std::holds_alternative<fm::PropertyNull>(p.value))return "N";if(const auto* b=std::get_if<bool>(&p.value))return *b?"T":"F";if(const auto* n=std::get_if<double>(&p.value))return "D"+std::to_string(static_cast<int>(*n));return "S"+std::get<std::string>(p.value);}
struct Input{std::string_view kind,properties,children;bool trace;std::string_view expected;};const Input inputs[]={${rows}};
std::string observe(const Input& i){Factory f;if(i.trace)f.setNodeClass("mi",m::classBinding<Trace>());std::string error="ok",result;
 try{m::Children children;if(i.children!="null"){children=std::make_shared<tr::ChildArray<m::MmlNode>>();children->resize(i.children.size());for(std::size_t j=0;j<i.children.size();++j){const char c=i.children[j];if(c=='H')continue;children->set(j,c=='U'?m::NodeValue(fm::Missing{}):c=='N'?m::NodeValue(fm::PropertyNull{}):m::NodeValue(f.seed()));}}
  std::any props=i.properties=="null"?std::any(nullptr):i.properties=="list"?std::any(fm::PropertyList{{"10",10},{"2",2},{"z","exact"},{"id","kept"},{"accent",true}}):std::any(fm::PropertyList{});const std::array<std::any,2> args{props,children};result=f.tag(f.create(i.kind,args));
 }catch(const tr::FactoryError& e){error=e.name+"|"+std::string(e.message.begin(),e.message.end());}
 std::string out=error+"|"+result+"|";for(std::size_t k=0;k<f.nodes.size();++k){auto* n=f.nodes[k];const auto a=n->childNodes;std::string own,has,values;for(std::size_t j=0;j<a->size();++j){own+=a->hasOwn(j)?'1':'0';has+=a->has(j)?'1':'0';if(j)values+=',';values+=f.tag(a->get(j));}const auto parent=f.tag(n->getParent());const auto attrs=n->attributes();out+="n"+std::to_string(k)+":"+n->kind()+":"+parent+":"+std::to_string(a->size())+":"+own+":"+has+":"+values+":"+(attrs?"true":"false")+":";if(attrs){const auto p=attrs->getAllAttributes();bool first=true;for(const auto& key:p.keys()){if(!first)out+=',';first=false;out+=key+"="+scalar(p.getOwn(key));}}out+=';';}out+='|';bool first=true;for(const auto& e:f.trace){if(!first)out+=',';first=false;out+=e;}return out;
}
}
suite math_port_mml_constructor=[] {
 "math_port_mml_constructor_actual_source_registry"_test=[] {for(const auto& i:inputs){expect(observe(i)==i.expected);}};
 "math_port_mml_constructor_actual_entry_points"_test=[] {Factory f;auto check=[](auto call){try{call();expect(true);}catch(const tr::FactoryError& e){expect(e.name!="Unimplemented")<<e.name;}};check([&]{f.create("mi",{});});if(!f.adopted.empty()){auto* n=f.adopted.front();check([&]{n->setChildren(std::make_shared<tr::ChildArray<m::MmlNode>>());});check([&]{n->appendChild(fm::PropertyNull{});});}else{expect(false);expect(false);}};
};
`;
const out=path.join(root,'tests/math/reference_mml_constructor.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),text);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,text);}console.log(JSON.stringify({source_cases:data.cases.length,native_entry_assertions:3}));
