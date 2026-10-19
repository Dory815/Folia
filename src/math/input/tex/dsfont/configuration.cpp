// Native port of MathJax 4.1.3 input/tex/dsfont/DsfontConfiguration.ts.
module folia.math.input.tex.dsfont.configuration;
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
}
ConfigurationRef DsfontConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"dsfont");if(const auto*p=std::get_if<ConfigurationRef>(&loaded);p&&*p)return *p;
    const auto baseMathFont=BaseMethods::get(runtime,u"MathFont");
    const auto choose=makeParseCallable([baseMathFont](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto* p=dynamic_cast<TexParser*>(&context);if(!p)property_host::typeError("Dsfont requires a TexParser");
        const auto* config=std::get_if<ParseOptions*>(&p->configuration);if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
        auto& cx=(**config).runtime().context();const Value dsfont=cx.get(p->options(),key(u"dsfont"));const Value sans=cx.get(dsfont,key(u"sans"));
        const std::array<std::any,1> args{std::any(Property(options::truthy(sans)?"-ds-ss":"-ds-rm"))};
        return invokeParseMethod(baseMathFont,FunctionReceiver{},*p,atom,args);
    });
    const Value chooseValue=BaseMethods::bind(runtime,choose,u"ChooseFont");
    if(const auto* fn=std::get_if<ObjectRef>(&chooseValue.data))runtime.context().define(*fn,key(u"length"),Descriptor::data(Value(2),false,false,true));
    PropertyList commands;commands.setOwn("mathds",Property(choose));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("dsfont"),Property(commands));
    auto&cx=runtime.context();const Value handler=record(cx,{{u"macro",array(cx,{Value(u"dsfont")})}});
    const Value options=record(cx,{{u"dsfont",record(cx,{{u"sans",Value(false)}})}});
    return Configuration::create(runtime,u"dsfont",record(cx,{{u"handler",handler},{u"options",options}}));
}
}
