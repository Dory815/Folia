// Frozen by root from pinned MathJax factory operations. Do not hand-edit.
#include "support/folia_test.hpp"
import folia.math.core.tree.node_factory;
import folia.math.core.tree.wrapper_factory;
import folia.math.util.option_types;
namespace tr=folia::math::tree;namespace opts=folia::math::options;using namespace boost::ut;
namespace {
struct Probe:tr::FactoryNode {tr::FactoryServices<Probe>* factory=nullptr;std::vector<std::any> args;std::string report="reported-b";std::string kind() const override{return report;}};
struct BaseProbe:Probe{};struct DerivedProbe:BaseProbe{DerivedProbe(){report="reported-d";}};struct UnrelatedProbe:Probe{};
static_assert(!std::is_copy_constructible_v<tr::AbstractFactory<Probe>> && !std::is_move_constructible_v<tr::AbstractFactory<Probe>> && !std::is_copy_assignable_v<tr::AbstractFactory<Probe>> && !std::is_move_assignable_v<tr::AbstractFactory<Probe>>);
template<class T>tr::FactoryClassRef<Probe> klass(){auto c=std::make_shared<tr::FactoryClass<Probe>>();c->nativeType=typeid(T);c->construct=[](tr::FactoryServices<Probe>& factory,tr::FactoryArgs args){auto p=std::make_unique<T>();p->factory=&factory;p->args.assign(args.begin(),args.end());return factory.adopt(std::move(p));};c->isInstance=[](const Probe* p){return dynamic_cast<const T*>(p)!=nullptr;};return c;}
struct DefaultGuard {tr::FactoryClassMap<Probe> old=tr::AbstractFactory<Probe>::defaultNodes;~DefaultGuard(){tr::AbstractFactory<Probe>::defaultNodes=std::move(old);}};
opts::Key keyOf(std::string_view key){return opts::Key(std::u16string(key.begin(),key.end()));}
opts::Value valueOf(std::string_view key){return opts::Value(std::u16string(key.begin(),key.end()));}
struct Exposed:tr::AbstractFactory<Probe>{using tr::AbstractFactory<Probe>::AbstractFactory;void registerOnly(std::string key,tr::FactoryClassRef<Probe> c){nodeMapAccess()->set(valueOf(key),tr::FactoryClassMapValue<Probe>::native(std::move(c)));}};
tr::FactoryCreatorRef<Probe> own(tr::FactoryCreatorView<Probe> view,std::string_view key){for(auto& e:view->entries)if(e.first==keyOf(key))return e.second.creator;return {};}
void writeOwn(tr::FactoryCreatorView<Probe> view,std::string key,tr::FactoryCreatorRef<Probe> c){for(auto& e:view->entries)if(e.first==keyOf(key)){e.second.creator=std::move(c);return;}view->entries.emplace_back(keyOf(key),tr::FactoryCreatorSlot<Probe>{std::move(c),{}});}
bool same_properties(const std::any& x,const folia::math::PropertyList& p){return std::any_cast<folia::math::PropertyList>(x).getOwn("v")==p.getOwn("v") && folia::math::Property(std::any_cast<folia::math::PropertyList>(x))==folia::math::Property(p);}
}
suite math_port_factory_reference=[] {
"math_port_factory_defaults"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
DefaultGuard guard;tr::AbstractFactory<Probe>::defaultNodes={{"first",b},{"unknown",u}};tr::AbstractFactory<Probe> inherited;
expect(bool(inherited.getKinds()==std::vector<std::string>{"first","unknown"}));
expect(bool((inherited.create("absent")->kind())=="reported-b"));
tr::AbstractFactory<Probe> empty(tr::FactoryClassMap<Probe>{});
expect(bool(empty.getKinds()==std::vector<std::string>{}));
{bool caught=false;try{(void)empty.create("first");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
tr::FactoryClassMap<Probe> derived_defaults={{"own",d}};tr::AbstractFactory<Probe> child(std::nullopt,&derived_defaults);
expect(bool(child.getKinds()==std::vector<std::string>{"own"}));
expect(bool((child.create("own")->kind())=="reported-d"));
tr::AbstractFactory<Probe>::defaultNodes={{"after",d}};tr::AbstractFactory<Probe> later;
expect(bool(later.getKinds()==std::vector<std::string>{"after"}));
expect(bool(inherited.getKinds()==std::vector<std::string>{"first","unknown"}));
};
"math_port_factory_map_order_and_class_identity"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"10",b},{"2",d},{"alpha",u},{"01",b},{"4294967295",d}});
expect(bool(f.getKinds()==std::vector<std::string>{"2","10","alpha","01","4294967295"}));
f.setNodeClass("alpha",b);
expect(bool(f.getKinds()==std::vector<std::string>{"2","10","alpha","01","4294967295"}));
f.deleteNodeClass("2");f.setNodeClass("2",d);
expect(bool(f.getKinds()==std::vector<std::string>{"10","alpha","01","4294967295","2"}));
f.setNodeClass("alias",b);
expect(bool((f.getNodeClass("alias")==f.getNodeClass("alpha"))==true));
auto* original=f.create("alpha");auto* derived=f.create("2");auto unrelated=std::make_unique<UnrelatedProbe>();
expect(bool((f.nodeIsKind(original,"alpha"))==true));
expect(bool((f.nodeIsKind(derived,"alpha"))==true));
expect(bool((f.nodeIsKind(unrelated.get(),"alpha"))==false));
f.setNodeClass("alpha",d);
expect(bool((f.nodeIsKind(original,"alpha"))==false));
expect(bool((f.nodeIsKind(derived,"alpha"))==true));
expect(bool((f.nodeIsKind(original,"alias"))==true));
expect(bool((f.create("alpha")->kind())=="reported-d"));
expect(bool((f.nodeIsKind(nullptr,"alias"))==false));
{bool caught=false;try{(void)f.nodeIsKind(original,"missing");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
expect(bool((!f.getNodeClass("missing"))==true));
f.setNodeClass("invalid",{});
expect(bool(f.getKinds()==std::vector<std::string>{"10","alpha","01","4294967295","2","alias","invalid"}));
{bool caught=false;try{(void)f.create("invalid");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
{bool caught=false;try{(void)f.nodeIsKind(original,"invalid");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
};
"math_port_factory_capture_and_arguments"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});
std::array<std::any,3> args{3.0,std::string("arg"),nullptr};auto* r=f.create("chosen",args);
expect(bool((r->factory==&f)==true));
expect(bool((r->args.size())==3));
expect(bool((std::any_cast<double>(r->args[0]))==3));
expect(bool((std::any_cast<std::string>(r->args[1]))=="arg"));
expect(bool((std::any_cast<std::nullptr_t>(r->args[2])==nullptr)==true));
f.defaultKind=std::u16string(u"chosen");
expect(bool((f.create("absent",std::array<std::any,1>{4.0})->kind())=="reported-b"));
f.deleteNodeClass("chosen");
{bool caught=false;try{(void)f.create("absent");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
auto metadata=std::make_shared<int>(7);b->staticData=metadata;
expect(bool((std::any_cast<std::shared_ptr<int>>(b->staticData)==metadata)==true));
tr::AbstractFactory<Probe> h(tr::FactoryClassMap<Probe>{{"chosen",b}});folia::math::PropertyList handle{{"v",7}};auto* hr=h.create("chosen",std::array<std::any,2>{handle,handle});
expect(bool((same_properties(hr->args[0],handle))==true));
expect(bool((folia::math::Property(std::any_cast<folia::math::PropertyList>(hr->args[0]))==folia::math::Property(std::any_cast<folia::math::PropertyList>(hr->args[1])))==true));
};
"math_port_factory_prototype_creator_retained"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"unknown",u}});
{bool caught=false;try{(void)f.create("__proto__");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
f.setNodeClass("__proto__",b);
expect(bool(f.getKinds()==std::vector<std::string>{"unknown","__proto__"}));
expect(bool((f.create("__proto__")->kind())=="reported-b"));
f.deleteNodeClass("__proto__");
expect(bool(f.getKinds()==std::vector<std::string>{"unknown"}));
expect(bool((f.create("__proto__")->kind())=="reported-b"));
expect(bool((!f.getNodeClass("__proto__"))==true));
{bool caught=false;try{(void)f.nodeIsKind(f.create("__proto__"),"__proto__");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
};
"math_port_factory_node_factory_direct_and_defaults"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
tr::AbstractNodeFactory<Probe> f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});
{bool caught=false;try{(void)f.create("absent");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
auto* one=f.create("chosen");auto* two=f.create("chosen");
expect(bool((one->args.size())==2));
expect(bool((folia::math::Property(std::any_cast<folia::math::PropertyList>(one->args[0]))!=folia::math::Property(std::any_cast<folia::math::PropertyList>(two->args[0])))==true));
expect(bool((std::any_cast<tr::FactoryChildren<Probe>>(one->args[1])!=std::any_cast<tr::FactoryChildren<Probe>>(two->args[1]))==true));
folia::math::PropertyList props{{"v",7}};auto children=std::make_shared<tr::ChildArray<Probe>>(std::initializer_list<tr::ChildValue<Probe>>{one,nullptr});auto* r=f.create("chosen",props,children);
expect(bool((same_properties(r->args[0],props))==true));
expect(bool((std::any_cast<tr::FactoryChildren<Probe>>(r->args[1])==children)==true));
tr::FactoryServices<Probe>& service=f;std::array<std::any,3> raw_args{std::any{},nullptr,std::string("discarded")};auto* raw=service.create("chosen",raw_args);
expect(bool((raw->args.size())==2));
expect(bool((std::any_cast<std::nullptr_t>(raw->args[1])==nullptr)==true));
std::array<std::any,2> null_args{nullptr,nullptr};auto* all=service.create("chosen",null_args);
expect(bool((std::any_cast<std::nullptr_t>(all->args[0])==nullptr)==true));
};
"math_port_factory_wrapper_factory_forwarding"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
tr::AbstractWrapperFactory<Probe,Probe> f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});
struct View:Probe {mutable int reads=0;std::string kind()const override{++reads;return "chosen";}} node;std::array<std::any,2> args{7.0,std::string("extra")};auto* r=f.wrap(&node,args);
expect(bool((node.reads)==1));
expect(bool((r->factory==&f)==true));
expect(bool((r->args.size())==3));
expect(bool((std::any_cast<Probe*>(r->args[0])==&node)==true));
expect(bool((std::any_cast<double>(r->args[1]))==7));
expect(bool((std::any_cast<std::string>(r->args[2]))=="extra"));
Probe missing;missing.report="absent";
expect(bool((f.wrap(&missing)->kind())=="reported-b"));
{bool caught=false;try{(void)f.wrap(nullptr);}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
};
"math_port_factory_independent_live_creator_object"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
Exposed f(tr::FactoryClassMap<Probe>{{"chosen",b},{"unknown",u}});auto view=f.creatorView();auto saved=own(view,"chosen");
expect(bool((view==f.creatorView())==true));
f.registerOnly("chosen",d);
expect(bool((f.getNodeClass("chosen")==d)==true));
expect(bool((f.create("chosen")->kind())=="reported-b"));
expect(bool((f.nodeIsKind(f.create("chosen"),"chosen"))==false));
f.setNodeClass("chosen",d);
expect(bool((view==f.creatorView())==true));
expect(bool((saved->call({})->kind())=="reported-b"));
expect(bool((own(view,"chosen")->call({})->kind())=="reported-d"));
writeOwn(view,"alias",saved);
expect(bool((f.create("alias")->kind())=="reported-b"));
expect(bool((!f.getNodeClass("alias"))==true));
{bool caught=false;try{(void)f.nodeIsKind(f.create("alias"),"alias");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
writeOwn(view,"chosen",{});
expect(bool((f.create("chosen")->kind())=="reported-b"));
expect(bool((f.getNodeClass("chosen")==d)==true));
f.deleteNodeClass("chosen");
expect(bool((saved->call({})->kind())=="reported-b"));
expect(bool((bool(own(view,"chosen")))==false));
};
"math_port_factory_own_proto_and_falsy_direct"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"base",b},{"derived",d},{"unknown",u}});auto view=f.creatorView();
f.setNodeClass("__proto__",b);
auto retained=view->prototypeCreator;
writeOwn(view,"__proto__",own(view,"derived"));
expect(bool((f.create("__proto__")->kind())=="reported-d"));
f.setNodeClass("__proto__",d);
expect(bool((view->prototypeCreator==retained)==true));
f.deleteNodeClass("__proto__");
expect(bool((f.create("__proto__")->kind())=="reported-b"));
writeOwn(view,"toString",{});
expect(bool((f.create("toString")->kind())=="reported-b"));
tr::AbstractNodeFactory<Probe> direct(tr::FactoryClassMap<Probe>{{"base",b},{"unknown",u}});writeOwn(direct.creatorView(),"base",{});
{bool caught=false;try{(void)direct.create("base");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
direct.setNodeClass("__proto__",b);direct.deleteNodeClass("__proto__");
expect(bool((direct.create("__proto__")->kind())=="reported-b"));
{bool caught=false;try{(void)f.create("call");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
{bool caught=false;try{(void)f.create("apply");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
{bool caught=false;try{(void)f.create("bind");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
{bool caught=false;try{(void)f.create("arguments");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
{bool caught=false;try{(void)f.create("caller");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
{bool caught=false;try{(void)f.create("prototype");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
expect(bool((f.create("length")->kind())=="reported-b"));
expect(bool((f.create("name")->kind())=="reported-b"));
writeOwn(view,"call",{});
expect(bool((f.create("call")->kind())=="reported-b"));
writeOwn(view,"bad",std::make_shared<tr::FactoryCreator<Probe>>());
{bool caught=false;try{(void)f.create("bad");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
};
"math_port_factory_predicate_is_not_construction"_test=[] {
auto b=klass<BaseProbe>(),d=klass<DerivedProbe>(),u=klass<UnrelatedProbe>();
auto throwing=klass<DerivedProbe>();throwing->nativeType=typeid(void);throwing->construct=[](tr::FactoryServices<Probe>&,tr::FactoryArgs)->Probe*{throw tr::FactoryError("TypeError",u"body");};tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{{"chosen",throwing},{"unknown",u}});DerivedProbe r;
expect(bool((f.nodeIsKind(&r,"chosen"))==true));
{bool caught=false;try{(void)f.create("chosen");}catch(const tr::FactoryError& e){caught=true;expect(bool(e.name=="TypeError"));}expect(caught);}
tr::AbstractFactory<Probe> duplicate(tr::FactoryClassMap<Probe>{{"a",b},{"10",b},{"2",b},{"a",d}});
expect(bool(duplicate.getKinds()==std::vector<std::string>{"2","10","a"}));
expect(bool((duplicate.create("a")->kind())=="reported-d"));
};
"math_port_factory_native_session_lifetime"_test=[] {
int live=0;struct Live:Probe {int* count;explicit Live(int* c):count(c){++*count;}~Live(){--*count;}};
{tr::AbstractFactory<Probe> f(tr::FactoryClassMap<Probe>{});auto partial=std::make_shared<tr::FactoryClass<Probe>>();partial->construct=[&live](tr::FactoryServices<Probe>& service,tr::FactoryArgs)->Probe*{service.adopt(std::make_unique<Live>(&live));throw tr::FactoryError("TypeError",u"initialization failed");};f.setNodeClass("partial",partial);bool caught=false;try{(void)f.create("partial");}catch(const tr::FactoryError&){caught=true;}expect(caught);expect(bool(live==1));(void)f.adopt(std::make_unique<Live>(&live));expect(bool(live==2));}
expect(bool(live==0));
};
};
