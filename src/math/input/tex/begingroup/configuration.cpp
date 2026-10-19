// MathJax 4.1.3 BegingroupConfiguration.ts; actual Configuration, maps and callbacks.
module folia.math.input.tex.begingroup.configuration;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.token_map;
import folia.math.core.input_jax;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
Value array(Context& context,std::initializer_list<Value> values){
    const ObjectRef result=context.array(static_cast<std::uint32_t>(values.size()));
    std::uint32_t index=0;
    for(const auto& value:values){const auto ascii=std::to_string(index++);context.set(result,key(std::u16string(ascii.begin(),ascii.end())),value);}
    return Value(result);
}
Value record(Context& context,std::initializer_list<std::pair<std::u16string_view,Value>> values){
    const ObjectRef result=context.object();for(const auto& [name,value]:values)context.set(Value(result),key(name),value);return Value(result);
}
ParseMethod method(ParserRuntime& runtime,std::u16string_view name){
    const auto table=PropertyList::record(runtime.begingroupMethods);
    if(!table)throw Exception("TypeError",u"Begingroup method table is not initialized");
    const Property value=table->get(property_host::debugText(name));
    if(const auto* function=std::get_if<FunctionRef>(&value.value))return *function;
    return functions::fromProperty(value);
}
void createCommandMap(ParserRuntime& runtime){
    const auto table=PropertyList::record(runtime.begingroupMethods);
    if(!table)throw Exception("TypeError",u"Begingroup method table is not initialized");
    const Property macro=BaseMethods::get(runtime,u"Macro");
    PropertyList commands;
    for(const std::u16string_view name:{u"begingroup",u"endgroup",u"global",u"begingroupReset",u"begingroupSandbox"})
        commands.setOwn(property_host::debugText(name),table->get(property_host::debugText(name==u"begingroupReset"?u"reset":name==u"begingroupSandbox"?u"sandbox":name)));
    commands.setOwn("gdef",Property(PropertyArray{macro,Property("\\global\\def")}));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("begingroup"),Property(commands));
}
ParseOptionsRef parseOptions(ParserRuntime& runtime,const Value& value){
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"Begingroup callback requires ParseOptions");
    return runtime.parseOptionsIdentities.resolve(*object);
}
ParseOptionsRef filterOptions(const FunctionArgs& arguments){
    if(arguments.empty())throw Exception("TypeError",u"Begingroup filter data is missing");
    if(const auto* direct=std::any_cast<ParseOptionsRef>(&arguments[0]);direct&&*direct)return *direct;
    if(const auto* data=std::any_cast<core::InputJaxFilterDataRef>(&arguments[0]);data&&*data){
        const auto current=(*data)->currentData();
        if(const auto* direct=std::any_cast<ParseOptionsRef>(&current);direct&&*direct)return *direct;
    }
    throw Exception("TypeError",u"Begingroup filter data is not ParseOptions");
}
Value bindConfigCallback(ParserRuntime& runtime){
    auto body=std::make_shared<Function>();
    body->sourceMetadata=FunctionSourceMetadata{u"",2};
    body->call=[&runtime](const FunctionReceiver&,FunctionArgs arguments)->std::any{
        if(arguments.size()<2)throw Exception("TypeError",u"Begingroup config requires configuration and TeX");
        const auto* configuration=std::any_cast<ParserConfigurationRef>(&arguments[0]);
        const auto* jax=std::any_cast<TeX*>(&arguments[1]);
        if(!configuration||!*configuration||!jax||!*jax)throw Exception("TypeError",u"Begingroup config arguments are invalid");
        const auto parseValue=(**jax).parseOptions();
        const auto* parse=std::get_if<std::shared_ptr<ParseOptions>>(&parseValue);
        if(!parse||!*parse)property_host::typeError("Cannot read properties of null or undefined (reading 'packageData')");
        auto* packageData=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&(*parse)->packageData);
        if(!packageData||!*packageData)property_host::typeError("Cannot read properties of null or undefined (reading 'set')");
        Context& context=runtime.context();
        const auto* constructor=std::get_if<ObjectRef>(&runtime.begingroupStackClass.data);
        if(!constructor)throw Exception("TypeError",u"BegingroupStack class metadata is not initialized");
        const auto parseIdentity=runtime.parseOptionsIdentities.find(*parse);
        if(!parseIdentity)throw Exception("TypeError",u"ParseOptions has no canonical Context identity");
        const std::array<Value,1> constructorArguments{Value(*parseIdentity)};
        const ObjectRef stackObject=context.construct(*constructor,constructorArguments);
        const Value package=record(context,{{u"stack",Value(stackObject)}});
        (*packageData)->set(u"begingroup",package);
        return std::monostate{};
    };
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    binding.arguments=[&runtime](Context&,std::span<const Value> arguments){
        if(arguments.size()<2)throw Exception("TypeError",u"Begingroup config requires (configuration, jax)");
        const auto* configuration=std::get_if<ObjectRef>(&arguments[0].data);
        const auto* jax=std::get_if<ObjectRef>(&arguments[1].data);
        if(!configuration||!jax)throw Exception("TypeError",u"Begingroup config identities are invalid");
        return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*configuration)),
                                     std::any(runtime.texIdentities.resolve(*jax).get())};
    };
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    return runtime.functions().bind(std::move(body),std::move(binding),u"BegingroupConfig");
}
Value filterCallback(ParserRuntime& runtime,bool before){
    auto body=std::make_shared<Function>();
    body->sourceMetadata=FunctionSourceMetadata{u"",1};
    body->call=[before](const FunctionReceiver&,FunctionArgs arguments)->std::any{
        auto options=filterOptions(arguments);
        const auto stack=begingroupStack(*options);
        if(before)stack->remove();else stack->finish();
        return std::monostate{};
    };
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    binding.arguments=[&runtime](Context& context,std::span<const Value> arguments){
        if(arguments.empty())throw Exception("TypeError",u"Begingroup filter data is missing");
        const Value data=context.get(arguments[0],key(u"data"));
        return std::vector<std::any>{std::any(parseOptions(runtime,data))};
    };
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    return runtime.functions().bind(std::move(body),std::move(binding),before?u"begingroupRemove":u"begingroupFinish");
}
}

void BegingroupConfiguration::initializeModule(ParserRuntime& runtime){
    BegingroupStack::initializeModule(runtime);
    const Property methods=BegingroupMethods::initializeModule(runtime);
    (void)methods;
    // BegingroupMethods.ts and BegingroupConfiguration.ts each evaluate their
    // own top-level CommandMap construction in Source import order.
    createCommandMap(runtime);

    Context& context=runtime.context();
    const Value handler=record(context,{{u"macro",array(context,{Value(u"begingroup")})}});
    const Value allowGlobal=array(context,{Value(u"let"),Value(u"def"),Value(u"newcommand"),
        Value(u"DeclareMathOperator"),Value(u"Newextarrow")});
    const Value options=record(context,{{u"begingroup",record(context,{{u"allowGlobal",allowGlobal}})}});
    const Value preprocessors=array(context,{filterCallback(runtime,true)});
    const Value postprocessors=array(context,{filterCallback(runtime,false)});
    const Value config=record(context,{{u"handler",handler},{u"options",options},
        {u"config",bindConfigCallback(runtime)},{u"preprocessors",preprocessors},
        {u"postprocessors",postprocessors}});
    (void)method(runtime,u"global");
    (void)Configuration::create(runtime,u"begingroup",config);
}
}
