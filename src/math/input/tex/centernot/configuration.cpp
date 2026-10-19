// Native implementation of CenternotConfiguration.ts.
module folia.math.input.tex.centernot.configuration;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.input.tex.node_util;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
Value array(Context& cx,std::initializer_list<Value> values){
    const auto result=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t index=0;
    for(const auto& value:values){const auto decimal=std::to_string(index++);cx.set(result,key(std::u16string(decimal.begin(),decimal.end())),value);}return Value(result);
}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){
    Value result(cx.object());for(const auto&[name,value]:fields)cx.set(result,key(name),value);return result;
}
std::shared_ptr<tree::ChildArray<mml::MmlNode>> children(std::initializer_list<mml::NodeValue> values){
    auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();for(const auto& value:values)result->push_back(value);return result;
}
FunctionRef centerOver(ParserRuntime& runtime){
    return makeParseCallable([&runtime](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{
        auto* parser=dynamic_cast<TexParser*>(&source);if(!parser)property_host::typeError("centerOver requires TexParser");
        Property rawName=std::holds_alternative<Property>(atom)?std::get<Property>(atom):std::get<TokenRef>(atom)->token();
        const std::u16string name=property_host::string(rawName);
        const std::u16string baseInput=u"{"+property_host::string(parser->GetArgument(name))+u"}";
        const mml::NodeValue over=parser->ParseArg(name);
        const auto* configuration=std::get_if<ParseOptions*>(&parser->configuration);
        if(!configuration||!*configuration)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");
        const Property env=std::visit([](const auto& current)->Property{using T=std::decay_t<decltype(current)>;if constexpr(std::is_same_v<T,std::shared_ptr<Stack>>)return current?current->env():Property(Missing{});else return Property(Missing{});},parser->stack);
        const mml::NodeValue base=TexParser::make(baseInput,env,**configuration)->mml();
        const mml::NodeValue baseForAtom=TexParser::make(baseInput,env,**configuration)->mml();
        const Property centered=Property(PropertyList{{"width",Property(0.0)},{"lspace",Property("-.5width")}});
        const std::array<std::any,3> overArgs{std::any(Property("mpadded")),std::any(children({over})),std::any(centered)};
        const mml::NodeValue paddedOver=parser->create(u"node",FunctionArgs(overArgs));
        const std::array<std::any,2> phantomArgs{std::any(Property("mphantom")),std::any(children({base}))};
        const mml::NodeValue phantom=parser->create(u"node",FunctionArgs(phantomArgs));
        const std::array<std::any,3> overlayArgs{std::any(Property("mpadded")),std::any(children({paddedOver,phantom})),std::any(centered)};
        const mml::NodeValue overlay=parser->create(u"node",FunctionArgs(overlayArgs));
        const auto outerChildren=children({baseForAtom,overlay});
        const std::array<std::any,2> atomArgs{std::any(Property("TeXAtom")),std::any(outerChildren)};
        const mml::NodeValue result=parser->create(u"node",FunctionArgs(atomArgs));
        (**configuration).addNode(Property("centerOver"),base);
        parser->Push(result);
        return Property(Missing{});
    });
}
ParseOptionsRef parseOptionsFor(ParserRuntime& runtime,Context& cx,const Value& recordValue){
    const Value data=cx.get(recordValue,key(u"data"));const auto* identity=std::get_if<ObjectRef>(&data.data);
    if(!identity)property_host::typeError("Centernot filter data is not ParseOptions");
    return runtime.parseOptionsIdentities.resolve(*identity);
}
Value bindFilterCenterOver(ParserRuntime& runtime){
    auto body=std::make_shared<Function>();
    body->sourceMetadata=FunctionSourceMetadata{u"filterCenterOver",1};
    body->call=[](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.empty())throw Exception("TypeError",u"Centernot filter data is missing");
        ParseOptionsRef options;
        if(const auto* direct=std::any_cast<ParseOptionsRef>(&args[0]);direct&&*direct){
            options=*direct;
        }else if(const auto* filterData=std::any_cast<core::InputJaxFilterDataRef>(&args[0]);filterData&&*filterData){
            const auto current=(*filterData)->currentData();
            const auto* data=std::any_cast<ParseOptionsRef>(&current);
            if(data&&*data)options=*data;
        }
        if(!options)throw Exception("TypeError",u"Centernot filter requires ParseOptions");
        const auto bases=options->getList(u"centerOver");
        if(!bases)return std::any(std::monostate{});
        for(std::size_t i=0;i<bases->size();++i){
            const mml::NodeValue base=bases->get(i);
            const mml::NodeValue first=NodeUtil::instance().getChildAt(base,0);
            const mml::NodeValue nested=NodeUtil::instance().getChildAt(first,0);
            const Property texClass=NodeUtil::instance().getTexClass(nested);
            if(std::holds_alternative<PropertyNull>(texClass.value))continue;
            mml::NodeValue target=base;
            for(int level=0;level<6;++level){auto* node=target.requireNode("parent");target=node->getParent();}
            NodeUtil::instance().setProperties(target,Property(PropertyList{{"texClass",texClass}}));
        }
        return std::any(std::monostate{});
    };
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver{std::move(receiver)};};
    binding.arguments=[&runtime](Context& cx,std::span<const Value> args){if(args.empty())throw Exception("TypeError",u"Cannot destructure property 'data' of undefined");return std::vector<std::any>{std::any(parseOptionsFor(runtime,cx,args[0]))};};
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    return runtime.functions().bind(std::move(body),std::move(binding),u"filterCenterOver");
}
}
Property CenternotMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.centernotMethods.missing())return runtime.centernotMethods;
    const Value bound=bindFilterCenterOver(runtime);
    const auto* identity=std::get_if<ObjectRef>(&bound.data);
    if(!identity)throw Exception("TypeError",u"filterCenterOver has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(1.0),false,false,true));
    const auto function=runtime.functions().find(*identity);
    if(!function||!*function)throw Exception("TypeError",u"filterCenterOver has no canonical FunctionRef");
    PropertyList exports;
    exports.setOwn("filterCenterOver",Property(*function));
    runtime.centernotMethods=Property(std::move(exports));
    return runtime.centernotMethods;
}
FunctionRef CenternotMethods::filterCenterOver(ParserRuntime& runtime){
    const Property methods=initializeModule(runtime);
    const Property value=property_host::get(methods,"filterCenterOver");
    const auto* function=std::get_if<FunctionRef>(&value.value);
    if(!function||!*function)throw Exception("TypeError",u"Centernot filterCenterOver export is not callable");
    return *function;
}
ConfigurationRef CenternotConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"centernot");
    if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const FunctionRef filterMethod=CenternotMethods::filterCenterOver(runtime);
    const auto filterIdentity=runtime.functions().find(filterMethod);
    if(!filterIdentity)throw Exception("TypeError",u"Centernot filterCenterOver lost its Context identity");
    const Property baseMacro=BaseMethods::get(runtime,u"Macro");const auto* macro=std::get_if<FunctionRef>(&baseMacro.value);
    if(!macro||!*macro)throw Exception("TypeError",u"BaseMethods.Macro is unavailable");
    const FunctionRef centerOverMethod=centerOver(runtime);
    const Value centerOverIdentity=BaseMethods::bind(runtime,centerOverMethod,u"CenterOver");
    if(const auto* identity=std::get_if<ObjectRef>(&centerOverIdentity.data))
        runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    else throw Exception("TypeError",u"CenterOver function identity was not registered");
    (void)BaseMethods::bind(runtime,*macro,u"Macro");
    PropertyList entries;entries.set("centerOver",Property(centerOverMethod));
    entries.set("centernot",Property(PropertyArray{Property(*macro),Property("\\centerOver{#1}{{\u29F8}}"),Property(1.0)}));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("centernot"),Property(std::move(entries)));
    Context& cx=runtime.context();const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"centernot")})}});
    const Value processors=array(cx,{Value(*filterIdentity)});
    return Configuration::create(runtime,u"centernot",record(cx,{{u"handler",handlers},{u"postprocessors",processors}}));
}
}
