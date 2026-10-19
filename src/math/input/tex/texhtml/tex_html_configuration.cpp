/*************************************************************
 * Native port of MathJax 4.1.3 input/tex/texhtml/TexHtmlConfiguration.ts.
 * Package data is scoped to each actual TeX compilation.
 *************************************************************/
module folia.math.input.tex.texhtml.tex_html_configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.input.tex.texhtml.tex_html_methods;
import folia.math.input.tex.texhtml.tex_html_services;
import folia.math.core.input_jax;
import folia.math.handlers.html.html_dom_strings;
import folia.math.input.tex.base_methods;
import folia.math.util.runtime_modules;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value array(Context& cx,std::initializer_list<Value> values){
    const ObjectRef result=cx.array(static_cast<std::uint32_t>(values.size()));
    std::uint32_t index=0;
    for(const auto& value:values){
        const auto text=std::to_string(index++);
        cx.set(Value(result),key(std::u16string(text.begin(),text.end())),value);
    }
    return Value(result);
}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){
    const ObjectRef result=cx.object();
    for(const auto& [name,value]:fields)cx.set(Value(result),key(name),value);
    return Value(result);
}
Value bindFilter(ParserRuntime& runtime,std::u16string name,
    std::function<void(core::InputJaxFilterDataRef)> run){
    auto body=std::make_shared<Function>();
    body->sourceMetadata=FunctionSourceMetadata{u"",1};
    body->call=[run=std::move(run)](const FunctionReceiver&,FunctionArgs arguments)->std::any{
        if(arguments.empty())throw Exception("TypeError",u"TexHtml filter data is missing");
        const auto* data=std::any_cast<core::InputJaxFilterDataRef>(&arguments[0]);
        if(!data||!*data)throw Exception("TypeError",u"TexHtml filter data is invalid");
        run(*data);
        return std::monostate{};
    };
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    binding.arguments=[](Context& cx,std::span<const Value> arguments){
        if(arguments.empty())throw Exception("TypeError",u"TexHtml filter data is missing");
        const auto* object=std::get_if<ObjectRef>(&arguments[0].data);
        if(!object)throw Exception("TypeError",u"TexHtml filter identity is invalid");
        const auto* data=std::any_cast<core::InputJaxFilterDataRef>(&cx.payload(*object));
        if(!data||!*data)throw Exception("TypeError",u"TexHtml filter identity is invalid");
        return std::vector<std::any>{std::any(*data)};
    };
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    const Value result=runtime.functions().bind(std::move(body),std::move(binding),name);
    if(const auto* function=std::get_if<ObjectRef>(&result.data))
    {
        runtime.context().define(*function,key(u"length"),Descriptor::data(Value(1.0),false,false,true));
        runtime.context().define(*function,key(u"name"),Descriptor::data(Value(u""),false,false,true));
    }
    return result;
}
std::shared_ptr<SourceStringMap<Value>> packageData(ParseOptions& options){
    if(const auto* value=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&options.packageData);
       value&&*value)return *value;
    property_host::typeError("Cannot read properties of null or undefined (reading 'set')");
}
}

ConfigurationRef TexHtmlConfiguration::initializeModule(ParserRuntime& runtime,util::RuntimeModules& modules,
    TexHtmlAdaptorResolver resolver){
    const auto loaded=runtime.configurationHandler.get(u"texhtml");
    if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const Property methods=TexHtmlMethods::initializeModule(runtime,resolver);
    const Property texHtmlMethod=property_host::get(methods,"TexHTML");
    const auto* texHtmlFunction=std::get_if<FunctionRef>(&texHtmlMethod.value);
    if(!texHtmlFunction||!*texHtmlFunction)
        throw Exception("TypeError",u"TexHtml source method is not callable");
    const Value texHtmlIdentity=BaseMethods::bind(runtime,*texHtmlFunction,u"TexHTML");
    if(const auto* identity=std::get_if<ObjectRef>(&texHtmlIdentity.data))
        runtime.context().define(*identity,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    else throw Exception("TypeError",u"TexHtml source method has no Context identity");
    const auto domStrings=handlers::html::HTMLDomStringsModule::initializeModule(modules,runtime.properties,runtime.regexp());
    auto configBody=std::make_shared<Function>();
    configBody->sourceMetadata=FunctionSourceMetadata{u"",0};
    configBody->call=[domStrings](const FunctionReceiver&,FunctionArgs)->std::any{
        domStrings->installTexHtmlHook();
        return std::monostate{};
    };
    OptionCallBinding configBinding;
    configBinding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    configBinding.arguments=[](Context&,std::span<const Value> arguments){
        std::vector<std::any> result;result.reserve(arguments.size());
        for(const auto& argument:arguments)result.emplace_back(argument);
        return result;
    };
    configBinding.result=[](Context&,const std::any&){return Value(Undefined{});};
    const Value configHook=runtime.functions().bind(std::move(configBody),
        std::move(configBinding),u"TexHtmlConfiguration.CONFIG");
    const Property macro=property_host::get(methods,"TexHTML");
    (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property("tex-html"),
        Property(PropertyList{{"<",macro}}));

    const Value preprocessor=bindFilter(runtime,u"texHtmlPreprocessor",
        [&runtime](core::InputJaxFilterDataRef data){
            const auto current=data->currentData();
            const auto* options=std::any_cast<ParseOptionsRef>(&current);
            if(!options||!*options)throw Exception("TypeError",u"TexHtml preprocessor requires ParseOptions");
            const auto documentReference=data->documentReference();
            const auto* document=std::get_if<core::MathDocumentService*>(&documentReference);
            if(!document||!*document)property_host::typeError("Cannot read properties of undefined (reading 'adaptor')");

            Context& context=runtime.context();
            const auto identity=(*document)->sourceIdentity();
            const Value adaptor=context.get(Value(identity),key(u"adaptor"));
            const Value package=record(context,{{u"adaptor",adaptor}});
            packageData(**options)->set(u"texhtml",package);
        });
    const Value postprocessor=bindFilter(runtime,u"texHtmlPostprocessor",
        [&runtime](core::InputJaxFilterDataRef data){
            const auto current=data->currentData();
            const auto* options=std::any_cast<ParseOptionsRef>(&current);
            if(!options||!*options)throw Exception("TypeError",u"TexHtml postprocessor requires ParseOptions");
            Context& context=runtime.context();
            packageData(**options)->set(u"texhtml",record(context,{{u"adaptor",Value(Null{})}}));
        });
    const Value handlers=record(runtime.context(),{{u"character",array(runtime.context(),{Value(u"tex-html")})}});
    const Value options=record(runtime.context(),{{u"allowTexHTML",Value(false)}});
    const Value preprocessors=array(runtime.context(),{preprocessor});
    const Value postprocessors=array(runtime.context(),{postprocessor});
    const Value configuration=record(runtime.context(),{{u"handler",handlers},{u"options",options},{u"config",Value(configHook)},
        {u"preprocessors",preprocessors},{u"postprocessors",postprocessors}});
    return Configuration::create(runtime,u"texhtml",configuration);
}
}
