// Native port of MathJax 4.1.3 input/tex/cancel/CancelConfiguration.ts.
module folia.math.input.tex.cancel.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.enclose.configuration;
import folia.math.input.tex.parser;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
TexParser& parser(ParserContext& value){auto* p=dynamic_cast<TexParser*>(&value);if(!p)property_host::typeError("Cancel requires a TexParser");return *p;}
ParseOptions& parseOptions(TexParser& p){const auto* v=std::get_if<ParseOptions*>(&p.configuration);if(!v||!*v)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");return **v;}
Property restProperty(ParseRest rest,std::size_t index){if(index>=rest.size())return Missing{};if(const auto* p=std::any_cast<Property>(&rest[index]))return *p;throw Exception("TypeError",u"Cancel macro argument is not a Property");}
Property definition(TexParser& p,std::u16string attr,std::u16string notation){
    auto& runtime=parseOptions(p).runtime();auto& cx=runtime.context();
    Property result=ParseUtil::keyvalOptions(runtime,std::move(attr),runtime.properties.to(EncloseConfiguration::options()));
    auto record=PropertyList::record(result);if(!record)property_host::typeError("Cannot assign to a primitive value");
    record->setOwn("notation",property_host::debugText(notation));return result;
}
FunctionRef cancel(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
    auto& p=parser(context);const auto name=property_host::string(std::get<Property>(atom));
    const auto attr=property_host::string(p.GetBrackets(name,Property("")));
    const auto math=p.ParseArg(name);const auto notation=property_host::string(restProperty(rest,0));
    auto attrs=definition(p,attr,notation);auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(math);
    const std::array<std::any,3> args{std::any(Property("menclose")),std::any(children),std::any(attrs)};
    p.Push(p.create(u"node",FunctionArgs(args)));return Property(Missing{});
});}
FunctionRef cancelTo(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(context);const auto name=property_host::string(std::get<Property>(atom));
    const auto attr=property_host::string(p.GetBrackets(name,Property("")));
    auto value=p.ParseArg(name);const auto math=p.ParseArg(name);
    const auto notation=PropertyList::record(TexConstant.get("Notation"));
    if(!notation)throw Exception("TypeError",u"TexConstant.Notation is not an object");
    const std::u16string encoded=property_host::string(notation->get("UPDIAGONALSTRIKE"))+u" "+
        property_host::string(notation->get("UPDIAGONALARROW"))+u" "+property_host::string(notation->get("NORTHEASTARROW"));
    auto attrs=definition(p,attr,encoded);
    auto paddedAttrs=Property(PropertyList{{"depth",Property("-.1em")},{"height",Property("+.1em")},{"voffset",Property(".1em")}});
    auto paddedChildren=std::make_shared<tree::ChildArray<mml::MmlNode>>();paddedChildren->push_back(value);
    const std::array<std::any,3> paddedArgs{std::any(Property("mpadded")),std::any(paddedChildren),std::any(paddedAttrs)};
    value=p.create(u"node",FunctionArgs(paddedArgs));
    auto enclosedChildren=std::make_shared<tree::ChildArray<mml::MmlNode>>();enclosedChildren->push_back(math);
    const std::array<std::any,3> enclosedArgs{std::any(Property("menclose")),std::any(enclosedChildren),std::any(attrs)};
    const auto enclosed=p.create(u"node",FunctionArgs(enclosedArgs));
    auto supChildren=std::make_shared<tree::ChildArray<mml::MmlNode>>();supChildren->push_back(enclosed);supChildren->push_back(value);
    const std::array<std::any,2> supArgs{std::any(Property("msup")),std::any(supChildren)};
    p.Push(p.create(u"node",FunctionArgs(supArgs)));return Property(Missing{});
});}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const auto o=cx.object();for(const auto&[k,v]:fields)cx.set(o,key(k),v);return Value(o);}
Value array(Context& cx,std::initializer_list<Value> fields){const auto o=cx.array(fields.size());std::size_t i=0;for(const auto& v:fields){auto n=std::to_string(i++);cx.set(o,key(std::u16string(n.begin(),n.end())),v);}return Value(o);}
}
ConfigurationRef CancelConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"cancel");
    if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    // CancelConfiguration imports EncloseConfiguration in Source; the shared
    // command map and ENCLOSE_OPTIONS object are initialized by that module.
    (void)EncloseConfiguration::initializeModule(runtime);
    const auto method=cancel(),to=cancelTo();
    for(const auto& [body,name]:std::array{
            std::pair{method,std::u16string_view(u"Cancel")},
            std::pair{to,std::u16string_view(u"CancelTo")}}){
        const auto bound=BaseMethods::bind(runtime,body,std::u16string(name));
        const auto* identity=std::get_if<ObjectRef>(&bound.data);
        if(!identity)throw Exception("TypeError",u"Cancel method has no Context identity");
        const double length=body==method?3.0:2.0;
        runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(length),false,false,true));
    }
    const auto notation=PropertyList::record(TexConstant.get("Notation"));
    if(!notation)throw options::Exception("TypeError",u"TexConstant.Notation is not an object");
    PropertyList commands;
    commands.setOwn("cancel",Property(PropertyArray{Property(method),notation->get("UPDIAGONALSTRIKE")}));
    commands.setOwn("bcancel",Property(PropertyArray{Property(method),notation->get("DOWNDIAGONALSTRIKE")}));
    commands.setOwn("xcancel",Property(PropertyArray{Property(method),Property(property_host::debugText(property_host::string(notation->get("UPDIAGONALSTRIKE"))+u" "+property_host::string(notation->get("DOWNDIAGONALSTRIKE"))))}));
    commands.setOwn("cancelto",Property(to));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("cancel"),Property(commands));
    auto& cx=runtime.context();const auto handler=record(cx,{{u"macro",array(cx,{Value(u"cancel")})}});
    return Configuration::create(runtime,u"cancel",record(cx,{{u"handler",handler}}));
}
}
