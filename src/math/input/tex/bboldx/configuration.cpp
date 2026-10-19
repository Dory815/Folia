// Native port of MathJax 4.1.3 input/tex/bboldx/BboldxConfiguration.ts.
module folia.math.input.tex.bboldx.configuration;
import folia.math.input.tex.bboldx.mappings;
import folia.math.input.tex.handler_types;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace { using namespace options;
Key key(std::u16string_view s){return Key(std::u16string(s));}
Value record(Context& c,std::initializer_list<std::pair<std::u16string_view,Value>> fields){Value r(c.object());for(const auto&[k,v]:fields)c.set(r,key(k),v);return r;}
Value array(Context& c,std::initializer_list<Value> fields){Value a(c.array(static_cast<std::uint32_t>(fields.size())));std::uint32_t i=0;for(const auto&v:fields){const auto digits=std::to_string(i++);c.set(a,key(std::u16string(digits.begin(),digits.end())),v);}return a;}
std::shared_ptr<Function> callback(std::function<std::any(FunctionArgs)> body){auto f=std::make_shared<Function>();f->sourceMetadata=FunctionSourceMetadata{u"",2};f->call=[body=std::move(body)](const FunctionReceiver&,FunctionArgs args){return body(std::move(args));};return f;}
Value bind(ParserRuntime& runtime,FunctionRef body){OptionCallBinding b;b.receiver=[](Context&,Value r){return FunctionReceiver(std::move(r));};b.arguments=[&runtime](Context&,std::span<const Value> a){if(a.size()<2)throw Exception("TypeError",u"Bboldx CONFIG requires Configuration and TeX");const auto* configuration=std::get_if<ObjectRef>(&a[0].data);const auto* jax=std::get_if<ObjectRef>(&a[1].data);if(!configuration||!jax)throw Exception("TypeError",u"Bboldx CONFIG identities are invalid");return std::vector<std::any>{runtime.parserConfigurationIdentities.resolve(*configuration),runtime.texIdentities.resolve(*jax).get()};};b.result=[](Context&,const std::any&){return Value(Undefined{});};return runtime.functions().bind(std::move(body),std::move(b),u"BboldxConfig");}
}
ConfigurationRef BboldxConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"bboldx");if(const auto*p=std::get_if<ConfigurationRef>(&loaded);p&&*p)return *p;
    BboldxMappings::initializeModule(runtime);auto&cx=runtime.context();
    const Value textHandlers=record(cx,{{u"macro",array(cx,{Value(u"text-bboldx"),Value(u"text-bboldx-mathchar0miNormal"),Value(u"text-bboldx-delimiterNormal"),Value(u"text-bboldx-mathchar0miBold"),Value(u"text-bboldx-delimiterBold")})}});
    (void)Configuration::create(runtime,u"text-bboldx",record(cx,{{u"parser",Value(u"text")},{u"handler",textHandlers}}));
    auto body=callback([&runtime](FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"Bboldx CONFIG requires Configuration and TeX");
        const auto* config=std::any_cast<ParserConfigurationRef>(&args[0]);const auto* jax=std::any_cast<TeX*>(&args[1]);
        if(!config||!*config||!jax||!*jax)throw Exception("TypeError",u"Bboldx CONFIG arguments are invalid");
        const auto optionsValue=(**jax).parseOptions();const auto* parse=std::get_if<ParseOptionsRef>(&optionsValue);
        if(!parse||!*parse)property_host::typeError("Cannot read properties of null or undefined (reading 'packageData')");
        const auto* data=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&(*parse)->packageData);
        if(!data||!*data)property_host::typeError("Cannot read properties of null or undefined (reading 'get')");
        const auto package=(*data)->get(u"textmacros");if(!package)return std::any(std::monostate{});
        Context&cx=runtime.context();const Value recordValue=*package;const Value packages=cx.get(cx.get(recordValue,key(u"parseOptions")),key(u"options"));
        const Value textOptions=cx.get(packages,key(u"textmacros"));const Value list=cx.get(textOptions,key(u"packages"));
        const auto push=cx.get(list,key(u"push"));const auto* pushFunction=std::get_if<ObjectRef>(&push.data);if(!pushFunction)property_host::typeError("packages.push is not callable");const std::array<Value,1> append{Value(u"text-bboldx")};cx.call(*pushFunction,list,append);
        const Value textConfValue=cx.get(recordValue,key(u"textConf"));const auto* textConf=std::get_if<ObjectRef>(&textConfValue.data);
        if(!textConf)property_host::typeError("Cannot read properties of undefined (reading 'add')");
        auto owner=runtime.parserConfigurationIdentities.resolve(*textConf);owner->add(u"text-bboldx",**jax,Value(cx.object()));
        return std::any(std::monostate{});
    });
    Value configCallback=bind(runtime,body);if(const auto*f=std::get_if<ObjectRef>(&configCallback.data))cx.define(*f,key(u"length"),Descriptor::data(Value(2),false,false,true));
    const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"bboldx"),Value(u"bboldx-mathchar0miNormal"),Value(u"bboldx-delimiterNormal"),Value(u"bboldx-mathchar0miBold"),Value(u"bboldx-delimiterBold")})}});
    const Value options=record(cx,{{u"bboldx",record(cx,{{u"bfbb",Value(false)},{u"light",Value(false)}})}});
    return Configuration::create(runtime,u"bboldx",record(cx,{{u"handler",handlers},{u"options",options},{u"config",configCallback},{u"priority",Value(3.0)}}));
}
}
