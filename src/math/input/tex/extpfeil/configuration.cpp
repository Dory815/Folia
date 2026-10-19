// Native port of MathJax 4.1.3 input/tex/extpfeil/ExtpfeilConfiguration.ts.
module folia.math.input.tex.extpfeil.configuration;
import folia.math.input.tex.ams.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.token_map;
import folia.math.util.js_numeric_input;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& source){auto* value=dynamic_cast<TexParser*>(&source);if(!value)property_host::typeError("Extpfeil requires a TexParser");return *value;}
std::u16string atomString(const ParseAtom& atom){if(const auto* value=std::get_if<Property>(&atom))return property_host::string(*value);if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)return property_host::string((*token)->character());property_host::typeError("Extpfeil macro name is not a string");}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> values){Value out(cx.object());for(const auto& [name,value]:values)cx.set(out,key(name),value);return out;}
bool matches(std::u16string_view value,std::string_view pattern){const auto text=property_host::debugText(value);return std::regex_match(text,std::regex(std::string(pattern),std::regex::ECMAScript|std::regex::icase));}
[[noreturn]] void badArgument(std::u16string id,std::u16string_view message,const std::u16string& name){const std::array<Property,1> rest{property_host::debugText(name)};throw TexError(std::move(id),message,rest);}
Property restAt(ParseRest rest,std::size_t index){if(index>=rest.size())return Missing{};if(const auto* p=std::any_cast<Property>(&rest[index]))return *p;if(const auto* n=std::any_cast<double>(&rest[index]))return *n;if(const auto* s=std::any_cast<std::u16string>(&rest[index]))return property_host::debugText(*s);return Missing{};}
FunctionRef newExtArrow(FunctionRef xArrow){return makeParseCallable([xArrow=std::move(xArrow)](const FunctionReceiver& receiver,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(source);const std::u16string name=atomString(atom);const std::u16string control=property_host::string(p.GetArgument(name));const std::u16string space=property_host::string(p.GetArgument(name));const std::u16string character=property_host::string(p.GetArgument(name));
    if(!matches(control,R"(^\\([a-z]+|.)$)"))badArgument(u"NewextarrowArg1",u"First argument to %1 must be a control sequence name",name);
    if(!matches(space,R"(^\d+,\d+$)"))badArgument(u"NewextarrowArg2",u"Second argument to %1 must be two integers separated by a comma",name);
    if(!matches(character,R"(^([0-9]+|0x[0-9A-F]+)$)"))badArgument(u"NewextarrowArg3",u"Third argument to %1 must be a unicode character number",name);
    const auto comma=space.find(u',');const double left=js_input::parseInt(std::u16string_view(space).substr(0,comma),js_input::IntegerRadix::Decimal);const double right=js_input::parseInt(std::u16string_view(space).substr(comma+1),js_input::IntegerRadix::Decimal);
    const double code=js_input::parseInt(character,js_input::IntegerRadix::Automatic);const std::u16string macro=control.substr(1);
    NewcommandUtil::addMacro(p,macro,ParseMethod{xArrow},Property(PropertyArray{Property(code),Property(left),Property(right)}));
    auto item=p.itemFactory();const auto* factory=std::get_if<std::shared_ptr<StackItemFactory>>(&item);if(!factory||!*factory)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");
    BaseItem* nullItem=(*factory)->create("null",{});if(!nullItem)property_host::typeError("Cannot read properties of null (reading 'isKind')");p.Push((*factory)->reference(nullItem));return Property(Missing{});
});}
}
ConfigurationRef ExtpfeilConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"extpfeil");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    NewcommandConfiguration::initializeModule(runtime);
    const ParseMethod xArrowMethod=functions::fromProperty(AmsMethods::get(runtime,u"xArrow"));
    const auto* xArrowValue=std::get_if<FunctionRef>(&xArrowMethod);
    if(!xArrowValue||!*xArrowValue)throw Exception("TypeError",u"xArrow is not callable");
    const FunctionRef xArrow=*xArrowValue;const FunctionRef define=newExtArrow(xArrow);
    const auto bound=BaseMethods::bind(runtime,define,u"NewExtArrow");
    const auto* identity=std::get_if<ObjectRef>(&bound.data);
    if(!identity)throw Exception("TypeError",u"NewExtArrow method has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    const PropertyList commands{
        {"xtwoheadrightarrow",Property(PropertyArray{Property(xArrow),Property(0x21a0),Property(12),Property(16)})},
        {"xtwoheadleftarrow",Property(PropertyArray{Property(xArrow),Property(0x219e),Property(17),Property(13)})},
        {"xmapsto",Property(PropertyArray{Property(xArrow),Property(0x21a6),Property(6),Property(7)})},
        {"xlongequal",Property(PropertyArray{Property(xArrow),Property(0x003d),Property(7),Property(7)})},
        {"xtofrom",Property(PropertyArray{Property(xArrow),Property(0x21c4),Property(12),Property(12)})},
        {"Newextarrow",Property(define)}};
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("extpfeil"),Property(commands));
    const auto newcommand=runtime.configurationHandler.get(u"newcommand");const auto* newcommandPtr=std::get_if<ConfigurationRef>(&newcommand);
    if(!newcommandPtr||!*newcommandPtr)throw Exception("Unimplemented",u"Newcommand configuration is unavailable");
    auto& cx=runtime.context();const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"extpfeil")})}});
    return Configuration::create(runtime,u"extpfeil",record(cx,{{u"handler",handlers},{u"config",(*newcommandPtr)->config()}}));
}
}
