// Native TextMacros package configuration; source algorithms and package
// identity wiring mirror TextMacrosConfiguration.ts.
module folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.parser;
import folia.math.input.tex.handler_types;
import folia.math.core.input_jax;
import folia.math.core.mml_tree.mml_factory;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
Value prop(const Property& value){
    if(value.missing())return Value(Undefined{});
    if(std::holds_alternative<PropertyNull>(value.value))return Value(Null{});
    if(const auto* p=std::get_if<bool>(&value.value))return Value(*p);
    if(const auto* p=std::get_if<double>(&value.value))return Value(*p);
    if(const auto* p=std::get_if<std::string>(&value.value))return Value(property_host::text(*p));
    throw Exception("Unimplemented",u"TextMacros option needs its canonical native identity");
}
Value array(Context& context,std::initializer_list<Value> values){
    const ObjectRef result=context.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;
    for(const auto& value:values){const auto ascii=std::to_string(i++);const std::u16string name(ascii.begin(),ascii.end());context.define(result,key(name),Descriptor::data(value));}
    return Value(result);
}
void put(Context& context,Value object,std::u16string_view name,Value value){context.set(std::move(object),key(name),std::move(value));}
std::shared_ptr<Function> makeCallback(std::function<std::any(FunctionArgs)> call,
        std::optional<FunctionSourceMetadata> sourceMetadata=std::nullopt){
    auto function=std::make_shared<Function>();function->call=[call=std::move(call)](const FunctionReceiver&,FunctionArgs args){return call(args);};
    function->sourceMetadata=std::move(sourceMetadata);return function;
}
Value bindCallback(ParserRuntime& runtime,FunctionRef body,std::u16string label){
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver)->FunctionReceiver{return FunctionReceiver{std::move(receiver)};};
    binding.arguments=[](Context&,std::span<const Value> arguments){
        std::vector<std::any> result;result.reserve(arguments.size());
        for(const auto& item:arguments)result.emplace_back(item);
        return result;
    };
    binding.result=[](Context&,const std::any& result){if(!result.has_value()||std::any_cast<std::monostate>(&result))return Value(Undefined{});if(const auto* v=std::any_cast<Value>(&result))return *v;return Value(Undefined{});};
    return runtime.functions().bind(std::move(body),std::move(binding),std::move(label));
}
Value sourceArgument(ParserRuntime& runtime,const std::any& argument){
    if(const auto* value=std::any_cast<Value>(&argument))return *value;
    if(const auto* parser=std::any_cast<std::reference_wrapper<ParserContext>>(&argument))
        return Value(parser->get().contextIdentity().object);
    if(const auto* parser=std::any_cast<TexParserRef>(&argument);parser&&*parser)
        return Value((*parser)->instanceIdentity());
    if(const auto* atom=std::any_cast<ParseAtom>(&argument))return std::visit([&](const auto& value)->Value{
        using T=std::decay_t<decltype(value)>;
        if constexpr(std::same_as<T,Property>)return runtime.properties.to(value);
        else if constexpr(std::same_as<T,TokenRef>)return runtime.tokenMaps.tokenLookupToValue(TokenLookup{value});
        else if constexpr(std::same_as<T,StackItemRef>){
            const auto identity=runtime.stackItemIdentities.find(value);
            if(!identity)throw Exception("Unimplemented",u"TextMacros ParseAtom stack item has no Context identity");
            return Value(*identity);
        }else if constexpr(std::same_as<T,NativeObjectRef>){
            const auto parser=std::dynamic_pointer_cast<TexParser>(value);
            if(parser){const auto identity=runtime.parserIdentities.find(parser);if(identity)return Value(*identity);}
            throw Exception("Unimplemented",u"TextMacros ParseAtom native object has no canonical Context identity");
        }
    },*atom);
    if(const auto* property=std::any_cast<Property>(&argument))return runtime.properties.to(*property);
    if(const auto* text=std::any_cast<std::u16string>(&argument))return Value(*text);
    throw Exception("TypeError",u"TextMacros callback argument has no Source value projection");
}
Value sourceArgumentAt(ParserRuntime& runtime,FunctionArgs args,std::size_t index){
    if(index>=args.size())return Value(Undefined{});
    return sourceArgument(runtime,args[index]);
}
ParserConfigurationRef parserConfiguration(ParserRuntime& runtime,Value packages){
    auto value=std::make_shared<ParserConfiguration>(runtime);value->initializeParserConfiguration(std::move(packages),Value(array(runtime.context(),{Value(u"tex"),Value(u"text")})));return value;
}
Value configurationObject(ParserRuntime& runtime,std::initializer_list<std::pair<std::u16string_view,Value>> values){
    Context& context=runtime.context();Value object(context.object());for(const auto& [name,value]:values)put(context,object,name,value);return object;
}
ObjectRef sourceCallable(Context& context,const Value& value,std::u16string_view expression){
    const auto* function=std::get_if<ObjectRef>(&value.data);
    if(!function||context.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",std::u16string(expression)+u" is not a function");
    return *function;
}
Value callSourceMethod(Context& context,const Value& function,const Value& receiver,
                       std::span<const Value> arguments,std::u16string_view expression){
    return context.call(sourceCallable(context,function,expression),receiver,arguments);
}
Value runTextMacrosPreprocessor(ParserRuntime& runtime,Value data){
    Context& context=runtime.context();
    const Value parseOptions=context.get(data,key(u"data"));
    const Value packageData=context.get(parseOptions,key(u"packageData"));
    const Value getMethod=context.get(packageData,key(u"get"));
    const std::array<Value,1> packageArguments{Value(u"textmacros")};
    const Value config=callSourceMethod(context,getMethod,packageData,packageArguments,u"data.data.packageData.get");

    const Value parseOptionsForFactory=context.get(config,key(u"parseOptions"));
    const Value nodeFactory=context.get(parseOptionsForFactory,key(u"nodeFactory"));
    const Value setMmlFactory=context.get(nodeFactory,key(u"setMmlFactory"));
    const Value jax=context.get(config,key(u"jax"));
    const Value mmlFactory=context.get(jax,key(u"mmlFactory"));
    const std::array<Value,1> factoryArguments{mmlFactory};
    (void)callSourceMethod(context,setMmlFactory,nodeFactory,factoryArguments,
                           u"config.parseOptions.nodeFactory.setMmlFactory");

    // Source evaluates config.parseOptions again after setMmlFactory returns;
    // this intentionally observes a replacement getter or clear method.
    const Value parseOptionsForClear=context.get(config,key(u"parseOptions"));
    const Value clearMethod=context.get(parseOptionsForClear,key(u"clear"));
    (void)callSourceMethod(context,clearMethod,parseOptionsForClear,{},u"config.parseOptions.clear");
    return Value(Undefined{});
}
Value sourcePreprocessorArgument(FunctionArgs args){
    if(args.empty())return Value(Undefined{});
    if(const auto* value=std::any_cast<Value>(&args[0]))return *value;
    if(const auto* record=std::any_cast<core::InputJaxFilterDataRef>(&args[0]);record&&*record)
        return Value((*record)->sourceIdentity());
    throw Exception("TypeError",u"TextMacros PREPROCESSOR data is invalid");
}
Value boundMacroFallback(ParserRuntime& runtime){
    auto body=makeCallback([&runtime](FunctionArgs args)->std::any{
        const Value parser=sourceArgumentAt(runtime,args,0);
        const Value name=sourceArgumentAt(runtime,args,1);
        Context& context=runtime.context();
        const Value kind=runtime.properties.to(HandlerType.get("MACRO"));
        const Value texParser=context.get(parser,key(u"texParser"));
        const Value lookup=context.get(texParser,key(u"lookup"));
        const std::array<Value,2> lookupArguments{kind,name};
        const Value macro=callSourceMethod(context,lookup,texParser,lookupArguments,u"texParser.lookup");
        if(options::truthy(macro)){
            const Value actual=context.get(macro,key(u"_func"));
            const Property canonical=TextMacrosMethods::get(runtime,u"Macro");
            const Value expected=runtime.tokenMaps.parseMethodToValue(functions::fromProperty(canonical));
            if(actual!=expected){
                const Value error=context.get(parser,key(u"Error"));
                const Value macroName=property_host::add(context,Value(u"\\"),name);
                const std::array<Value,3> errorArguments{Value(u"MathMacro"),Value(u"%1 is only supported in math mode"),macroName};
                (void)callSourceMethod(context,error,parser,errorArguments,u"parser.Error");
            }
        }
        const Value parse=context.get(texParser,key(u"parse"));
        const Value input=array(context,{parser,name});
        const std::array<Value,2> parseArguments{kind,input};
        (void)callSourceMethod(context,parse,texParser,parseArguments,u"texParser.parse");
        return std::any(Undefined{});
    },FunctionSourceMetadata{u"",2});
    return bindCallback(runtime,std::move(body),u"");
}
}

