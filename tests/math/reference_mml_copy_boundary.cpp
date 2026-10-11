// Root actual MML copy boundary Source; registration after original construction.
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
 {"source-null","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mrow:N:N;n3:mrow:N:0:::|n3|U|U|F"},
 {"inferred-inner-null","TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|n0:mi:N:0:::;n1:mi:N:0:::;n2:mrow:N:1:1:1:n3;n3:inferredMrow:N:N;n4:mrow:N:0:::||U|U|F"},
 {"factory-null","TypeError|Cannot set properties of null (setting 'properties')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mrow:N:2:11:11:n0,N||U|U|F"},
 {"destination-null-falsey","TypeError|Cannot read properties of null (reading 'push')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mrow:N:2:11:11:N,n0;n3:mrow:N:N||U|U|F"},
 {"destination-rebind-after-copy","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mrow:N:2:11:11:n0,N;n3:mrow:N:3:111:111:n1,n4,N;n4:mi:n3:0:::|n3|U|U|F"},
 {"retained-properties","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mrow:N:2:11:11:n0,N;n3:mrow:N:2:11:11:n4,N;n4:mi:n3:0:::|n3|D123|U|F"},
 };for(const auto& [mode,expected]:cases)expect(eq(run(mode),expected))<<mode;};};
}
