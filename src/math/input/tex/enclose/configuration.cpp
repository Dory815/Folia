// Native port of MathJax 4.1.3 input/tex/enclose/EncloseConfiguration.ts.
module folia.math.input.tex.enclose.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.parser;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
const Property& EncloseConfiguration::options(){
    static const Property value(PropertyList{{"data-arrowhead",1},{"color",1},{"mathcolor",1},
        {"background",1},{"mathbackground",1},{"data-padding",1},{"data-thickness",1}});
    return value;
}
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
TexParser& parser(ParserContext& value){auto* p=dynamic_cast<TexParser*>(&value);if(!p)property_host::typeError("Enclose requires a TexParser");return *p;}
ParseOptions& parseOptions(TexParser& p){const auto* value=std::get_if<ParseOptions*>(&p.configuration);if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");return **value;}
FunctionRef enclose(){
    return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parser(context);const auto name=property_host::string(std::get<Property>(atom));
        std::u16string notation=property_host::string(p.GetArgument(name));
        std::replace(notation.begin(),notation.end(),u',',u' ');
        const auto attr=property_host::string(p.GetBrackets(name,Property("")));
        const auto math=p.ParseArg(name);
        Property definition=ParseUtil::keyvalOptions(parseOptions(p).runtime(),attr,
            parseOptions(p).runtime().properties.to(EncloseConfiguration::options()));
        auto attrs=PropertyList::record(definition);
        if(!attrs)property_host::typeError("Cannot assign to a primitive value");
        attrs->setOwn("notation",property_host::debugText(notation));
        auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(math);
        const std::array<std::any,3> args{std::any(Property("menclose")),std::any(children),std::any(definition)};
        p.Push(p.create(u"node",FunctionArgs(args)));return Property(Missing{});
    });
}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const auto o=cx.object();for(const auto&[k,v]:fields)cx.set(o,key(k),v);return Value(o);}
Value array(Context& cx,std::initializer_list<Value> fields){const auto o=cx.array(fields.size());std::size_t i=0;for(const auto& v:fields){auto n=std::to_string(i++);cx.set(o,key(std::u16string(n.begin(),n.end())),v);}return Value(o);}
}
ConfigurationRef EncloseConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"enclose");
    if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const auto method=enclose();
    const auto bound=BaseMethods::bind(runtime,method,u"Enclose");
    const auto* identity=std::get_if<ObjectRef>(&bound.data);
    if(!identity)throw Exception("TypeError",u"Enclose method has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("enclose"),Property(PropertyList{{"enclose",Property(method)}}));
    auto& cx=runtime.context();const auto handler=record(cx,{{u"macro",array(cx,{Value(u"enclose")})}});
    return Configuration::create(runtime,u"enclose",record(cx,{{u"handler",handler}}));
}
}
