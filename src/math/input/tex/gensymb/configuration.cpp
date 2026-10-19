// Native port of MathJax 4.1.3 input/tex/gensymb/GensymbConfiguration.ts.
module folia.math.input.tex.gensymb.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& source){auto* value=dynamic_cast<TexParser*>(&source);if(!value)property_host::typeError("Gensymb requires a TexParser");return *value;}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> values){Value out(cx.object());for(const auto& [name,value]:values)cx.set(out,key(name),value);return out;}
FunctionRef mathcharUnit(){return makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(source);const auto* token=std::get_if<TokenRef>(&atom);if(!token||!*token)property_host::typeError("Cannot read properties of null or undefined (reading 'attributes')");
    Property attributes=(*token)->attributes();if(!property_host::truthy(attributes))attributes=Property(PropertyList{});
    auto record=PropertyList::record(attributes);if(!record)property_host::typeError("Cannot create property 'mathvariant' on a primitive value");
    const auto normal=property_host::get(property_host::get(TexConstant,"Variant"),"NORMAL");record->setOwn("mathvariant",normal);record->setOwn("class",Property("MathML-Unit"));
    const std::array<std::any,3> args{std::any(Property("mi")),std::any(attributes),std::any((*token)->character())};const auto node=p.create(u"token",FunctionArgs(args));p.Push(node);return Property(Missing{});
});}
}
ConfigurationRef GensymbConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"gensymb");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const Property mappings(PropertyList{{"ohm",Property("Ω")},{"degree",Property("°")},{"celsius",Property("℃")},{"perthousand",Property("‰")},{"micro",Property("µ")}});
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("gensymb-symbols"),ParseMethod{mathcharUnit()},mappings);
    auto& cx=runtime.context();const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"gensymb-symbols")})}});
    return Configuration::create(runtime,u"gensymb",record(cx,{{u"handler",handlers}}));
}
}
