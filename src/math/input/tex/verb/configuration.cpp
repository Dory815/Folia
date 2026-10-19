// Native port of MathJax 4.1.3 input/tex/verb/VerbConfiguration.ts.
module folia.math.input.tex.verb.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& source){auto* value=dynamic_cast<TexParser*>(&source);if(!value)property_host::typeError("Verb requires a TexParser");return *value;}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> values){Value out(cx.object());for(const auto& [name,value]:values)cx.set(out,key(name),value);return out;}
FunctionRef verb(){return makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(source);std::u16string name;
    if(const auto* value=std::get_if<Property>(&atom))name=property_host::string(*value);
    else if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)name=property_host::string((*token)->character());
    else property_host::typeError("Verb macro name is not a string");
    const std::u16string delimiter=p.GetNext();const std::size_t start=static_cast<std::size_t>(++p.i);
    if(delimiter.empty()){const std::array<Property,1> rest{property_host::debugText(name)};throw TexError(u"MissingArgFor",u"Missing argument for %1",rest);}
    const std::u16string input=p.string();
    while(p.i<static_cast<double>(input.size())&&input[static_cast<std::size_t>(p.i)]!=delimiter.front())++p.i;
    if(p.i==static_cast<double>(input.size())){const std::array<Property,1> rest{p.currentCS()};throw TexError(u"NoClosingDelim",u"Can't find closing delimiter for %1",rest);}
    std::u16string content=input.substr(start,static_cast<std::size_t>(p.i)-start);
    std::replace(content.begin(),content.end(),u' ',u'\u00A0');++p.i;
    const Property variant=property_host::get(property_host::get(TexConstant,"Variant"),"MONOSPACE");
    const std::array<std::any,3> args{std::any(Property("mtext")),std::any(Property(PropertyList{{"mathvariant",variant}})),std::any(Property(property_host::debugText(content)))};
    p.Push(p.create(u"token",FunctionArgs(args)));return Property(Missing{});
});}
}
ConfigurationRef VerbConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"verb");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const auto method=verb();const auto bound=BaseMethods::bind(runtime,method,u"Verb");
    const auto* identity=std::get_if<ObjectRef>(&bound.data);
    if(!identity)throw Exception("TypeError",u"Verb method has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("verb"),Property(PropertyList{{"verb",Property(method)}}));
    auto& cx=runtime.context();const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"verb")})}});
    return Configuration::create(runtime,u"verb",record(cx,{{u"handler",handlers}}));
}
}
