// Root stimuli replay actual hash-pinned Source observations. Token maps here
// are interface fixtures; no concrete TokenMap/parser implementation is claimed.
#include "support/folia_test.hpp"
import folia.math.input.tex.map_handler;
import folia.math.core.tree.factory_types;
import folia.math.util.property_host;
namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t=folia::math::tex;
struct Parser:t::ParserContext {Property cs;Property currentCS()const override{return cs;}void setCurrentCS(Property p)override{cs=std::move(p);}};
struct Map:t::TokenMap {
 Property key; t::ParseMethod method=Property(PropertyNull{});
 std::function<Property()> nameFn;
 std::function<bool(const Property&)> containsFn=[](const auto&){return true;};
 std::function<t::TokenLookup(const Property&)> lookupFn=[](const auto&)->t::TokenLookup{return Property(PropertyNull{});};
 std::function<t::ParseResult(const t::ParseInput&)> parseFn=[](const auto&)->t::ParseResult{return Property(PropertyNull{});};
 explicit Map(Property name):key(std::move(name)){}
 Property name()const override{return nameFn?nameFn():key;}
 t::ParseMethod parser()const override{return method;}
 void parser(t::ParseMethod m)override{method=std::move(m);}
 bool contains(const Property&p)override{return containsFn(p);}
 t::ParseMethod parserFor(const Property&)override{return method;}
 t::TokenLookup lookup(const Property&p)override{return lookupFn(p);}
 t::ParseResult parse(const t::ParseInput&i)override{return parseFn(i);}
};
struct CharacterMap:Map,t::CharacterMapIdentity {using Map::Map;};
struct Handler:t::SubHandler {
 using t::SubHandler::SubHandler;
 std::vector<Property> names(){std::vector<Property> r;for(auto&i:configuration_)r.push_back(i.item->name());return r;}
 std::vector<std::shared_ptr<FunctionListItem>> items(){std::vector<std::shared_ptr<FunctionListItem>> r;for(auto i=fallback_.begin();i!=fallback_.end();++i)r.push_back(i.record());return r;}
 static std::vector<std::shared_ptr<FunctionListItem>> itemsOf(t::SubHandler&h){auto member=&Handler::fallback_;auto&list=h.*member;std::vector<std::shared_ptr<FunctionListItem>>r;for(auto i=list.begin();i!=list.end();++i)r.push_back(i.record());return r;}
 double configurationPriority(){auto i=configuration_.begin();return i==configuration_.end()?std::numeric_limits<double>::quiet_NaN():i.record()->priority;}
};
FunctionRef fn(t::ParseBody body){return t::makeParseCallable(std::move(body));}
FunctionRef noOp(){return fn([](const auto&,auto&,const auto&,auto)->t::ParseResult{return Property(Missing{});});}
auto map(Property name){auto m=std::make_shared<Map>(std::move(name));t::MapHandler::registerMap(m);return m;}
NativeObjectRef receiver(const FunctionReceiver&r){auto p=std::get_if<NativeObjectRef>(&r);return p?*p:NativeObjectRef{};}
bool missing(const t::ParseResult&r){auto p=std::get_if<Property>(&r);return p&&p->missing();}
bool prop(const t::ParseResult&r,const Property&v){auto p=std::get_if<Property>(&r);return p&&*p==v;}
bool lookupProp(const t::TokenLookup&r,const Property&v){auto p=std::get_if<Property>(&r);return p&&*p==v;}
template<class F>bool typeError(F&&f){try{f();}catch(const tree::FactoryError&e){return e.name=="TypeError";}catch(...){return false;}return false;}
template<std::size_t N>void compare(const std::vector<bool>&flags,const std::array<bool,N>&expected){expect(flags.size()==N);if(flags.size()!=N)return;for(std::size_t i=0;i<N;++i)expect(flags[i]==expected[i])<<"Source observation "<<i;}
std::vector<std::string> keys(t::SubHandlers&h){std::vector<std::string>r;auto it=h.keys();while(auto v=it.next())r.push_back(*v);return r;}
Property nameAt(const std::vector<Property>&names,std::size_t i){return i<names.size()?names[i]:Property(Missing{});}
bool firstFunction(const std::vector<std::shared_ptr<FunctionListItem>>&items,const FunctionRef&f){if(items.empty()||!items[0])return false;auto p=std::get_if<FunctionRef>(&items[0]->item);return p&&*p==f;}
double firstPriority(const std::vector<std::shared_ptr<FunctionListItem>>&items){return items.empty()||!items[0]?std::numeric_limits<double>::quiet_NaN():items[0]->priority;}
// Source's instanceof SubHandler observations bind to the exact native class.
std::vector<std::shared_ptr<FunctionListItem>> items(const t::SubHandlerRef&h){
 // Protected member pointer legally observes the actual base object; it does
 // not replace handlers created by the production factory or use a downcast.
 return Handler::itemsOf(*h);
}
using Handlers=t::SubHandlers;
#include "map_handler_source.inc"
suite map_handlers=[] {
"math_port_map_handler_registry"_test=[] {
 auto a=map("p20-registry"),b=std::make_shared<Map>("p20-registry");std::vector<bool>f{t::MapHandler::getMap(a->key)==a};t::MapHandler::registerMap(b);f.insert(f.end(),{t::MapHandler::getMap(a->key)==b,!t::MapHandler::getMap("p20-absent")});
 PropertyList object;for(const Property key:std::vector<Property>{std::numeric_limits<double>::quiet_NaN(),0,PropertyNull{},Missing{},object,false}){auto m=map(key);f.push_back(t::MapHandler::getMap(key)==m);}
 auto m=map(-0.0);f.push_back(t::MapHandler::getMap(0)==m);int reads=0;auto getter=std::make_shared<Map>("p20-getter");getter->nameFn=[&]{++reads;return Property("p20-getter");};t::MapHandler::registerMap(getter);f.insert(f.end(),{reads==1,t::MapHandler::getMap("p20-getter")==getter,typeError([]{t::MapHandler::registerMap({});})});compare(f,source_registry);
};
"math_port_map_handler_priority_remove"_test=[] {
 Handler s;auto a=map("p20-a"),b=map("p20-b"),c=map("p20-c");auto callback=noOp();s.add(PropertyArray{a->key,b->key,c->key},callback,5);std::vector<bool>f{s.toString()=="p20-c, p20-b, p20-a",s.names()==std::vector<Property>{c->key,b->key,a->key},firstFunction(s.items(),callback),firstPriority(s.items())==5};s.add(PropertyArray{a->key},Property(PropertyNull{}),1);f.insert(f.end(),{s.retrieve(a->key)==a,nameAt(s.names(),0)==a->key});s.remove(PropertyArray{a->key});f.push_back(s.names()==std::vector<Property>{a->key,c->key,b->key});s.remove(PropertyArray{},callback);f.push_back(s.items().empty());compare(f,source_priority_remove);
};
"math_port_map_handler_missing_partial"_test=[] {
 std::vector<std::u16string>w;Handler s([&](const auto&v){w.push_back(v);});auto a=map("p20-part-a"),c=map("p20-part-c");s.add(PropertyArray{a->key,"p20-missing",c->key},noOp());compare({s.toString()=="p20-part-c",s.items().empty(),w.size()==1,w==std::vector<std::u16string>{u"TexParser Warning: Configuration 'p20-missing' not found! Omitted."}},source_missing_partial);
};
"math_port_map_handler_duplicates"_test=[] {
 Handler s;auto a=map("p20-dup");auto callback=noOp();s.add(PropertyArray{a->key,a->key},callback,3);s.add(PropertyArray{},callback,1);s.remove(PropertyArray{a->key},callback);compare({s.names().size()==1,nameAt(s.names(),0)==a->key,s.items().size()==1,firstPriority(s.items())==1},source_duplicates);
};
"math_port_map_handler_falsy_parse"_test=[] {
 std::vector<bool>f;int i=0;for(const Property value:std::vector<Property>{false,0,"",PropertyNull{},Missing{},std::numeric_limits<double>::quiet_NaN()}){
 Handler s;Parser p;std::vector<std::string>events;auto a=map("p20-falsy-"+std::to_string(i++));a->parseFn=[&](const auto&)->t::ParseResult{events.push_back("map");return value;};a->containsFn=[](const auto&)->bool{throw std::runtime_error("parse must not call contains");};std::shared_ptr<FunctionListItem>record;
 auto fallback=fn([&](const auto&r,auto&env,const auto&atom,auto)->t::ParseResult{events.push_back("fallback");f.insert(f.end(),{receiver(r)==record,&env==&p,std::get<Property>(atom)==Property("x")});return Property(PropertyList{{"ignored",true}});});s.add(PropertyArray{a->key},fallback);record=s.items()[0];const bool undefined=missing(s.parse({p,"x"}));f.insert(f.end(),{undefined,events==std::vector<std::string>{"map","fallback"}});
 }compare(f,source_falsy_parse);
};
"math_port_map_handler_fallback_break"_test=[] {
 Handler s;Parser p;std::vector<std::string>events;auto a=map("p20-break-a"),b=map("p20-break-b");a->parseFn=[&](const auto&){events.push_back("a");return t::SubHandler::FALLBACK;};b->parseFn=[&](const auto&)->t::ParseResult{events.push_back("b");return Property(true);};auto fallback=fn([&](const auto&,auto&,const auto&,auto)->t::ParseResult{events.push_back("fallback");return Property(7);});s.add(PropertyArray{b->key,a->key},fallback);const bool undefined=missing(s.parse({p,"x"}));compare({undefined,events==std::vector<std::string>{"a","fallback"}},source_fallback_break);
 auto other=std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"fallback"});Handler q;auto m=map("p20-other-symbol");m->parseFn=[&](const auto&)->t::ParseResult{return other;};q.add(PropertyArray{m->key},fn([](const auto&,auto&,const auto&,auto)->t::ParseResult{throw std::runtime_error("wrong fallback");}));compare({q.parse({p,"x"})==t::ParseResult{other}},source_other_symbol);
};
"math_port_map_handler_live_iteration"_test=[] {
 Handler s;Parser p;std::vector<std::string>events;auto a=map("p20-live-a"),b=map("p20-live-b"),c=map("p20-live-c");a->parseFn=[&](const auto&)->t::ParseResult{events.push_back("a");s.remove(PropertyArray{a->key});return Property(false);};b->parseFn=[&](const auto&)->t::ParseResult{events.push_back("b");return Property(true);};c->parseFn=[&](const auto&)->t::ParseResult{events.push_back("c");return Property(7);};s.add(PropertyArray{c->key,b->key,a->key},noOp());const bool seven=prop(s.parse({p,"x"}),7);compare({seven,events==std::vector<std::string>{"a","c"}},source_live_iteration);
};
"math_port_map_handler_truthy_errors"_test=[] {
 std::vector<bool>f;Parser p;std::vector<t::ParseResult>values{Property(true),Property(7),Property("x"),Property(PropertyList{}),std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"x"})};int i=0;for(const auto&value:values){Handler s;auto m=map("p20-truthy-"+std::to_string(i++));m->parseFn=[&](const auto&){return value;};s.add(PropertyArray{m->key},noOp());f.push_back(s.parse({p,"x"})==value);}compare(f,source_truthy_results);Handler s;compare({typeError([&]{s.parse({p,"x"});})},source_empty_fallback);Handler q;q.add(PropertyArray{},Property(true));compare({typeError([&]{q.parse({p,"x"});})},source_noncallable_fallback);
};
"math_port_map_handler_lookup"_test=[] {
 Handler s;std::vector<std::string>events;auto a=map("p20-look-a"),b=map("p20-look-b");a->containsFn=[&](const auto&){events.push_back("a:x");return false;};a->lookupFn=[](const auto&)->t::TokenLookup{throw std::runtime_error("wrong lookup");};b->containsFn=[&](const auto&){events.push_back("b:x");return true;};b->lookupFn=[&](const auto&)->t::TokenLookup{events.push_back("lookup:x");return Property(false);};s.add(PropertyArray{b->key,a->key},Property(PropertyNull{}));std::vector<bool>f{lookupProp(s.lookup("x"),false),events==std::vector<std::string>{"a:x","b:x","lookup:x"},s.applicable("x")==b,s.contains("x")};compare(f,source_lookup);Handler q;compare({lookupProp(q.lookup("x"),PropertyNull{}),!q.applicable("x"),!q.retrieve("x"),!q.contains("x"),q.toString().empty()},source_empty_queries);
};
"math_port_map_handler_character_null"_test=[] {
 Handler s;auto m=std::make_shared<CharacterMap>("p20-char");t::MapHandler::registerMap(m);auto nil=std::make_shared<t::Token>("nil",PropertyNull{}),undef=std::make_shared<t::Token>("missing",Missing{}),fals=std::make_shared<t::Token>("false",false),text=std::make_shared<t::Token>("text","y");m->lookupFn=[&](const Property&p)->t::TokenLookup{if(p==Property("nil"))return nil;if(p==Property("missing"))return undef;if(p==Property("false"))return fals;return text;};s.add(PropertyArray{m->key},Property(PropertyNull{}));compare({!s.contains("nil"),s.contains("missing"),s.contains("false"),s.contains("text"),s.applicable("nil")==m,s.lookup("nil")==t::TokenLookup{t::TokenRef(nil)}},source_character_null);m->lookupFn=[](const auto&)->t::TokenLookup{return Property(PropertyNull{});};compare({typeError([&]{s.contains("x");})},source_character_missing_lookup);
};
"math_port_map_handler_strict_name"_test=[] {
 Handler s;auto m=map(std::numeric_limits<double>::quiet_NaN());s.add(PropertyArray{m->key},Property(PropertyNull{}));compare({!s.retrieve(m->key),s.toString()=="NaN"},source_strict_name);
};
"math_port_map_handler_fallback_receiver"_test=[] {
 Handler s;Parser p;std::vector<std::string>events;std::vector<bool>r;FunctionRef first;auto later=fn([&](const auto&,auto&,const auto&,auto)->t::ParseResult{events.push_back("later");return Property(Missing{});});first=fn([&](const auto&receiverValue,auto&env,const auto&atom,auto)->t::ParseResult{events.push_back("first");auto actual=std::dynamic_pointer_cast<FunctionListItem>(receiver(receiverValue));r={actual&&std::get<FunctionRef>(actual->item)==first,actual&&actual->priority==1,&env==&p,std::get<Property>(atom)==Property("x")};s.remove(PropertyArray{},first);if(actual){actual->item=later;actual->priority=9;}return Property(false);});s.add(PropertyArray{},later,5);s.add(PropertyArray{},first,1);auto record=s.items()[0];bool undefined=missing(s.parse({p,"x"}));compare(r,source_fallback_receiver);compare({undefined,events==std::vector<std::string>{"first"},std::get<FunctionRef>(record->item)==later,record->priority==9,s.items().size()==1},source_fallback_result);
 PropertyList reason;auto throwing=fn([&](const auto&,auto&,const auto&,auto)->t::ParseResult{throw async::ThrownValue{Property(reason)};});s.add(PropertyArray{},throwing,0);bool same=false;try{s.parse({p,"x"});}catch(const async::ThrownValue&e){same=std::any_cast<Property>(e.value)==Property(reason);}compare({same},source_throw_identity);
};
"math_port_map_handler_subhandlers"_test=[] {
 Handlers all;auto a=map("p20-hand-a"),b=map("p20-hand-b");PropertyList inherited{{"macro",PropertyArray{b->key}}};auto h=PropertyList::withPrototype(inherited);h.setOwn("environment",PropertyArray{a->key});h.setOwn("character",PropertyArray{b->key});auto callback=noOp();MethodConfig parent;parent.setOwn("environment",callback);auto fallbacks=MethodConfig::withPrototype(parent);all.add(h,fallbacks,4);compare({bool(all.get("environment")),!all.get("macro"),all.get("character")->retrieve(b->key)==b,firstFunction(items(all.get("environment")),callback),all.retrieve(a->key)==a,keys(all)==std::vector<std::string>{"environment","character"}},source_subhandlers_add);
 all.remove(PropertyList{{"environment",PropertyArray{a->key}},{"absent",PropertyArray{}}},fallbacks);compare({!all.retrieve(a->key),bool(all.get("environment")),items(all.get("environment")).empty()},source_subhandlers_remove);auto replacement=std::make_shared<Handler>();all.set("environment",replacement);compare({all.get("environment")==replacement,keys(all)==std::vector<std::string>{"environment","character"},!all.retrieve("p20-absent")},source_subhandlers_set);
};
"math_port_map_handler_live_keys"_test=[] {
 t::SubHandlers all;all.add(PropertyList{{"10",PropertyArray{}},{"2",PropertyArray{}},{"macro",PropertyArray{}},{"character",PropertyArray{}}},MethodConfig{});auto it=all.keys();std::vector<bool>f{it.next()==std::optional<std::string>("2")};all.set("later",std::make_shared<Handler>());std::vector<std::string>rest;while(auto v=it.next())rest.push_back(*v);f.push_back(rest==std::vector<std::string>{"10","macro","character","later"});all.set("after-done",std::make_shared<Handler>());f.push_back(!it.next());compare(f,source_live_keys);
};
"math_port_map_handler_add_snapshot"_test=[] {
 PropertyArray array{"p20-array-change-missing","p20-array-change-last"};auto last=map(array.get(1));std::vector<std::u16string>logs;Handler s([&](const auto&v){logs.push_back(v);array.set(0,last->key);});s.add(array,Property(PropertyNull{}));compare({s.names().size()==1,nameAt(s.names(),0)==last->key,logs.size()==1},source_add_snapshot);
};
"math_port_map_handler_mutable_fallback"_test=[] {
 struct Restore {t::ParseResult saved=t::SubHandler::FALLBACK;~Restore(){t::SubHandler::FALLBACK=saved;}}restore;
 std::vector<bool>f;std::vector<t::ParseResult>values{Property(false),Property(PropertyNull{}),Property(Missing{}),Property(0),Property(""),Property(PropertyList{}),std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"changed"})};int i=0;for(const auto&value:values){t::SubHandler::FALLBACK=value;Handler s;Parser p;std::vector<std::string>events;auto a=map("p20-static-"+std::to_string(i)),b=map("p20-static-later-"+std::to_string(i++));a->parseFn=[&](const auto&){events.push_back("a");return value;};b->parseFn=[&](const auto&)->t::ParseResult{events.push_back("b");return Property(true);};s.add(PropertyArray{b->key,a->key},fn([&](const auto&,auto&,const auto&,auto)->t::ParseResult{events.push_back("f");return Property(7);}));const bool undefined=missing(s.parse({p,"x"}));f.insert(f.end(),{undefined,events==std::vector<std::string>{"a","f"}});}
 t::SubHandler::FALLBACK=Property(std::numeric_limits<double>::quiet_NaN());Handler s;Parser p;auto a=map("p20-static-nan"),b=map("p20-static-nan-b");a->parseFn=[](const auto&)->t::ParseResult{return Property(std::numeric_limits<double>::quiet_NaN());};b->parseFn=[](const auto&)->t::ParseResult{return Property(7);};s.add(PropertyArray{b->key,a->key},noOp());f.push_back(prop(s.parse({p,"x"}),7));compare(f,source_mutable_fallback);
};
"math_port_map_handler_nullish_arrays"_test=[] {
 Handler s;auto a=map(PropertyNull{}),b=map(Missing{}),c=map(false);s.add(PropertyArray{c->key,b->key,a->key},Property(PropertyNull{}));compare({s.toString()==", , false",s.retrieve(PropertyNull{})==a,s.retrieve(Missing{})==b},source_nullish_names);
 PropertyArray holes;holes.resize(2);auto inherited=PropertyList{{"0",Missing{}}};holes.record().set("__proto__",inherited);holes.set(1,false);Handler q;q.add(holes,Property(PropertyNull{}));compare({q.names().size()==2,nameAt(q.names(),0)==Property(false),nameAt(q.names(),1).missing()},source_array_holes);
};
"math_port_map_handler_fallback_shadow_retained_keys"_test=[] {
 auto callback=noOp();MethodConfig parent;parent.setOwn("macro",callback);auto child=MethodConfig::withPrototype(parent);child.setOwn("macro",Property(Missing{}));Handlers all;all.add(PropertyList{{"macro",PropertyArray{}}},child);std::vector<bool>f{items(all.get("macro")).empty()};child.erase("macro");all.add(PropertyList{{"macro",PropertyArray{}}},child);f.push_back(firstFunction(items(all.get("macro")),callback));compare(f,source_fallback_shadow);
 auto owner=std::make_unique<t::SubHandlers>();owner->set("retained",std::make_shared<Handler>());auto it=owner->keys();owner.reset();compare({it.next()==std::optional<std::string>("retained"),!it.next()},source_retained_keys);
};
"math_port_map_handler_warning_throw_live_values"_test=[] {
 PropertyList reason;Handler s([&](const auto&){throw async::ThrownValue{Property(reason)};});auto a=map("p20-throw-partial");bool same=false;try{s.add(PropertyArray{"p20-throw-missing",a->key},noOp());}catch(const async::ThrownValue&e){same=std::any_cast<Property>(e.value)==Property(reason);}compare({same,s.toString()=="p20-throw-partial",s.items().empty()},source_warning_throw);
 struct Live:Handler {std::function<t::TokenMapRef(const Property&)> custom;t::TokenMapRef retrieve(const Property&p)override{return custom?custom(p):Handler::retrieve(p);}};
 t::SubHandlers all;auto a2=map("p20-values-a"),b2=map("p20-values-b");auto one=std::make_shared<Live>();auto two=std::make_shared<Handler>();one->add(PropertyArray{a2->key});two->add(PropertyArray{b2->key});all.set("one",one);bool once=true;one->custom=[&](const auto&)->t::TokenMapRef{if(once){once=false;all.set("two",two);}return {};};compare({all.retrieve(b2->key)==b2},source_live_values);
};
"math_port_map_handler_native_boundaries"_test=[] {
 // Ownership/type-boundary witnesses extend Source probes without declaring a
 // native null pointer distinct from JS null, or accepting concrete token maps.
 Handler s;Parser p;expect(typeError([&]{s.add(Property(false));}));expect(typeError([&]{s.remove(Property("x"));}));
 struct Stack:t::StackItemContext{};auto value=std::make_shared<Stack>();auto m=map("p20-native-stack");m->parseFn=[&](const auto&)->t::ParseResult{return t::StackItemRef(value);};s.add(PropertyArray{m->key});expect(std::get<t::StackItemRef>(s.parse({p,"x"}))==value);
 struct Restore{t::ParseResult old=t::SubHandler::FALLBACK;~Restore(){t::SubHandler::FALLBACK=old;}}restore;t::SubHandler::FALLBACK=Property(PropertyNull{});
 for(const t::ParseResult&nil:std::vector<t::ParseResult>{t::StackItemRef{},t::ParseSymbolRef{}}){Handler q;int calls=0;auto a=map("p20-native-null"),b=map("p20-native-null-later");a->parseFn=[&](const auto&){return nil;};b->parseFn=[&](const auto&)->t::ParseResult{++calls;return Property(true);};q.add(PropertyArray{b->key,a->key},noOp());expect(missing(q.parse({p,"x"})));expect(calls==0);}
 // Property-shaped lookup values retain their own/inherited char slot.
 auto c=std::make_shared<CharacterMap>("p20-native-char");t::MapHandler::registerMap(c);Handler q;q.add(PropertyArray{c->key});PropertyList box{{"char",PropertyNull{}}};c->lookupFn=[&](const auto&)->t::TokenLookup{return Property(box);};expect(!q.contains("x"));box.setOwn("char",false);expect(q.contains("x"));
 t::SubHandlers all;all.set("null",{});expect(typeError([&]{all.retrieve("x");}));
};
"math_port_map_handler_remove_identity_live_array"_test=[] {
 t::SubHandlers all;all.add(PropertyList{{"macro",PropertyArray{}}},MethodConfig{});auto original=all.get("macro");all.remove(PropertyList{{"macro",PropertyArray{}}},MethodConfig{});compare({all.get("macro")==original},source_remove_identity);
 struct Live:Handler{std::function<t::TokenMapRef(const Property&)>custom;t::TokenMapRef retrieve(const Property&p)override{return custom(p);}}s;
 auto a=map("p20-remove-live-a"),b=map("p20-remove-live-b"),c=map("p20-remove-live-c");s.add(PropertyArray{c->key,b->key,a->key});PropertyArray array{a->key,b->key};std::vector<Property>events;s.custom=[&](const auto&name){events.push_back(name);if(events.size()==1){array.set(1,a->key);array.set(2,c->key);}return s.Handler::retrieve(name);};s.remove(array);compare({s.toString()=="p20-remove-live-b",events==std::vector<Property>{a->key,a->key,c->key}},source_remove_array_live);
};
"math_port_map_handler_default_priority_unicode_warning"_test=[] {
 struct Restore{double old=PrioritizedList<t::TokenMapRef>::DEFAULTPRIORITY;~Restore(){PrioritizedList<t::TokenMapRef>::DEFAULTPRIORITY=old;}}restore;
 Handler s;auto a=map("p20-default-priority");PrioritizedList<t::TokenMapRef>::DEFAULTPRIORITY=13;s.add(PropertyArray{a->key},noOp());compare({s.configurationPriority()==13,firstPriority(s.items())==13},source_dynamic_priority);
 std::ostringstream output;struct Capture{std::streambuf*old;explicit Capture(std::streambuf*p):old(std::cout.rdbuf(p)){}~Capture(){std::cout.rdbuf(old);}};
 std::u16string name=u"p20-警告-";name.push_back(char16_t(0xd800));{Capture capture(output.rdbuf());Handler q;q.add(PropertyArray{property_host::debugText(name)},Property(PropertyNull{}));}
 const auto text=output.str();compare({std::count(text.begin(),text.end(),'\n')==1,text==property_host::debugText(u"TexParser Warning: Configuration '"+name+u"' not found! Omitted.\n")},source_unicode_warning);
};
};
}
