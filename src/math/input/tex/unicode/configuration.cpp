// Native port of MathJax 4.1.3 input/tex/unicode/UnicodeConfiguration.ts.
module folia.math.input.tex.unicode.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.token_map;
import folia.math.input.tex.unit_util;
import folia.math.util.entities;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
struct UnicodeMetric { double height=800,depth=200;std::u16string font; };
std::map<std::string,UnicodeMetric> unicodeCache;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& source){auto* value=dynamic_cast<TexParser*>(&source);if(!value)property_host::typeError("Unicode requires a TexParser");return *value;}
std::u16string atomString(const ParseAtom& atom){if(const auto* value=std::get_if<Property>(&atom))return property_host::string(*value);if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)return property_host::string((*token)->character());property_host::typeError("Unicode macro name is not a string");}
std::u16string replaceSpaces(std::u16string value){value.erase(std::remove(value.begin(),value.end(),u' '),value.end());return value;}
std::vector<std::u16string> splitComma(std::u16string_view value){std::vector<std::u16string> out;std::size_t pos=0;while(true){const auto comma=value.find(u',',pos);out.emplace_back(value.substr(pos,comma==std::u16string_view::npos?value.size()-pos:comma-pos));if(comma==std::u16string_view::npos)break;pos=comma+1;}return out;}
std::optional<std::vector<std::string>> match(std::u16string_view value,std::string_view pattern){const auto utf=property_host::debugText(value);std::smatch found;if(!std::regex_search(utf,found,std::regex(std::string(pattern),std::regex::ECMAScript)))return {};std::vector<std::string> captures;captures.reserve(found.size());for(const auto& item:found)captures.push_back(item.str());return captures;}
double parseInteger(std::u16string_view value,bool hexadecimal=false){return js_input::parseInt(value,hexadecimal?js_input::IntegerRadix::Hex:js_input::IntegerRadix::Decimal);}
std::uint32_t checkedCodepoint(double value){if(!std::isfinite(value)||std::trunc(value)!=value||value<0||value>0x10ffff)throw Exception("RangeError",u"Invalid code point "+property_host::text(js_number::toString(value)));return static_cast<std::uint32_t>(value);}
std::u16string fromCodepoint(std::uint32_t cp){if(cp<=0xffff)return std::u16string(1,static_cast<char16_t>(cp));cp-=0x10000;return {static_cast<char16_t>(0xd800+(cp>>10)),static_cast<char16_t>(0xdc00+(cp&0x3ff))};}
std::u16string numeric(double cp){return entities::EntityModule{}.numeric(property_host::text(js_number::toString(cp)));}
FunctionRef unicode(){return makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(source);const std::u16string name=atomString(atom);Property hd=p.GetBrackets(name);std::optional<std::vector<std::u16string>> hdSplit;std::u16string font;
    if(property_host::truthy(hd)){const std::u16string header=property_host::string(hd);const std::u16string compact=replaceSpaces(header);
        if(std::regex_match(property_host::debugText(compact),std::regex(R"((\d+(\.\d*)?|\.\d+),(\d+(\.\d*)?|\.\d+))"))){hdSplit=splitComma(compact);font=property_host::string(p.GetBrackets(name,Property("")));}
        else font=header;}
    if(font.find(u';')!=std::u16string::npos){const std::array<Property,1> rest{p.currentCS()};throw TexError(u"BadFont",u"Font name for %1 can't contain semicolons",rest);}
    std::u16string number=property_host::string(UnitUtil::instance().trimSpaces(p.GetArgument(name)));if(number.rfind(u"0x",0)==0)number.replace(0,2,u"x");
    if(!std::regex_match(property_host::debugText(number),std::regex(R"((x[0-9A-Fa-f]+|[0-9]+))"))){const std::array<Property,1> rest{p.currentCS()};throw TexError(u"BadUnicode",u"Argument to %1 must be a number",rest);}
    const double code=number.front()==u'x'?parseInteger(std::u16string_view(number).substr(1),true):parseInteger(number);
    const auto cacheKey=js_number::toString(code);auto found=unicodeCache.find(cacheKey);if(found==unicodeCache.end())found=unicodeCache.emplace(cacheKey,UnicodeMetric{800,200,font}).first;else if(font.empty())font=found->second.font;
    if(hdSplit){found->second.height=std::floor(std::strtod(property_host::debugText((*hdSplit)[0]).c_str(),nullptr)*1000);found->second.depth=std::floor(std::strtod(property_host::debugText((*hdSplit)[1]).c_str(),nullptr)*1000);}
    const auto stackValue=p.stack;const auto* stack=std::get_if<std::shared_ptr<Stack>>(&stackValue);if(!stack||!*stack)property_host::typeError("Cannot read properties of null or undefined (reading 'env')");
    Property variant=property_host::get((*stack)->env(),"font");PropertyList definition;
    if(!font.empty()){
        found->second.font=font;definition.setOwn("fontfamily",property_host::debugText(font));
        if(property_host::truthy(variant)){const auto* value=std::get_if<std::string>(&variant.value);if(!value)property_host::typeError("variant.match is not a function");const auto text=property_host::text(*value);if(text.find(u"bold")!=std::u16string::npos)definition.setOwn("fontweight",Property("bold"));if(text.find(u"italic")!=std::u16string::npos||text.find(u"-mathit")!=std::u16string::npos)definition.setOwn("fontstyle",Property("italic"));}
    }else if(property_host::truthy(variant))definition.setOwn("mathvariant",variant);
    const std::array<std::any,3> args{std::any(Property("mtext")),std::any(Property(definition)),std::any(Property(property_host::debugText(numeric(code))))};
    const auto node=p.create(u"token",FunctionArgs(args));NodeUtil::instance().setProperty(node,"unicode",Property(true));p.Push(node);return Property(Missing{});
});}
FunctionRef rawUnicode(){return makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(source);const std::u16string name=atomString(atom);const std::u16string hex=property_host::string(UnitUtil::instance().trimSpaces(p.GetArgument(name)));
    if(!std::regex_match(property_host::debugText(hex),std::regex(R"([0-9A-F]{1,6})"))){const std::array<Property,1> rest{p.currentCS()};throw TexError(u"BadRawUnicode",u"Argument to %1 must a hexadecimal number with 1 to 6 digits",rest);}
    const auto code=checkedCodepoint(parseInteger(hex,true));const std::u16string input=p.string();const auto offset=static_cast<std::size_t>(p.i);p.setString(fromCodepoint(code)+input.substr(std::min(offset,input.size())));p.i=0;return Property(Missing{});
});}
FunctionRef charMethod(){return makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(source);const std::u16string name=atomString(atom);const std::u16string next=p.GetNext();const std::u16string input=p.string().substr(static_cast<std::size_t>(p.i));std::u16string character;std::size_t consumed=0;
    if(next==u"'"){if(const auto m=match(input,R"(^'([0-7]{1,7}) ?)");m){character=fromCodepoint(checkedCodepoint(js_input::parseInt(property_host::text((*m)[1]),js_input::IntegerRadix::Octal)));consumed=(*m)[0].length();}}
    else if(next==u"\""){if(const auto m=match(input,R"(^\"([0-9A-F]{1,6}) ?)");m){const auto cap=(*m)[1];character=fromCodepoint(checkedCodepoint(std::strtoul(cap.c_str(),nullptr,16)));consumed=(*m)[0].length();}}
    else if(next==u"`"){
        if(const auto m=match(input,R"(^`(?:(\\\S)|(.)))");m){if(!(*m)[2].empty()){character=property_host::text((*m)[2]);consumed=(*m)[0].length();}else{p.i+=2;const auto cs=p.GetCS();if(cs.size()>1){const std::array<Property,1> rest{p.currentCS()};throw TexError(u"InvalidAlphanumeric",u"Invalid alphanumeric constant for %1",rest);}character=cs;}}
    }else if(const auto m=match(input,R"(^([0-9]{1,7}) ?)");m){character=fromCodepoint(checkedCodepoint(parseInteger(property_host::text((*m)[1]))));consumed=(*m)[0].length();}
    if(character.empty()){const std::array<Property,1> rest{p.currentCS()};throw TexError(u"MissingNumber",u"Missing numeric constant for %1",rest);}
    p.i+=static_cast<double>(consumed);
    if(character.size()==1&&character[0]>=u'0'&&character[0]<=u'9')p.Push(p.create(u"token",FunctionArgs(std::array<std::any,3>{std::any(Property("mn")),std::any(Property(PropertyList{})),std::any(Property(property_host::debugText(character)))})));
    else if(character.size()==1&&((character[0]>=u'A'&&character[0]<=u'Z')||(character[0]>=u'a'&&character[0]<=u'z')))p.Push(p.create(u"token",FunctionArgs(std::array<std::any,3>{std::any(Property("mi")),std::any(Property(PropertyList{})),std::any(Property(property_host::debugText(character)))})));
    else {const auto* po=std::get_if<ParseOptions*>(&p.configuration);if(!po||!*po)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");auto& runtime=(**po).runtime();const auto other=functions::fromProperty(property_host::get(runtime.parseMethods,"Other"));return invokeParseMethod(other,FunctionReceiver{},source,ParseAtom{Property(property_host::debugText(character))});}
    return Property(Missing{});
});}
}
ConfigurationRef UnicodeConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"unicode");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const std::array methods{
        std::pair{unicode(),std::u16string_view(u"Unicode")},
        std::pair{rawUnicode(),std::u16string_view(u"RawUnicode")},
        std::pair{charMethod(),std::u16string_view(u"Char")}};
    PropertyList methodMap;
    for(const auto& [method,name]:methods){
        const auto bound=BaseMethods::bind(runtime,method,std::u16string(name));
        const auto* identity=std::get_if<ObjectRef>(&bound.data);
        if(!identity)throw Exception("TypeError",u"Unicode method has no Context identity");
        runtime.context().define(*identity,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    }
    methodMap.setOwn("unicode",Property(methods[0].first));
    methodMap.setOwn("U",Property(methods[1].first));
    methodMap.setOwn("char",Property(methods[2].first));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("unicode"),Property(methodMap));
    auto& cx=runtime.context();const Value handlers=cx.object();const ObjectRef names=cx.array(1);cx.set(Value(names),key(u"0"),Value(u"unicode"));cx.set(handlers,key(u"macro"),Value(names));
    const Value definition=cx.object();cx.set(definition,key(u"handler"),handlers);
    return Configuration::create(runtime,u"unicode",definition);
}
}
