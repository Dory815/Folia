// Root stimuli for complete TokenMap. Expected observations are actual CJS.
#include "support/folia_test.hpp"
import folia.math.input.tex.token_map;
import folia.math.input.tex.map_handler;
import folia.math.util.property_host;
import folia.math.core.tree.factory_types;
namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t=folia::math::tex;
struct Parser:t::ParserContext{Property cs="saved";std::vector<std::string>*events=nullptr;Property currentCS()const override{if(events)events->push_back("get-cs");return cs;}void setCurrentCS(Property v)override{if(events)events->push_back("set-cs:"+property_host::debugText(property_host::string(v)));cs=std::move(v);}};
std::string hexText(const Property&p){std::string out;constexpr char digits[]="0123456789abcdef";for(auto v:property_host::text(std::get<std::string>(p.value)))for(int shift=12;shift>=0;shift-=4)out.push_back(digits[(v>>shift)&15]);return out;}
template<class F>bool typeError(F&&call){try{call();}catch(const tree::FactoryError&e){return e.name=="TypeError";}return false;}
template<std::size_t N>void compare(const std::vector<bool>&actual,const std::array<bool,N>&source){expect(actual.size()==N);if(actual.size()==N)for(std::size_t i=0;i<N;++i)expect(actual[i]==source[i])<<"Source flag "<<i;}
struct Data{
 PropertyList object;PropertyArray array;FunctionRef function=std::make_shared<Function>();t::ParseSymbolRef symbol=std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"result"});t::TokenRef token; t::MacroRef macro;
 Data(){function->call=[](const auto&,auto)->std::any{return Property(Missing{});};token=std::make_shared<t::Token>("token","T");macro=std::make_shared<t::Macro>("macro",FunctionValue{function},Property(PropertyArray{17}));}
 std::vector<t::ParseResult> values()const{return{Property(Missing{}),Property(PropertyNull{}),Property(false),Property(true),Property(0),Property(-0.0),Property(std::numeric_limits<double>::quiet_NaN()),Property(7),Property(""),Property("x"),Property(object),Property(array),Property(function),symbol};}
 std::vector<t::TokenLookup> mapped()const{return{Property(Missing{}),Property(PropertyNull{}),Property(false),Property(0),Property(""),Property(std::numeric_limits<double>::quiet_NaN()),Property(7),Property("x"),Property(object),Property(array),token,macro,Property(function)};}
 std::string label(const Property&p)const{if(p.missing())return "undefined";if(std::holds_alternative<PropertyNull>(p.value))return "null";if(auto b=std::get_if<bool>(&p.value))return *b?"true":"false";if(auto n=std::get_if<double>(&p.value)){if(std::isnan(*n))return "NaN";if(*n==0&&std::signbit(*n))return "-0";return "num:"+property_host::debugText(property_host::string(p));}if(std::holds_alternative<std::string>(p.value))return "str:"+hexText(p);if(p==Property(object))return "object";if(p==Property(array))return "array";if(p==Property(function))return "function";return "unknown";}
 std::string label(const t::ParseResult&r)const{if(auto p=std::get_if<Property>(&r))return label(*p);if(auto p=std::get_if<t::ParseSymbolRef>(&r);p&&*p==symbol)return "symbol";return "unknown";}
 std::string label(const t::ParseAtom&a)const{if(auto p=std::get_if<Property>(&a))return label(*p);if(auto p=std::get_if<t::TokenRef>(&a);p&&*p==token)return "token";if(auto p=std::get_if<NativeObjectRef>(&a);p&&*p==macro)return "macro";return "unknown";}
};
struct BaseObservation{int method,map;bool contains;int result;std::string value,error;std::vector<std::string>events;};
struct RegexObservation{std::array<bool,2>sequence;std::string value;std::vector<std::string>events;int calls;};
// Actual Source observations only.
const std::array<bool,14> source_parse_result{true,true,true,true,true,true,true,true,true,true,true,true,true,true};
const std::array<bool,19> source_registry_constructor{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true};
const std::array<bool,60> source_ordinary_map{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true};
const std::array<bool,48> source_character_data{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true};
const std::array<bool,6> source_constructor_enumeration{true,true,true,true,true,true};
const std::array<bool,22> source_macro_data{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true};
const std::array<bool,10> source_macro_dispatch{true,true,true,true,true,true,true,true,true,true};
const std::array<bool,6> source_delimiter_dispatch{true,true,true,true,true,true};
const std::array<bool,12> source_command_dispatch{true,true,true,true,true,true,true,true,true,true,true,true};
const std::array<bool,13> source_environment_dispatch{true,false,true,true,true,true,true,true,true,true,true,true,true};
const std::array<bool,8> source_regexp_live_service_dependency{true,true,true,true,true,true,true,true};
const std::array<bool,2> source_new_lookup_bridge{true,true};
const std::array<BaseObservation,208> source_base{{
BaseObservation{0,0,false,0,"null","",{"contains","lookup"}},
BaseObservation{0,0,true,0,"null","",{"contains","parser","lookup"}},
BaseObservation{0,1,false,1,"null","",{"contains","lookup"}},
BaseObservation{0,1,true,1,"null","",{"contains","parser","lookup"}},
BaseObservation{0,2,false,2,"null","",{"contains","lookup"}},
BaseObservation{0,2,true,2,"null","",{"contains","parser","lookup"}},
BaseObservation{0,3,false,3,"null","",{"contains","lookup"}},
BaseObservation{0,3,true,3,"null","",{"contains","parser","lookup"}},
BaseObservation{0,4,false,4,"null","",{"contains","lookup"}},
BaseObservation{0,4,true,4,"null","",{"contains","parser","lookup"}},
BaseObservation{0,5,false,5,"null","",{"contains","lookup"}},
BaseObservation{0,5,true,5,"null","",{"contains","parser","lookup"}},
BaseObservation{0,6,false,6,"null","",{"contains","lookup"}},
BaseObservation{0,6,true,6,"null","",{"contains","parser","lookup"}},
BaseObservation{0,7,false,7,"null","",{"contains","lookup"}},
BaseObservation{0,7,true,7,"null","",{"contains","parser","lookup"}},
BaseObservation{0,8,false,8,"null","",{"contains","lookup"}},
BaseObservation{0,8,true,8,"null","",{"contains","parser","lookup"}},
BaseObservation{0,9,false,9,"null","",{"contains","lookup"}},
BaseObservation{0,9,true,9,"null","",{"contains","parser","lookup"}},
BaseObservation{0,10,false,10,"null","",{"contains","lookup"}},
BaseObservation{0,10,true,10,"null","",{"contains","parser","lookup"}},
BaseObservation{0,11,false,11,"null","",{"contains","lookup"}},
BaseObservation{0,11,true,11,"null","",{"contains","parser","lookup"}},
BaseObservation{0,12,false,12,"null","",{"contains","lookup"}},
BaseObservation{0,12,true,12,"null","",{"contains","parser","lookup"}},
BaseObservation{1,0,false,13,"null","",{"contains","lookup"}},
BaseObservation{1,0,true,13,"null","",{"contains","parser","lookup"}},
BaseObservation{1,1,false,0,"null","",{"contains","lookup"}},
BaseObservation{1,1,true,0,"null","",{"contains","parser","lookup"}},
BaseObservation{1,2,false,1,"null","",{"contains","lookup"}},
BaseObservation{1,2,true,1,"null","",{"contains","parser","lookup"}},
BaseObservation{1,3,false,2,"null","",{"contains","lookup"}},
BaseObservation{1,3,true,2,"null","",{"contains","parser","lookup"}},
BaseObservation{1,4,false,3,"null","",{"contains","lookup"}},
BaseObservation{1,4,true,3,"null","",{"contains","parser","lookup"}},
BaseObservation{1,5,false,4,"null","",{"contains","lookup"}},
BaseObservation{1,5,true,4,"null","",{"contains","parser","lookup"}},
BaseObservation{1,6,false,5,"null","",{"contains","lookup"}},
BaseObservation{1,6,true,5,"null","",{"contains","parser","lookup"}},
BaseObservation{1,7,false,6,"null","",{"contains","lookup"}},
BaseObservation{1,7,true,6,"null","",{"contains","parser","lookup"}},
BaseObservation{1,8,false,7,"null","",{"contains","lookup"}},
BaseObservation{1,8,true,7,"null","",{"contains","parser","lookup"}},
BaseObservation{1,9,false,8,"null","",{"contains","lookup"}},
BaseObservation{1,9,true,8,"null","",{"contains","parser","lookup"}},
BaseObservation{1,10,false,9,"null","",{"contains","lookup"}},
BaseObservation{1,10,true,9,"null","",{"contains","parser","lookup"}},
BaseObservation{1,11,false,10,"null","",{"contains","lookup"}},
BaseObservation{1,11,true,10,"null","",{"contains","parser","lookup"}},
BaseObservation{1,12,false,11,"null","",{"contains","lookup"}},
BaseObservation{1,12,true,11,"null","",{"contains","parser","lookup"}},
BaseObservation{2,0,false,12,"null","",{"contains","lookup"}},
BaseObservation{2,0,true,12,"null","",{"contains","parser","lookup"}},
BaseObservation{2,1,false,13,"null","",{"contains","lookup"}},
BaseObservation{2,1,true,13,"null","",{"contains","parser","lookup"}},
BaseObservation{2,2,false,0,"null","",{"contains","lookup"}},
BaseObservation{2,2,true,0,"null","",{"contains","parser","lookup"}},
BaseObservation{2,3,false,1,"null","",{"contains","lookup"}},
BaseObservation{2,3,true,1,"null","",{"contains","parser","lookup"}},
BaseObservation{2,4,false,2,"null","",{"contains","lookup"}},
BaseObservation{2,4,true,2,"null","",{"contains","parser","lookup"}},
BaseObservation{2,5,false,3,"null","",{"contains","lookup"}},
BaseObservation{2,5,true,3,"null","",{"contains","parser","lookup"}},
BaseObservation{2,6,false,4,"null","",{"contains","lookup"}},
BaseObservation{2,6,true,4,"null","",{"contains","parser","lookup"}},
BaseObservation{2,7,false,5,"null","",{"contains","lookup"}},
BaseObservation{2,7,true,5,"null","",{"contains","parser","lookup"}},
BaseObservation{2,8,false,6,"null","",{"contains","lookup"}},
BaseObservation{2,8,true,6,"null","",{"contains","parser","lookup"}},
BaseObservation{2,9,false,7,"null","",{"contains","lookup"}},
BaseObservation{2,9,true,7,"null","",{"contains","parser","lookup"}},
BaseObservation{2,10,false,8,"null","",{"contains","lookup"}},
BaseObservation{2,10,true,8,"null","",{"contains","parser","lookup"}},
BaseObservation{2,11,false,9,"null","",{"contains","lookup"}},
BaseObservation{2,11,true,9,"null","",{"contains","parser","lookup"}},
BaseObservation{2,12,false,10,"null","",{"contains","lookup"}},
BaseObservation{2,12,true,10,"null","",{"contains","parser","lookup"}},
BaseObservation{3,0,false,11,"null","",{"contains","lookup"}},
BaseObservation{3,0,true,11,"null","",{"contains","parser","lookup"}},
BaseObservation{3,1,false,12,"null","",{"contains","lookup"}},
BaseObservation{3,1,true,12,"null","",{"contains","parser","lookup"}},
BaseObservation{3,2,false,13,"null","",{"contains","lookup"}},
BaseObservation{3,2,true,13,"null","",{"contains","parser","lookup"}},
BaseObservation{3,3,false,0,"null","",{"contains","lookup"}},
BaseObservation{3,3,true,0,"null","",{"contains","parser","lookup"}},
BaseObservation{3,4,false,1,"null","",{"contains","lookup"}},
BaseObservation{3,4,true,1,"null","",{"contains","parser","lookup"}},
BaseObservation{3,5,false,2,"null","",{"contains","lookup"}},
BaseObservation{3,5,true,2,"null","",{"contains","parser","lookup"}},
BaseObservation{3,6,false,3,"null","",{"contains","lookup"}},
BaseObservation{3,6,true,3,"null","",{"contains","parser","lookup"}},
BaseObservation{3,7,false,4,"null","",{"contains","lookup"}},
BaseObservation{3,7,true,4,"null","",{"contains","parser","lookup"}},
BaseObservation{3,8,false,5,"null","",{"contains","lookup"}},
BaseObservation{3,8,true,5,"null","",{"contains","parser","lookup"}},
BaseObservation{3,9,false,6,"null","",{"contains","lookup"}},
BaseObservation{3,9,true,6,"null","",{"contains","parser","lookup"}},
BaseObservation{3,10,false,7,"null","",{"contains","lookup"}},
BaseObservation{3,10,true,7,"null","",{"contains","parser","lookup"}},
BaseObservation{3,11,false,8,"null","",{"contains","lookup"}},
BaseObservation{3,11,true,8,"null","",{"contains","parser","lookup"}},
BaseObservation{3,12,false,9,"null","",{"contains","lookup"}},
BaseObservation{3,12,true,9,"null","",{"contains","parser","lookup"}},
BaseObservation{4,0,false,10,"null","",{"contains","lookup"}},
BaseObservation{4,0,true,10,"null","",{"contains","parser","lookup"}},
BaseObservation{4,1,false,11,"null","",{"contains","lookup"}},
BaseObservation{4,1,true,11,"null","",{"contains","parser","lookup"}},
BaseObservation{4,2,false,12,"null","",{"contains","lookup"}},
BaseObservation{4,2,true,12,"null","",{"contains","parser","lookup"}},
BaseObservation{4,3,false,13,"null","",{"contains","lookup"}},
BaseObservation{4,3,true,13,"null","",{"contains","parser","lookup"}},
BaseObservation{4,4,false,0,"null","",{"contains","lookup"}},
BaseObservation{4,4,true,0,"null","",{"contains","parser","lookup"}},
BaseObservation{4,5,false,1,"null","",{"contains","lookup"}},
BaseObservation{4,5,true,1,"null","",{"contains","parser","lookup"}},
BaseObservation{4,6,false,2,"null","",{"contains","lookup"}},
BaseObservation{4,6,true,2,"null","",{"contains","parser","lookup"}},
BaseObservation{4,7,false,3,"null","",{"contains","lookup"}},
BaseObservation{4,7,true,3,"null","",{"contains","parser","lookup"}},
BaseObservation{4,8,false,4,"null","",{"contains","lookup"}},
BaseObservation{4,8,true,4,"null","",{"contains","parser","lookup"}},
BaseObservation{4,9,false,5,"null","",{"contains","lookup"}},
BaseObservation{4,9,true,5,"null","",{"contains","parser","lookup"}},
BaseObservation{4,10,false,6,"null","",{"contains","lookup"}},
BaseObservation{4,10,true,6,"null","",{"contains","parser","lookup"}},
BaseObservation{4,11,false,7,"null","",{"contains","lookup"}},
BaseObservation{4,11,true,7,"null","",{"contains","parser","lookup"}},
BaseObservation{4,12,false,8,"null","",{"contains","lookup"}},
BaseObservation{4,12,true,8,"null","",{"contains","parser","lookup"}},
BaseObservation{5,0,false,9,"null","",{"contains","lookup"}},
BaseObservation{5,0,true,9,"null","",{"contains","parser","lookup"}},
BaseObservation{5,1,false,10,"null","",{"contains","lookup"}},
BaseObservation{5,1,true,10,"null","",{"contains","parser","lookup"}},
BaseObservation{5,2,false,11,"null","",{"contains","lookup"}},
BaseObservation{5,2,true,11,"null","",{"contains","parser","lookup"}},
BaseObservation{5,3,false,12,"null","",{"contains","lookup"}},
BaseObservation{5,3,true,12,"null","",{"contains","parser","lookup"}},
BaseObservation{5,4,false,13,"null","",{"contains","lookup"}},
BaseObservation{5,4,true,13,"null","",{"contains","parser","lookup"}},
BaseObservation{5,5,false,0,"null","",{"contains","lookup"}},
BaseObservation{5,5,true,0,"null","",{"contains","parser","lookup"}},
BaseObservation{5,6,false,1,"null","",{"contains","lookup"}},
BaseObservation{5,6,true,1,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{5,7,false,2,"null","",{"contains","lookup"}},
BaseObservation{5,7,true,2,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{5,8,false,3,"null","",{"contains","lookup"}},
BaseObservation{5,8,true,3,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{5,9,false,4,"null","",{"contains","lookup"}},
BaseObservation{5,9,true,4,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{5,10,false,5,"null","",{"contains","lookup"}},
BaseObservation{5,10,true,5,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{5,11,false,6,"null","",{"contains","lookup"}},
BaseObservation{5,11,true,6,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{5,12,false,7,"null","",{"contains","lookup"}},
BaseObservation{5,12,true,7,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{6,0,false,8,"null","",{"contains","lookup"}},
BaseObservation{6,0,true,8,"null","",{"contains","parser","lookup"}},
BaseObservation{6,1,false,9,"null","",{"contains","lookup"}},
BaseObservation{6,1,true,9,"null","",{"contains","parser","lookup"}},
BaseObservation{6,2,false,10,"null","",{"contains","lookup"}},
BaseObservation{6,2,true,10,"null","",{"contains","parser","lookup"}},
BaseObservation{6,3,false,11,"null","",{"contains","lookup"}},
BaseObservation{6,3,true,11,"null","",{"contains","parser","lookup"}},
BaseObservation{6,4,false,12,"null","",{"contains","lookup"}},
BaseObservation{6,4,true,12,"null","",{"contains","parser","lookup"}},
BaseObservation{6,5,false,13,"null","",{"contains","lookup"}},
BaseObservation{6,5,true,13,"null","",{"contains","parser","lookup"}},
BaseObservation{6,6,false,0,"null","",{"contains","lookup"}},
BaseObservation{6,6,true,0,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{6,7,false,1,"null","",{"contains","lookup"}},
BaseObservation{6,7,true,1,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{6,8,false,2,"null","",{"contains","lookup"}},
BaseObservation{6,8,true,2,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{6,9,false,3,"null","",{"contains","lookup"}},
BaseObservation{6,9,true,3,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{6,10,false,4,"null","",{"contains","lookup"}},
BaseObservation{6,10,true,4,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{6,11,false,5,"null","",{"contains","lookup"}},
BaseObservation{6,11,true,5,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{6,12,false,6,"null","",{"contains","lookup"}},
BaseObservation{6,12,true,6,"","TypeError",{"contains","parser","lookup"}},
BaseObservation{7,0,false,7,"null","",{"contains","lookup"}},
BaseObservation{7,0,true,7,"null","",{"contains","parser","lookup"}},
BaseObservation{7,1,false,8,"null","",{"contains","lookup"}},
BaseObservation{7,1,true,8,"null","",{"contains","parser","lookup"}},
BaseObservation{7,2,false,9,"null","",{"contains","lookup"}},
BaseObservation{7,2,true,9,"null","",{"contains","parser","lookup"}},
BaseObservation{7,3,false,10,"null","",{"contains","lookup"}},
BaseObservation{7,3,true,10,"null","",{"contains","parser","lookup"}},
BaseObservation{7,4,false,11,"null","",{"contains","lookup"}},
BaseObservation{7,4,true,11,"null","",{"contains","parser","lookup"}},
BaseObservation{7,5,false,12,"null","",{"contains","lookup"}},
BaseObservation{7,5,true,12,"null","",{"contains","parser","lookup"}},
BaseObservation{7,6,false,13,"null","",{"contains","lookup"}},
BaseObservation{7,6,true,13,"symbol","",{"contains","parser","lookup","body:num:7","bare"}},
BaseObservation{7,7,false,0,"null","",{"contains","lookup"}},
BaseObservation{7,7,true,0,"true","",{"contains","parser","lookup","body:str:0078","bare"}},
BaseObservation{7,8,false,1,"null","",{"contains","lookup"}},
BaseObservation{7,8,true,1,"null","",{"contains","parser","lookup","body:object","bare"}},
BaseObservation{7,9,false,2,"null","",{"contains","lookup"}},
BaseObservation{7,9,true,2,"false","",{"contains","parser","lookup","body:array","bare"}},
BaseObservation{7,10,false,3,"null","",{"contains","lookup"}},
BaseObservation{7,10,true,3,"true","",{"contains","parser","lookup","body:token","bare"}},
BaseObservation{7,11,false,4,"null","",{"contains","lookup"}},
BaseObservation{7,11,true,4,"num:0","",{"contains","parser","lookup","body:macro","bare"}},
BaseObservation{7,12,false,5,"null","",{"contains","lookup"}},
BaseObservation{7,12,true,5,"-0","",{"contains","parser","lookup","body:function","bare"}}
}};
const std::array<RegexObservation,4> source_regex{{
RegexObservation{{true,true},"true",{"test:str:0078","regex-receiver","test:str:0078","regex-receiver","body:str:0078","bare"},2},
RegexObservation{{true,false},"null",{"test:str:0078","regex-receiver","test:str:0078","regex-receiver"},2},
RegexObservation{{false,true},"null",{"test:str:0078","regex-receiver","test:str:0078","regex-receiver"},2},
RegexObservation{{false,false},"null",{"test:str:0078","regex-receiver","test:str:0078","regex-receiver"},2}
}};
struct BaseMap:t::AbstractTokenMap{explicit BaseMap(t::TokenMapAllocation tag):AbstractTokenMap(tag){}static inline bool contained;static inline t::TokenLookup mapped;static inline std::vector<std::string>*events;bool contains(const Property&)override{events->push_back("contains");return contained;}t::TokenLookup lookup(const Property&)override{events->push_back("lookup");return mapped;}t::ParseMethod parser()const override{events->push_back("parser");return AbstractTokenMap::parser();}void parser(t::ParseMethod v)override{AbstractTokenMap::parser(std::move(v));}};
struct InspectCharacter:t::CharacterMap{using CharacterMap::CharacterMap;static inline std::vector<bool>*flags;static inline FunctionRef expected;static inline const InspectCharacter*seen;Property name()const override{seen=this;flags->push_back(parser()==FunctionValue{expected});flags->push_back(!map_.has_value());flags->push_back(typeError([&]{const_cast<InspectCharacter*>(this)->contains("x");}));return t::AbstractTokenMap::name();}void add(Property k,t::TokenLookup v)override{flags->push_back(t::MapHandler::getMap(t::AbstractTokenMap::name()).get()==this);flags->push_back(map_.has_value());t::AbstractParseMap::add(std::move(k),std::move(v));}};
struct ControlledRegex:t::RegExpContext{std::function<bool(const Property&)>body;const ControlledRegex*seen=nullptr;bool test(const Property&p)override{seen=this;return body(p);}};
struct InspectRegex:t::RegExpMap{using RegExpMap::RegExpMap;static inline std::vector<bool>*flags;static inline const InspectRegex*seen;t::RegExpRef dependency()const{return regexp_;}Property name()const override{seen=this;flags->push_back(!regexp_);flags->push_back(typeError([&]{const_cast<InspectRegex*>(this)->contains("x");}));return t::AbstractTokenMap::name();}};
struct OrdinaryMap:t::AbstractParseMap{using AbstractParseMap::AbstractParseMap;std::vector<Property>keys()const{std::vector<Property>r;for(const auto&[k,v]:map_.value())r.push_back(k);return r;}};
struct RecordCharacter:t::CharacterMap{using CharacterMap::CharacterMap;static inline std::vector<std::string>*keys;void add(Property k,t::TokenLookup v)override{keys->push_back(std::get<std::string>(k.value));t::AbstractParseMap::add(std::move(k),std::move(v));}};
struct RecordMacro:t::MacroMap{using MacroMap::MacroMap;static inline std::vector<std::string>*keys;void add(Property k,t::TokenLookup v)override{keys->push_back(std::get<std::string>(k.value));t::AbstractParseMap::add(std::move(k),std::move(v));}};
struct ChangeCharacter:t::CharacterMap{using CharacterMap::CharacterMap;static inline PropertyList*json;void add(Property k,t::TokenLookup v)override{if(k==Property("a")){json->set("b","Changed");json->set("c","C");}t::AbstractParseMap::add(std::move(k),std::move(v));}};
struct ChangeMacro:t::MacroMap{using MacroMap::MacroMap;static inline PropertyList*json;static inline FunctionRef changed;void add(Property k,t::TokenLookup v)override{if(k==Property("a")){json->set("b",Property(changed));json->set("c",Property(changed));}t::AbstractParseMap::add(std::move(k),std::move(v));}};
struct SequenceMacro:t::MacroMap{using MacroMap::MacroMap;static inline t::MacroRef a,b;static inline int index;static inline std::vector<std::string>*events;t::TokenLookup lookup(const Property&)override{events->push_back("lookup");return index++%2==0?a:b;}};
struct ArgsCommand:t::CommandMap{using CommandMap::CommandMap;static inline t::MacroRef value;t::TokenLookup lookup(const Property&)override{return value;}};
struct InspectEnvironment:t::EnvironmentMap{using EnvironmentMap::EnvironmentMap;static inline std::vector<bool>*flags;static inline std::vector<std::string>*events;t::ParseMethod parser()const override{events->push_back("get-parser");return t::AbstractTokenMap::parser();}void parser(t::ParseMethod v)override{events->push_back("set-parser");t::AbstractTokenMap::parser(std::move(v));}Property name()const override{flags->push_back(t::AbstractTokenMap::parser()==FunctionValue{Property(PropertyNull{})});return t::AbstractTokenMap::name();}};
struct BridgeCharacter:t::CharacterMap{using CharacterMap::CharacterMap;static inline t::MacroRef value;bool contains(const Property&)override{return true;}t::TokenLookup lookup(const Property&)override{return t::TokenLookup{value};}};
suite token_maps=[] {
"math_port_token_map_parse_result"_test=[] {Data d;std::vector<bool>flags;for(const auto&v:d.values()){auto r=t::parseResult(v);auto p=std::get_if<Property>(&v);flags.push_back(d.label(r)==(p&&p->missing()?"true":d.label(v)));}compare(flags,source_parse_result);auto stack=std::make_shared<t::StackItemContext>();expect(t::parseResult(t::ParseResult{t::StackItemRef(stack)})==t::ParseResult{t::StackItemRef(stack)});expect(t::parseResult(t::ParseResult{t::StackItemRef{}})==t::ParseResult{t::StackItemRef{}});};
"math_port_token_map_base_matrix"_test=[] {Data d;Parser env;int id=0;for(const auto&c:source_base){std::vector<std::string>events;BaseMap::events=&events;BaseMap::contained=c.contains;BaseMap::mapped=d.mapped()[c.map];auto body=t::makeParseCallable([&](const FunctionReceiver&r,auto&,const t::ParseAtom&a,auto)->t::ParseResult{events.push_back("body:"+d.label(a));events.push_back(std::holds_alternative<Missing>(r)?"bare":"wrong-receiver");return d.values()[c.result];});std::vector<t::ParseMethod>methods{Property(Missing{}),Property(PropertyNull{}),Property(false),Property(0),Property(""),Property(true),Property(3),body};auto map=t::TokenMapHost::create<BaseMap>("p22b-native-base-"+std::to_string(id++),methods[c.method]);std::string value,error;try{value=d.label(map->parse({env,"x"}));}catch(const tree::FactoryError&e){error=e.name;}expect(value==c.value);expect(error==c.error);expect(events==c.events);}};
"math_port_token_map_registry_constructor"_test=[] {Data d;std::vector<bool>flags;InspectCharacter::flags=&flags;InspectCharacter::expected=d.function;auto m=t::TokenMapHost::create<InspectCharacter>("p22b-native-registration",FunctionValue{d.function},Property(PropertyList{{"x","X"}}));flags.insert(flags.end(),{InspectCharacter::seen==m.get(),t::MapHandler::getMap("p22b-native-registration")==m,m->contains("x")});auto old=t::TokenMapHost::create<t::CharacterMap>("p22b-native-registration",FunctionValue{d.function},Property(PropertyList{{"old","O"}})),next=t::TokenMapHost::create<t::CharacterMap>("p22b-native-registration",FunctionValue{d.function},Property(PropertyList{{"next","N"}}));flags.insert(flags.end(),{t::MapHandler::getMap("p22b-native-registration")==next,old!=next});flags.push_back(typeError([&]{t::TokenMapHost::create<t::CharacterMap>("p22b-native-partial",FunctionValue{d.function},Property(PropertyNull{}));}));auto partial=std::dynamic_pointer_cast<t::CharacterMap>(t::MapHandler::getMap("p22b-native-partial"));flags.push_back(static_cast<bool>(partial));if(partial)flags.insert(flags.end(),{partial->name()==Property("p22b-native-partial"),!partial->contains("x")});auto regex=std::make_shared<ControlledRegex>();regex->body=[](const auto&){return true;};InspectRegex::flags=&flags;auto r=t::TokenMapHost::create<InspectRegex>("p22b-native-regex-registration",FunctionValue{d.function},t::RegExpRef(regex));flags.insert(flags.end(),{InspectRegex::seen==r.get(),r->dependency()==regex,r->contains("x")});compare(flags,source_registry_constructor);};
"math_port_token_map_ordinary_map"_test=[] {Data d;auto m=t::TokenMapHost::create<OrdinaryMap>("p22b-native-ordinary",FunctionValue{d.function});PropertyList key,other;auto fn=std::make_shared<Function>();std::vector<bool>flags;for(const Property k:std::vector<Property>{std::numeric_limits<double>::quiet_NaN(),0,-0.0,Missing{},PropertyNull{},false,7,"7",key,other,Property(fn)}){m->add(k,Property(Missing{}));flags.insert(flags.end(),{m->contains(k),m->lookup(k)==t::TokenLookup{Property(Missing{})}});m->add(k,Property(d.object));flags.push_back(m->lookup(k)==t::TokenLookup{Property(d.object)});m->remove(k);flags.insert(flags.end(),{!m->contains(k),m->lookup(k)==t::TokenLookup{Property(Missing{})}});}m->add("a",d.token);m->add("b",d.macro);m->add("a",Property(d.array));flags.insert(flags.end(),{m->keys()==std::vector<Property>{"a","b"},m->lookup("a")==t::TokenLookup{Property(d.array)},m->lookup("b")==t::TokenLookup{d.macro}});m->remove("a");m->add("a",d.token);flags.push_back(m->keys()==std::vector<Property>{"b","a"});m->add(-0.0,Property(7));flags.push_back(!std::signbit(std::get<double>(m->keys().back().value)));compare(flags,source_ordinary_map);};
"math_port_token_map_character_data"_test=[] {Data d;PropertyList attrs{{"weight","bold"}},proto{{"0","Inherited"},{"1",Property(attrs)}};PropertyArray a{"A",Property(attrs),"unused"},sparse,shortArray,empty;sparse.resize(2);shortArray.resize(1);for(auto arr:{sparse,shortArray,empty})arr.record().set("__proto__",proto);PropertyList json{{"9","nine"},{"2",Property(a)},{"x",Property(sparse)},{"short",Property(shortArray)},{"empty",Property(empty)},{"number",7},{"like",Property(PropertyList{{"0","Like"},{"1",Property(attrs)}})},{"\\left","L"}};std::vector<std::string>keys;RecordCharacter::keys=&keys;auto m=t::TokenMapHost::create<RecordCharacter>("p22b-native-character",FunctionValue{d.function},Property(json));std::vector<bool>flags{keys==std::vector<std::string>{"2","9","x","short","empty","number","like","\\left"}};struct Row{std::string key;Property value,attrs;};for(const auto&r:std::vector<Row>{{"2","A",attrs},{"9","nine",PropertyNull{}},{"x","Inherited",attrs},{"short","Inherited",Missing{}},{"empty",Missing{},Missing{}},{"number",Missing{},Missing{}},{"like","Like",attrs},{"\\left","L",PropertyNull{}}}){auto result=m->lookup(r.key);auto token=std::get_if<t::TokenRef>(&result);flags.push_back(token&&*token);if(token&&*token)flags.insert(flags.end(),{(*token)->token()==Property(r.key),(*token)->character()==r.value,(*token)->attributes()==r.attrs,m->lookup(r.key)==result});}auto numeric=m->lookup("2");flags.insert(flags.end(),{m->contains("2"),!m->contains(2),m->parserFor("2")==FunctionValue{d.function},m->parserFor("absent")==FunctionValue{Property(PropertyNull{})}});m->parser(Property(PropertyNull{}));flags.insert(flags.end(),{m->parser()==FunctionValue{Property(PropertyNull{})},m->parserFor("2")==FunctionValue{Property(PropertyNull{})},m->lookup("2")==numeric});compare(flags,source_character_data);};
"math_port_token_map_constructor_enumeration"_test=[] {Data d;std::vector<bool>flags;PropertyList chars{{"a","A"},{"b","B"}},macros{{"a",Property(d.function)},{"b",Property(d.function)}};ChangeCharacter::json=&chars;auto c=t::TokenMapHost::create<ChangeCharacter>("p22b-native-char-enumeration",FunctionValue{d.function},Property(chars));flags.insert(flags.end(),{std::get<t::TokenRef>(c->lookup("b"))->character()==Property("Changed"),!c->contains("c")});ChangeMacro::json=&macros;ChangeMacro::changed=std::make_shared<Function>();auto m=t::TokenMapHost::create<ChangeMacro>("p22b-native-macro-enumeration",Property(macros));flags.insert(flags.end(),{std::get<t::MacroRef>(m->lookup("b"))->func()==FunctionValue{d.function},!m->contains("c")});auto utf=t::TokenMapHost::create<t::CharacterMap>("p22b-native-string-keys",FunctionValue{d.function},Property("😀"));flags.insert(flags.end(),{property_host::text(std::get<std::string>(std::get<t::TokenRef>(utf->lookup("0"))->character().value))==std::u16string(1,0xD83D),property_host::text(std::get<std::string>(std::get<t::TokenRef>(utf->lookup("1"))->character().value))==std::u16string(1,0xDE00)});compare(flags,source_constructor_enumeration);};
"math_port_token_map_macro_data"_test=[] {Data d;auto g=std::make_shared<Function>();PropertyArray a{Property(d.function),Missing{},Property(d.object)},sparse;sparse.resize(3);sparse.set(0,Property(d.function));sparse.record().set("__proto__",PropertyList{{"1",Property(d.array)}});PropertyList functions{{"named",Property(d.function)}},json{{"9",Property(g)},{"2",Property(a)},{"direct",Property(d.function)},{"byName",Property(PropertyArray{"named",7})},{"missing","absent"},{"sparse",Property(sparse)}};std::vector<std::string>keys;RecordMacro::keys=&keys;auto m=t::TokenMapHost::create<RecordMacro>("p22b-native-macro-data",Property(json),Property(functions));auto at=[&](const char*k){return std::get<t::MacroRef>(m->lookup(k));};auto s=at("sparse");auto sa=PropertyArray::array(s->args()).value(),a2=PropertyArray::array(at("2")->args()).value();std::vector<bool>flags{keys==std::vector<std::string>{"2","9","direct","byName","missing","sparse"},m->parser()==FunctionValue{Property(PropertyNull{})},at("2")->func()==FunctionValue{d.function},at("2")->args()!=Property(a),a2.get(0).missing(),a2.has(0),a2.get(1)==Property(d.object),at("direct")->func()==FunctionValue{d.function},PropertyArray::array(at("direct")->args())->size()==0,at("byName")->func()==FunctionValue{d.function},PropertyArray::array(at("byName")->args())->get(0)==Property(7),at("missing")->func()==FunctionValue{Property(Missing{})},m->parserFor("missing")==FunctionValue{Property(Missing{})},m->parserFor("absent")==FunctionValue{Property(PropertyNull{})},sa.size()==2,sa.get(0)==Property(d.array),sa.has(0),!sa.has(1)};auto inherited=PropertyList::withPrototype(PropertyList{{"named",Property(g)}});auto n=t::TokenMapHost::create<t::MacroMap>("p22b-native-inherited-method",Property(PropertyList{{"x","named"}}),Property(inherited));flags.push_back(std::get<t::MacroRef>(n->lookup("x"))->func()==FunctionValue{g});flags.push_back(typeError([&]{t::TokenMapHost::create<t::MacroMap>("p22b-native-null-methods",Property(PropertyList{{"good",Property(d.function)},{"bad","named"}}),Property(PropertyNull{}));}));auto partial=t::MapHandler::getMap("p22b-native-null-methods");flags.insert(flags.end(),{partial&&partial->contains("good"),partial&&!partial->contains("bad")});compare(flags,source_macro_data);};
"math_port_token_map_macro_dispatch"_test=[] {Data d;Parser env;PropertyArray args{Property(d.object),Property(d.array)};std::vector<std::string>events;FunctionReceiver receiver;std::vector<Property>rest;Property atom;Parser*parser=nullptr;auto record=[&](const FunctionReceiver&r,t::ParserContext&p,const t::ParseAtom&a,t::ParseRest values){receiver=r;parser=dynamic_cast<Parser*>(&p);atom=std::get<Property>(a);rest.clear();for(const auto&v:values)rest.push_back(std::any_cast<Property>(v));};auto one=t::makeParseCallable([&](const auto&r,auto&p,const auto&a,auto values)->t::ParseResult{events.push_back("first-body");record(r,p,a,values);return Property(Missing{});}),two=t::makeParseCallable([&](const auto&r,auto&p,const auto&a,auto values)->t::ParseResult{events.push_back("second-body");record(r,p,a,values);return Property(false);});SequenceMacro::a=std::make_shared<t::Macro>("first",FunctionValue{one},Property(args));SequenceMacro::b=std::make_shared<t::Macro>("second",FunctionValue{two},Property(PropertyArray{99}));SequenceMacro::index=0;SequenceMacro::events=&events;auto m=t::TokenMapHost::create<SequenceMacro>("p22b-native-dispatch",Property(PropertyList{}));auto result=m->parse({env,"x"});std::vector<bool>flags{result==t::ParseResult{Property(false)},std::holds_alternative<Missing>(receiver),parser==&env,atom==Property("first"),rest.size()>0&&rest[0]==Property(d.object),rest.size()>1&&rest[1]==Property(d.array),events==std::vector<std::string>{"lookup","lookup","second-body"}};auto n=t::TokenMapHost::create<t::MacroMap>("p22b-native-live-args",Property(PropertyList{{"x",Property(PropertyArray{Property(one),1})}}));auto macro=std::get<t::MacroRef>(n->lookup("x"));auto live=PropertyArray::array(macro->args()).value();live.set(live.size(),Property(d.object));n->parse({env,"x"});flags.insert(flags.end(),{rest.size()==2,rest.size()>1&&rest[1]==Property(d.object)});n->remove("x");flags.push_back(n->parse({env,"x"})==t::ParseResult{Property(PropertyNull{})});compare(flags,source_macro_dispatch);};
"math_port_token_map_delimiter_dispatch"_test=[] {Parser env;FunctionReceiver receiver; t::ParserContext*parser=nullptr;t::TokenRef token;auto method=t::makeParseCallable([&](const auto&r,auto&p,const auto&a,auto)->t::ParseResult{receiver=r;parser=&p;token=std::get<t::TokenRef>(a);return Property(false);});auto m=t::TokenMapHost::create<t::DelimiterMap>("p22b-native-delimiter",FunctionValue{method},Property(PropertyList{{"\\x","X"},{"\\null","N"}}));bool result=m->parse({env,"x"})==t::ParseResult{Property(false)};std::vector<bool>flags{result,std::holds_alternative<Missing>(receiver),parser==&env,m->lookup("\\x")==t::TokenLookup{token},m->parse({env,"absent"})==t::ParseResult{Property(PropertyNull{})},m->parse({env,PropertyNull{}})==t::ParseResult{Property(false)}};compare(flags,source_delimiter_dispatch);};
"math_port_token_map_command_dispatch"_test=[] {Data d;Parser env;std::vector<std::string>events;env.events=&events;FunctionReceiver receiver;t::ParserContext*parser=nullptr;Property atom,seenCS;std::vector<Property>rest;auto method=t::makeParseCallable([&](const auto&r,auto&p,const auto&a,auto values)->t::ParseResult{events.push_back("body");receiver=r;parser=&p;atom=std::get<Property>(a);for(const auto&v:values)rest.push_back(std::any_cast<Property>(v));seenCS=env.cs;return Property(Missing{});});auto m=t::TokenMapHost::create<t::CommandMap>("p22b-native-command",Property(PropertyList{{"x",Property(PropertyArray{Property(method),Property(d.object)})}}));bool result=m->parse({env,"x"})==t::ParseResult{Property(true)};std::vector<bool>flags{result,std::holds_alternative<Missing>(receiver),parser==&env,atom==Property("\\x"),rest.size()==1&&rest[0]==Property(d.object),seenCS==Property("\\x"),env.cs==Property("saved"),events==std::vector<std::string>{"get-cs","set-cs:\\x","body","set-cs:saved"}};PropertyList thrown{{"id","failure"}};auto bad=t::makeParseCallable([&](const auto&,auto&,const auto&,auto)->t::ParseResult{throw async::ThrownValue{Property(thrown)};});auto n=t::TokenMapHost::create<t::CommandMap>("p22b-native-command-error",Property(PropertyList{{"bad",Property(bad)}}));try{n->parse({env,"bad"});flags.push_back(false);}catch(const async::ThrownValue&e){flags.push_back(std::any_cast<Property>(e.value)==Property(thrown));}flags.push_back(env.cs==Property("\\bad"));ArgsCommand::value=std::make_shared<t::Macro>("arg-error",FunctionValue{method},Property(PropertyNull{}));auto a=t::TokenMapHost::create<ArgsCommand>("p22b-native-command-args-error",Property(PropertyList{}));flags.push_back(typeError([&]{a->parse({env,"args"});}));flags.push_back(env.cs==Property("\\args"));compare(flags,source_command_dispatch);};
"math_port_token_map_environment_dispatch"_test=[] {Data d;Parser env;std::vector<bool>flags;std::vector<std::string>events;InspectEnvironment::flags=&flags;InspectEnvironment::events=&events;auto specific=t::makeParseCallable([](const auto&,auto&,const auto&,auto)->t::ParseResult{return Property(Missing{});});FunctionReceiver receiver;t::ParserContext*parser=nullptr;Property atom,args;FunctionRef passed;auto general=t::makeParseCallable([&](const auto&r,auto&p,const auto&a,auto rest)->t::ParseResult{events.push_back("body");receiver=r;parser=&p;atom=std::get<Property>(a);passed=std::any_cast<FunctionRef>(rest[0]);args=std::any_cast<Property>(rest[1]);return Property(false);});auto m=t::TokenMapHost::create<InspectEnvironment>("p22b-native-environment",FunctionValue{general},Property(PropertyList{{"x",Property(PropertyArray{Property(specific),Property(d.object)})}}));std::vector<bool>observed;observed.push_back(events==std::vector<std::string>{"set-parser"});observed.push_back(m->parse({env,"x"})==t::ParseResult{Property(false)});observed.push_back(events==std::vector<std::string>{"set-parser","get-parser","body"});observed.push_back(std::get<NativeObjectRef>(receiver)==m);observed.push_back(parser==&env);observed.push_back(atom==Property("x"));observed.push_back(passed==specific);observed.push_back(args==std::get<t::MacroRef>(m->lookup("x"))->args());observed.push_back(PropertyArray::array(args)->get(0)==Property(d.object));observed.push_back(m->parse({env,"absent"})==t::ParseResult{Property(PropertyNull{})});observed.push_back(t::MapHandler::getMap(m->name())==m);flags.insert(flags.end(),observed.begin(),observed.end());compare(flags,source_environment_dispatch);};
"math_port_token_map_regexp_dependency"_test=[] {Data d;Parser env;int id=0;for(const auto&c:source_regex){std::vector<std::string>events;int calls=0;auto regex=std::make_shared<ControlledRegex>();regex->body=[&](const Property&p){events.push_back("test:"+d.label(p));events.push_back(regex->seen==regex.get()?"regex-receiver":"wrong");return c.sequence[calls++%2];};auto method=t::makeParseCallable([&](const auto&r,auto&,const auto&a,auto)->t::ParseResult{events.push_back("body:"+d.label(a));events.push_back(std::holds_alternative<Missing>(r)?"bare":"wrong");return Property(Missing{});});auto m=t::TokenMapHost::create<t::RegExpMap>("p22b-native-regexp-"+std::to_string(id++),FunctionValue{method},t::RegExpRef(regex));expect(d.label(m->parse({env,"x"}))==c.value);expect(events==c.events);expect(calls==c.calls);}std::vector<bool>flags;int lastIndex=0;auto regex=std::make_shared<ControlledRegex>();regex->body=[&](const auto&){bool matched=lastIndex==0;lastIndex=matched?1:0;return matched;};auto m=t::TokenMapHost::create<t::RegExpMap>("p22b-native-regexp-live",FunctionValue{d.function},t::RegExpRef(regex));flags.push_back(m->lookup("x")==t::TokenLookup{Property("x")});flags.push_back(lastIndex==1);flags.push_back(m->lookup("x")==t::TokenLookup{Property(PropertyNull{})});flags.push_back(lastIndex==0);flags.push_back(m->contains("x"));flags.push_back(lastIndex==1);lastIndex=0;flags.push_back(m->parse({env,"x"})==t::ParseResult{Property(PropertyNull{})});flags.push_back(lastIndex==0);compare(flags,source_regexp_live_service_dependency);};
"math_port_token_map_new_lookup_bridge"_test=[] {Data d;BridgeCharacter::value=std::make_shared<t::Macro>("x",FunctionValue{d.function},Property(PropertyArray{}));auto m=t::TokenMapHost::create<BridgeCharacter>("p22b-native-handler-macro",FunctionValue{d.function},Property(PropertyList{}));t::SubHandler h;h.add(PropertyArray{m->name()});std::vector<bool>flags{h.contains("x")};BridgeCharacter::value.reset();flags.push_back(typeError([&]{h.contains("x");}));compare(flags,source_new_lookup_bridge);};
};}
