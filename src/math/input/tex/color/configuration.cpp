// Native port of MathJax 4.1.3 input/tex/color/ColorConfiguration.ts.
module folia.math.input.tex.color.configuration;
import folia.math.input.tex.color.util;
import folia.math.input.tex.color.constants;
import folia.math.input.tex.color.methods;
import folia.math.input.tex.token_map;
import folia.math.input.tex.handler_types;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
using namespace options;
namespace {
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const auto o=cx.object();for(const auto&[name,value]:fields)cx.set(o,key(name),value);return Value(o);}
Value array(Context& cx,std::initializer_list<Value> fields){const auto o=cx.array(fields.size());std::size_t i=0;for(const auto& v:fields){auto n=std::to_string(i++);cx.set(o,key(std::u16string(n.begin(),n.end())),v);}return Value(o);}
}
ConfigurationRef ColorConfiguration::initializeModule(ParserRuntime& runtime,
    std::shared_ptr<MapContextBindings> maps){
    const auto loaded=runtime.configurationHandler.get(u"color");
    if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    auto& cx=runtime.context();
    if(!maps||&maps->context()!=&cx)
        throw Exception("TypeError",u"Color package requires the session Map bindings");
    const auto constants=ColorConstants::createMap(maps);
    const auto colorUtil=ColorUtil::initializeModule(runtime,maps,constants);
    const Property methods=ColorMethods::initializeModule(runtime);
    const auto methodTable=PropertyList::record(methods);
    if(!methodTable)throw Exception("TypeError",u"Color methods did not return an object");
    auto method=[&](std::u16string_view name){
        const Property value=methodTable->get(property_host::debugText(name));
        const auto* function=std::get_if<FunctionRef>(&value.value);
        if(!function||!*function)throw Exception("TypeError",u"Color method is unavailable");
        return *function;
    };
    PropertyList commands;
    commands.setOwn("color",Property(method(u"Color")));
    commands.setOwn("textcolor",Property(method(u"TextColor")));
    commands.setOwn("definecolor",Property(method(u"DefineColor")));
    commands.setOwn("colorbox",Property(method(u"ColorBox")));
    commands.setOwn("fcolorbox",Property(method(u"FColorBox")));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("color"),Property(std::move(commands)));
    auto constructorRef=std::make_shared<ObjectRef>(colorUtil.constructor);
    auto callback=std::make_shared<Function>();callback->sourceMetadata=FunctionSourceMetadata{u"config",2};callback->call=[&runtime,constructorRef](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"Color config requires configuration and TeX");const auto* jax=std::any_cast<TeX*>(&args[1]);if(!jax||!*jax)throw Exception("TypeError",u"Color config TeX is invalid");
        const auto parse=(*jax)->parseOptions();const auto* options=std::get_if<std::shared_ptr<ParseOptions>>(&parse);if(!options||!*options)property_host::typeError("Cannot read properties of null or undefined (reading 'packageData')");
        auto& cx=runtime.context();const ObjectRef identity=cx.construct(*constructorRef);
        const auto package=record(cx,{{u"model",Value(identity)}});auto* data=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&(*options)->packageData);if(!data||!*data)property_host::typeError("Cannot read properties of null or undefined (reading 'set')");(*data)->set(u"color",package);return std::monostate{};
    };
    OptionCallBinding binding;binding.receiver=[](Context&,Value value){return FunctionReceiver(std::move(value));};
    binding.arguments=[&runtime](Context&,std::span<const Value> args){if(args.size()<2)throw Exception("TypeError",u"Color config arguments are missing");const auto* cfg=std::get_if<ObjectRef>(&args[0].data);const auto* jax=std::get_if<ObjectRef>(&args[1].data);if(!cfg||!jax)throw Exception("TypeError",u"Color config identities are invalid");return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*cfg)),std::any(runtime.texIdentities.resolve(*jax).get())};};
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};const auto configFn=runtime.functions().bind(std::move(callback),std::move(binding),u"ColorConfig");
    const auto handler=record(cx,{{u"macro",array(cx,{Value(u"color")})}});
    const auto options=record(cx,{{u"color",record(cx,{{u"padding",Value(u"5px")},{u"borderWidth",Value(u"2px")}})}});
    return Configuration::create(runtime,u"color",record(cx,{{u"handler",handler},{u"options",options},{u"config",configFn}}));
}
}