void TextMacrosConfiguration::initializeModule(ParserRuntime& runtime){
    TextMacrosMappings::initializeModule(runtime);
    Context& context=runtime.context();
    const Value characterMaps=array(context,{Value(u"command"),Value(u"text-special")});
    const Value macroMaps=array(context,{Value(u"text-macros")});
    const Value handlers=configurationObject(runtime,{{u"character",characterMaps},{u"macro",macroMaps}});
    const Value fallbacks=configurationObject(runtime,{
        {u"character",bindCallback(runtime,makeCallback([&runtime](FunctionArgs args)->std::any{
            const Value parser=sourceArgumentAt(runtime,args,0);
            const Value character=sourceArgumentAt(runtime,args,1);
            Context& context=runtime.context();
            const Value current=context.get(parser,key(u"text"));
            context.set(parser,key(u"text"),property_host::add(context,current,character));
            return std::any(std::monostate{});
        },FunctionSourceMetadata{u"",2}),u"")},
        {u"macro",boundMacroFallback(runtime)}});
    Value items(context.object());
    const Value base=runtime.baseItemsModule;
    for(const auto name:{u"StartItem",u"StopItem",u"MmlItem",u"StyleItem"}){
        const Value constructor=context.get(base,key(name));
        const Value prototype=context.get(constructor,key(u"prototype"));
        const Value kind=context.get(prototype,key(u"kind"));
        const auto* kindName=std::get_if<std::u16string>(&kind.data);
        if(!kindName)throw Exception("TypeError",u"BaseItem.prototype.kind must be a string");
        context.set(items,key(*kindName),constructor);
    }
    Value baseConfig=configurationObject(runtime,{
        {u"parser",Value(u"text")},{u"priority",Value(1.0)},{u"handler",handlers},
        {u"fallback",fallbacks},{u"items",items}});
    (void)Configuration::create(runtime,u"text-base",baseConfig);

    auto configBody=makeCallback([&runtime](FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"TextMacros CONFIG requires ParserConfiguration and TeX");
        const auto* config=std::any_cast<ParserConfigurationRef>(&args[0]);
        const auto* jaxPointer=std::any_cast<TeX*>(&args[1]);
        if(!config||!*config||!jaxPointer||!*jaxPointer)throw Exception("TypeError",u"TextMacros CONFIG arguments are invalid");
        TeX& jax=**jaxPointer;Context& context=runtime.context();
        const ParseOptionsRef outer=[&](){const auto value=jax.parseOptions();const auto* result=std::get_if<ParseOptionsRef>(&value);if(!result||!*result)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'options')");return *result;}();
        const Value definition=context.get(outer->options,key(u"textmacros"));
        const Value packages=context.get(definition,key(u"packages"));
        auto textConf=parserConfiguration(runtime,packages);textConf->init();
        auto inner=ParseOptions::make(runtime,*textConf,Value(context.array()));
        inner->options=outer->options;
        textConf->config(jax);
        runtime.tagsFactory.addTags(textConf->sourceField(u"tags"));
        const TagsRef tags=runtime.tagsFactory.getDefault();inner->setTags(tags);
        if(!tags)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'configuration')");
        const auto innerIdentity=runtime.parseOptionsIdentities.find(inner);if(!innerIdentity)throw Exception("TypeError",u"ParseOptions has no Context identity");
        auto* abstractTags=dynamic_cast<AbstractTags*>(tags.get());if(!abstractTags)throw Exception("TypeError",u"TextMacros default Tags instance is not AbstractTags");
        abstractTags->setField(u"configuration",Value(*innerIdentity));
        inner->packageData=outer->packageData;
        const auto* packageData=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&outer->packageData);
        if(!packageData||!*packageData)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'set')");
        const auto jaxIdentity=jax.instanceIdentity();
        Value record(context.object());
        const auto textConfIdentity=runtime.parserConfigurationIdentities.find(textConf);
        if(!textConfIdentity)throw Exception("TypeError",u"TextMacros ParserConfiguration has no Context identity");
        put(context,record,u"textConf",Value(*textConfIdentity));
        put(context,record,u"parseOptions",Value(*innerIdentity));
        put(context,record,u"jax",Value(jaxIdentity));put(context,record,u"texParser",Value(Null{}));
        (*packageData)->set(u"textmacros",record);
        put(context,outer->options,u"internalMath",TextParser::internalMathFunction(runtime));
        return std::any(std::monostate{});
    },FunctionSourceMetadata{u"",2});
    OptionCallBinding configBinding;
    configBinding.receiver=[](Context&,Value receiver){return FunctionReceiver{std::move(receiver)};};
    configBinding.arguments=[&runtime](Context&,std::span<const Value> arguments){
        if(arguments.size()<2)throw Exception("TypeError",u"TextMacros CONFIG requires ParserConfiguration and TeX");
        const auto* config=std::get_if<ObjectRef>(&arguments[0].data);
        const auto* jax=std::get_if<ObjectRef>(&arguments[1].data);
        if(!config||!jax)throw Exception("TypeError",u"TextMacros CONFIG identities are invalid");
        return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*config)),
                                     std::any(runtime.texIdentities.resolve(*jax).get())};
    };
    configBinding.result=[](Context&,const std::any& result){
        if(!result.has_value()||std::any_cast<std::monostate>(&result))return Value(Undefined{});
        if(const auto* value=std::any_cast<Value>(&result))return *value;
        return Value(Undefined{});
    };
    Value configCallback=runtime.functions().bind(std::move(configBody),std::move(configBinding),u"textMacrosConfig");

    auto preprocessor=makeCallback([&runtime](FunctionArgs args)->std::any{
        (void)runTextMacrosPreprocessor(runtime,sourcePreprocessorArgument(args));
        return std::any(std::monostate{});
    },FunctionSourceMetadata{u"",1});
    Value preprocessors=array(context,{bindCallback(runtime,std::move(preprocessor),u"textMacrosPreprocessor")});
    Value innerOptions=configurationObject(runtime,{{u"packages",array(context,{Value(u"text-base")})}});
    Value options=configurationObject(runtime,{{u"textmacros",innerOptions}});
    Value packageConfig=configurationObject(runtime,{{u"priority",Value(1.0)},{u"config",configCallback},
        {u"preprocessors",preprocessors},{u"options",options}});
    (void)Configuration::create(runtime,u"textmacros",packageConfig);
}
}
