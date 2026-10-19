// Native callback body for the pinned TextcompMappings.Insert method.
module folia.math.input.tex.textcomp.methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.textmacros.methods;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
std::u16string text(const Property& value){return property_host::string(value);}
Property argument(const ParseAtom& atom){
    if(const auto* value=std::get_if<Property>(&atom))return *value;
    if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)return (*token)->token();
    property_host::typeError("Textcomp callback received an invalid macro name");
}
Property raw(const ParseRest& args,std::size_t i){
    if(i>=args.size())return Missing{};
    if(const auto* value=std::any_cast<Property>(&args[i]))return *value;
    if(const auto* value=std::any_cast<std::u16string>(&args[i]))return property_host::debugText(*value);
    if(const auto* value=std::any_cast<double>(&args[i]))return *value;
    if(const auto* value=std::any_cast<bool>(&args[i]))return *value;
    return Missing{};
}
}
Property TextcompMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.textcompInsert.missing())return runtime.textcompInsert;
    auto body=[&runtime](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest args)->ParseResult{
        auto* parser=dynamic_cast<TexParser*>(&source);
        if(!parser)property_host::typeError("Textcomp Insert requires a TexParser");
        const Property character=raw(args,0);
        const Property font=raw(args,1);
        if(auto* textParser=dynamic_cast<TextParser*>(parser)){
            if(font.missing()||!property_host::truthy(font)){
                const Property insert=TextMacrosMethods::get(runtime,u"Insert");
                const auto* function=std::get_if<FunctionRef>(&insert.value);
                if(!function||!*function)property_host::typeError("TextMacros.Insert is unavailable");
                const std::array<std::any,1> forwarded{std::any(character)};
                (void)invokeParseMethod(ParseMethod{*function},FunctionReceiver{},*textParser,atom,forwarded);
                return Property(Missing{});
            }
            textParser->saveText();
        }
        const Property definition=property_host::truthy(font)
            ? Property(PropertyList{{"mathvariant",font}})
            : Property(PropertyList{});
        parser->Push(ParseUtil::internalText(*parser,text(character),definition));
        return Property(Missing{});
    };
    const FunctionRef function=makeParseCallable(std::move(body));
    const Value bound=BaseMethods::bind(runtime,function,u"Insert");
    const auto* identity=std::get_if<ObjectRef>(&bound.data);
    if(!identity)throw Exception("TypeError",u"Textcomp Insert method has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(4.0),false,false,true));
    runtime.textcompInsert=Property(function);
    return runtime.textcompInsert;
}
}
