// Root actual Return32 labeled-node observer; factory still creates original inferred children.
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
        {"mstyle-core-H","mstyle","core","H",false,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:0:U||"},
        {"mstyle-coreMO-H","mstyle","coreMO","H",false,false,"TypeError|Cannot read properties of undefined (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:0:U||"},
        {"mstyle-core-U","mstyle","core","U",false,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:U||"},
        {"mstyle-coreMO-U","mstyle","coreMO","U",false,false,"TypeError|Cannot read properties of undefined (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:U||"},
        {"mstyle-core-N","mstyle","core","N",false,false,"ok|N|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:N||"},
        {"mstyle-coreMO-N","mstyle","coreMO","N",false,false,"TypeError|Cannot read properties of null (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:N||"},
        {"mstyle-core-a","mstyle","core","a",false,false,"ok|a|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:a||"},
        {"mstyle-coreMO-a","mstyle","coreMO","a",false,false,"ok|a|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:a||"},
        {"mfrac-core-H","mfrac","core","H",false,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:0:U||"},
        {"mfrac-coreMO-H","mfrac","coreMO","H",false,false,"TypeError|Cannot read properties of undefined (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:0:U||"},
        {"mfrac-core-U","mfrac","core","U",false,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:U||"},
        {"mfrac-coreMO-U","mfrac","coreMO","U",false,false,"TypeError|Cannot read properties of undefined (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:U||"},
        {"mfrac-core-N","mfrac","core","N",false,false,"ok|N|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:N||"},
        {"mfrac-coreMO-N","mfrac","coreMO","N",false,false,"TypeError|Cannot read properties of null (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:N||"},
        {"mfrac-core-a","mfrac","core","a",false,false,"ok|a|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:a||"},
        {"mfrac-coreMO-a","mfrac","coreMO","a",false,false,"ok|a|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:a||"},
        {"mstyle-notParent-H","mstyle","notParent","H",false,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:0:U||"},
        {"mstyle-notParent-U","mstyle","notParent","U",false,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:U||"},
        {"mstyle-notParent-N","mstyle","notParent","N",false,false,"ok|N|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:N||"},
        {"mstyle-notParent-a","mstyle","notParent","a",false,false,"ok|false|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:a||"},
        {"mrow-core-getter-rebind-H","mrow","core","H",true,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:0:U|original=1:1:b|embellished:true"},
        {"mrow-coreMO-getter-rebind-H","mrow","coreMO","H",true,false,"TypeError|Cannot read properties of undefined (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:0:U|original=1:1:b|embellished:true"},
        {"mrow-core-getter-rebind-U","mrow","core","U",true,false,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:U|original=1:1:b|embellished:true"},
        {"mrow-coreMO-getter-rebind-U","mrow","coreMO","U",true,false,"TypeError|Cannot read properties of undefined (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:U|original=1:1:b|embellished:true"},
        {"mrow-core-getter-rebind-N","mrow","core","N",true,false,"ok|N|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:N|original=1:1:b|embellished:true"},
        {"mrow-coreMO-getter-rebind-N","mrow","coreMO","N",true,false,"TypeError|Cannot read properties of null (reading 'coreMO')|THREW|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:N|original=1:1:b|embellished:true"},
        {"mrow-core-getter-rebind-a","mrow","core","a",true,false,"ok|a|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:a|original=1:1:b|embellished:true"},
        {"mrow-coreMO-getter-rebind-a","mrow","coreMO","a",true,false,"ok|a|a:N:0::;b:N:0::;c:N:0::;target:N:1:1:a|original=1:1:b|embellished:true"},
        {"mstyle-notParent-flag-length-H","mstyle","notParent","H",false,true,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:2:01:U,b||"},
        {"mstyle-notParent-flag-length-U","mstyle","notParent","U",false,true,"ok|U|a:N:0::;b:N:0::;c:N:0::;target:N:2:11:U,b||"},
        {"mstyle-notParent-flag-length-N","mstyle","notParent","N",false,true,"ok|N|a:N:0::;b:N:0::;c:N:0::;target:N:2:11:N,b||"},
        {"mstyle-notParent-flag-length-a","mstyle","notParent","a",false,true,"ok|false|a:N:0::;b:N:0::;c:N:0::;target:N:2:11:a,b||"},
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.name;};};
}
