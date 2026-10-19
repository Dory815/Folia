// MathJax 4.1.3 BegingroupStack.ts: actual group maps and global shadow deletion.
module folia.math.input.tex.begingroup.stack;
import folia.math.input.tex.parser;
import folia.math.input.tex.newcommand.util;
import folia.math.input.tex.token_map;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import folia.math.util.native_data_fields;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
std::u16string text(const Property& value){return property_host::string(value);}
std::string utf8(std::u16string_view value){return property_host::debugText(value);}
std::u16string methodName(std::u16string_view map){
    if(map==u"new-Delimiter")return u"delimiter";
    if(map==u"new-Environment")return u"environment";
    if(map==u"new-Command")return u"macro";
    return {};
}
Property shifted(PropertyArray array){
    const std::size_t length=array.size();
    if(!length)return Missing{};
    const Property first=array.get(0);
    for(std::size_t index=1;index<length;++index){
        if(array.has(index))array.set(index-1,array.get(index));else array.erase(index-1);
    }
    array.erase(length-1);array.resize(length-1);
    return first;
}
Property parseMethod(const Property& table,std::u16string_view name){
    const auto record=PropertyList::record(table);
    if(!record)property_host::typeError("ParseMethods is not an object");
    return record->get(property_host::debugText(name));
}
}

BegingroupStack::BegingroupStack(ParserRuntime& runtime,ParseOptions& parser):runtime_(runtime){initializeStack(parser);}
void BegingroupStack::initializeStack(ParseOptions& parser){
    marker_=runtime_.context().symbol(u"marker");
    i_=top_=base_=Value(NEWCOMMAND_PRIORITY);
    const auto* handlers=std::get_if<std::shared_ptr<SubHandlers>>(&parser.handlers);
    if(!handlers||!*handlers)property_host::typeError("Cannot read properties of undefined (reading 'get')");
    handlers_=*handlers;
    getGlobal();
}
double BegingroupStack::index()const{return runtime_.context().toNumber(i_);}
double BegingroupStack::top()const{return runtime_.context().toNumber(top_);}
double BegingroupStack::base()const{return runtime_.context().toNumber(base_);}
void BegingroupStack::bindSourceFields(ObjectRef identity){
    auto owner=shared_from_this();Context& context=runtime_.context();
    bindNativeValue(context,identity,key(u"i"),owner,&BegingroupStack::i_);
    bindNativeValue(context,identity,key(u"top"),owner,&BegingroupStack::top_);
    bindNativeValue(context,identity,key(u"base"),owner,&BegingroupStack::base_);
    bindNativeValue(context,identity,key(u"MARKER"),owner,&BegingroupStack::marker_);
}
void BegingroupStack::getGlobal(){
    global_[0]=runtime_.mapHandler.getMap(Property("new-Delimiter"));
    global_[1]=runtime_.mapHandler.getMap(Property("new-Environment"));
    global_[2]=runtime_.mapHandler.getMap(Property("new-Command"));
}
TokenMapRef BegingroupStack::global(std::u16string_view name)const{
    if(name==u"new-Delimiter")return global_[0];
    if(name==u"new-Environment")return global_[1];
    if(name==u"new-Command")return global_[2];
    return {};
}

std::vector<TokenMapRef> BegingroupStack::checkGlobal(PropertyArray tokens,PropertyArray maps){
    for(std::size_t index=0;index<maps.size();++index){
        const std::u16string name=text(maps.get(index));
        // Source mutates the caller's same token array with shift() before any
        // handler or map side effect for this entry.
        const Property token=shifted(tokens);
        const std::u16string handlerName=methodName(name);
        const SubHandlerRef handler=handlers_?handlers_->get(utf8(handlerName)):SubHandlerRef{};
        if(!handler)property_host::typeError("Cannot read properties of undefined (reading 'applicable')");
        const TokenMapRef target=global(name);
        auto* globalMap=dynamic_cast<AbstractParseMap*>(target.get());
        if(!globalMap)property_host::typeError("Cannot read properties of undefined (reading 'add')");
        const Property marker=runtime_.properties.from(marker_);
        globalMap->add(token,TokenLookup{marker});
        TokenMapRef map;
        TokenLookup item=Property(Missing{});
        do{
            map=handler->applicable(token);
            if(!map)property_host::typeError("Cannot read properties of undefined (reading 'lookup')");
            item=map->lookup(token);
            auto* parseMap=dynamic_cast<AbstractParseMap*>(map.get());
            if(!parseMap)property_host::typeError("map.remove is not a function");
            parseMap->remove(token);
        }while(std::visit([&](const auto& value){
            using T=std::decay_t<decltype(value)>;
            if constexpr(std::same_as<T,Property>)return property_host::truthy(value)&&value!=runtime_.properties.from(marker_);
            else return bool(value);
        },item));
    }
    // Source performs maps.map only after all token/map mutations complete;
    // values can therefore be changed by the preceding handler callbacks.
    std::vector<TokenMapRef> result;
    for(std::size_t index=0;index<maps.size();++index)
        result.push_back(global(text(maps.get(index))));
    return result;
}

