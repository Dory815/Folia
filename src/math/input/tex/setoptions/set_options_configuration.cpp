// Native port of the single Source owner SetOptionsConfiguration.ts.
module folia.math.input.tex.setoptions.set_options_configuration;
import folia.math.input.tex.parser;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.input.tex.macro;
import folia.math.input.tex.base_methods;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import folia.math.util.options;
import folia.math.util.regexp_binding;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view text){return Key(std::u16string(text));}
Value string(std::u16string_view text){return Value(std::u16string(text));}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){
    const ObjectRef out=cx.object();for(const auto& [name,value]:fields)cx.set(Value(out),key(name),value);return Value(out);
}
Value array(Context& cx,std::initializer_list<Value> fields){
    const ObjectRef out=cx.array(static_cast<std::uint32_t>(fields.size()));std::uint32_t i=0;
    for(const auto& value:fields){const auto index=std::to_string(i++);cx.set(Value(out),key(std::u16string(index.begin(),index.end())),value);}return Value(out);
}
TexParser& parser(ParserContext& source){auto* result=dynamic_cast<TexParser*>(&source);if(!result)property_host::typeError("SetOptions requires TexParser");return *result;}
ParseOptions& parseOptions(TexParser& value){
    const auto* options=std::get_if<ParseOptions*>(&value.configuration);
    if(!options||!*options)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
    return **options;
}
Value callOption(Context& cx,const Value& receiver,std::u16string_view name,std::span<const Value> args){
    const Value callable=cx.get(receiver,key(name));const auto* function=std::get_if<ObjectRef>(&callable.data);
    if(!function||cx.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",std::u16string(u"config.")+std::u16string(name)+u" is not a function");
    return cx.call(*function,receiver,args);
}
std::vector<std::u16string> enumerableKeys(Context& cx,const Value& object){
    if(std::holds_alternative<Null>(object.data)||std::holds_alternative<Undefined>(object.data))
        property_host::typeError("Cannot convert undefined or null to object");
    std::vector<std::u16string> result;
    for(const auto& propertyKey:cx.ownKeys(object)){
        const auto* name=std::get_if<std::u16string>(&propertyKey.data);if(!name)continue;
        const auto descriptor=cx.descriptor(object,propertyKey);if(descriptor&&descriptor->enumerable)result.push_back(*name);
    }
    return result;
}
std::u16string text(const Property& value){return property_host::string(value);}
ParseResult setOptions(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){
    auto& p=parser(source);const std::u16string name=std::holds_alternative<Property>(atom)?text(std::get<Property>(atom)):
        property_host::string(std::get<TokenRef>(atom)->character());
    Property extensionValue=p.GetBrackets(name);
    std::u16string extension=property_host::truthy(extensionValue)?text(extensionValue):u"tex";
    const Property rawOptions=p.GetArgument(name);
    ParseOptions& parse=parseOptions(p);ParserRuntime& runtime=parse.runtime();Context& cx=runtime.context();
    const Property parsed=ParseUtil::keyvalOptions(runtime,property_host::string(rawOptions));
    const Value options=p.options();const Value config=cx.get(options,key(u"setoptions"));
    const std::array<Value,2> packageArgs{Value(p.instanceIdentity()),Value(extension)};
    if(!options::truthy(callOption(cx,config,u"filterPackage",packageArgs)))return Property(Missing{});
    const Value definitions=runtime.properties.to(parsed);
    const Value packageOptions=extension==u"tex"?options:cx.get(options,key(extension));
    for(const auto& option:enumerableKeys(cx,definitions)){
        const std::array<Value,3> optionArgs{Value(p.instanceIdentity()),Value(extension),Value(option)};
        if(!options::truthy(callOption(cx,config,u"filterOption",optionArgs)))continue;
        const Value rawValue=cx.get(definitions,key(option));
        const std::array<Value,4> valueArgs{Value(p.instanceIdentity()),Value(extension),Value(option),rawValue};
        const Value filtered=callOption(cx,config,u"filterValue",valueArgs);
        cx.set(packageOptions,key(option),filtered);
    }
    const auto factory=p.itemFactory();const auto* itemFactory=std::get_if<std::shared_ptr<StackItemFactory>>(&factory);
    if(!itemFactory||!*itemFactory)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");
    BaseItem* nullItem=(**itemFactory).itemFactory().create("null",tree::FactoryArgs{});
    if(!nullItem)property_host::typeError("NullItem constructor is not registered");
    p.Push((**itemFactory).reference(nullItem));
    return Property(Missing{});
}
FunctionRef setOptionsFunction(){return makeParseCallable(setOptions);}
Value configCallback(ParserRuntime& runtime,FunctionRef setOptionsMethod){
    auto body=std::make_shared<Function>();
    body->sourceMetadata=FunctionSourceMetadata{u"setoptionsConfig",2};
    body->call=[&runtime,setOptionsMethod=std::move(setOptionsMethod)](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"SetOptions config arguments are missing");
        const auto* jax=std::any_cast<TeX*>(&args[1]);if(!jax||!*jax)throw Exception("TypeError",u"SetOptions config TeX is invalid");
        const auto parse=(*jax)->parseOptions();const auto* options=std::get_if<std::shared_ptr<ParseOptions>>(&parse);
        if(!options||!*options)property_host::typeError("Cannot read properties of null or undefined (reading 'handlers')");
        const auto* handlers=std::get_if<std::shared_ptr<SubHandlers>>(&(*options)->handlers);
        if(!handlers||!*handlers)property_host::typeError("Cannot read properties of null or undefined (reading 'get')");
        auto macroHandler=(*handlers)->get("macro");if(!macroHandler)property_host::typeError("Cannot read properties of undefined (reading 'add')");
        Context& cx=runtime.context();const auto names=array(cx,{Value(u"setoptions")});
        (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("setoptions"),Property(PropertyList{{"setOptions",Property(setOptionsMethod)}}));
        macroHandler->add(cx,names,Property(Missing{}),PrioritizedList<TokenMapRef>::DEFAULTPRIORITY-1);
        const auto found=macroHandler->lookup(Property("require"));const auto* require=std::get_if<MacroRef>(&found);
        if(require&&*require){
            const auto map=runtime.mapHandler.getMap(Property("setoptions"));auto* setMap=dynamic_cast<CommandMap*>(map.get());
            if(!setMap)property_host::typeError("SetOptions CommandMap is not registered");
            setMap->add(Property("Require"),std::make_shared<Macro>(Property("Require"),(*require)->func()));
            const Property base=BaseMethods::get(runtime,u"Macro");const auto* baseFn=std::get_if<FunctionRef>(&base.value);
            if(!baseFn||!*baseFn)property_host::typeError("BaseMethods.Macro is not callable");
            const Property args(PropertyArray{Property("\\Require{#2}\\setOptions[#2]{#1}"),Property(2.0),Property("")});
            setMap->add(Property("require"),std::make_shared<Macro>(Property("require"),*baseFn,args));
        }
        return std::monostate{};
    };
    OptionCallBinding binding;binding.receiver=[](Context&,Value value){return FunctionReceiver(std::move(value));};
    binding.arguments=[&runtime](Context&,std::span<const Value> args){
        if(args.size()<2)throw Exception("TypeError",u"SetOptions config arguments are missing");
        const auto* configuration=std::get_if<ObjectRef>(&args[0].data);const auto* jax=std::get_if<ObjectRef>(&args[1].data);
        if(!configuration||!jax)throw Exception("TypeError",u"SetOptions config identities are invalid");
        return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*configuration)),std::any(runtime.texIdentities.resolve(*jax).get())};
    };
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    const Value result=runtime.functions().bind(body,std::move(binding),u"setoptionsConfig");
    if(const auto* identity=std::get_if<ObjectRef>(&result.data))runtime.context().define(*identity,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    return result;
}
Value utilObject(ParserRuntime& runtime,Context& cx){
    auto parserBinding=[&runtime](Context&,std::span<const Value> args)->TexParser*{
        if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'options')");
        const auto* identity=std::get_if<ObjectRef>(&args[0].data);if(!identity)throw Exception("TypeError",u"SetOptions parser is not an object");
        return runtime.parserIdentities.resolve(*identity).get();
    };
    auto bind=[&](std::u16string name,FunctionRef body,std::size_t count){
        OptionCallBinding binding;binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
        binding.arguments=[parserBinding,count](Context& context,std::span<const Value> args){
            if(args.size()<count)throw Exception("TypeError",u"SetOptions method arguments are missing");
            std::vector<std::any> result;result.reserve(count);
            result.emplace_back(parserBinding(context,args));
            for(std::size_t i=1;i<count;++i){
                if(i+1==count&&count==4){result.emplace_back(args[i]);continue;}
                const auto* text=std::get_if<std::u16string>(&args[i].data);if(!text)property_host::typeError("SetOptions argument is not a string");result.emplace_back(*text);
            }
            return result;
        };
        binding.result=[](Context&,const std::any& result){
            if(const auto* boolean=std::any_cast<bool>(&result))return Value(*boolean);
            if(const auto* value=std::any_cast<Value>(&result))return *value;
            throw Exception("TypeError",u"SetOptions method returned an invalid value");
        };
        const Value bound=runtime.functions().bind(std::move(body),std::move(binding),name);
        if(const auto* identity=std::get_if<ObjectRef>(&bound.data))
            cx.define(*identity,key(u"length"),Descriptor::data(Value(static_cast<double>(count)),false,false,true));
        return bound;
    };
    auto packageBody=std::make_shared<Function>();packageBody->call=[](const FunctionReceiver&,FunctionArgs a)->std::any{return SetOptionsUtil::filterPackage(*std::any_cast<TexParser*>(a[0]),std::any_cast<const std::u16string&>(a[1]));};
    auto optionBody=std::make_shared<Function>();optionBody->call=[](const FunctionReceiver&,FunctionArgs a)->std::any{return SetOptionsUtil::filterOption(*std::any_cast<TexParser*>(a[0]),std::any_cast<const std::u16string&>(a[1]),std::any_cast<const std::u16string&>(a[2]));};
    auto valueBody=std::make_shared<Function>();valueBody->call=[](const FunctionReceiver&,FunctionArgs a)->std::any{return SetOptionsUtil::filterValue(*std::any_cast<TexParser*>(a[0]),std::any_cast<const std::u16string&>(a[1]),std::any_cast<const std::u16string&>(a[2]),std::any_cast<const Value&>(a[3]));};
    const Value package=bind(u"filterPackage",packageBody,2);
    const Value option=bind(u"filterOption",optionBody,3);
    const Value value=bind(u"filterValue",valueBody,4);
    return record(cx,{{u"filterPackage",package},{u"filterOption",option},{u"filterValue",value},
      {u"allowPackageDefault",Value(true)},{u"allowOptionsDefault",Value(true)},
      {u"allowOptions",options::expandable(cx,record(cx,{{u"tex",record(cx,{{u"FindTeX",Value(false)},{u"formatError",Value(false)},{u"package",Value(false)},{u"baseURL",Value(false)},{u"tags",Value(false)},{u"maxBuffer",Value(false)},{u"maxMaxros",Value(false)},{u"macros",Value(false)},{u"environments",Value(false)}})},
        {u"setoptions",Value(false)},{u"autoload",Value(false)},{u"require",Value(false)},{u"configmacros",Value(false)},{u"tagformat",Value(false)}}))}});
}
}
bool SetOptionsUtil::filterPackage(TexParser& parserValue,std::u16string_view extension){
    auto& parse=parseOptions(parserValue);auto& runtime=parse.runtime();Context& cx=runtime.context();const Value opts=parserValue.options();const Value config=cx.get(opts,key(u"setoptions"));
    if(extension!=u"tex"){
        const auto registered=runtime.configurationHandler.get(extension);
        const auto* value=std::get_if<std::shared_ptr<Configuration>>(&registered);
        if(!value||!*value)throw TexError(u"NotAPackage",u"Not a defined package: %1",std::array<Property,1>{property_host::debugText(extension)});
    }
    const Value allowOptions=cx.get(config,key(u"allowOptions"));const Value allowed=cx.get(allowOptions,key(extension));
    const Value packageDefault=cx.get(config,key(u"allowPackageDefault"));
    if((std::holds_alternative<Undefined>(allowed.data)&&!options::truthy(packageDefault))||
        (std::holds_alternative<bool>(allowed.data)&&!std::get<bool>(allowed.data)))
        throw TexError(u"PackageNotSettable",u"Options can't be set for package \"%1\"",std::array<Property,1>{property_host::debugText(extension)});
    return true;
}
bool SetOptionsUtil::filterOption(TexParser& parserValue,std::u16string_view extension,std::u16string_view option){
    auto& parse=parseOptions(parserValue);Context& cx=parse.runtime().context();const Value opts=parserValue.options();const Value config=cx.get(opts,key(u"setoptions"));
    const Value allowedPackages=cx.get(config,key(u"allowOptions"));Value allowed=cx.get(allowedPackages,key(extension));
    if(!options::truthy(allowed))allowed=Value(cx.object());
    const bool own=cx.hasOwn(allowed,key(option));const Value candidate=own?cx.get(allowed,key(option)):Value(Null{});
    const Value allowedOption=own&&!options::isObject(cx,candidate)?candidate:Value(Null{});
    const Value allowDefault=cx.get(config,key(u"allowOptionsDefault"));
    if((std::holds_alternative<bool>(allowedOption.data)&&!std::get<bool>(allowedOption.data))||
       (std::holds_alternative<Null>(allowedOption.data)&&!options::truthy(allowDefault))){
        if(extension==u"tex")throw TexError(u"TeXOptionNotSettable",u"Option \"%1\" is not allowed to be set",std::array<Property,1>{property_host::debugText(option)});
        throw TexError(u"OptionNotSettable",u"Option \"%1\" is not allowed to be set for package %2",std::array<Property,2>{property_host::debugText(option),property_host::debugText(extension)});
    }
    const Value target=extension==u"tex"?opts:cx.get(opts,key(extension));
    if(!options::truthy(target)||!cx.hasOwn(target,key(option))){
        if(extension==u"tex")throw TexError(u"InvalidTexOption",u"Invalid TeX option \"%1\"",std::array<Property,1>{property_host::debugText(option)});
        throw TexError(u"InvalidOptionKey",u"Invalid option \"%1\" for package \"%2\"",std::array<Property,2>{property_host::debugText(option),property_host::debugText(extension)});
    }
    return true;
}
Value SetOptionsUtil::filterValue(TexParser& parserValue,std::u16string_view extension,std::u16string_view option,Value value){
    (void)extension;(void)option;auto& runtime=parseOptions(parserValue).runtime();
    const auto* source=std::get_if<std::u16string>(&value.data);if(!source)return value;
    regexp_host::RegExp matcher(u"^/(.*)/([dgimsuvy]*)$");const auto match=matcher.exec(*source);if(!match)return value;
    const auto pattern=match->at(1),flags=match->at(2);if(!pattern||!flags)return value;
    return Value(runtime.regexp().create(*pattern,*flags));
}
ConfigurationRef SetOptionsConfiguration::initializeModule(ParserRuntime& runtime){
    const auto prior=runtime.configurationHandler.get(u"setoptions");if(const auto* value=std::get_if<ConfigurationRef>(&prior);value&&*value)return *value;
    Context& cx=runtime.context();
    const FunctionRef setOptionsMethod=setOptionsFunction();
    const Value setOptionsIdentity=BaseMethods::bind(runtime,setOptionsMethod,u"SetOptions");
    if(const auto* identity=std::get_if<ObjectRef>(&setOptionsIdentity.data))cx.define(*identity,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    else throw Exception("TypeError",u"SetOptions function identity is unavailable");
    const Value setoptions=utilObject(runtime,cx);
    const Value opts=record(cx,{{u"setoptions",setoptions}});
    return Configuration::create(runtime,u"setoptions",record(cx,{{u"options",opts},{u"config",configCallback(runtime,setOptionsMethod)},{u"priority",Value(3.0)}}));
}
}
