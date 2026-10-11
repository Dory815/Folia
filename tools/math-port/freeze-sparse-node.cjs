// Root observer of generic Node cases from the unchanged actual Source111 corpus.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23-sparse-child-source.json')));
const selected=source.cases.filter(c=>/^(setChildren-|copy-|appendChild-|replaceChild-|removeChild-|childIndex-)/.test(c.name));
assert.equal(selected.length,64);
const snap=a=>`${a.length}:${a.own.map(Number).join('')}:${a.has.map(Number).join('')}:${a.values.join(',')};`;
function fingerprint(c){let s=(c.error?c.error.name+'|'+c.error.message:'ok')+'|'+snap(c.old)+snap(c.new);for(const[k,v]of Object.entries(c.parents))s+=k+'='+v+';';s+='|';if('sameArray'in c.extra)s+=c.extra.sameArray?'true':'false';if('result'in c.extra)s+=c.extra.result?'true':'false';if('index'in c.extra)s+=c.extra.index===null?'null':String(c.extra.index);return s;}
const rows=selected.map(c=>`{${JSON.stringify(c.name)},${JSON.stringify(fingerprint(c))}}`).join(',\n');
const text=`// Root-generated independent actual generic Node evidence; no native gold.
#include "support/folia_test.hpp"
import folia.math.core.tree.node;
import folia.math.core.tree.node_factory;
namespace tr=folia::math::tree;namespace fm=folia::math;using namespace boost::ut;namespace {
struct Probe:tr::Node<Probe>{using Node::Node;};
struct Generic:tr::AbstractNode<Probe,Probe>{using AbstractNode::AbstractNode;};
using Value=tr::ChildValue<Probe>;using Array=tr::ChildArray<Probe>;using Handle=tr::FactoryChildren<Probe>;
struct Factory:tr::AbstractNodeFactory<Probe>{
 std::vector<std::pair<std::string,Probe*>> nodes;std::string pending;
 static tr::FactoryClassMap<Probe> classes(){auto c=std::make_shared<tr::FactoryClass<Probe>>();c->construct=[](tr::FactoryServices<Probe>& f,tr::FactoryArgs args){auto n=std::make_unique<Generic>(f);auto* p=n.get();f.adopt(std::move(n));p->initializeNode(args);return p;};c->isInstance=[](const Probe* n){return dynamic_cast<const Generic*>(n)!=nullptr;};return {{"unknown",c}};}
 Factory():tr::AbstractNodeFactory<Probe>(classes()){}
 Probe* adopt(std::unique_ptr<Probe> n)override{auto* p=tr::AbstractNodeFactory<Probe>::adopt(std::move(n));nodes.emplace_back(pending.empty()?"copy"+std::to_string(nodes.size()):std::exchange(pending,{}),p);return p;}
 Probe* seed(std::string name){pending=std::move(name);return create("unknown");}
 std::string tag(Value v)const{if(v.kind()==tr::ChildKind::Undefined)return "U";if(v.kind()==tr::ChildKind::Null)return "N";for(const auto&[name,node]:nodes)if(v.requireNode()==node)return name;return "UNBOUND";}
};
struct Context{
 Factory factory;Probe* old=factory.seed("old"),*fresh=factory.seed("new"),*a=factory.seed("a"),*b=factory.seed("b"),*c=factory.seed("c");std::string extra;
 Value value(char symbol){if(symbol=='U')return fm::Missing{};if(symbol=='N')return fm::PropertyNull{};return symbol=='a'?Value(a):symbol=='b'?Value(b):Value(c);}
 Handle array(std::string_view shape){auto out=std::make_shared<Array>();out->resize(shape.size());for(std::size_t i=0;i<shape.size();++i)if(shape[i]!='H')out->set(i,value(shape[i]));return out;}
 std::string snapshot(const Handle& h)const{std::string own,has,values;for(std::size_t i=0;i<h->size();++i){own+=h->hasOwn(i)?'1':'0';has+=h->has(i)?'1':'0';if(i)values+=',';values+=factory.tag(h->get(i));}return std::to_string(h->size())+":"+own+":"+has+":"+values+";";}
 void run(std::string_view name){
  if(name.starts_with("setChildren-")){auto shape=name.substr(12);std::string compact{shape[0],shape[2],shape[4]};auto input=array(compact);old->childNodes=array("b");try{old->setChildren(input);}catch(...){fresh->childNodes=input;throw;}fresh->childNodes=input;}
  else if(name.starts_with("copy-")){auto shape=name.substr(5);old->childNodes=array(std::string{shape[0],shape[2],shape[4]});auto* copied=old->copy();fresh->childNodes=copied->childNodes;extra=copied->childNodes==old->childNodes?"true":"false";}
  else if(name.starts_with("appendChild-")){old->childNodes=array("b");const auto input=value(name.back());extra=old->appendChild(input)==input?"true":"false";}
  else if(name.starts_with("replaceChild-")){old->childNodes=array("ab");a->setParent(old);old->replaceChild(value(name.back()),a);}
  else if(name.starts_with("removeChild-")){old->childNodes=array(std::string{'a',name.back(),'b'});old->removeChild(value(name.back()));}
  else {old->childNodes=array("HUNab");auto index=old->childIndex(value(name.back()));extra=index?std::to_string(*index):"null";}
 }
 std::string observe(std::string_view name){std::string error="ok";try{run(name);}catch(const tr::FactoryError& e){error=e.name+"|"+std::string(e.message.begin(),e.message.end());}std::string out=error+"|"+snapshot(old->childNodes)+snapshot(fresh->childNodes);for(const auto&[name,node]:factory.nodes)out+=name+"="+factory.tag(node->getParent())+";";return out+"|"+extra;}
};
struct Input{std::string_view name,expected;};const Input inputs[]={${rows}};
}
suite math_port_sparse_node=[] {
 "math_port_sparse_node_actual_source_generic_cases"_test=[] {for(const auto& input:inputs){Context context;expect(context.observe(input.name)==input.expected);}};
};
`;
const out=path.join(root,'tests/math/reference_sparse_node.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),text);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,text);}
console.log(JSON.stringify({actual_source_cases:selected.length,assertions:selected.length,tests:1,remaining_node_util_cases:47}));
