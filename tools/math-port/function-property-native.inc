// Root native stimuli for shared data bindings, not concrete TokenMap algorithms.
#include "support/folia_test.hpp"
import folia.math.util.function_types;
import folia.math.util.function_list;
import folia.math.util.property_host;
import folia.math.input.tex.macro;
namespace {
using namespace boost::ut;
using namespace folia::math;
struct Parser:tex::ParserContext{Property cs="saved";Property currentCS()const override{return cs;}void setCurrentCS(Property v)override{cs=std::move(v);}};
struct SliceObservation{std::size_t length,mask,first,size;std::vector<bool>has;std::vector<int>values;bool fresh;};
// SOURCE_OBSERVATIONS
suite function_property=[] {
"math_port_callable_property_slices"_test=[] {
 auto f=std::make_shared<Function>(),g=std::make_shared<Function>();f->call=[](const auto&,auto)->std::any{return Property(Missing{});};g->call=f->call;
 for(const auto&c:source_slices){PropertyArray a;a.resize(c.length);PropertyList proto;for(std::size_t i=0;i<c.length;++i){auto mode=(c.mask+i)%5;if(mode==0)a.set(i,Property(f));else if(mode==1)a.set(i,Property(g));else if(mode==2)a.set(i,Missing{});else if(mode==3)proto.setOwn(std::to_string(i),i%2?Property(f):Property(Missing{}));}a.record().set("__proto__",proto);
  auto b=a.slice(c.first);expect(b.size()==c.size);expect((Property(b)!=Property(a))==c.fresh);if(b.size()!=c.size)continue;
  for(std::size_t i=0;i<b.size();++i){expect(b.has(i)==c.has[i]);auto v=b.get(i);int id=v==Property(f)?1:v==Property(g)?2:0;expect(id==c.values[i]);}
 }
};
"math_port_callable_property_graph_and_calls"_test=[] {
 int count=0;auto f=std::make_shared<Function>(),g=std::make_shared<Function>();f->call=[&](const auto&,auto)->std::any{return Property(++count);};g->call=[&](const auto&,auto)->std::any{return Property(++count+10);};
 PropertyList graph{{"9",Property(f)},{"x",Property(f)},{"2",Property(g)}};std::vector<std::pair<std::string,Property>>snapshot;for(const auto&key:property_host::ownKeys(graph))snapshot.emplace_back(key,property_host::get(graph,key));graph.set("x",Property(g));
 PropertyArray tuple{Property(f),Missing{}};auto args=tuple.slice(1);tex::Macro macro("x",functions::fromProperty(tuple.get(0)),Property(args));FunctionList list;list.add(macro.func());bool first=list.execute();std::vector<bool>flags;
 std::string keys;for(const auto&[k,v]:snapshot){if(!keys.empty())keys+=',';keys+=k;}flags={keys=="2,9,x",snapshot[2].second==Property(f),graph.get("x")==Property(g),functions::reference(macro.func())==f,PropertyArray::array(macro.args())->get(0).missing(),tuple.get(0)==Property(f),property_host::truthy(Property(f)),!PropertyArray::array(Property(f)).has_value(),first==true,count==1};
 list.remove(functions::fromProperty(graph.get("9")));flags.push_back(list.begin()==list.end());list.add(functions::fromProperty(graph.get("x")));list.execute();flags.push_back(count==2);Parser parser;const auto callback=functions::reference(macro.func());auto result=callback->call(FunctionReceiver{Missing{}},{});flags.push_back(std::any_cast<Property>(result)==Property(3));NativeObjectRef saved;
 auto fn=std::make_shared<Function>();fn->call=[&](const FunctionReceiver&r,auto)->std::any{saved=std::get<NativeObjectRef>(r);return Property(false);};FunctionList l;l.add(fn,13);auto record=l.begin().record();flags.push_back(l.execute()==false);flags.push_back(saved==record);flags.push_back(functions::reference(record->item)==fn);expect(flags.size()==source_flags.size());if(flags.size()==source_flags.size())for(std::size_t i=0;i<flags.size();++i)expect(flags[i]==source_flags[i]);
 // Typed/raw binding roundtrip and actual TeX invocation retain one owner/body.
 auto method=tex::makeParseCallable([&](const auto&,auto&,const auto&,auto)->tex::ParseResult{return Property(++count);});Property raw(method);expect(functions::reference(functions::fromProperty(raw))==method);expect(functions::toProperty(FunctionValue{method})==raw);auto parsed=tex::invokeParseMethod(FunctionValue{raw},FunctionReceiver{Missing{}},parser,tex::ParseAtom{Property("x")});expect(std::get<Property>(parsed)==Property(4));
};
"math_port_callable_property_strict_identity"_test=[] {
 auto f=std::make_shared<Function>(),g=std::make_shared<Function>();f->call=[](const auto&,auto)->std::any{return {};};g->call=f->call;PropertyList obj;PropertyArray arr;
 std::vector<Property>values{Property(f),Property(g),Missing{},PropertyNull{},false,true,0,-0.0,std::numeric_limits<double>::quiet_NaN(),"","x",Property(obj),Property(arr)};
 std::size_t index=0;for(const auto&a:values)for(const auto&b:values){expect((functions::fromProperty(a)==functions::fromProperty(b))==source_equal[index]);expect((FunctionValue{a}==functions::fromProperty(b))==source_equal[index]);expect((functions::fromProperty(a)==FunctionValue{b})==source_equal[index]);expect((FunctionValue{a}==FunctionValue{b})==source_equal[index]);++index;}
 FunctionList list;list.add(FunctionValue{Property(f)});list.remove(FunctionValue{f});expect(list.begin()==list.end());list.add(f);list.remove(FunctionValue{Property(f)});expect(list.begin()==list.end());
};
"math_port_callable_property_retains_same_closure"_test=[] {
 std::weak_ptr<Function>weak;Property retained;{
  auto f=std::make_shared<Function>();f->call=[n=0](const auto&,auto)mutable->std::any{return Property(++n);};weak=f;PropertyArray a{Property(f)};retained=a.slice(0).get(0);auto method=functions::fromProperty(retained);auto body=functions::reference(method)->call;expect(std::any_cast<Property>(body(FunctionReceiver{Missing{}},{}))==Property(1));expect(std::any_cast<Property>(f->call(FunctionReceiver{Missing{}},{}))==Property(2));
 }expect(!weak.expired());retained=Missing{};expect(weak.expired());
};
"math_port_callable_property_invalid_native_boundary"_test=[] {
 auto error=[](auto call){try{call();}catch(const options::Exception&e){return e.name=="TypeError";}return false;};expect(error([]{Property p(FunctionRef{});}));expect(error([]{functions::toProperty(FunctionValue{FunctionRef{}});}));auto f=std::make_shared<Function>();expect(error([&]{property_host::get(Property(f),"name");}));expect(!functions::reference(FunctionValue{Property(3)}));expect(functions::fromProperty(Property(3))==FunctionValue{Property(3)});expect(functions::toProperty(FunctionValue{Property(3)})==Property(3));
};
};}
