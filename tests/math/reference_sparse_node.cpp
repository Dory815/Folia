// Root-generated independent actual generic Node evidence; no native gold.
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
struct Input{std::string_view name,expected;};const Input inputs[]={{"setChildren-H-H-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:001:001:U,U,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-H-H-c","ok|3:001:001:U,U,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-H-U-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:011:011:U,U,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-H-U-c","ok|3:011:011:U,U,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-H-N-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:011:011:U,N,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-H-N-c","ok|3:011:011:U,N,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-H-a-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:011:011:U,a,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-H-a-c","ok|3:011:011:U,a,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-H-b-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:011:011:U,b,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-H-b-c","ok|3:011:011:U,b,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-U-H-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:101:101:U,U,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-U-H-c","ok|3:101:101:U,U,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-U-U-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:111:111:U,U,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-U-U-c","ok|3:111:111:U,U,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-U-N-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:111:111:U,N,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-U-N-c","ok|3:111:111:U,N,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-U-a-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:111:111:U,a,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-U-a-c","ok|3:111:111:U,a,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-U-b-c","TypeError|Cannot set properties of undefined (setting 'parent')|1:1:1:U;3:111:111:U,b,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-U-b-c","ok|3:111:111:U,b,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-N-H-c","TypeError|Cannot set properties of null (setting 'parent')|1:1:1:N;3:101:101:N,U,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-N-H-c","ok|3:101:101:N,U,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-N-U-c","TypeError|Cannot set properties of null (setting 'parent')|1:1:1:N;3:111:111:N,U,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-N-U-c","ok|3:111:111:N,U,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-N-N-c","TypeError|Cannot set properties of null (setting 'parent')|1:1:1:N;3:111:111:N,N,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-N-N-c","ok|3:111:111:N,N,c;1:1:1:copy6;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;|false"},
{"setChildren-N-a-c","TypeError|Cannot set properties of null (setting 'parent')|1:1:1:N;3:111:111:N,a,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-N-a-c","ok|3:111:111:N,a,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-N-b-c","TypeError|Cannot set properties of null (setting 'parent')|1:1:1:N;3:111:111:N,b,c;old=N;new=N;a=N;b=N;c=N;|"},
{"copy-N-b-c","ok|3:111:111:N,b,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-a-H-c","TypeError|Cannot set properties of undefined (setting 'parent')|2:11:11:a,U;3:101:101:a,U,c;old=N;new=N;a=old;b=N;c=N;|"},
{"copy-a-H-c","ok|3:101:101:a,U,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-a-U-c","TypeError|Cannot set properties of undefined (setting 'parent')|2:11:11:a,U;3:111:111:a,U,c;old=N;new=N;a=old;b=N;c=N;|"},
{"copy-a-U-c","ok|3:111:111:a,U,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-a-N-c","TypeError|Cannot set properties of null (setting 'parent')|2:11:11:a,N;3:111:111:a,N,c;old=N;new=N;a=old;b=N;c=N;|"},
{"copy-a-N-c","ok|3:111:111:a,N,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-a-a-c","ok|3:111:111:a,a,c;3:111:111:a,a,c;old=N;new=N;a=old;b=N;c=old;|"},
{"copy-a-a-c","ok|3:111:111:a,a,c;3:111:111:copy6,copy7,copy8;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;copy8=copy5;|false"},
{"setChildren-a-b-c","ok|3:111:111:a,b,c;3:111:111:a,b,c;old=N;new=N;a=old;b=old;c=old;|"},
{"copy-a-b-c","ok|3:111:111:a,b,c;3:111:111:copy6,copy7,copy8;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;copy8=copy5;|false"},
{"setChildren-b-H-c","TypeError|Cannot set properties of undefined (setting 'parent')|2:11:11:b,U;3:101:101:b,U,c;old=N;new=N;a=N;b=old;c=N;|"},
{"copy-b-H-c","ok|3:101:101:b,U,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-b-U-c","TypeError|Cannot set properties of undefined (setting 'parent')|2:11:11:b,U;3:111:111:b,U,c;old=N;new=N;a=N;b=old;c=N;|"},
{"copy-b-U-c","ok|3:111:111:b,U,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-b-N-c","TypeError|Cannot set properties of null (setting 'parent')|2:11:11:b,N;3:111:111:b,N,c;old=N;new=N;a=N;b=old;c=N;|"},
{"copy-b-N-c","ok|3:111:111:b,N,c;2:11:11:copy6,copy7;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;|false"},
{"setChildren-b-a-c","ok|3:111:111:b,a,c;3:111:111:b,a,c;old=N;new=N;a=old;b=old;c=old;|"},
{"copy-b-a-c","ok|3:111:111:b,a,c;3:111:111:copy6,copy7,copy8;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;copy8=copy5;|false"},
{"setChildren-b-b-c","ok|3:111:111:b,b,c;3:111:111:b,b,c;old=N;new=N;a=N;b=old;c=old;|"},
{"copy-b-b-c","ok|3:111:111:b,b,c;3:111:111:copy6,copy7,copy8;old=N;new=N;a=N;b=N;c=N;copy5=N;copy6=copy5;copy7=copy5;copy8=copy5;|false"},
{"appendChild-U","TypeError|Cannot set properties of undefined (setting 'parent')|2:11:11:b,U;0:::;old=N;new=N;a=N;b=N;c=N;|"},
{"replaceChild-U","TypeError|Cannot set properties of undefined (setting 'parent')|2:11:11:U,b;0:::;old=N;new=N;a=old;b=N;c=N;|"},
{"removeChild-U","TypeError|Cannot set properties of undefined (setting 'parent')|2:11:11:a,b;0:::;old=N;new=N;a=N;b=N;c=N;|"},
{"appendChild-N","TypeError|Cannot set properties of null (setting 'parent')|2:11:11:b,N;0:::;old=N;new=N;a=N;b=N;c=N;|"},
{"replaceChild-N","TypeError|Cannot set properties of null (setting 'parent')|2:11:11:N,b;0:::;old=N;new=N;a=old;b=N;c=N;|"},
{"removeChild-N","TypeError|Cannot set properties of null (setting 'parent')|2:11:11:a,b;0:::;old=N;new=N;a=N;b=N;c=N;|"},
{"appendChild-a","ok|2:11:11:b,a;0:::;old=N;new=N;a=old;b=N;c=N;|true"},
{"replaceChild-a","ok|2:11:11:a,b;0:::;old=N;new=N;a=N;b=N;c=N;|"},
{"removeChild-a","ok|2:11:11:a,b;0:::;old=N;new=N;a=N;b=N;c=N;|"},
{"childIndex-hole-present-U","ok|5:01111:01111:U,U,N,a,b;0:::;old=N;new=N;a=N;b=N;c=N;|1"},
{"childIndex-hole-present-N","ok|5:01111:01111:U,U,N,a,b;0:::;old=N;new=N;a=N;b=N;c=N;|2"},
{"childIndex-hole-present-a","ok|5:01111:01111:U,U,N,a,b;0:::;old=N;new=N;a=N;b=N;c=N;|3"},
{"childIndex-hole-present-b","ok|5:01111:01111:U,U,N,a,b;0:::;old=N;new=N;a=N;b=N;c=N;|4"},
{"childIndex-hole-present-c","ok|5:01111:01111:U,U,N,a,b;0:::;old=N;new=N;a=N;b=N;c=N;|null"}};
}
suite math_port_sparse_node=[] {
 "math_port_sparse_node_actual_source_generic_cases"_test=[] {for(const auto& input:inputs){Context context;expect(context.observe(input.name)==input.expected);}};
};
