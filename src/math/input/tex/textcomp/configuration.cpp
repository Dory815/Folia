// Native implementation of the two actual TextcompConfiguration records and
// its source CONFIG callback.
module folia.math.input.tex.textcomp.configuration;
import folia.math.input.tex.parser;
import folia.math.input.tex.textcomp.mappings;
import folia.math.input.tex.handler_types;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value array(Context& cx,std::initializer_list<Value> values){
    const ObjectRef result=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;
    for(const auto& value:values){const auto decimal=std::to_string(i++);cx.set(result,key(std::u16string(decimal.begin(),decimal.end())),value);}return Value(result);
}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){
    Value result(cx.object());for(const auto&[name,value]:fields)cx.set(result,key(name),value);return result;
}
Value configCallback(ParserRuntime& runtime){
    auto function=std::make_shared<Function>();
    function->sourceMetadata=FunctionSourceMetadata{u"",2};
    function->call=[&runtime](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"Textcomp CONFIG requires ParserConfiguration and TeX");
        const auto* configuration=std::any_cast<ParserConfigurationRef>(&args[0]);
        const auto* jaxPointer=std::any_cast<TeX*>(&args[1]);
        if(!configuration||!*configuration||!jaxPointer||!*jaxPointer)throw Exception("TypeError",u"Textcomp CONFIG arguments are invalid");
        TeX& jax=**jaxPointer;
        const auto parsed=jax.parseOptions();const auto* options=std::get_if<ParseOptionsRef>(&parsed);
        if(!options||!*options)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'packageData')");
        const auto* packages=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&(*options)->packageData);
        if(!packages||!*packages)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'get')");
        const auto text=(*packages)->get(u"textmacros");if(!text)return std::any(std::monostate{});
        Context& cx=runtime.context();
        const Value parseOptionsValue=cx.get(*text,key(u"parseOptions"));
        const auto* parseIdentity=std::get_if<ObjectRef>(&parseOptionsValue.data);
        if(!parseIdentity)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'options')");
        const auto textOptions=runtime.parseOptionsIdentities.resolve(*parseIdentity);
        const Value optionsRecord=cx.get(textOptions->options,key(u"textmacros"));
        const Value enabled=cx.get(optionsRecord,key(u"packages"));
        const auto* enabledIdentity=std::get_if<ObjectRef>(&enabled.data);
        if(!enabledIdentity)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'push')");
        const Value push=cx.get(*enabledIdentity,key(u"push"));const auto* pushFunction=std::get_if<ObjectRef>(&push.data);
        if(!pushFunction)property_host::typeError("packages.push is not callable");
        const std::array<Value,1> append{Value(u"text-textcomp")};(void)cx.call(*pushFunction,enabled,append);
        const Value textConfigurationValue=cx.get(*text,key(u"textConf"));
        const auto* identity=std::get_if<ObjectRef>(&textConfigurationValue.data);
        if(!identity)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'add')");
        const Value add=cx.get(*identity,key(u"add"));const auto* addFunction=std::get_if<ObjectRef>(&add.data);
        if(!addFunction)property_host::typeError("textConf.add is not callable");
        const std::array<Value,3> addArguments{Value(u"text-textcomp"),Value(jax.instanceIdentity()),Value(cx.object())};
        (void)cx.call(*addFunction,Value(*identity),addArguments);
        return std::any(std::monostate{});
    };
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver{std::move(receiver)};};
    binding.arguments=[&runtime](Context&,std::span<const Value> values){
        if(values.size()<2)throw Exception("TypeError",u"Textcomp CONFIG requires ParserConfiguration and TeX");
        const auto* configuration=std::get_if<ObjectRef>(&values[0].data);
        const auto* jax=std::get_if<ObjectRef>(&values[1].data);
        if(!configuration||!jax)throw Exception("TypeError",u"Textcomp CONFIG identities are invalid");
        return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*configuration)),
                                     std::any(runtime.texIdentities.resolve(*jax).get())};
    };
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    return runtime.functions().bind(FunctionRef(function),std::move(binding),u"TextcompConfiguration.config");
}
}
ConfigurationRef TextcompConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"textcomp");
    if(const auto* value=std::get_if<ConfigurationRef>(&loaded);value&&*value)return *value;
    TextcompMappings::initializeModule(runtime);
    Context& cx=runtime.context();
    const Value textHandlers=record(cx,{{u"macro",array(cx,{Value(u"textcomp-macros")})}});
    (void)Configuration::create(runtime,u"text-textcomp",record(cx,{{u"parser",Value(u"text")},{u"handler",textHandlers}}));
    const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"textcomp-macros")})}});
    return Configuration::create(runtime,u"textcomp",record(cx,{{u"handler",handlers},{u"config",configCallback(runtime)}}));
}
}
