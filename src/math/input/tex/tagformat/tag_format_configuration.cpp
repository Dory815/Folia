// Native port of the single Source owner TagFormatConfiguration.ts.
module folia.math.input.tex.tagformat.tag_format_configuration;
import folia.math.input.tex.parser;
import folia.math.util.js_numeric_input;
import folia.math.util.option_functions;
import folia.math.util.js_number;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view text){return Key(std::u16string(text));}
[[noreturn]] void typeError(std::u16string text){throw Exception("TypeError",std::move(text));}
std::u16string toText(Context& context,const Value& value){
    const Value primitive=context.toPrimitive(value,PrimitiveHint::stringHint);
    if(const auto* text=std::get_if<std::u16string>(&primitive.data))return *text;
    if(std::holds_alternative<Undefined>(primitive.data))return u"undefined";
    if(std::holds_alternative<Null>(primitive.data))return u"null";
    if(const auto* boolean=std::get_if<bool>(&primitive.data))return *boolean?u"true":u"false";
    if(const auto* number=std::get_if<double>(&primitive.data))return property_host::text(js_number::toString(*number));
    if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data)){
        (void)context.symbolString(*symbol);
        typeError(u"Cannot convert a Symbol value to a string");
    }
    typeError(u"Cannot convert object to primitive value");
}
Value callOption(ParserRuntime& runtime,const ParseOptionsRef& parse,std::u16string_view name,std::span<const Value> args){
    Context& cx=runtime.context();const Value format=cx.get(parse->options,key(u"tagformat"));
    const Value callable=cx.get(format,key(name));
    const auto* function=std::get_if<ObjectRef>(&callable.data);
    if(!function||cx.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",std::u16string(u"tagformat.")+std::u16string(name)+u" is not a function");
    return cx.call(*function,format,args);
}
Value callMethod(Context& context,Value receiver,std::u16string_view name,
                 std::span<const Value> args,std::u16string_view expression){
    const Value method=context.get(receiver,key(name));
    const auto* function=std::get_if<ObjectRef>(&method.data);
    if(!function||context.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",std::u16string(expression)+u" is not a function");
    return context.call(*function,std::move(receiver),args);
}
std::u16string primitiveString(Context& context,const Value& value){
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    if(std::holds_alternative<Undefined>(value.data))return u"undefined";
    if(std::holds_alternative<Null>(value.data))return u"null";
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
    if(const auto* number=std::get_if<double>(&value.data))return property_host::text(js_number::toString(*number));
    if(const auto* symbol=std::get_if<SymbolRef>(&value.data)){
        (void)context.symbolString(*symbol);
        typeError(u"Cannot convert a Symbol value to a string");
    }
    typeError(u"Cannot convert object to primitive value");
}
void installStringAndNumberMethods(ParserRuntime& runtime){
    Context& context=runtime.context();
    const auto& intrinsic=context.intrinsics();
    if(!context.hasOwn(Value(intrinsic.numberPrototype),key(u"toString"))){
        const ObjectRef method=context.function(u"toString",[](Context&,Value receiver,std::span<const Value>)->Value{
            const auto* number=std::get_if<double>(&receiver.data);
            if(!number)throw Exception("TypeError",u"Number.prototype.toString requires that 'this' be a Number");
            return Value(property_host::text(js_number::toString(*number)));
        });
        context.define(method,key(u"length"),Descriptor::data(Value(0.0),false,false,true));
        context.define(intrinsic.numberPrototype,key(u"toString"),Descriptor::data(Value(method),true,false,true));
    }
    if(!context.hasOwn(Value(intrinsic.stringPrototype),key(u"replace"))){
        const ObjectRef method=context.function(u"replace",[&runtime](Context& context,Value receiver,std::span<const Value> args)->Value{
            const Value primitive=context.toPrimitive(receiver,PrimitiveHint::stringHint);
            const auto* text=std::get_if<std::u16string>(&primitive.data);
            if(!text)typeError(u"String.prototype.replace called on a non-string value");
            if(args.size()<2)throw Exception("TypeError",u"String.prototype.replace requires a pattern and replacement");
            const auto* regexp=std::get_if<ObjectRef>(&args[0].data);
            if(!regexp)throw Exception("TypeError",u"String.prototype.replace requires an actual RegExp pattern");
            auto expression=runtime.regexp().resolve(args[0]);
            expression->lastIndex=0;
            const std::u16string replacement=toText(context,args[1]);
            std::u16string result;
            std::size_t copied=0;
            while(const auto match=expression->exec(*text)){
                const auto matched=match->at(0);
                if(!matched)break;
                if(match->index<copied||match->index+matched->size()>text->size())
                    throw Exception("TypeError",u"RegExp match index is invalid");
                result.append(text->substr(copied,match->index-copied));
                result+=replacement;
                copied=match->index+matched->size();
            }
            result.append(text->substr(copied));
            return Value(std::move(result));
        });
        context.define(method,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
        context.define(intrinsic.stringPrototype,key(u"replace"),Descriptor::data(Value(method),true,false,true));
    }
}
std::u16string encodeComponent(std::u16string_view input){
    constexpr char hex[]="0123456789ABCDEF";
    std::u16string output;
    const auto emit=[&](std::uint32_t cp){
        std::string bytes;
        if(cp<=0x7f)bytes.push_back(static_cast<char>(cp));
        else if(cp<=0x7ff){bytes.push_back(static_cast<char>(0xc0|(cp>>6)));bytes.push_back(static_cast<char>(0x80|(cp&63)));}
        else if(cp<=0xffff){bytes.push_back(static_cast<char>(0xe0|(cp>>12)));bytes.push_back(static_cast<char>(0x80|((cp>>6)&63)));bytes.push_back(static_cast<char>(0x80|(cp&63)));}
        else{bytes.push_back(static_cast<char>(0xf0|(cp>>18)));bytes.push_back(static_cast<char>(0x80|((cp>>12)&63)));bytes.push_back(static_cast<char>(0x80|((cp>>6)&63)));bytes.push_back(static_cast<char>(0x80|(cp&63)));}
        for(const unsigned char byte:bytes){
            const bool safe=(byte>='A'&&byte<='Z')||(byte>='a'&&byte<='z')||(byte>='0'&&byte<='9')||
                byte=='-'||byte=='_'||byte=='.'||byte=='!'||byte=='~'||byte=='*'||byte=='\''||byte=='('||byte==')';
            if(safe)output.push_back(static_cast<char16_t>(byte));
            else{output.push_back(u'%');output.push_back(static_cast<char16_t>(hex[byte>>4]));output.push_back(static_cast<char16_t>(hex[byte&15]));}
        }
    };
    for(std::size_t i=0;i<input.size();++i){
        std::uint32_t cp=static_cast<std::uint16_t>(input[i]);
        if(cp>=0xd800&&cp<=0xdbff){
            if(i+1>=input.size())throw Exception("URIError",u"URI malformed");
            const auto low=static_cast<std::uint16_t>(input[++i]);
            if(low<0xdc00||low>0xdfff)throw Exception("URIError",u"URI malformed");
            cp=0x10000+((cp-0xd800)<<10)+(low-0xdc00);
        }else if(cp>=0xdc00&&cp<=0xdfff)throw Exception("URIError",u"URI malformed");
        emit(cp);
    }
    return output;
}
}
ConfigurationRef TagFormatConfiguration::initializeModule(ParserRuntime& runtime){
    const auto prior=runtime.configurationHandler.get(u"tagformat");
    if(const auto* loaded=std::get_if<ConfigurationRef>(&prior);loaded&&*loaded)return *loaded;
    Context& cx=runtime.context();
    installStringAndNumberMethods(runtime);
    const auto tagID=std::make_shared<std::uint64_t>(0);
    auto body=std::make_shared<Function>();
    body->sourceMetadata=FunctionSourceMetadata{u"tagformatConfig",2};
    body->call=[&runtime,tagID](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"TagFormat config arguments are missing");
        const auto* jax=std::any_cast<TeX*>(&args[1]);if(!jax||!*jax)throw Exception("TypeError",u"TagFormat config TeX is invalid");
        const auto* configuration=std::any_cast<ParserConfigurationRef>(&args[0]);
        if(!configuration||!*configuration)throw Exception("TypeError",u"TagFormat config parser is invalid");
        const auto shared=(*jax)->parseOptions();const auto* parse=std::get_if<ParseOptionsRef>(&shared);
        if(!parse||!*parse)property_host::typeError("Cannot read properties of null or undefined (reading 'options')");
        Context& context=runtime.context();const Value selected=context.get((*parse)->options,key(u"tags"));
        const auto* name=std::get_if<std::u16string>(&selected.data);if(!name)throw Exception("TypeError",u"Tags class name is not a string");
        if(*name!=u"base"){
            const Value customTags=(*configuration)->sourceField(u"tags");
            if(context.hasOwn(customTags,key(*name))){
                const Value customClass=context.get(customTags,key(*name));
                const auto* constructor=std::get_if<ObjectRef>(&customClass.data);
                if(!constructor)throw Exception("TypeError",u"Tags class must be a constructor object");
                const auto actual=runtime.tagClassIdentities.resolve(*constructor);
                if(!actual)throw Exception("TypeError",u"Tags class constructor is unavailable");
                runtime.tagsFactory.add(*name,actual);
            }
        }
        const TagsRef original=runtime.tagsFactory.create(*name);
        const Value constructor=context.get(Value(original->sourceIdentity()),key(u"constructor"));
        const auto* parent=std::get_if<ObjectRef>(&constructor.data);if(!parent)throw Exception("TypeError",u"Tags class constructor is unavailable");
        const ParseOptionsRef parseOptions=*parse;
        const TagsClassRef tagClass=TagsFactory::registerSourceSubclass(runtime,u"TagFormat",*parent,{},
            [&runtime,parseOptions](ObjectRef prototype){
                Context& cx=runtime.context();
                const auto install=[&](std::u16string name,double length,
                    std::function<Value(Context&,Value,std::span<const Value>)> call){
                    const ObjectRef function=cx.function(u"",std::move(call));
                    cx.define(function,key(u"length"),Descriptor::data(Value(length),false,false,true));
                    cx.define(prototype,key(name),Descriptor::data(Value(function),true,true,true));
                };
                const auto optionCall=[&runtime,parseOptions](std::u16string_view option,std::span<const Value> args){
                    return callOption(runtime,parseOptions,option,args);
                };
                install(u"formatNumber",1,[optionCall](Context&,Value,std::span<const Value> args){return optionCall(u"number",args);});
                install(u"formatTag",1,[optionCall](Context&,Value,std::span<const Value> args){return optionCall(u"tag",args);});
                install(u"formatRef",1,[&runtime,parseOptions](Context&,Value receiver,std::span<const Value> args){
                    Context& context=runtime.context();
                    const Value format=context.get(parseOptions->options,key(u"tagformat"));
                    const Value ref=context.get(format,key(u"ref"));
                    if(options::truthy(ref)){
                        const auto* function=std::get_if<ObjectRef>(&ref.data);
                        if(!function||context.kind(*function)!=ObjectKind::function)
                            throw Exception("TypeError",u"tagformat.ref is not a function");
                        return context.call(*function,Value(Undefined{}),args);
                    }
                    const Value method=context.get(receiver,key(u"formatTag"));
                    const auto* function=std::get_if<ObjectRef>(&method.data);
                    if(!function||context.kind(*function)!=ObjectKind::function)
                        throw Exception("TypeError",u"this.formatTag is not a function");
                    return context.call(*function,receiver,args);
                });
                install(u"formatId",1,[optionCall](Context&,Value,std::span<const Value> args){return optionCall(u"id",args);});
                install(u"formatUrl",2,[optionCall](Context&,Value,std::span<const Value> args){return optionCall(u"url",args);});
            });
        const std::u16string tagName=u"configTags-"+property_host::text(js_number::toString(static_cast<double>(++*tagID)));
        runtime.tagsFactory.add(tagName,tagClass);
        context.set((*parse)->options,key(u"tags"),Value(tagName));
        return std::monostate{};
    };
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    binding.arguments=[&runtime](Context&,std::span<const Value> args){
        if(args.size()<2)throw Exception("TypeError",u"TagFormat config arguments are missing");
        const auto* config=std::get_if<ObjectRef>(&args[0].data);const auto* jax=std::get_if<ObjectRef>(&args[1].data);
        if(!config||!jax)throw Exception("TypeError",u"TagFormat config identities are invalid");
        return std::vector<std::any>{std::any(runtime.parserConfigurationIdentities.resolve(*config)),std::any(runtime.texIdentities.resolve(*jax).get())};
    };
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    const Value config=runtime.functions().bind(body,std::move(binding),u"tagformatConfig");
    if(const auto* fn=std::get_if<ObjectRef>(&config.data))cx.define(*fn,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    const auto optionsObject=cx.object();
    const auto callback=[&](std::u16string name,double length,std::function<Value(Context&,std::span<const Value>)> call){
        const ObjectRef function=cx.function(name,[call=std::move(call)](Context& context,Value,std::span<const Value> args){return call(context,args);});
        cx.define(function,key(u"length"),Descriptor::data(Value(length),false,false,true));return Value(function);
    };
    const Value number=callback(u"number",1,[](Context& context,std::span<const Value> args){
        const Value value=args.empty()?Value(Undefined{}):args[0];
        const std::array<Value,0> noArgs{};
        return callMethod(context,value,u"toString",noArgs,u"n.toString");
    });
    const Value tag=callback(u"tag",1,[](Context& context,std::span<const Value> args){
        const ObjectRef out=context.array(3);context.set(out,key(u"0"),u"(");context.set(out,key(u"1"),args.empty()?Value(Undefined{}):args[0]);context.set(out,key(u"2"),u")");return Value(out);
    });
    const Value id=callback(u"id",1,[&runtime](Context& context,std::span<const Value> args){
        const Value input=args.empty()?Value(Undefined{}):args[0];
        const ObjectRef whitespace=runtime.regexp().create(u"\\s",u"g");
        const std::array<Value,2> replaceArgs{Value(whitespace),Value(u"_")};
        const Value replaced=callMethod(context,input,u"replace",replaceArgs,u"id.replace");
        return Value(std::u16string(u"mjx-eqn:")+primitiveString(context,context.toPrimitive(replaced,PrimitiveHint::defaultHint)));
    });
    const Value url=callback(u"url",2,[](Context& context,std::span<const Value> args){
        const std::u16string base=args.size()>1?toText(context,args[1]):u"undefined";
        const std::u16string id=args.empty()?u"undefined":toText(context,args[0]);
        return Value(base+u"#"+encodeComponent(id));
    });
    cx.set(optionsObject,key(u"number"),number);cx.set(optionsObject,key(u"tag"),tag);cx.set(optionsObject,key(u"ref"),Value(u""));
    cx.set(optionsObject,key(u"id"),id);cx.set(optionsObject,key(u"url"),url);
    const ObjectRef options=cx.object();cx.set(options,key(u"tagformat"),Value(optionsObject));
    const ObjectRef configTuple=cx.array(2);cx.set(configTuple,key(u"0"),config);cx.set(configTuple,key(u"1"),Value(10.0));
    const ObjectRef definition=cx.object();cx.set(definition,key(u"config"),Value(configTuple));cx.set(definition,key(u"options"),Value(options));
    return Configuration::create(runtime,u"tagformat",Value(definition));
}
void tagformatConfig(ParserRuntime& runtime,ParserConfiguration& configuration,TeX& jax){
    const auto loaded=runtime.configurationHandler.get(u"tagformat");
    const auto* registered=std::get_if<ConfigurationRef>(&loaded);
    if(!registered||!*registered)throw Exception("TypeError",u"TagFormat configuration is unavailable");
    const Value callback=(*registered)->config();
    const auto* function=std::get_if<ObjectRef>(&callback.data);
    if(!function||runtime.context().kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",u"tagformatConfig is not callable");
    const auto configIdentity=runtime.parserConfigurationIdentities.find(configuration.shared_from_this());
    if(!configIdentity)throw Exception("TypeError",u"ParserConfiguration identity is unavailable");
    const std::array<Value,2> args{Value(*configIdentity),Value(jax.instanceIdentity())};
    (void)runtime.context().call(*function,Value(Undefined{}),args);
}
}
