// Root-generated actual MML constructor/registry expectations; no native gold.
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
struct Input{std::string_view kind,properties,children;bool trace;std::string_view expected;};const Input inputs[]={{"math","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:math:N:1:1:1:n0:true:;|"},
{"mi","empty","",false,"ok|n0|n0:mi:N:0::::true:;|"},
{"mn","empty","",false,"ok|n0|n0:mn:N:0::::true:;|"},
{"mo","empty","",false,"ok|n0|n0:mo:N:0::::true:;|"},
{"mtext","empty","",false,"ok|n0|n0:mtext:N:0::::true:;|"},
{"mspace","empty","",false,"ok|n0|n0:mspace:N:0::::true:;|"},
{"ms","empty","",false,"ok|n0|n0:ms:N:0::::true:;|"},
{"mrow","empty","",false,"ok|n0|n0:mrow:N:0::::true:;|"},
{"inferredMrow","empty","",false,"ok|n0|n0:inferredMrow:N:0::::true:;|"},
{"mfrac","empty","",false,"ok|n0|n0:mfrac:N:0::::true:;|"},
{"msqrt","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:msqrt:N:1:1:1:n0:true:;|"},
{"mroot","empty","",false,"ok|n0|n0:mroot:N:0::::true:;|"},
{"mstyle","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:mstyle:N:1:1:1:n0:true:;|"},
{"merror","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:merror:N:1:1:1:n0:true:;|"},
{"mpadded","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:mpadded:N:1:1:1:n0:true:;|"},
{"mphantom","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:mphantom:N:1:1:1:n0:true:;|"},
{"mfenced","empty","",false,"ok|n0|n0:mfenced:N:0::::true:;|"},
{"menclose","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:menclose:N:1:1:1:n0:true:;|"},
{"maction","empty","",false,"ok|n0|n0:maction:N:0::::true:;|"},
{"msub","empty","",false,"ok|n0|n0:msub:N:0::::true:;|"},
{"msup","empty","",false,"ok|n0|n0:msup:N:0::::true:;|"},
{"msubsup","empty","",false,"ok|n0|n0:msubsup:N:0::::true:;|"},
{"munder","empty","",false,"ok|n0|n0:munder:N:0::::true:;|"},
{"mover","empty","",false,"ok|n0|n0:mover:N:0::::true:;|"},
{"munderover","empty","",false,"ok|n0|n0:munderover:N:0::::true:;|"},
{"mmultiscripts","empty","",false,"ok|n0|n0:mmultiscripts:N:0::::true:;|"},
{"mprescripts","empty","",false,"ok|n0|n0:mprescripts:N:0::::true:;|"},
{"none","empty","",false,"ok|n0|n0:none:N:0::::true:;|"},
{"mtable","empty","",false,"ok|n0|n0:mtable:N:0::::true:;|"},
{"mlabeledtr","empty","",false,"ok|n0|n0:mlabeledtr:N:0::::true:;|"},
{"mtr","empty","",false,"ok|n0|n0:mtr:N:0::::true:;|"},
{"mtd","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:mtd:N:1:1:1:n0:true:;|"},
{"maligngroup","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:maligngroup:N:1:1:1:n0:true:;|"},
{"malignmark","empty","",false,"ok|n0|n0:malignmark:N:0::::true:;|"},
{"mglyph","empty","",false,"ok|n0|n0:mglyph:N:0::::true:;|"},
{"semantics","empty","",false,"ok|n0|n0:semantics:N:0::::true:;|"},
{"annotation","empty","",false,"ok|n0|n0:annotation:N:0::::true:;|"},
{"annotation-xml","empty","",false,"ok|n0|n0:annotation-xml:N:0::::true:;|"},
{"TeXAtom","empty","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:TeXAtom:N:1:1:1:n0:true:;|"},
{"MathChoice","empty","",false,"ok|n0|n0:MathChoice:N:0::::true:;|"},
{"text","empty","",false,"ok|n0|n0:text:N:0::::false:;|"},
{"XML","empty","",false,"ok|n0|n0:XML:N:0::::false:;|"},
{"html","empty","",false,"ok|n0|n0:html:N:0::::false:;|"},
{"mi","list","",false,"ok|n0|n0:mi:N:0::::true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mi","null","",false,"ok|n0|n0:mi:N:0::::true:;|"},
{"mrow","list","",false,"ok|n0|n0:mrow:N:0::::true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mrow","null","",false,"ok|n0|n0:mrow:N:0::::true:;|"},
{"mfrac","list","",false,"ok|n0|n0:mfrac:N:0::::true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mfrac","null","",false,"ok|n0|n0:mfrac:N:0::::true:;|"},
{"math","list","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:math:N:1:1:1:n0:true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"math","null","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:math:N:1:1:1:n0:true:;|"},
{"mstyle","list","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:mstyle:N:1:1:1:n0:true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mstyle","null","",false,"ok|n1|n0:inferredMrow:n1:0::::true:;n1:mstyle:N:1:1:1:n0:true:;|"},
{"msubsup","list","",false,"ok|n0|n0:msubsup:N:0::::true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"msubsup","null","",false,"ok|n0|n0:msubsup:N:0::::true:;|"},
{"mtable","list","",false,"ok|n0|n0:mtable:N:0::::true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mtable","null","",false,"ok|n0|n0:mtable:N:0::::true:;|"},
{"mtr","list","",false,"ok|n0|n0:mtr:N:0::::true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mtr","null","",false,"ok|n0|n0:mtr:N:0::::true:;|"},
{"mrow","list","ab",false,"ok|n2|n0:mi:n2:0::::true:;n1:mi:n2:0::::true:;n2:mrow:N:2:11:11:n0,n1:true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mrow","list","aH",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:mrow:N:1:1:1:n0:false:;|"},
{"mrow","list","aN",false,"TypeError|Cannot read properties of null (reading 'isInferred')||n0:mi:n1:0::::true:;n1:mrow:N:1:1:1:n0:false:;|"},
{"mrow","list","aU",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:mrow:N:1:1:1:n0:false:;|"},
{"mrow","list","null",false,"TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|||"},
{"math","list","ab",false,"ok|n3|n0:mi:n2:0::::true:;n1:mi:n2:0::::true:;n2:inferredMrow:n3:2:11:11:n0,n1:true:;n3:math:N:1:1:1:n2:true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"math","list","aH",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:inferredMrow:n2:1:1:1:n0:true:;n2:math:N:1:1:1:n1:false:;|"},
{"math","list","aN",false,"TypeError|Cannot read properties of null (reading 'isInferred')||n0:mi:n1:0::::true:;n1:inferredMrow:n2:1:1:1:n0:true:;n2:math:N:1:1:1:n1:false:;|"},
{"math","list","aU",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:inferredMrow:n2:1:1:1:n0:true:;n2:math:N:1:1:1:n1:false:;|"},
{"math","list","null",false,"TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))||n0:inferredMrow:n1:0::::true:;n1:math:N:1:1:1:n0:false:;|"},
{"mstyle","list","ab",false,"ok|n3|n0:mi:n2:0::::true:;n1:mi:n2:0::::true:;n2:inferredMrow:n3:2:11:11:n0,n1:true:;n3:mstyle:N:1:1:1:n2:true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mstyle","list","aH",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:inferredMrow:n2:1:1:1:n0:true:;n2:mstyle:N:1:1:1:n1:false:;|"},
{"mstyle","list","aN",false,"TypeError|Cannot read properties of null (reading 'isInferred')||n0:mi:n1:0::::true:;n1:inferredMrow:n2:1:1:1:n0:true:;n2:mstyle:N:1:1:1:n1:false:;|"},
{"mstyle","list","aU",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:inferredMrow:n2:1:1:1:n0:true:;n2:mstyle:N:1:1:1:n1:false:;|"},
{"mstyle","list","null",false,"TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))||n0:inferredMrow:n1:0::::true:;n1:mstyle:N:1:1:1:n0:false:;|"},
{"mfrac","list","ab",false,"ok|n2|n0:mi:n2:0::::true:;n1:mi:n2:0::::true:;n2:mfrac:N:2:11:11:n0,n1:true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|"},
{"mfrac","list","aH",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:mfrac:N:1:1:1:n0:false:;|"},
{"mfrac","list","aN",false,"TypeError|Cannot read properties of null (reading 'isInferred')||n0:mi:n1:0::::true:;n1:mfrac:N:1:1:1:n0:false:;|"},
{"mfrac","list","aU",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')||n0:mi:n1:0::::true:;n1:mfrac:N:1:1:1:n0:false:;|"},
{"mfrac","list","null",false,"TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|||"},
{"mi","empty","",true,"ok|n0|n0:mi:N:0::::true:;|children:attrs=false"},
{"mi","list","",true,"ok|n0|n0:mi:N:0::::true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|children:attrs=false"},
{"mi","null","",true,"ok|n0|n0:mi:N:0::::true:;|children:attrs=false"},
{"mi","empty","ab",true,"ok|n2|n0:mi:n2:0::::true:;n1:mi:n2:0::::true:;n2:mi:N:2:11:11:n0,n1:true:;|children:attrs=false,children:attrs=false,children:attrs=false"},
{"mi","list","ab",true,"ok|n2|n0:mi:n2:0::::true:;n1:mi:n2:0::::true:;n2:mi:N:2:11:11:n0,n1:true:2=D2,10=D10,z=Sexact,id=Skept,accent=T;|children:attrs=false,children:attrs=false,children:attrs=false"},
{"mi","null","ab",true,"ok|n2|n0:mi:n2:0::::true:;n1:mi:n2:0::::true:;n2:mi:N:2:11:11:n0,n1:true:;|children:attrs=false,children:attrs=false,children:attrs=false"},
{"mi","empty","null",true,"TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|||children:attrs=false"},
{"mi","list","null",true,"TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|||children:attrs=false"},
{"mi","null","null",true,"TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|||children:attrs=false"}};
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