void BegingroupStack::push(){
    const Property parseMethods=runtime_.parseMethods;
    const ParseMethod delimiter=functions::fromProperty(parseMethod(parseMethods,u"delimiter"));
    (void)TokenMapHost::create<DelimiterMap>(runtime_.mapHandler,Property("new-Delimiter"),delimiter,Property(PropertyList{}));
    (void)TokenMapHost::create<CommandMap>(runtime_.mapHandler,Property("new-Command"),Property(PropertyList{}));
    const ParseMethod environment=functions::fromProperty(parseMethod(parseMethods,u"environment"));
    (void)TokenMapHost::create<EnvironmentMap>(runtime_.mapHandler,Property("new-Environment"),environment,Property(PropertyList{}));
    HandlerConfig config;
    config.setOwn("delimiter",Property(PropertyArray{Property("new-Delimiter")}));
    config.setOwn("environment",Property(PropertyArray{Property("new-Environment")}));
    config.setOwn("macro",Property(PropertyArray{Property("new-Delimiter"),Property("new-Command")}));
    i_=Value(runtime_.context().toNumber(i_)-1);
    handlers_->add(config,FallbackConfig{},runtime_.context().toNumber(i_));
}
void BegingroupStack::pop(){
    if(i_==base_)throw TexError(u"MissingBegingroup",u"Missing \\begingroup or extra \\endgroup");
    HandlerConfig config;
    config.setOwn("delimiter",Property(PropertyArray{Property("new-Delimiter")}));
    config.setOwn("environment",Property(PropertyArray{Property("new-Environment")}));
    config.setOwn("macro",Property(PropertyArray{Property("new-Delimiter"),Property("new-Command")}));
    handlers_->remove(config,FallbackConfig{});
    for(const std::u16string_view name:{u"new-Command",u"new-Environment",u"new-Delimiter"})
        runtime_.mapHandler.registerMap(handlers_->retrieve(Property(property_host::debugText(name))));
    i_=Value(runtime_.context().toNumber(i_)+1);
}
void BegingroupStack::finish(){top_=i_;}
void BegingroupStack::remove(){while(runtime_.context().toNumber(i_)<runtime_.context().toNumber(top_))pop();}
void BegingroupStack::reset(){top_=base_;remove();}
void BegingroupStack::sandbox(){base_=Value(NEWCOMMAND_PRIORITY);reset();push();getGlobal();base_=Value(NEWCOMMAND_PRIORITY-1);}

BegingroupStackRef begingroupStack(ParseOptions& parser){
    auto* runtime=&parser.runtime();
    auto* data=std::get_if<std::shared_ptr<SourceStringMap<options::Value>>>(&parser.packageData);
    if(!data||!*data)property_host::typeError("Cannot read properties of undefined (reading 'get')");
    const auto package=(*data)->get(u"begingroup");
    if(!package)property_host::typeError("Cannot read properties of undefined (reading 'stack')");
    const options::Value value=runtime->context().get(*package,key(u"stack"));
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)property_host::typeError("Cannot read properties of undefined (reading 'push')");
    return std::dynamic_pointer_cast<BegingroupStack>(runtime->globalDefinitionScopeIdentities.resolve(*object));
}
BegingroupStackRef begingroupStack(TexParser& parser){
    const auto* configuration=std::get_if<ParseOptions*>(&parser.configuration);
    if(!configuration||!*configuration)property_host::typeError("Cannot read properties of undefined (reading 'options')");
    return begingroupStack(**configuration);
}

