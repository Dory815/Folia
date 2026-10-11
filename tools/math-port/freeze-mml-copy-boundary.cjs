// Root actual-factory boundary observer; expected values from original Source only.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),gold=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23g-copy-boundary-source.json')));assert.equal(gold.cases.length,6);
const rows=gold.cases.map(c=>' {'+JSON.stringify(c.mode)+','+JSON.stringify(c.expected)+'},').join('\n');
const data=`// Root actual MML copy boundary Source; registration after original construction.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct CopyHook:MmlMi{using MmlMi::MmlMi;std::function<void()> after;MmlNode* copy(bool keepIds=false)override{auto* n=MmlMi::copy(keepIds);if(after)after();return n;}};
struct Observed:MmlFactory{std::string mode;bool active=false;MmlNode* target=nullptr;std::optional<PropertyList> held;std::vector<MmlNode*> nodes;
 explicit Observed(std::string m):mode(std::move(m)){setNodeClass("mi",classBinding<CopyHook>());}
 MmlNode* create(std::string_view kind,tree::FactoryArgs args)override{if(active&&kind=="mrow"&&mode=="factory-null")return nullptr;auto* n=MmlFactory::create(kind,args);nodes.push_back(n);if(active&&kind=="mrow"){target=n;if(mode=="destination-null-falsey")n->childNodes.reset();if(mode=="retained-properties"){n->setProperty("sentinel",123);held=n->getAllProperties();}}return n;}
 std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return it==nodes.end()?"UNBOUND":"n"+std::to_string(it-nodes.begin());}
};
std::string scalar(const Property& p){if(p.missing())return "U";if(std::holds_alternative<PropertyNull>(p.value))return "N";if(auto b=std::get_if<bool>(&p.value))return *b?"T":"F";if(auto n=std::get_if<double>(&p.value))return "D"+std::to_string(static_cast<int>(*n));return "S"+std::get<std::string>(p.value);}
std::string array(const Children& a,const Observed& f){if(!a)return "N";std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=f.tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;}
std::string run(std::string mode){Observed f(mode);auto* a=f.create("mi",{});auto* b=f.create("mi",{});auto* n=f.create("mrow",{});n->setProperty("top",5);
 if(mode=="source-null")n->childNodes.reset();else if(mode=="inferred-inner-null"){auto* r=f.create("inferredMrow",{});r->childNodes.reset();n->childNodes=std::make_shared<tree::ChildArray<MmlNode>>(std::initializer_list<NodeValue>{r});}else if(mode=="destination-null-falsey")n->childNodes=std::make_shared<tree::ChildArray<MmlNode>>(std::initializer_list<NodeValue>{PropertyNull{},a});else n->childNodes=std::make_shared<tree::ChildArray<MmlNode>>(std::initializer_list<NodeValue>{a,PropertyNull{}});
 if(mode=="destination-rebind-after-copy")dynamic_cast<CopyHook*>(a)->after=[&]{f.target->childNodes=std::make_shared<tree::ChildArray<MmlNode>>(std::initializer_list<NodeValue>{b});};
 f.active=true;std::string error="ok",result;try{result=f.tag(n->copy());}catch(const Error& e){std::string message;for(auto c:e.message)message+=static_cast<char>(c);error=e.name+'|'+message;}
 std::string out=error+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto* n=f.nodes[i];out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+array(n->childNodes,f);}out+='|';out+=result;out+='|';out+=scalar(f.held?f.held->get("sentinel"):Property(Missing{}));out+='|';out+=scalar(f.held?f.held->get("top"):Property(Missing{}));out+='|';out+=scalar(f.held&&f.target&&Property(*f.held)==Property(f.target->getAllProperties()));return out;
}
suite boundary=[] {"math_port_mml_copy_boundary_actual_source"_test=[] {const std::pair<std::string,std::string> cases[]={
${rows}
 };for(const auto& [mode,expected]:cases)expect(eq(run(mode),expected))<<mode;};};
}
`;
const out=path.join(root,'tests/math/reference_mml_copy_boundary.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}console.log('6 actual Source copy boundary cases frozen');
