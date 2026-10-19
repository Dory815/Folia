// Native port of MathJax 4.1.3 input/tex/bbm/BbmConfiguration.ts.
module folia.math.input.tex.bbm.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view s){return Key(std::u16string(s));}
Value record(Context& c,std::initializer_list<std::pair<std::u16string_view,Value>> fields){Value r(c.object());for(const auto&[k,v]:fields)c.set(r,key(k),v);return r;}
Value array(Context& c,std::initializer_list<Value> values){Value a(c.array(static_cast<std::uint32_t>(values.size())));std::uint32_t i=0;for(const auto&v:values){const auto digits=std::to_string(i++);c.set(a,key(std::u16string(digits.begin(),digits.end())),v);}return a;}
TexParser& parser(ParserContext& c){auto*p=dynamic_cast<TexParser*>(&c);if(!p)property_host::typeError("Bbm method requires a TexParser");return *p;}
}
ConfigurationRef BbmConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"bbm");if(const auto*p=std::get_if<ConfigurationRef>(&loaded);p&&*p)return *p;
    const auto baseMathFont=BaseMethods::get(runtime,u"MathFont");
    const auto choose=makeParseCallable([baseMathFont](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto&p=parser(c);const auto* configured=std::get_if<ParseOptions*>(&p.configuration);if(!configured||!*configured)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");auto& cx=(**configured).runtime().context();const Value opts=p.options();const Value bbm=cx.get(opts,key(u"bbm"));const Value bold=cx.get(bbm,key(u"bold"));
        const std::size_t index=options::truthy(bold)?1:0;Property font=Missing{};
        if(index<rest.size()){
            if(const auto* value=std::any_cast<Property>(&rest[index]))font=*value;
            else if(const auto* value=std::any_cast<std::u16string>(&rest[index]))font=property_host::debugText(*value);
            else if(const auto* value=std::any_cast<bool>(&rest[index]))font=*value;
            else if(const auto* value=std::any_cast<double>(&rest[index]))font=*value;
            else if(const auto* value=std::any_cast<FunctionRef>(&rest[index]))font=*value;
        }
        const std::array<std::any,1> args{std::any(std::move(font))};return invokeParseMethod(baseMathFont,FunctionReceiver{},p,atom,args);
    });
    const auto change=makeParseCallable([](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto&p=parser(c);const auto* config=std::get_if<ParseOptions*>(&p.configuration);if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
        const auto font=p.GetArgument(property_host::string(std::get<Property>(atom)));auto& cx=(**config).runtime().context();
        const Value bbm=cx.get(p.options(),key(u"bbm"));
        cx.set(bbm,key(u"bold"),Value(property_host::string(font)==u"bold"));return Property(Missing{});
    });
    const auto bind=[&](FunctionRef fn,std::u16string name,double length){const Value value=BaseMethods::bind(runtime,fn,std::move(name));if(const auto*o=std::get_if<ObjectRef>(&value.data))runtime.context().define(*o,key(u"length"),Descriptor::data(Value(length),false,false,true));return fn;};
    const auto chooseBound=bind(choose,u"ChooseFont",4),changeBound=bind(change,u"ChangeBold",2);
    PropertyList commands;commands.setOwn("mathversion",Property(changeBound));commands.setOwn("mathbbm",Property(PropertyArray{Property(chooseBound),Property("-bbm-normal"),Property("-bbm-bold")}));
    commands.setOwn("mathbbmss",Property(PropertyArray{Property(chooseBound),Property("-bbm-sans-serif"),Property("-bbm-sans-serif-bold")}));
    commands.setOwn("mathbbmtt",Property(PropertyArray{baseMathFont,Property("-bbm-monospace")}));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("bbm"),Property(commands));
    auto&cx=runtime.context();const Value handler=record(cx,{{u"macro",array(cx,{Value(u"bbm")})}});
    const Value options=record(cx,{{u"bbm",record(cx,{{u"bold",Value(false)}})}});
    return Configuration::create(runtime,u"bbm",record(cx,{{u"handler",handler},{u"options",options}}));
}
}
