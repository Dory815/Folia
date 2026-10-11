'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),gold=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23b-sparse-mml-source.json'))),cases=gold.cases.filter(c=>c.name.startsWith('arity-')||c.name.startsWith('superscript-first-'));assert.equal(cases.length,54);
const snap=a=>a.length+':'+a.own.map(Number).join('')+':'+a.has.map(Number).join('')+':'+a.values.join(',');
const expected=c=>(c.error?c.error.name+'|'+c.error.message:'ok')+'|'+c.nodes.map(n=>n.id+':'+n.kind+':'+n.parent+':'+snap(n.children)).join(';')+'|'+Object.entries(c.captured).map(([k,a])=>k+'='+snap(a)).join(';')+'|'+Object.entries(c.extra).map(([k,v])=>k+'='+(typeof v==='object'?snap(v):String(v))).join(';')+'|'+c.trace.join(',');
const rows=cases.map(c=>' {'+JSON.stringify(c.name)+','+JSON.stringify(expected(c))+'},').join('\n');
const data=`// Root final 54 consumers from unchanged Source299; factory, inheritance and NodeUtil are real.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.node_util;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct Observed:MmlFactory{std::vector<MmlNode*> nodes;MmlNode* create(std::string_view k,tree::FactoryArgs a)override{auto*n=MmlFactory::create(k,a);if(std::find(nodes.begin(),nodes.end(),n)==nodes.end())nodes.push_back(n);return n;}std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto p=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return p==nodes.end()?"UNBOUND":"n"+std::to_string(p-nodes.begin());}};
struct Case{std::string name,expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
std::string run(const Case& c){Observed f;std::map<char,MmlNode*> nodes;for(char x:{'a','b','c'})nodes[x]=f.create(std::string_view("mi"),tree::FactoryArgs{});tex::NodeUtil util;std::map<std::string,Children> captured;std::vector<std::pair<std::string,std::string>> extra;
 const auto array=[&](std::string_view s){auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(s.size());for(std::size_t p=0;p<s.size();++p)if(s[p]!='H')a->set(p,s[p]=='U'?NodeValue(Missing{}):s[p]=='N'?NodeValue(PropertyNull{}):NodeValue(nodes.at(s[p])));return a;};
 const auto snap=[&](const Children&a){std::string own,has,values;for(std::size_t p=0;p<a->size();++p){own+=a->hasOwn(p)?'1':'0';has+=a->has(p)?'1':'0';if(p)values+=',';values+=f.tag(a->get(p));}return std::to_string(a->size())+':'+own+':'+has+':'+values;};
 std::string error="ok";try{if(c.name.starts_with("arity-")){const bool truncate=c.name.starts_with("arity-truncate-");auto*n=f.create(std::string_view(truncate?"mfrac":"msubsup"),tree::FactoryArgs{});auto suffix=c.name.substr(truncate?15:11);std::string shape;for(char x:suffix)if(x!='-')shape+=x;auto a=array(truncate?shape+"ab":shape.substr(0,2));n->childNodes=a;captured["original"]=a;n->setInheritedAttributes(PropertyList{},false,0,false);extra.push_back({"rebound",n->childNodes==a?"false":"true"});}
 else{auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->push_back(nodes['a']);std::array<std::any,2> args={PropertyList{},a};auto*n=f.create(std::string_view("msubsup"),args);util.setChild(n,2,nodes['c']);captured["superscript"]=n->childNodes;extra.push_back({"before",snap(n->childNodes)});if(c.name=="superscript-first-copy")extra.push_back({"result",f.tag(n->copy())});else if(c.name=="superscript-first-copyChildren"){auto*target=f.create(std::string_view("msubsup"),tree::FactoryArgs{});util.copyChildren(n,target);extra.push_back({"result",f.tag(target)});}else{if(c.name=="superscript-first-fill-and-inherit")util.setChild(n,1,nodes['b']);n->setInheritedAttributes(PropertyList{},false,0,false);}}
 }catch(const Error&e){error=e.name+'|'+ascii(e.message);}std::string out=error+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto*n=f.nodes[i];out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+snap(n->childNodes);}out+='|';for(const auto&[k,a]:captured){if(out.back()!='|')out+=';';out+=k+'='+snap(a);}out+='|';for(const auto&[k,v]:extra){if(out.back()!='|')out+=';';out+=k+'='+v;}return out+'|';}
suite inheritance_consumers=[] {"math_port_mml_inheritance_source299_consumers"_test=[] {const Case cases[]={
${rows}
 };for(const auto&c:cases)expect(eq(run(c),c.expected))<<c.name;};};
}
`;
const out=path.join(root,'tests/math/reference_mml_inheritance_consumers.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}console.log(cases.length+' unchanged Source299 arity/NodeUtil consumers bound');