void BegingroupStack::initializeModule(ParserRuntime& runtime){
    Context& context=runtime.context();
    if(!std::holds_alternative<Undefined>(runtime.begingroupStackClass.data))
        throw Exception("TypeError",u"BegingroupStack module metadata already initialized");
    const ObjectRef constructor=context.nativeConstructor(u"BegingroupStack",
        [&runtime](Context&,std::span<const Value> arguments){
            if(arguments.empty())throw Exception("TypeError",u"BegingroupStack requires ParseOptions");
            const auto* object=std::get_if<ObjectRef>(&arguments[0].data);
            if(!object)throw Exception("TypeError",u"BegingroupStack requires ParseOptions");
            const ParseOptionsRef parse=runtime.parseOptionsIdentities.resolve(*object);
            auto stack=std::make_shared<BegingroupStack>(runtime,*parse);
            const ObjectRef instance=runtime.globalDefinitionScopeIdentities.instance(
                stack,std::get<ObjectRef>(runtime.begingroupStackClass.data),std::static_pointer_cast<void>(stack));
            stack->bindSourceFields(instance);
            return instance;
        });
    context.define(constructor,key(u"length"),Descriptor::data(Value(1),false,false,true));
    runtime.begingroupStackClass=Value(constructor);
    const ObjectRef prototype=std::get<ObjectRef>(context.get(runtime.begingroupStackClass,key(u"prototype")).data);
    auto install=[&](std::u16string_view name,std::function<void(BegingroupStack&)> action){
        const std::u16string label(name);
        const ObjectRef function=context.function(label,[&runtime,action=std::move(action)](Context&,Value receiver,std::span<const Value>)->Value{
            const auto* identity=std::get_if<ObjectRef>(&receiver.data);
            if(!identity)throw Exception("TypeError",u"BegingroupStack method receiver is not an object");
            auto stack=std::dynamic_pointer_cast<BegingroupStack>(runtime.globalDefinitionScopeIdentities.resolve(*identity));
            if(!stack)throw Exception("TypeError",u"BegingroupStack method receiver has the wrong class");
            action(*stack);return Undefined{};
        });
        context.define(prototype,key(name),Descriptor::data(Value(function),true,false,true));
    };
    install(u"getGlobal",[](BegingroupStack& stack){stack.getGlobal();});
    const ObjectRef checkGlobal=context.function(u"checkGlobal",[](Context&,Value,std::span<const Value>)->Value{
        throw Exception("Unimplemented",u"Context checkGlobal requires canonical TokenMap Context identities and live PropertyArray bindings");
    });
    context.define(checkGlobal,key(u"length"),Descriptor::data(Value(2),false,false,true));
    context.define(prototype,key(u"checkGlobal"),Descriptor::data(Value(checkGlobal),true,false,true));
    install(u"push",[](BegingroupStack& stack){stack.push();});
    install(u"pop",[](BegingroupStack& stack){stack.pop();});
    install(u"finish",[](BegingroupStack& stack){stack.finish();});
    install(u"remove",[](BegingroupStack& stack){stack.remove();});
    install(u"reset",[](BegingroupStack& stack){stack.reset();});
    install(u"sandbox",[](BegingroupStack& stack){stack.sandbox();});
    const ObjectRef handlerConfig=context.object();
    context.set(Value(handlerConfig),key(u"delimiter"),Value(context.array(1)));
    context.set(context.get(Value(handlerConfig),key(u"delimiter")),key(u"0"),Value(u"new-Delimiter"));
    context.set(Value(handlerConfig),key(u"environment"),Value(context.array(1)));
    context.set(context.get(Value(handlerConfig),key(u"environment")),key(u"0"),Value(u"new-Environment"));
    context.set(Value(handlerConfig),key(u"macro"),Value(context.array(2)));
    const Value macroArray=context.get(Value(handlerConfig),key(u"macro"));
    context.set(macroArray,key(u"0"),Value(u"new-Delimiter"));context.set(macroArray,key(u"1"),Value(u"new-Command"));
    context.define(constructor,key(u"handlerConfig"),Descriptor::data(Value(handlerConfig)));
    const ObjectRef handlerMap=context.object();
    context.set(Value(handlerMap),key(u"new-Delimiter"),Value(u"delimiter"));
    context.set(Value(handlerMap),key(u"new-Environment"),Value(u"environment"));
    context.set(Value(handlerMap),key(u"new-Command"),Value(u"macro"));
    context.define(constructor,key(u"handlerMap"),Descriptor::data(Value(handlerMap)));
}
}
