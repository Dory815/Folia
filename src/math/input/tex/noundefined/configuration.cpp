// Native port of MathJax 4.1.3 input/tex/noundefined/NoUndefinedConfiguration.ts.
module folia.math.input.tex.noundefined.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& source){auto* value=dynamic_cast<TexParser*>(&source);if(!value)property_host::typeError("NoUndefined requires a TexParser");return *value;}
std::u16string atomString(const ParseAtom& atom){if(const auto* value=std::get_if<Property>(&atom))return property_host::string(*value);if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)return property_host::string((*token)->character());property_host::typeError("NoUndefined macro name is not a string");}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> values){Value out(cx.object());for(const auto& [name,value]:values)cx.set(out,key(name),value);return out;}
FunctionRef noUndefined(){return makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(source);const std::u16string name=atomString(atom);const Value options=p.options();
    const auto* configuration=std::get_if<ParseOptions*>(&p.configuration);if(!configuration||!*configuration)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
    auto& runtime=(**configuration).runtime();auto& cx=runtime.context();const Value noundefined=cx.get(options,key(u"noundefined"));
    PropertyList definition;
    for(const std::u16string_view id:{u"color",u"background",u"size"}){
        const Value value=cx.get(noundefined,key(id));if(!truthy(value))continue;
        const std::u16string nameKey=u"math"+std::u16string(id);
        definition.setOwn(property_host::debugText(nameKey),runtime.properties.from(value));
    }
    const std::array<std::any,1> textArgs{std::any(Property(property_host::debugText(u"\\"+name)))};
    const mml::NodeValue textNode=p.create(u"text",FunctionArgs(textArgs));
    auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    const std::array<std::any,4> nodeArgs{std::any(Property("mtext")),std::any(children),std::any(Property(definition)),std::any(textNode)};
    p.Push(p.create(u"node",FunctionArgs(nodeArgs)));return Property(Missing{});
});}
}
ConfigurationRef NoUndefinedConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"noundefined");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const auto method=noUndefined();
    const auto bound=BaseMethods::bind(runtime,method,u"NoUndefined");
    const auto* identity=std::get_if<ObjectRef>(&bound.data);
    if(!identity)throw Exception("TypeError",u"NoUndefined method has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    auto& cx=runtime.context();const Value fallback=record(cx,{{u"macro",bound}});
    const Value options=record(cx,{{u"noundefined",record(cx,{{u"color",Value(u"red")},{u"background",Value(u"")},{u"size",Value(u"")}})}});
    return Configuration::create(runtime,u"noundefined",record(cx,{{u"fallback",fallback},{u"options",options},{u"priority",Value(3.0)}}));
}
}
