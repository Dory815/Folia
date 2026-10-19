// Root native invocation stimuli, without map/handler algorithm implementations.
#include "support/folia_test.hpp"
import folia.math.input.tex.tex_parser_context;
import folia.math.input.tex.macro;
import folia.math.util.function_list;
import folia.math.core.tree.factory_types;
namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t=folia::math::tex;
struct Parser:t::ParserContext {Property cs="saved";Property currentCS()const override{return cs;}void setCurrentCS(Property v)override{cs=std::move(v);}};
struct MapReceiver:NativeObject {FunctionRef parser;};
struct Stack:t::StackItemContext {};
bool missingReceiver(const FunctionReceiver&r){return std::holds_alternative<Missing>(r);}
NativeObjectRef objectReceiver(const FunctionReceiver&r){if(auto p=std::get_if<NativeObjectRef>(&r))return *p;return {};}
template<std::size_t N>void compare(const std::vector<bool>&actual,const std::array<bool,N>&expected){expect(actual.size()==N);if(actual.size()!=N)return;for(std::size_t i=0;i<N;++i)expect(actual[i]==expected[i]);}
template<class F>bool typeError(F&&f){try{f();}catch(const tree::FactoryError&e){return e.name=="TypeError";}catch(...){return false;}return false;}
}
// Generated from actual pinned CJS; maps themselves are not accepted by these host tests.
namespace {const std::array<bool,4> source_character{true,true,true,true};}
namespace {const std::array<bool,6> source_macro{true,true,true,true,true,true};}
namespace {const std::array<bool,6> source_command{true,true,true,true,true,true};}
namespace {const std::array<bool,12> source_environment{true,true,true,true,true,true,true,true,true,true,true,true};}
namespace {const std::array<bool,10> source_fallback{true,true,true,true,true,true,true,true,true,true};}
namespace {const std::array<bool,6> source_typed_false{true,true,true,true,true,true};}
namespace {const std::array<bool,2> source_throw{true,true};}
namespace {const std::array<bool,4> source_closure_state{true,true,true,true};}
namespace {suite callable_receivers=[] {
"math_port_callable_bare"_test=[] {
 for(int kind=0;kind<3;++kind){Parser parser;PropertyList box{{"value",7}};PropertyArray arg{false};auto token=std::make_shared<t::Token>("x","y");std::vector<bool>flags;
 auto fn=t::makeParseCallable([&](const FunctionReceiver&r,t::ParserContext&p,const t::ParseAtom&atom,t::ParseRest rest)->t::ParseResult{
 bool sameAtom=kind==0?std::get<t::TokenRef>(atom)==token:std::get<Property>(atom)==Property(kind==2?"\\x":"x");
 flags.insert(flags.end(),{missingReceiver(r),&p==&parser,sameAtom,rest.size()==(kind==0?0u:2u)});
 if(kind!=0){flags.push_back(std::any_cast<Property>(rest[0])==Property(box));flags.push_back(std::any_cast<Property>(rest[1])==Property(arg));}return Property(false);});
 std::array<std::any,2>rest{Property(box),Property(arg)};t::invokeParseMethod(t::ParseMethod{fn},FunctionReceiver{Missing{}},parser,kind==0?t::ParseAtom{token}:t::ParseAtom{Property(kind==2?"\\x":"x")},kind==0?t::ParseRest{}:t::ParseRest{rest});
 if(kind==0)compare(flags,source_character);else if(kind==1)compare(flags,source_macro);else compare(flags,source_command);
 }
};
"math_port_callable_environment"_test=[] {
 Parser parser;auto map=std::make_shared<MapReceiver>();NativeObjectRef saved;auto envParser=t::makeParseCallable([](const auto&,auto&,const auto&,auto)->t::ParseResult{return Property(Missing{});});PropertyArray args{false},result{false};std::vector<bool>flags;
 std::array<std::any,2>rest{envParser,Property(args)};
 map->parser=t::makeParseCallable([&](const FunctionReceiver&r,t::ParserContext&p,const t::ParseAtom&atom,t::ParseRest a)->t::ParseResult{saved=objectReceiver(r);flags.insert(flags.end(),{saved==map,&p==&parser,std::get<Property>(atom)==Property("x"),std::any_cast<FunctionRef>(a[0])==envParser,std::any_cast<Property>(a[1])==Property(args)});return Property(result);});
 auto value=t::invokeParseMethod(t::ParseMethod{map->parser},FunctionReceiver{map},parser,t::ParseAtom{Property("x")},rest);flags.push_back(std::get<Property>(value)==Property(result));flags.push_back(saved==map);
 map->parser=t::makeParseCallable([&](const FunctionReceiver&r,t::ParserContext&p,const auto&,t::ParseRest a)->t::ParseResult{flags.insert(flags.end(),{objectReceiver(r)==saved,&p==&parser,std::any_cast<FunctionRef>(a[0])==envParser,std::any_cast<Property>(a[1])==Property(args)});return Property(false);});
 flags.push_back(std::get<Property>(t::invokeParseMethod(t::ParseMethod{map->parser},FunctionReceiver{map},parser,t::ParseAtom{Property("x")},rest))==Property(false));compare(flags,source_environment);
};
"math_port_callable_fallback"_test=[] {
 Parser parser;FunctionList list;NativeObjectRef saved;FunctionList::ItemRef record;std::vector<bool>flags;FunctionRef callback;
 auto other=t::makeParseCallable([](const auto&,auto&,const auto&,auto)->t::ParseResult{return Property(Missing{});});
 callback=t::makeParseCallable([&](const FunctionReceiver&r,t::ParserContext&p,const t::ParseAtom&atom,t::ParseRest)->t::ParseResult{saved=objectReceiver(r);auto actual=std::dynamic_pointer_cast<FunctionListItem>(saved);flags.insert(flags.end(),{actual==record.nativeRecord(),std::get<FunctionRef>(std::get<FunctionValue>(record.readItem()))==callback,record.readPriority()==13,&p==&parser,std::get<Property>(atom)==Property("x")});list.remove(callback);record.writeItem(FunctionValue{other});record.writePriority(81);return Property(false);});
 t::Macro macro("x",t::ParseMethod{callback});list.add(callback,13);record=list.begin().record();flags.push_back(std::get<FunctionRef>(macro.func())==std::get<FunctionRef>(std::get<FunctionValue>(record.readItem())));
 std::array<std::any,2>slots{std::ref(static_cast<t::ParserContext&>(parser)),t::ParseAtom{Property("x")}};functions::invoke(record.nativeRecord(),slots);
 flags.insert(flags.end(),{saved==std::static_pointer_cast<NativeObject>(record.nativeRecord()),std::get<FunctionRef>(std::get<FunctionValue>(record.readItem()))==other,record.readPriority()==81,list.begin()==list.end()});compare(flags,source_fallback);
};
"math_port_callable_typed_false"_test=[] {
 Parser parser;auto token=std::make_shared<t::Token>("x","y");PropertyList rest;std::vector<bool>flags;FunctionList list;FunctionList::ItemRef record;FunctionRef callback;
 callback=t::makeParseCallable([&](const FunctionReceiver&r,t::ParserContext&p,const t::ParseAtom&atom,t::ParseRest args)->t::ParseResult{auto actual=std::dynamic_pointer_cast<FunctionListItem>(objectReceiver(r));flags.insert(flags.end(),{actual==record.nativeRecord(),std::get<FunctionRef>(std::get<FunctionValue>(record.readItem()))==callback,&p==&parser,std::get<t::TokenRef>(atom)==token,std::any_cast<Property>(args[0])==Property(rest)});return Property(false);});
 auto second=t::makeParseCallable([&](const auto&,auto&,const auto&,auto)->t::ParseResult{flags.push_back(false);return Property(true);});list.add(callback,5);list.add(second,10);record=list.begin().record();
 std::array<std::any,3>slots{std::ref(static_cast<t::ParserContext&>(parser)),t::ParseAtom{token},Property(rest)};flags.push_back(list.execute(slots)==false);compare(flags,source_typed_false);
};
"math_port_callable_throw"_test=[] {
 Parser parser;PropertyList reason{{"value",7}};std::vector<bool>flags;auto fn=t::makeParseCallable([&](const auto&,auto&,const auto&,auto)->t::ParseResult{throw async::ThrownValue{Property(reason)};});
 try{t::invokeParseMethod(t::ParseMethod{fn},FunctionReceiver{Missing{}},parser,t::ParseAtom{Property("x")});flags.push_back(false);}catch(const async::ThrownValue&e){flags.push_back(std::any_cast<Property>(e.value)==Property(reason));}
 FunctionList list;list.add(fn);std::array<std::any,2>slots{std::ref(static_cast<t::ParserContext&>(parser)),t::ParseAtom{Property("x")}};try{list.execute(slots);flags.push_back(false);}catch(const async::ThrownValue&e){flags.push_back(std::any_cast<Property>(e.value)==Property(reason));}compare(flags,source_throw);
};
"math_port_callable_owned_receiver"_test=[] {
 NativeObjectRef saved;std::weak_ptr<FunctionListItem>weak;
 {FunctionList list;auto fn=std::make_shared<Function>();fn->call=[&](const FunctionReceiver&r,FunctionArgs)->std::any{saved=objectReceiver(r);return true;};list.add(fn);{auto record=list.begin().record();weak=record.nativeRecord();}list.execute({});list.remove(fn);expect(!weak.expired());}
 expect(!weak.expired());saved.reset();expect(weak.expired());
};
"math_port_callable_invalid_native_protocol"_test=[] {
 Parser parser;auto fn=t::makeParseCallable([](const auto&,auto&,const auto&,auto)->t::ParseResult{return Property(false);});FunctionReceiver bare{Missing{}},nullObject{NativeObjectRef{}};t::ParseAtom atom{Property("x")};std::array<std::any,2>valid{std::ref(static_cast<t::ParserContext&>(parser)),atom},badParser{Property("parser"),atom},badAtom{std::ref(static_cast<t::ParserContext&>(parser)),Property("x")};
 expect(typeError([&]{t::makeParseCallable({});}));expect(typeError([&]{fn->call(nullObject,valid);}));expect(typeError([&]{fn->call(bare,{});}));expect(typeError([&]{fn->call(bare,badParser);}));expect(typeError([&]{fn->call(bare,badAtom);}));
 expect(typeError([&]{t::invokeParseMethod(t::ParseMethod{Property(false)},bare,parser,atom);}));expect(typeError([&]{t::invokeParseMethod(t::ParseMethod{FunctionRef{}},bare,parser,atom);}));expect(typeError([&]{t::invokeParseMethod(t::ParseMethod{fn},nullObject,parser,atom);}));
 auto generic=std::make_shared<Function>();expect(typeError([&]{t::invokeParseMethod(t::ParseMethod{generic},bare,parser,atom);}));generic->call=[](const auto&,auto)->std::any{return {};};expect(std::get<Property>(t::invokeParseMethod(t::ParseMethod{generic},bare,parser,atom)).missing());
 generic->call=[](const auto&,auto)->std::any{return false;};expect(typeError([&]{t::invokeParseMethod(t::ParseMethod{generic},bare,parser,atom);}));generic->call=[](const auto&,auto)->std::any{return t::ParseResult{Property(false)};};expect(typeError([&]{t::invokeParseMethod(t::ParseMethod{generic},bare,parser,atom);}));
 expect(typeError([&]{functions::invoke({},{});}));auto record=std::make_shared<FunctionListItem>(5,FunctionValue{Property(false)});expect(typeError([&]{functions::invoke(record,{});}));
};
"math_port_callable_results_and_snapshot"_test=[] {
 Parser parser;auto stack=std::make_shared<Stack>();auto symbol=std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"fallback"});PropertyList result{{"value",7}};std::size_t index=0;
 auto fn=t::makeParseCallable([&](const auto&,auto&,const auto&,auto)->t::ParseResult{switch(index++){case 0:return Property(result);case 1:return t::StackItemRef(stack);default:return t::ParseSymbolRef(symbol);}});auto invoke=[&]{return t::invokeParseMethod(t::ParseMethod{fn},FunctionReceiver{Missing{}},parser,t::ParseAtom{Property("x")});};
 expect(std::get<Property>(invoke())==Property(result));expect(std::get<t::StackItemRef>(invoke())==stack);expect(std::get<t::ParseSymbolRef>(invoke())==symbol);
 FunctionRef self;self=t::makeParseCallable([&](const auto&,auto&,const auto&,auto)->t::ParseResult{self->call={};return Property(result);});expect(std::get<Property>(t::invokeParseMethod(t::ParseMethod{self},FunctionReceiver{Missing{}},parser,t::ParseAtom{Property("x")}))==Property(result));
};
"math_port_callable_closure_state"_test=[] {
 Parser parser;auto fn=t::makeParseCallable([count=0](const auto&,auto&,const auto&,auto)mutable->t::ParseResult{return Property(++count);});
 auto invoke=[&]{return std::get<Property>(t::invokeParseMethod(t::ParseMethod{fn},FunctionReceiver{Missing{}},parser,t::ParseAtom{Property("x")}));};std::vector<bool>flags;flags.push_back(invoke()==Property(1));flags.push_back(invoke()==Property(2));
 auto generic=std::make_shared<Function>();generic->call=[count=0](const auto&,auto)mutable->std::any{return Property(++count);};auto record=std::make_shared<FunctionListItem>(5,FunctionValue{generic});flags.push_back(std::any_cast<Property>(functions::invoke(record,{}))==Property(1));flags.push_back(std::any_cast<Property>(functions::invoke(record,{}))==Property(2));compare(flags,source_closure_state);
};
};}
