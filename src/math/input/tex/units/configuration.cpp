// Native port of MathJax 4.1.3 input/tex/units/UnitsConfiguration.ts.
module folia.math.input.tex.units.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser& parser(ParserContext& source){auto* p=dynamic_cast<TexParser*>(&source);if(!p)property_host::typeError("Units method requires a TexParser");return *p;}
std::u16string string(const Property& value){return property_host::string(value);}
bool option(TexParser& p,std::u16string_view field){
    const auto* configured=std::get_if<ParseOptions*>(&p.configuration);
    if(!configured||!*configured)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
    auto& cx=(**configured).runtime().context();
    const Value units=cx.get(p.options(),Key(u"units"));
    return options::truthy(cx.get(units,Key(std::u16string(field))));
}
Property copyEnvironment(const Property& environment){
    PropertyList copy;
    if(const auto record=PropertyList::record(environment))
        for(const auto& name:record->keys())copy.setOwn(name,record->getOwn(name));
    return Property(std::move(copy));
}
ParseResult unit(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){
    auto& p=parser(source);const std::u16string name=string(std::get<Property>(atom));
    const Property value=p.GetBrackets(name);const std::u16string dimension=string(p.GetArgument(name));
    std::u16string replacement=u"\\mathrm{"+dimension+u"}";
    if(property_host::truthy(value))replacement=string(value)+(option(p,u"loose")?u"~":u"\\,")+replacement;
    const std::u16string input=p.string();const auto offset=std::isfinite(p.i)&&p.i>0?std::min(input.size(),static_cast<std::size_t>(p.i)):std::size_t{};
    p.setString(replacement+input.substr(offset));p.i=0;return Property(Missing{});
}
ParseResult unitFrac(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){
    auto& p=parser(source);const std::u16string name=string(std::get<Property>(atom));
    const Property value=p.GetBrackets(name);const std::u16string numerator=string(p.GetArgument(name));
    const std::u16string denominator=string(p.GetArgument(name));
    std::u16string replacement=u"\\nicefrac[\\mathrm]{"+numerator+u"}{"+denominator+u"}";
    if(property_host::truthy(value))replacement=string(value)+(option(p,u"loose")?u"~":u"\\,")+replacement;
    const std::u16string input=p.string();const auto offset=std::isfinite(p.i)&&p.i>0?std::min(input.size(),static_cast<std::size_t>(p.i)):std::size_t{};
    p.setString(replacement+input.substr(offset));p.i=0;return Property(Missing{});
}
ParseResult niceFrac(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){
    auto& p=parser(source);const std::u16string name=string(std::get<Property>(atom));
    const Property fontValue=p.GetBrackets(name,Property("\\mathrm"));
    const std::u16string font=string(fontValue),numerator=string(p.GetArgument(name)),denominator=string(p.GetArgument(name));
    auto* configuration=std::get_if<ParseOptions*>(&p.configuration);
    if(!configuration||!*configuration)property_host::typeError("Cannot read properties of undefined (reading 'configuration')");
    const Property activeEnvironment=std::visit([](const auto& stackValue)->Property{
        using T=std::decay_t<decltype(stackValue)>;
        if constexpr(std::is_same_v<T,std::shared_ptr<Stack>>)return stackValue?stackValue->env():Property(Missing{});
        else return Property(Missing{});
    },p.stack);
    auto parse=[&](const std::u16string& part){auto nested=TexParser::make(font+u"{"+part+u"}",copyEnvironment(activeEnvironment),**configuration);return nested->mml();};
    const std::array<mml::NodeValue,2> children{parse(numerator),parse(denominator)};
    Property definition=PropertyList{};
    if(!option(p,u"ugly"))definition=PropertyList{{"bevelled",Property(true)}};
    auto childList=std::make_shared<tree::ChildArray<mml::MmlNode>>();childList->push_back(children[0]);childList->push_back(children[1]);
    const std::array<std::any,3> args{std::any(Property("mfrac")),std::any(mml::Children(std::move(childList))),std::any(definition)};
    p.Push(p.create(u"node",FunctionArgs(args)));return Property(Missing{});
}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string digits=std::to_string(i++);cx.set(Value(out),Key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const ObjectRef out=cx.object();for(const auto&[name,value]:fields)cx.set(out,Key(std::u16string(name)),value);return Value(out);}
}
Property UnitsMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.unitsMethods.missing())return runtime.unitsMethods;
    const std::array<std::pair<std::u16string_view,ParseBody>,3> methods{{
        {u"Unit",unit},{u"UnitFrac",unitFrac},{u"NiceFrac",niceFrac},
    }};
    const std::array<std::u16string_view,3> commands{u"units",u"unitfrac",u"nicefrac"};
    PropertyList exports;
    PropertyList entries;
    Context& cx=runtime.context();
    for(std::size_t i=0;i<methods.size();++i){
        const auto&[name,body]=methods[i];
        const FunctionRef method=makeParseCallable(body);
        const Value bound=BaseMethods::bind(runtime,method,std::u16string(name));
        const auto* identity=std::get_if<ObjectRef>(&bound.data);
        if(!identity)throw Exception("TypeError",u"Units method has no Context identity");
        cx.define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
        exports.setOwn(property_host::debugText(name),Property(method));
        entries.set(property_host::debugText(commands[i]),Property(method));
    }
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("units"),Property(entries));
    runtime.unitsMethods=Property(std::move(exports));
    return runtime.unitsMethods;
}
ConfigurationRef UnitsConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"units");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    (void)UnitsMethods::initializeModule(runtime);
    Context& cx=runtime.context();
    const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"units")})}});
    const Value options=record(cx,{{u"units",record(cx,{{u"loose",Value(false)},{u"ugly",Value(false)}})}});
    return Configuration::create(runtime,u"units",record(cx,{{u"handler",handlers},{u"options",options}}));
}
}
