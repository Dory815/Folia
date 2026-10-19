// Root registered-class raw core return / positive / null / callback Source probes.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mo;
namespace {
using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct CacheObserver{virtual ~CacheObserver()=default;virtual Property observedCore()const=0;};
struct RowHook:MmlMrow,CacheObserver{using MmlMrow::MmlMrow;std::function<bool(bool)> after;bool isEmbellished()const override{const auto v=MmlMrow::isEmbellished();return after?after(v):v;}Property observedCore()const override{return core_;}void setObservedCore(Property v)const{core_=std::move(v);}};
struct InferredObserver:MmlInferredMrow,CacheObserver{using MmlInferredMrow::MmlInferredMrow;Property observedCore()const override{return core_;}};
struct MoHook:MmlMo{using MmlMo::MmlMo;std::function<NodeValue(NodeValue)> after;NodeValue coreMO()override{auto v=MmlMo::coreMO();return after?after(v):v;}};
struct Observed:MmlFactory{
 std::vector<MmlNode*> nodes;
 Observed(){setNodeClass("mrow",classBinding<RowHook>());setNodeClass("inferredMrow",classBinding<InferredObserver>());setNodeClass("mo",classBinding<MoHook>());}
 MmlNode* create(std::string_view kind,tree::FactoryArgs args)override{auto* n=MmlFactory::create(kind,args);nodes.push_back(n);return n;}
 std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return it==nodes.end()?"UNBOUND":"n"+std::to_string(it-nodes.begin());}
};
struct Case{std::string kind,shape,member,prototype,inner,rowMode,moResult;bool innerSet,innerNull,arrayNull,moHook;std::string expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
std::string observedCoreText(const Property& core){if(core.missing()||std::holds_alternative<PropertyNull>(core.value))return "N";if(const auto* number=std::get_if<double>(&core.value))return std::to_string(static_cast<std::size_t>(*number));return "UNBOUND";}
std::string array(const Children& a,const Observed& f){if(!a)return "N";std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=f.tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;}
std::string run(const Case& c){Observed f;std::map<char,MmlNode*> nodes;for(const auto& [label,kind]:std::array<std::pair<char,const char*>,4>{{{'a',"mi"},{'b',"mo"},{'c',"mi"},{'s',"mspace"}}})nodes[label]=f.create(kind,{});auto* node=f.create(c.kind,{});
 const auto shape=[&](std::string_view s){auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(s.size());for(std::size_t i=0;i<s.size();++i){auto v=s[i];if(v!='H')a->set(i,v=='U'?NodeValue(Missing{}):v=='N'?NodeValue(PropertyNull{}):NodeValue(nodes.at(v)));}return a;};
 if(c.innerSet)nodes['a']->childNodes=shape(c.inner);if(c.innerNull)nodes['a']->childNodes={};node->childNodes=c.arrayNull?Children{}:shape(c.shape);auto original=node->childNodes;if(!c.prototype.empty())original->setPrototype(shape(c.prototype));std::vector<std::string> trace;
 if(!c.rowMode.empty()){auto* row=dynamic_cast<RowHook*>(node);row->after=[&](bool v){trace.push_back(v?"emb:true":"emb:false");if(c.rowMode.find("none")!=std::string::npos)row->setObservedCore(Property(Missing{}));if(c.rowMode.find("outside")!=std::string::npos)row->setObservedCore(999);if(c.rowMode.find("null")!=std::string::npos)row->childNodes={};return !c.rowMode.starts_with("false");};}
 if(c.moHook||!c.moResult.empty())dynamic_cast<MoHook*>(nodes['b'])->after=[&](NodeValue v){trace.push_back("mo:coreMO");if(c.moHook)node->childNodes=shape("a");if(c.moResult.empty())return v;if(c.moResult=="U")return NodeValue(Missing{});if(c.moResult=="N")return NodeValue(PropertyNull{});return NodeValue(nodes.at(c.moResult[0]));};
 std::string result="THREW",error="ok";try{if(c.member=="core")result=f.tag(node->core());else if(c.member=="coreMO")result=f.tag(node->coreMO());else if(c.member=="isEmbellished")result=node->isEmbellished()?"true":"false";else if(c.member=="isSpacelike")result=node->isSpacelike()?"true":"false";else{const auto v=node->notParent();if(v.missing())result="U";else if(std::holds_alternative<PropertyNull>(v.value))result="N";else if(auto* b=std::get_if<bool>(&v.value))result=*b?"true":"false";else result="UNBOUND";}}catch(const Error& e){error=e.name+'|'+ascii(e.message);}
 std::string out=error+'|'+result+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto* n=f.nodes[i];std::string core="-";if(auto* observer=dynamic_cast<CacheObserver*>(n)){core=observedCoreText(observer->observedCore());}out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+array(n->childNodes,f)+':'+core;}out+='|'+array(original,f)+'|';for(std::size_t i=0;i<trace.size();++i){if(i)out+=',';out+=trace[i];}return out;
}
suite mml_return_probes=[] {"math_port_mml_returns_positive_callbacks"_test=[] {const Case cases[]={
        {"mrow","H","core","b","","","",false,false,false,false,"ok|n1|n0:mi:N:0::::-;n1:mo:N:0::::-;n2:mi:N:0::::-;n3:mspace:N:0::::-;n4:mrow:N:1:0:1:n1:0|1:0:1:n1|"},
        {"mrow","H","coreMO","b","","","",false,false,false,false,"ok|n1|n0:mi:N:0::::-;n1:mo:N:0::::-;n2:mi:N:0::::-;n3:mspace:N:0::::-;n4:mrow:N:1:0:1:n1:0|1:0:1:n1|"},
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.kind<<c.member<<c.shape<<c.rowMode<<c.moResult;};};
}
