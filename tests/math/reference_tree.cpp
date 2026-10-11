// Root-generated pinned Node/Wrapper expectations. Do not hand-edit.
#include "support/folia_test.hpp"
import folia.math.core.tree.node;
import folia.math.core.tree.wrapper;
import folia.math.core.tree.node_factory;
import folia.math.core.tree.wrapper_factory;
namespace tr=folia::math::tree;using namespace boost::ut;namespace {
struct Probe:tr::Node<Probe>{using tr::Node<Probe>::Node;};
struct A:tr::AbstractNode<Probe,Probe>{using tr::AbstractNode<Probe,Probe>::AbstractNode;std::string kind()const override{return "a";}};
struct D:A{using A::A;std::string kind()const override{return "d";}};
struct E:tr::AbstractEmptyNode<Probe,Probe>{using tr::AbstractEmptyNode<Probe,Probe>::AbstractEmptyNode;std::string kind()const override{return "e";}};
struct V:A{using A::A;std::vector<std::string> trace;std::string kind()const override{return "v";}void setProperty(std::string_view n,folia::math::Property p)override{trace.push_back("p:"+std::string(n));A::setProperty(n,std::move(p));}void setChildren(tr::FactoryChildren<Probe> c)override{trace.push_back("children");A::setChildren(std::move(c));}tr::ChildValue<Probe> appendChild(tr::ChildValue<Probe> c)override{trace.push_back("append");return A::appendChild(c);}};
struct C:A{using A::A;std::function<void()> onCopy;std::string kind()const override{return "c";}Probe* copy(bool keepIds=false)override{if(onCopy)onCopy();return A::copy(keepIds);}};
struct S:A{using A::A;std::function<void(Probe*)> onAppend;std::string kind()const override{return "s";}tr::ChildValue<Probe> appendChild(tr::ChildValue<Probe> c)override{auto result=A::appendChild(c);if(onAppend)onAppend(c.requireNode("onAppend"));return result;}};
struct T:E{using E::E;std::function<void()> onString;std::string kind()const override{return "t";}std::string toString()const override{if(onString)onString();return "t";}};
struct K:A{using A::A;std::function<void()> onKind;std::string kind()const override{if(onKind)onKind();return "k";}};
struct Plain:tr::AbstractNode<Probe,Probe>{using tr::AbstractNode<Probe,Probe>::AbstractNode;};
struct F:A{using A::A;inline static Probe* leaked=nullptr;std::string kind()const override{return "f";}void setProperty(std::string_view n,folia::math::Property v)override{leaked=this;A::setProperty(n,std::move(v));if(n=="stop")throw tr::FactoryError("TypeError",u"setter");}};
template<class T>tr::FactoryClassRef<Probe> nodeClass(){auto c=std::make_shared<tr::FactoryClass<Probe>>();c->construct=[](tr::FactoryServices<Probe>& f,tr::FactoryArgs args){auto p=std::make_unique<T>(f);auto* result=p.get();f.adopt(std::move(p));result->initializeNode(args);return result;};c->isInstance=[](const Probe* p){return dynamic_cast<const T*>(p)!=nullptr;};return c;}
tr::FactoryClassMap<Probe> classes(){return {{"a",nodeClass<A>()},{"d",nodeClass<D>()},{"e",nodeClass<E>()},{"v",nodeClass<V>()}};}
tr::FactoryChildren<Probe> children(std::initializer_list<Probe*> c){auto a=std::make_shared<tr::ChildArray<Probe>>();for(auto* p:c)a->push_back(p);return a;}
std::size_t count(const Probe* p){return p->childNodes?p->childNodes->size():0;}
struct Wrapped:tr::Wrapper<Wrapped,Probe>{using tr::Wrapper<Wrapped,Probe>::Wrapper;};
struct View:tr::AbstractWrapper<Wrapped,Probe,Wrapped>{using tr::AbstractWrapper<Wrapped,Probe,Wrapped>::AbstractWrapper;};
tr::FactoryClassRef<Wrapped> viewType(){auto c=std::make_shared<tr::FactoryClass<Wrapped>>();c->construct=[](tr::FactoryServices<Wrapped>& service,tr::FactoryArgs args){auto& f=dynamic_cast<tr::WrapperServices<Wrapped,Probe>&>(service);return service.adopt(std::make_unique<View>(f,std::any_cast<Probe*>(args[0])));};c->isInstance=[](const Wrapped* w){return dynamic_cast<const View*>(w)!=nullptr;};return c;}
struct SpyFactory:tr::AbstractWrapperFactory<Wrapped,Probe>{std::vector<std::size_t> calls;SpyFactory():tr::AbstractWrapperFactory<Wrapped,Probe>(tr::FactoryClassMap<Wrapped>{{"a",viewType()},{"d",viewType()},{"e",viewType()},{"v",viewType()}}){}Wrapped* wrap(Probe* n,tr::FactoryArgs args={})override{calls.push_back(args.size());return tr::AbstractWrapperFactory<Wrapped,Probe>::wrap(n,args);}};
}
suite math_port_tree_reference=[] {
"math_port_tree_initialization_virtual_and_properties"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* c=f.create("a");
folia::math::PropertyList props{{"10",10},{"2",2},{"z",7},{"a",9}};auto* r=f.create("v",props,children({c}));auto* v=dynamic_cast<V*>(r);
expect(bool(v->trace==std::vector<std::string>{"p:2","p:10","p:z","p:a","children","append"}));
expect(bool(r->getPropertyNames()==std::vector<std::string>{"2","10","z","a"}));
expect(bool((r->getParent()==nullptr)==true));
expect(bool((c->getParent()==r)==true));
expect(bool((folia::math::Property(r->getAllProperties())==folia::math::Property(props))==false));
auto live=r->getAllProperties();live.set("z",8);
expect(bool((std::get<double>(r->getProperty("z").value))==8));
live.erase("2");r->setProperty("2",99);
expect(bool(r->getPropertyNames()==std::vector<std::string>{"2","10","z","a"}));
r->removeProperty(std::array<std::string,2>{"z","missing"});
expect(bool(r->getPropertyNames()==std::vector<std::string>{"2","10","a"}));
expect(bool((r->getProperty("missing").missing())==true));
expect(bool((r->getProperty("toString")==folia::math::Property(folia::math::BuiltinProperty::toString))==true));
};
"math_port_tree_parent_aliases_and_replacements"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* p=f.create("a");auto* q=f.create("a");auto* a=f.create("a");auto* b=f.create("d");auto old=p->childNodes;
p->appendChild(a);q->appendChild(a);
expect(bool((p->childNodes && !p->childNodes->empty() && p->childNodes->get(0)==a)==true));
expect(bool((a->getParent()==q)==true));
auto replaced=p->replaceChild(b,a);
expect(bool((b->getParent()==p)==true));
expect(bool((a->getParent()==q)==true));
expect(bool((p->replaceChild(a,q)==a)==true));
auto supplied=children({a,b});p->setChildren(supplied);
expect(bool((p->childNodes==supplied)==false));
expect(bool((p->childNodes==old)==false));
expect(bool((old->size())==1));
expect(bool((a->getParent()==p)==true));
auto current=p->childNodes;p->setChildren(current);
expect(bool((current==p->childNodes)==false));
expect(bool((count(p))==2));
expect(p->replaceChild(a,a)==a);
expect(bool((a->getParent()==nullptr)==true));
q->appendChild(b);p->removeChild(b);
expect(bool((b->getParent()==nullptr)==true));
expect(bool((q->childNodes->get(q->childNodes->size()-1)==b)==true));
expect(bool((!p->childIndex(q))==true));
};
"math_port_tree_null_partial_mutations"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* p=f.create("a");auto* a=f.create("a");
{bool caught=false;try{p->appendChild(nullptr);}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
expect(bool((count(p))==1));
expect(bool((p->childIndex(nullptr).value_or(999))==0));
{bool caught=false;try{p->replaceChild(a,nullptr);}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
expect(bool((p->childNodes && !p->childNodes->empty() && p->childNodes->get(0)==a)==true));
expect(bool((a->getParent()==p)==true));
{bool caught=false;try{p->replaceChild(nullptr,a);}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
expect(bool((a->getParent()==p)==true));
{bool caught=false;try{p->removeChild(nullptr);}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
expect(bool((count(p))==0));
p->appendChild(a);
{bool caught=false;try{p->setChildren({});}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));expect(bool(std::string(error.message.begin(),error.message.end())=="object null is not iterable (cannot read property Symbol(Symbol.iterator))"));}expect(caught);}
expect(bool((count(p))==0));
expect(bool((a->getParent()==p)==true));
expect(bool((p->removeChild(nullptr)==nullptr)==true));
{bool caught=false;try{(void)f.create("a",std::array<std::any,1>{nullptr});}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
{bool caught=false;try{(void)f.create("a",std::array<std::any,2>{folia::math::PropertyList{},nullptr});}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
};
"math_port_tree_empty_overrides_and_class_identity"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* a=f.create("a");auto* d=f.create("d");auto* e=f.create("e",folia::math::PropertyList{{"p",7}},children({a}));
expect(bool((count(e))==0));
expect(bool((a->getParent()==nullptr)==true));
expect(bool((e->appendChild(nullptr)==nullptr)==true));
expect(bool((e->replaceChild(d,a)==a)==true));
expect(bool((!e->childIndex(nullptr))==true));
expect(bool((e->toString())=="e"));
expect(bool((d->isKind("a"))==true));
expect(bool((e->isKind("a"))==false));
f.setNodeClass("a",nodeClass<D>());
expect(bool((a->isKind("a"))==false));
expect(bool((d->isKind("a"))==true));
{bool caught=false;try{(void)a->isKind("missing");}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
e->childNodes=children({a});
expect(bool(([&]{std::vector<std::string> seen;e->walkTree([&](Probe* n,const std::any&){seen.push_back(n->kind());});return seen;})()==std::vector<std::string>{"e"}));
expect(bool((!e->childIndex(a))==true));
e->removeChild(a);
expect(bool((count(e))==1));
};
"math_port_tree_copy_spread_and_null_children"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* p=f.create("a",folia::math::PropertyList{{"v",7}});auto* a=f.create("d");auto* e=f.create("e");p->appendChild(a);p->childNodes->push_back(nullptr);p->childNodes->push_back(e);p->getAllProperties().setOwn("__proto__",13);auto* copied=p->copy();
expect(bool((copied!=p)==true));
expect(bool((copied->getParent()==nullptr)==true));
expect(bool((folia::math::Property(copied->getAllProperties())!=folia::math::Property(p->getAllProperties()))==true));
expect(bool((copied->getAllProperties().hasOwn("__proto__"))==false));
expect(bool((copied->getProperty("__proto__")==folia::math::Property(folia::math::BuiltinProperty::prototype))==true));
expect(bool((count(copied))==2));
if(count(copied)==2){
expect(bool((copied->childNodes->get(0)!=a)==true));
expect(bool((copied->childNodes->get(0).requireNode("parent")->getParent()==copied)==true));
expect(bool((copied->childNodes->get(1).requireNode("toString")->toString())=="e"));
}
expect(bool((p->toString())=="a(d(),,e)"));
p->childNodes.reset();
expect(bool((count(p->copy()))==0));
{bool caught=false;try{(void)p->toString();}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
};
"math_port_tree_live_walk_array_replacement"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* p=f.create("a");auto* a=f.create("d");auto* e=f.create("e");auto* late=f.create("v");p->setChildren(children({a,e}));
auto original=p->childNodes;auto data=std::make_shared<int>(0);std::vector<std::string> seen;auto result=p->walkTree([&](Probe* n,const std::any& d){seen.push_back(n->kind());++*std::any_cast<std::shared_ptr<int>>(d);if(n==a){original->push_back(late);p->childNodes=children({});}},data);
expect(bool(seen==std::vector<std::string>{"a","d","e","v"}));
expect(bool((*data)==4));
expect(bool((std::any_cast<std::shared_ptr<int>>(result)==data)==true));
expect(bool((count(p))==0));
p->childNodes.reset();int visits=0;
{bool caught=false;try{(void)p->walkTree([&](Probe*,const std::any&){++visits;});}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));expect(bool(std::string(error.message.begin(),error.message.end())=="object null is not iterable (cannot read property Symbol(Symbol.iterator))"));}expect(caught);}
expect(bool((visits)==1));
};
"math_port_tree_walk_reads_array_after_self_and_find"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* p=f.create("a");auto* a=f.create("d");auto* e=f.create("e");
std::vector<std::string> seen;p->walkTree([&](Probe* n,const std::any&){seen.push_back(n->kind());if(n==p)p->childNodes=children({a,nullptr,e});});
expect(bool(seen==std::vector<std::string>{"a","d","e"}));
expect(bool((p->findNodes("a").size())==2));
expect(bool((p->findNodes("a").front()==p)==true));
expect(bool((p->findNodes("a").back()==a)==true));
{bool caught=false;try{(void)p->findNodes("missing");}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
};
"math_port_tree_wrapper_storage_and_dispatch"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* a=f.create("a");auto* e=f.create("e");SpyFactory wf;auto* w=wf.wrap(a);
expect(bool((w->childNodes.has_value())==false));
expect(bool((w->node==a)==true));
expect(bool((w->kind())=="a"));
auto* other=w->wrap(e,std::array<std::any,1>{std::string("dropped")});
expect(bool((wf.calls.back())==0));
expect(bool((other->node==e)==true));
w->node=e;
expect(bool((w->kind())=="e"));
w->node=nullptr;
{bool caught=false;try{(void)w->kind();}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
};
"math_port_tree_wrapper_live_walk_presence"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* a=f.create("a");auto* e=f.create("e");SpyFactory wf;auto* w=wf.wrap(a);auto* x=wf.wrap(e);auto* late=wf.wrap(a);std::vector<std::string> absent;w->walkTree([&](Wrapped*,const std::any&){absent.push_back("self");});
expect(bool(absent==std::vector<std::string>{"self"}));
w->childNodes=std::make_shared<tr::ChildArray<Wrapped>>(std::initializer_list<tr::ChildValue<Wrapped>>{x,nullptr});auto original=*w->childNodes;auto data=std::make_shared<int>(0);std::vector<std::string> seen;auto result=w->walkTree([&](Wrapped* n,const std::any& d){seen.push_back(n->kind());++*std::any_cast<std::shared_ptr<int>>(d);if(n==x){original->push_back(late);w->childNodes=std::make_shared<tr::ChildArray<Wrapped>>();}},data);
expect(bool(seen==std::vector<std::string>{"a","e","a"}));
expect(bool((*data)==3));
expect(bool((std::any_cast<std::shared_ptr<int>>(result)==data)==true));
w->childNodes=tr::FactoryChildren<Wrapped>{};int visits=0;
{bool caught=false;try{(void)w->walkTree([&](Wrapped*,const std::any&){++visits;});}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));expect(bool(std::string(error.message.begin(),error.message.end())=="object null is not iterable (cannot read property Symbol(Symbol.iterator))"));}expect(caught);}
expect(bool((visits)==1));
};
"math_port_tree_copy_live_mutation_rebinding_and_spread_alias"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
f.setNodeClass("c",nodeClass<C>());auto* p=f.create("a");auto* c=f.create("c");auto* late=f.create("e");folia::math::PropertyList value{{"v",7}};p->setProperty("alias",value);p->setChildren(children({c}));auto original=p->childNodes;dynamic_cast<C*>(c)->onCopy=[&]{original->push_back(late);p->childNodes=children({});};auto* copied=p->copy();
expect(bool((count(copied))==2));
expect(bool((count(p))==0));
expect(bool((copied->getProperty("alias")==folia::math::Property(value))==true));
if(count(copied)==2){
expect(bool((copied->childNodes->get(1).requireNode("kind")->kind())=="e"));
expect(bool((copied->childNodes->get(1).requireNode("parent")->getParent()==copied)==true));
}
f.setNodeClass("a",nodeClass<D>());
expect(bool((p->copy()->kind())=="d"));
expect(bool((p->copy(true)->kind())=="d"));
};
"math_port_tree_live_setChildren_and_join_evaluation"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
f.setNodeClass("s",nodeClass<S>());f.setNodeClass("t",nodeClass<T>());f.setNodeClass("k",nodeClass<K>());auto* p=f.create("s");auto* a=f.create("a");auto* d=f.create("d");auto input=children({a});dynamic_cast<S*>(p)->onAppend=[&](Probe* n){if(n==a)input->push_back(d);};p->setChildren(input);
expect(bool((count(p))==2));
expect(bool((d->getParent()==p)==true));
auto* root=f.create("a");auto* t=f.create("t");auto* e=f.create("e");auto* late=f.create("a");root->childNodes=children({t,e});auto captured=root->childNodes;dynamic_cast<T*>(t)->onString=[&]{captured->push_back(late);root->childNodes=children({});};
expect(bool((root->toString())=="a(t,e)"));
root->childNodes=children({t,e,e});captured=root->childNodes;dynamic_cast<T*>(t)->onString=[&]{captured->resize(1);};
expect(bool((root->toString())=="a(t,,)"));
auto* k=f.create("k");k->childNodes.reset();dynamic_cast<K*>(k)->onKind=[&]{k->childNodes=children({});};
expect(bool((k->toString())=="k()"));
};
"math_port_tree_copy_assignment_prototype_and_wrapper_empty_array"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* p=f.create("a");folia::math::PropertyList proto{{"inherited",7}};p->getAllProperties().setOwn("__proto__",proto);auto* c=p->copy();
expect(bool((c->getAllProperties().hasOwn("__proto__"))==false));
expect(bool((std::get<double>(c->getProperty("inherited").value))==7));
expect(bool((c->getProperty("__proto__")==folia::math::Property(proto))==true));
SpyFactory wf;auto* w=wf.wrap(p);w->childNodes=std::make_shared<tr::ChildArray<Wrapped>>();auto data=std::make_shared<int>(0);auto result=w->walkTree([&](Wrapped*,const std::any& d){++*std::any_cast<std::shared_ptr<int>>(d);},data);
expect(bool((*data)==1));
expect(bool((std::any_cast<std::shared_ptr<int>>(result)==data)==true));
};
"math_port_tree_join_recursive_array_suppression_and_recovery"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
auto* p=f.create("a");auto* d=f.create("d");p->childNodes=children({p});
expect(bool((p->toString())=="a(a())"));
auto shared=children({d});p->childNodes=shared;d->childNodes=shared;
expect(bool((p->toString())=="a(d())"));
d->childNodes=children({});
expect(bool((p->toString())=="a(d())"));
};
"math_port_tree_join_abrupt_conversion_unwinds_active_array"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
f.setNodeClass("t",nodeClass<T>());auto* p=f.create("a");auto* t=f.create("t");p->childNodes=children({t});dynamic_cast<T*>(t)->onString=[] {throw tr::FactoryError("TypeError",u"conversion");};
{bool caught=false;try{(void)p->toString();}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
dynamic_cast<T*>(t)->onString={};
expect(bool((p->toString())=="a(t)"));
};
"math_port_tree_default_kind_data_and_abrupt_virtual_initialization"_test=[] {
tr::AbstractNodeFactory<Probe> f(classes());
f.setNodeClass("unknown",nodeClass<Plain>());f.setNodeClass("f",nodeClass<F>());auto* p=f.create("unknown",folia::math::PropertyList{{"id","retained"}});auto* a=f.create("a");
expect(bool((p->kind())=="unknown"));
expect(bool((p->toString())=="unknown()"));
expect(bool((std::get<std::string>(p->copy()->getProperty("id").value))=="retained"));
expect(bool((std::get<std::string>(p->copy(true)->getProperty("id").value))=="retained"));
bool absent=false;auto data=p->walkTree([&](Probe*,const std::any& d){absent=!d.has_value();});
expect(bool((absent)==true));
expect(bool((!data.has_value())==true));
{bool caught=false;try{(void)f.create("f",folia::math::PropertyList{{"early",1},{"stop",2},{"late",3}},children({a}));}catch(const tr::FactoryError& error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
expect(bool((F::leaked!=nullptr)==true));
if(F::leaked){
expect(bool(F::leaked->getPropertyNames()==std::vector<std::string>{"early","stop"}));
expect(bool((count(F::leaked))==0));
expect(bool((a->getParent()==nullptr)==true));
} F::leaked=nullptr;
};
"math_port_tree_native_entry_points"_test=[] {tr::AbstractNodeFactory<Probe> f(classes());auto* p=f.create("a");SpyFactory wf;auto* w=wf.wrap(p);auto check=[](auto call){try{call();expect(true);}catch(const tr::FactoryError& e){expect(false)<<e.name;}};check([&]{p->walkTree([](Probe*,const std::any&){});});check([&]{(void)p->toString();});check([&]{w->walkTree([](Wrapped*,const std::any&){});});};
"math_port_tree_native_resource_boundary"_test=[] {tr::AbstractNodeFactory<Probe> f(classes());auto* root=f.create("a");auto* tail=root;for(std::size_t i=1;i<512;++i){auto* next=f.create("a");tail->appendChild(next);tail=next;}std::size_t seen=0;root->walkTree([&](Probe*,const std::any&){++seen;});expect(bool(seen==512));auto* copied=root->copy();seen=0;copied->walkTree([&](Probe*,const std::any&){++seen;});expect(bool(seen==512));expect(bool(root->toString().size()==3*512));tail->appendChild(f.create("a"));bool caught=false;try{root->walkTree([](Probe*,const std::any&){});}catch(const tr::FactoryError& e){caught=e.name=="RangeError";}expect(caught);caught=false;try{(void)root->copy();}catch(const tr::FactoryError& e){caught=e.name=="RangeError";}expect(caught);caught=false;try{(void)root->toString();}catch(const tr::FactoryError& e){caught=e.name=="RangeError";}expect(caught);seen=0;f.create("e")->walkTree([&](Probe*,const std::any&){++seen;});expect(bool(seen==1));};
"math_port_tree_native_wrapper_resource_boundary"_test=[] {tr::AbstractNodeFactory<Probe> f(classes());auto* node=f.create("a");SpyFactory wf;auto* root=wf.wrap(node);auto* tail=root;for(std::size_t i=1;i<512;++i){auto* next=wf.wrap(node);tail->childNodes=std::make_shared<tr::ChildArray<Wrapped>>(std::initializer_list<tr::ChildValue<Wrapped>>{next});tail=next;}std::size_t seen=0;root->walkTree([&](Wrapped*,const std::any&){++seen;});expect(bool(seen==512));tail->childNodes=std::make_shared<tr::ChildArray<Wrapped>>(std::initializer_list<tr::ChildValue<Wrapped>>{wf.wrap(node)});bool caught=false;try{root->walkTree([](Wrapped*,const std::any&){});}catch(const tr::FactoryError& e){caught=e.name=="RangeError";}expect(caught);seen=0;wf.wrap(node)->walkTree([&](Wrapped*,const std::any&){++seen;});expect(bool(seen==1));};
};
