// MathJax 4.1.3 input/tex/TexParser.ts source methods; Apache-2.0.
module folia.math.input.tex.parser;
import :runtime;
import :parse_options;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.node_util;
import folia.math.input.tex.node_factory;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.unit_util;
import folia.math.util.property_host;
import folia.math.util.utf16;
import folia.math.cancellation;
import folia.math.input.tex.tex_parser_context_fields;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value arg(std::span<const Value> values,std::size_t i){return i<values.size()?values[i]:Value(Undefined{});}
std::u16string requireText(const Value& value,std::u16string_view name){
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    throw Exception("TypeError",std::u16string(name)+u" must be a string");
}
std::u16string propText(const Property& value){return property_host::string(value);}
Property fromOptions(const Value& value){
    if(std::holds_alternative<Undefined>(value.data))return Missing{};
    if(std::holds_alternative<Null>(value.data))return PropertyNull{};
    if(const auto* b=std::get_if<bool>(&value.data))return *b;
    if(const auto* n=std::get_if<double>(&value.data))return *n;
    if(const auto* s=std::get_if<std::u16string>(&value.data))return property_host::debugText(*s);
    throw tree::FactoryError("Unimplemented",u"TexParser callback result needs the native Property identity binding");
}
TexParser& requireParser(ParserContext& parser){
    auto* result=dynamic_cast<TexParser*>(&parser);
    if(!result)property_host::typeError("TexParser method called with a foreign ParserContext");
    return *result;
}
ParseOptions& requireConfiguration(const TexParser& parser,std::string_view member="configuration"){
    if(const auto* p=std::get_if<ParseOptions*>(&parser.configuration);p&&*p)return **p;
    property_host::typeError(std::string("Cannot read properties of undefined (reading '")+std::string(member)+"')");
}
StackItemFactory& requireItemFactory(const ParserShared<StackItemFactory>& value){
    if(const auto* p=std::get_if<std::shared_ptr<StackItemFactory>>(&value);p&&*p)return **p;
    property_host::typeError("Cannot read properties of null or undefined (reading 'create')");
}
Tags& requireTags(const ParserShared<Tags>& value){
    if(const auto* p=std::get_if<std::shared_ptr<Tags>>(&value);p&&*p)return **p;
    property_host::typeError("Cannot read properties of null or undefined (reading 'tags')");
}
Stack& requireStack(const ParserShared<Stack>& value){
    if(const auto* p=std::get_if<std::shared_ptr<Stack>>(&value);p&&*p)return **p;
    property_host::typeError("Cannot read properties of null or undefined (reading 'Top')");
}
SubHandlers& requireHandlers(const ParserShared<SubHandlers>& value){
    if(const auto* p=std::get_if<std::shared_ptr<SubHandlers>>(&value);p&&*p)return **p;
    property_host::typeError("Cannot read properties of null or undefined (reading 'get')");
}
std::u16string trim_source(std::u16string text){
    const auto white=[](char16_t c){
        return c==u' '||c==u'\t'||c==u'\n'||c==u'\r'||c==u'\v'||c==u'\f'||c==0x00A0||c==0x1680||
            (c>=0x2000&&c<=0x200A)||c==0x2028||c==0x2029||c==0x202F||c==0x205F||c==0x3000||c==0xFEFF;
    };
    std::size_t first=0,last=text.size();while(first<last&&white(text[first]))++first;while(last>first&&white(text[last-1]))--last;
    return text.substr(first,last-first);
}
std::size_t sliceIndex(double value,std::size_t length){
    if(std::isnan(value)||value==0)return 0;
    if(value==std::numeric_limits<double>::infinity())return length;
    if(value==-std::numeric_limits<double>::infinity())return 0;
    const double integer=std::trunc(value),size=static_cast<double>(length);
    return static_cast<std::size_t>(integer<0?std::max(size+integer,0.0):std::min(integer,size));
}
std::u16string string_slice(std::u16string_view source,double start,double end){
    const std::size_t first=sliceIndex(start,source.size()),last=sliceIndex(end,source.size());
    if(last<=first)return {};
    return std::u16string(source.substr(first,last-first));
}
[[noreturn]] void throwTex(std::u16string id,std::u16string_view message,std::span<const Property> args={}){throw TexError(std::move(id),message,args);}
void initializeParserConstructor(ParserRuntime& runtime){
    if(!std::holds_alternative<Undefined>(runtime.texParserClass.data))return;
    Context& cx=runtime.context();
    const ObjectRef ctor=cx.nativeConstructor(u"TexParser",[&runtime](Context& context,std::span<const Value> args)->ObjectRef{
        const std::u16string text=requireText(arg(args,0),u"string");
        const Value env=arg(args,1), configurationValue=arg(args,2);
        const auto* configObject=std::get_if<ObjectRef>(&configurationValue.data);
        if(!configObject)throw Exception("TypeError",u"TexParser requires its ParseOptions identity");
        ParseOptions& config=*runtime.parseOptionsIdentities.resolve(*configObject);
        if(std::holds_alternative<ObjectRef>(env.data))
            throw Exception("Unimplemented",u"TexParser constructor object env requires the canonical Property graph binding");
        Property nativeEnv=fromOptions(env);
        const auto parser=TexParser::make(text,std::move(nativeEnv),config);
        return parser->instanceIdentity();
    });
    cx.define(ctor,key(u"length"),Descriptor::data(Value(3),false,false,true));
    runtime.texParserClass=Value(ctor);
}
}

TexParserRef TexParser::make(std::u16string text,Property env,ParseOptions& configuration){
    ParserRuntime& runtime=configuration.runtime();
    if(std::holds_alternative<Undefined>(runtime.texParserClass.data))
        throw tree::FactoryError("Unimplemented",u"TexParser class constructor must be registered during loaded-module initialization");
    auto parser=std::make_shared<TexParser>();
    const ObjectRef constructor=std::get<ObjectRef>(runtime.texParserClass.data);
    const ObjectRef identity=runtime.parserIdentities.instance(parser,constructor,parser);
    parser->identity_=identity;
    parser->configuration=&configuration;
    parser->initializeTexParser(std::move(text),std::move(env),configuration);
    return parser;
}

ParserContextIdentity TexParser::contextIdentity()const{
    const auto* config=std::get_if<ParseOptions*>(&configuration);
    if(!config||!*config)throw options::Exception("TypeError",u"Parser has no live ParseOptions Context identity");
    auto& runtime=(**config).runtime();
    return {&runtime.context(),instanceIdentity()};
}

void TexParser::initializeModule(ParserRuntime& runtime){
    initializeParserConstructor(runtime);
    Context& cx=runtime.context();
    const ObjectRef ctor=std::get<ObjectRef>(runtime.texParserClass.data);
    const Value prototypeValue=cx.get(ctor,key(u"prototype"));
    const auto* prototype=std::get_if<ObjectRef>(&prototypeValue.data);
    if(!prototype)throw Exception("TypeError",u"TexParser constructor has no prototype");
    const ObjectRef proto=*prototype;

    const auto valueAt=[](FunctionArgs args,std::size_t index)->Value{
        if(index>=args.size())return Undefined{};
        if(const auto* value=std::any_cast<Value>(&args[index]))return *value;
        throw Exception("TypeError",u"TexParser native adapter expected a Context value");
    };
    const auto stringAt=[valueAt](FunctionArgs args,std::size_t index,std::u16string_view name){return requireText(valueAt(args,index),name);};
    const auto propertyValue=[](const Value& value){return fromOptions(value);};
    auto toValue=std::make_shared<std::function<Value(const std::any&)>>();
    *toValue=[&runtime,toValue](const std::any& result)->Value{
        if(const auto* value=std::any_cast<Value>(&result))return *value;
        if(const auto* property=std::any_cast<Property>(&result)){
            if(property->missing())return Undefined{};
            if(std::holds_alternative<PropertyNull>(property->value))return Null{};
            if(const auto* b=std::get_if<bool>(&property->value))return *b;
            if(const auto* n=std::get_if<double>(&property->value))return *n;
            if(const auto* s=std::get_if<std::string>(&property->value))return property_host::text(*s);
            throw Exception("Unimplemented",u"TexParser result requires a canonical Context object identity");
        }
        if(const auto* b=std::any_cast<bool>(&result))return *b;
        if(const auto* n=std::any_cast<double>(&result))return *n;
        if(const auto* s=std::any_cast<std::u16string>(&result))return *s;
        if(std::any_cast<std::monostate>(&result))return Undefined{};
        if(const auto* tags=std::any_cast<ParserShared<Tags>>(&result)){
            if(std::holds_alternative<Missing>(*tags))return Undefined{};
            if(std::holds_alternative<PropertyNull>(*tags))return Null{};
            const auto& owner=std::get<std::shared_ptr<Tags>>(*tags);
            if(const auto identity=runtime.tagIdentities.find(owner))return *identity;
            throw Exception("Unimplemented",u"TexParser tags getter has no canonical Context identity");
        }
        if(const auto* parsed=std::any_cast<ParseResult>(&result)){
            if(const auto* property=std::get_if<Property>(parsed))return (*toValue)(std::any(*property));
            if(const auto* item=std::get_if<StackItemRef>(parsed);item&&*item){
                if(const auto identity=runtime.stackItemIdentities.find(*item))return *identity;
                throw Exception("Unimplemented",u"TexParser result StackItem has no canonical Context identity");
            }
            throw Exception("Unimplemented",u"TexParser ParseSymbol result has no canonical Context identity");
        }
        if(std::any_cast<mml::NodeValue>(&result))
            throw Exception("Unimplemented",u"TexParser MML result has no canonical Context identity");
        if(const auto* lookup=std::any_cast<TokenLookup>(&result))
            return runtime.tokenMaps.tokenLookupToValue(*lookup);
        if(std::any_cast<ParserShared<StackItemFactory>>(&result))
            throw Exception("Unimplemented",u"TexParser itemFactory getter has no canonical Context identity");
        throw Exception("Unimplemented",u"TexParser native result has no Context conversion");
    };
    const auto addMethod=[&](std::u16string_view name,double length,
            std::function<std::any(TexParser&,FunctionArgs)> invoke){
        auto function=std::make_shared<Function>();
        function->call=[invoke](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
            const auto* owner=std::get_if<NativeObjectRef>(&receiver);
            const auto parser=owner?std::dynamic_pointer_cast<TexParser>(*owner):TexParserRef{};
            if(!parser)throw Exception("TypeError",u"TexParser method receiver is incompatible");
            return invoke(*parser,args);
        };
        OptionCallBinding binding;
        binding.receiver=[&runtime](Context&,Value receiver)->FunctionReceiver{
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)throw Exception("TypeError",u"TexParser method receiver is not an object");
            return std::static_pointer_cast<NativeObject>(runtime.parserIdentities.resolve(*object));
        };
        binding.arguments=[](Context&,std::span<const Value> args){
            std::vector<std::any> result;result.reserve(args.size());for(const auto& arg:args)result.emplace_back(arg);return result;
        };
        binding.result=[toValue](Context&,const std::any& result){return (*toValue)(result);};
        Value bound=runtime.functions().bind(function,std::move(binding),u"");
        const auto* functionObject=std::get_if<ObjectRef>(&bound.data);
        if(!functionObject)throw Exception("TypeError",u"TexParser method binding is not callable");
        cx.define(*functionObject,key(u"name"),Descriptor::data(Value(u""),false,false,true));
        cx.define(*functionObject,key(u"length"),Descriptor::data(Value(length),false,false,true));
        cx.define(proto,key(name),Descriptor::data(bound,true,true,true));
    };
    const auto addGetter=[&](std::u16string_view name,std::function<std::any(TexParser&)> invoke){
        auto function=std::make_shared<Function>();
        function->call=[invoke](const FunctionReceiver& receiver,FunctionArgs)->std::any{
            const auto* owner=std::get_if<NativeObjectRef>(&receiver);
            const auto parser=owner?std::dynamic_pointer_cast<TexParser>(*owner):TexParserRef{};
            if(!parser)throw Exception("TypeError",u"TexParser getter receiver is incompatible");
            return invoke(*parser);
        };
        OptionCallBinding binding;
        binding.receiver=[&runtime](Context&,Value receiver)->FunctionReceiver{
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)throw Exception("TypeError",u"TexParser getter receiver is not an object");
            return std::static_pointer_cast<NativeObject>(runtime.parserIdentities.resolve(*object));
        };
        binding.arguments=[](Context&,std::span<const Value>){return std::vector<std::any>{};};
        binding.result=[toValue](Context&,const std::any& result){return (*toValue)(result);};
        const Value bound=runtime.functions().bind(function,std::move(binding),u"get");
        const auto* getter=std::get_if<ObjectRef>(&bound.data);
        if(!getter)throw Exception("TypeError",u"TexParser getter binding is not callable");
        cx.define(*getter,key(u"name"),Descriptor::data(Value(u"get"),false,false,true));
        cx.define(*getter,key(u"length"),Descriptor::data(Value(0),false,false,true));
        cx.define(proto,key(name),Descriptor::access(*getter,std::nullopt,false,true));
    };
    const auto addSetter=[&](std::u16string_view name,std::function<std::any(TexParser&,FunctionArgs)> invoke){
        auto function=std::make_shared<Function>();
        function->call=[invoke](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
            const auto* owner=std::get_if<NativeObjectRef>(&receiver);
            const auto parser=owner?std::dynamic_pointer_cast<TexParser>(*owner):TexParserRef{};
            if(!parser)throw Exception("TypeError",u"TexParser setter receiver is incompatible");
            return invoke(*parser,args);
        };
        OptionCallBinding binding;
        binding.receiver=[&runtime](Context&,Value receiver)->FunctionReceiver{
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)throw Exception("TypeError",u"TexParser setter receiver is not an object");
            return std::static_pointer_cast<NativeObject>(runtime.parserIdentities.resolve(*object));
        };
        binding.arguments=[](Context&,std::span<const Value> args){
            std::vector<std::any> result;result.reserve(args.size());for(const auto& arg:args)result.emplace_back(arg);return result;
        };
        binding.result=[toValue](Context&,const std::any& result){return (*toValue)(result);};
        const Value bound=runtime.functions().bind(function,std::move(binding),u"set");
        const auto* setter=std::get_if<ObjectRef>(&bound.data);
        if(!setter)throw Exception("TypeError",u"TexParser setter binding is not callable");
        cx.define(*setter,key(u"name"),Descriptor::data(Value(u"set"),false,false,true));
        cx.define(*setter,key(u"length"),Descriptor::data(Value(1),false,false,true));
        const auto getter=cx.descriptor(Value(proto),key(name));
        if(!getter||!getter->accessor)throw Exception("TypeError",u"TexParser string property is not an accessor");
        cx.define(proto,key(name),Descriptor::access(getter->getter,*setter,false,true));
    };

    addGetter(u"options",[](TexParser& parser)->std::any{return std::any(parser.options());});
    addGetter(u"itemFactory",[](TexParser& parser)->std::any{return std::any(parser.itemFactory());});
    addGetter(u"tags",[](TexParser& parser)->std::any{return std::any(parser.tags());});
    addGetter(u"string",[](TexParser& parser)->std::any{return std::any(parser.string());});
    addSetter(u"string",[stringAt](TexParser& parser,FunctionArgs args)->std::any{parser.setString(stringAt(args,0,u"string"));return std::any(std::monostate{});});

    addMethod(u"parse",2,[valueAt,stringAt,propertyValue,&runtime](TexParser& parser,FunctionArgs args)->std::any{
        const Property kind=Property(property_host::debugText(stringAt(args,0,u"kind")));
        const Value tuple=valueAt(args,1);
        const Value sourceParser=runtime.context().get(tuple,key(u"0"));
        const auto* identity=std::get_if<ObjectRef>(&sourceParser.data);
        if(!identity)throw Exception("Unimplemented",u"TexParser.parse input receiver has no canonical parser identity");
        const auto inputParser=runtime.parserIdentities.resolve(*identity);
        const Value token=runtime.context().get(tuple,key(u"1"));
        return std::any(parser.parse(kind,ParseInput{*inputParser,propertyValue(token)}));
    });
    addMethod(u"lookup",2,[stringAt](TexParser& parser,FunctionArgs args)->std::any{
        return std::any(parser.lookup(Property(property_host::debugText(stringAt(args,0,u"kind"))),stringAt(args,1,u"token")));
    });
    addMethod(u"contains",2,[stringAt](TexParser& parser,FunctionArgs args)->std::any{
        return std::any(parser.contains(Property(property_host::debugText(stringAt(args,0,u"kind"))),stringAt(args,1,u"token")));
    });
    addMethod(u"toString",0,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.toString());});
    addMethod(u"Parse",0,[](TexParser& parser,FunctionArgs)->std::any{parser.Parse();return std::any(std::monostate{});});
    addMethod(u"Push",1,[valueAt,&runtime](TexParser& parser,FunctionArgs args)->std::any{
        const Value value=valueAt(args,0);const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object)throw Exception("Unimplemented",u"TexParser.Push input is not a bound StackItem or MML node");
        parser.Push(runtime.stackItemIdentities.resolve(*object));return std::any(std::monostate{});
    });
    addMethod(u"PushAll",1,[valueAt,&runtime](TexParser& parser,FunctionArgs args)->std::any{
        const Value value=valueAt(args,0);
        Context& context=runtime.context();
        context.forEachIterable(value,[&](Value item){
            StackEntry entry=Property(Missing{});
            if(const auto* object=std::get_if<ObjectRef>(&item.data))entry=runtime.stackItemIdentities.resolve(*object);
            else entry=fromOptions(item);
            const auto& stackItem=std::get<std::shared_ptr<Stack>>(parser.stack);
            stackItem->Push(std::span<const StackEntry>(&entry,1));
        });
        return std::any(std::monostate{});
    });
    addMethod(u"mml",0,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.mml());});
    addMethod(u"convertDelimiter",1,[stringAt](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.convertDelimiter(stringAt(args,0,u"c")));});
    addMethod(u"getCodePoint",0,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.getCodePoint());});
    addMethod(u"nextIsSpace",0,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.nextIsSpace());});
    addMethod(u"GetNext",0,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.GetNext());});
    addMethod(u"GetCS",0,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.GetCS());});
    addMethod(u"GetArgument",2,[valueAt](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.GetArgument(u"",options::truthy(valueAt(args,1))));});
    addMethod(u"GetBrackets",3,[valueAt,propertyValue](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.GetBrackets(u"",propertyValue(valueAt(args,1)),options::truthy(valueAt(args,2))));});
    addMethod(u"GetDelimiter",2,[valueAt](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.GetDelimiter(u"",options::truthy(valueAt(args,1))));});
    addMethod(u"GetDimen",1,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.GetDimen(u""));});
    addMethod(u"GetUpTo",2,[stringAt](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.GetUpTo(u"",stringAt(args,1,u"token")));});
    addMethod(u"ParseArg",1,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.ParseArg(u""));});
    addMethod(u"ParseUpTo",2,[stringAt](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.ParseUpTo(u"",stringAt(args,1,u"token")));});
    addMethod(u"GetDelimiterArg",1,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.GetDelimiterArg(u""));});
    addMethod(u"GetStar",0,[](TexParser& parser,FunctionArgs)->std::any{return std::any(parser.GetStar());});
    addMethod(u"create",1,[stringAt](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.create(stringAt(args,0,u"kind"),args.subspan(std::min<std::size_t>(1,args.size()))));});
    addMethod(u"trimTex",1,[stringAt](TexParser& parser,FunctionArgs args)->std::any{return std::any(parser.trimTex(stringAt(args,0,u"tex")));});
}

void TexParser::initializeTexParser(std::u16string text,Property env,ParseOptions& config){
    ParserRuntime& runtime=config.runtime();
    Context& cx=runtime.context();
    const ObjectRef identity=instanceIdentity();
    const TexParserRef owner=runtime.parserIdentities.resolve(identity);
    TexParserContextFields sourceFields(cx,identity,std::static_pointer_cast<void>(owner),runtime.executionLifetime());
    auto assignString=[&](std::u16string_view name,std::u16string& cell){
        auto* target=&cell;
        sourceFields.assign(name,Value(cell),[target]{return Value(*target);},[target](Value value){
            const auto* string=std::get_if<std::u16string>(&value.data);
            if(!string)throw Exception("TypeError",u"TexParser string field must be a string");
            *target=*string;
        });
    };
    auto assignNumber=[&](std::u16string_view name,double& cell){
        auto* target=&cell;
        sourceFields.assign(name,Value(cell),[target]{return Value(*target);},[target](Value value){
            const auto* number=std::get_if<double>(&value.data);
            if(!number)throw Exception("TypeError",u"TexParser numeric field must be a number");
            *target=*number;
        });
    };
    // Source assigns these own fields before it reads/deletes env.isInner.
    string_=std::move(text);assignString(u"_string",string_);
    configuration=&config;
    const auto initialConfiguration=runtime.parseOptionsIdentities.find(config.shared_from_this());
    if(!initialConfiguration)throw Exception("TypeError",u"TexParser configuration has no canonical ParseOptions identity");
    sourceFields.assign(u"configuration",Value(*initialConfiguration),
        [&runtime,this](){
            if(const auto* parse=std::get_if<ParseOptions*>(&configuration);parse&&*parse){
                const auto identity=runtime.parseOptionsIdentities.find((*parse)->shared_from_this());
                if(identity)return Value(*identity);
            }
            return std::holds_alternative<PropertyNull>(configuration)?Value(Null{}):Value(Undefined{});
        },[&runtime,this](Value value){
            if(std::holds_alternative<Undefined>(value.data)){configuration=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){configuration=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"TexParser.configuration must be ParseOptions");
            configuration=runtime.parseOptionsIdentities.resolve(*object).get();
        });
    macroCount=0;assignNumber(u"macroCount",macroCount);
    i=0;assignNumber(u"i",i);
    currentCS_=Property(std::string{});
    sourceFields.assign(u"currentCS",runtime.properties.to(currentCS_),[this,&runtime]{return runtime.properties.to(currentCS_);},
        [this,&runtime](Value value){currentCS_=runtime.properties.from(std::move(value));});
    saveI=0;assignNumber(u"saveI",saveI);

    // Preserve Source's order from the first env operation onward.
    auto own=PropertyList::record(env);
    if(!own)property_host::typeError(env.missing()?"Cannot convert undefined or null to object":"Cannot convert undefined or null to object");
    const bool inner=own->hasOwn("isInner");
    const Property innerValue=own->get("isInner");
    own->erase("isInner");
    Property environment=Missing{};
    if(property_host::truthy(env)){
        PropertyList copied;
        for(const auto& name:property_host::ownKeys(env))copied.set(name,property_host::get(env,name));
        environment=std::move(copied);
    }
    requireConfiguration(*this).pushParser(runtime.parserIdentities.resolve(instanceIdentity()));
    auto stackFactory=std::make_shared<Stack>(
        StackFactoryReference(&requireItemFactory(config.itemFactory)),environment,
        inner?innerValue:Property(true));
    stack=stackFactory;
    const ObjectRef stackIdentity=runtime.bindStack(stackFactory);
    sourceFields.assign(u"stack",Value(stackIdentity),[&runtime,this](){
        if(const auto* stack=std::get_if<std::shared_ptr<Stack>>(&this->stack);stack&&*stack)
            return Value(runtime.bindStack(*stack));
        return std::holds_alternative<PropertyNull>(this->stack)?Value(Null{}):Value(Undefined{});
    },[&runtime,this](Value value){
        if(std::holds_alternative<Undefined>(value.data)){this->stack=Missing{};return;}
        if(std::holds_alternative<Null>(value.data)){this->stack=PropertyNull{};return;}
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object)throw Exception("TypeError",u"TexParser.stack must be a Stack");
        this->stack=runtime.resolveStack(*object);
    });
    this->Parse();
    const std::array<std::any,0> noArguments{};
    const auto stop=requireItemFactory(config.itemFactory).create("stop",{});
    this->Push(requireItemFactory(config.itemFactory).reference(stop));
    requireStack(stack).setEnv(environment);
}

options::Value TexParser::options()const{return requireConfiguration(*this).options;}
ParserShared<StackItemFactory> TexParser::itemFactory()const{return requireConfiguration(*this).itemFactory;}
ParserShared<Tags> TexParser::tags()const{return requireConfiguration(*this).tags;}
void TexParser::setString(std::u16string value){string_=std::move(value);}
std::u16string TexParser::string()const{return string_;}
Property TexParser::currentCS()const{return currentCS_;}
void TexParser::setCurrentCS(Property value){currentCS_=std::move(value);}

ParseResult TexParser::parse(Property kind,const ParseInput& input){
    cancellation::checkpoint(cancellation::CheckpointStage::TeX);
    const double old=saveI;
    const bool character=kind==HandlerType.get("CHARACTER");
    const Property token=input.token;
    const std::u16string tokenText=propText(token);
    saveI=i-(character&&tokenText!=u"&"?static_cast<double>(tokenText.size()):0);
    const std::string name=property_host::debugText(propText(kind));
    auto& handler=requireHandlers(requireConfiguration(*this).handlers);
    const auto selected=handler.get(name);
    if(!selected)property_host::typeError("Cannot read properties of undefined (reading 'parse')");
    ParseResult result=selected->parse(input);
    if(!(kind==HandlerType.get("MACRO")))updateResult(tokenText,old);
    saveI=old;
    return result;
}
TokenLookup TexParser::lookup(Property kind,std::u16string token){
    auto selected=requireHandlers(requireConfiguration(*this).handlers).get(property_host::debugText(propText(kind)));
    if(!selected)property_host::typeError("Cannot read properties of undefined (reading 'lookup')");
    return selected->lookup(Property(property_host::debugText(token)));
}
bool TexParser::contains(Property kind,std::u16string token){
    auto selected=requireHandlers(requireConfiguration(*this).handlers).get(property_host::debugText(propText(kind)));
    if(!selected)property_host::typeError("Cannot read properties of undefined (reading 'contains')");
    return selected->contains(Property(property_host::debugText(token)));
}
std::u16string TexParser::toString()const{
    std::u16string result;
    auto iterator=requireHandlers(requireConfiguration(*this).handlers).keys();
    std::vector<std::string> keys;
    while(const auto name=iterator.next())keys.push_back(*name);
    for(const auto& name:keys){
        result+=property_host::text(name);result+=u": ";
        const auto selected=requireHandlers(requireConfiguration(*this).handlers).get(name);
        if(!selected)property_host::typeError("Cannot read properties of undefined (reading 'toString')");
        result+=property_host::text(selected->toString());result+=u"\n";
    }
    return result;
}
void TexParser::Parse(){
    while(i<static_cast<double>(string_.size())){
        cancellation::checkpoint(cancellation::CheckpointStage::TeX);
        const std::u16string c=getCodePoint();i+=static_cast<double>(c.size());
        const ParseInput input{*this,Property(property_host::debugText(c))};
        (void)parse(HandlerType.get("CHARACTER"),input);
    }
}
void TexParser::Push(StackEntry entry){
    if(auto* item=std::get_if<StackItemRef>(&entry);item&&*item){
        if(auto* base=dynamic_cast<BaseItem*>(item->get())){base->startI=saveI;base->stopI=i;base->startStr=string_;}
    }
    if(auto* node=std::get_if<mml::NodeValue>(&entry);node&&node->isNode()&&NodeUtil::instance().isInferred(*node)){
        const auto* inferred=node->requireNode("childNodes");const auto children=inferred->childNodes;
        if(!children)property_host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        PushAll(children);return;
    }
    const std::array<StackEntry,1> values{std::move(entry)};requireStack(stack).Push(values);
}
void TexParser::PushAll(CheckList values){
    if(!values)property_host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for(std::size_t i=0;i<values->size();++i){const StackEntry& item=values->at(i);requireStack(stack).Push(std::span<const StackEntry>(&item,1));}
}
void TexParser::PushAll(mml::Children values){
    if(!values)property_host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    const auto captured=std::move(values);
    for(std::size_t index=0;index<captured->size();++index){
        const mml::NodeValue child=captured->get(index);
        const StackEntry item=child.isNode()?StackEntry(child):StackEntry(child.kind()==tree::ChildKind::Null?Property(PropertyNull{}):Property(Missing{}));
        requireStack(stack).Push(std::span<const StackEntry>(&item,1));
    }
}
mml::NodeValue TexParser::mml(){
    requireConfiguration(*this).popParser();
    const auto kindTop=requireStack(stack).Top();
    BaseItem& kindItem=stack_host::item(kindTop,"isKind");
    if(!kindItem.isKind("mml"))return PropertyNull{};
    const auto resultTop=requireStack(stack).Top();
    BaseItem& resultItem=stack_host::item(resultTop,"First");
    const mml::NodeValue node=resultItem.First();
    const std::u16string latex=trimTex(string_);
    if(!latex.empty())node.requireNode("attributes")->attributes()->set("data-latex",Property(property_host::debugText(latex)));
    return node;
}

Property TexParser::convertDelimiter(std::u16string c){
    const TokenLookup lookupValue=lookup(HandlerType.get("DELIMITER"),c);
    if(const auto* token=std::get_if<TokenRef>(&lookupValue);token&&*token)return (*token)->character();
    if(std::holds_alternative<Property>(lookupValue)&&std::holds_alternative<PropertyNull>(std::get<Property>(lookupValue).value))return PropertyNull{};
    return PropertyNull{};
}
std::u16string TexParser::getCodePoint(){
    if(i<0||i>=static_cast<double>(string_.size()))return {};
    const auto index=static_cast<std::size_t>(i);const char16_t first=string_[index];
    if(first>=0xD800&&first<=0xDBFF&&index+1<string_.size()&&string_[index+1]>=0xDC00&&string_[index+1]<=0xDFFF)return string_.substr(index,2);
    return std::u16string(1,first);
}
bool TexParser::nextIsSpace(){
    if(i<0||i>=static_cast<double>(string_.size()))return false;
    const char16_t c=string_[static_cast<std::size_t>(i)];
    return c==u' '||c==u'\t'||c==u'\n'||c==u'\r'||c==u'\v'||c==u'\f'||c==0x00A0||c==0x1680||
      (c>=0x2000&&c<=0x200A)||c==0x2028||c==0x2029||c==0x202F||c==0x205F||c==0x3000||c==0xFEFF;
}
std::u16string TexParser::GetNext(){while(nextIsSpace()){cancellation::checkpoint(cancellation::CheckpointStage::TeX);++i;}return getCodePoint();}
std::u16string TexParser::GetCS(){
    cancellation::checkpoint(cancellation::CheckpointStage::TeX);
    if(i>=static_cast<double>(string_.size())){++i;return u" ";}
    const auto start=static_cast<std::size_t>(std::max(0.0,i));std::size_t end=start;
    if(string_[start]>=0xD800&&string_[start]<=0xDBFF){end=std::min(string_.size(),start+2);}
    else if((string_[start]>=u'a'&&string_[start]<=u'z')||(string_[start]>=u'A'&&string_[start]<=u'Z')){
        end=start+1;while(end<string_.size()&&((string_[end]>=u'a'&&string_[end]<=u'z')||(string_[end]>=u'A'&&string_[end]<=u'Z'))){cancellation::checkpoint(cancellation::CheckpointStage::TeX);++end;}
        if(end<string_.size()&&string_[end]==u' ')++end;
    }else end=start+1;
    const std::u16string raw=string_.substr(start,end-start);i=end;
    if(raw.size()>1&&raw.back()==u' '&&std::all_of(raw.begin(),raw.end()-1,[](char16_t c){return(c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z');}))return raw.substr(0,raw.size()-1);
    return raw;
}

Property TexParser::GetArgument(std::u16string name,bool noneOK){
    const std::u16string next=GetNext();
    if(next.empty()){
        if(!noneOK)throwTex(u"MissingArgFor",u"Missing argument for %1",std::array<Property,1>{currentCS_});
        return PropertyNull{};
    }
    if(next==u"}"){
        if(!noneOK)throwTex(u"ExtraCloseMissingOpen",u"Extra close brace or missing open brace");
        return PropertyNull{};
    }
    if(next==u"\\"){++i;return property_host::debugText(u"\\"+GetCS());}
    if(next==u"{"){
        const double j=++i;int braces=1;
        while(i<static_cast<double>(string_.size())){
            cancellation::checkpoint(cancellation::CheckpointStage::TeX);
            const char16_t c=string_[static_cast<std::size_t>(i++)];
            if(c==u'\\')++i;
            else if(c==u'{')++braces;
            else if(c==u'}'&&--braces==0)return property_host::debugText(string_.substr(static_cast<std::size_t>(j),static_cast<std::size_t>(i-j-1)));
        }
        throwTex(u"MissingCloseBrace",u"Missing close brace");
    }
    const auto cp=getCodePoint();i+=static_cast<double>(cp.size());return property_host::debugText(cp);
}
Property TexParser::GetBrackets(std::u16string,Property def,bool matchBrackets){
    if(GetNext()!=u"[")return def;
    const double j=++i;int braces=0,brackets=0;
    while(i<static_cast<double>(string_.size())){
        cancellation::checkpoint(cancellation::CheckpointStage::TeX);
        const char16_t c=string_[static_cast<std::size_t>(i++)];
        if(c==u'{')++braces;else if(c==u'\\')++i;else if(c==u'}'){if(braces--<=0)throwTex(u"ExtraCloseLooking",u"Extra close brace while looking for %1",std::array<Property,1>{Property("']'")});}
        else if(c==u'['&&braces==0)++brackets;else if(c==u']'&&braces==0){if(!matchBrackets||brackets==0)return property_host::debugText(string_.substr(static_cast<std::size_t>(j),static_cast<std::size_t>(i-j-1)));--brackets;}
    }
    throwTex(u"MissingCloseBracket",u"Could not find closing ']' for argument to %1",std::array<Property,1>{currentCS_});
}
Property TexParser::GetDelimiter(std::u16string name,bool braceOK){
    std::u16string c=GetNext();i+=static_cast<double>(c.size());
    if(i<=static_cast<double>(string_.size())){
        if(c==u"\\")c+=GetCS();else if(c==u"{"&&braceOK){--i;const Property arg=GetArgument(std::move(name));c=trim_source(propText(arg));}
        if(contains(HandlerType.get("DELIMITER"),c))return convertDelimiter(c);
    }
    throwTex(u"MissingOrUnrecognizedDelim",u"Missing or unrecognized delimiter for %1",std::array<Property,1>{currentCS_});
}
std::u16string TexParser::GetDimen(std::u16string){
    if(GetNext()==u"{"){
        const Property dimen=GetArgument(u"");const auto match=UnitUtil::instance().matchDimen(dimen);
        if(property_host::truthy(match[0]))return propText(match[0])+propText(match[1]);
    }else{
        const std::u16string dimen=string_.substr(static_cast<std::size_t>(std::max(0.0,i)));
        const auto match=UnitUtil::instance().matchDimen(Property(property_host::debugText(dimen)),true);
        if(property_host::truthy(match[0])){i+=property_host::number(match[2]);return propText(match[0])+propText(match[1]);}
    }
    throwTex(u"MissingDimOrUnits",u"Missing dimension or its units for %1",std::array<Property,1>{currentCS_});
}
std::u16string TexParser::GetUpTo(std::u16string,std::u16string token){
    while(nextIsSpace())++i;const double j=i;int braces=0;
    while(i<static_cast<double>(string_.size())){
        cancellation::checkpoint(cancellation::CheckpointStage::TeX);
        const double k=i;std::u16string c=GetNext();i+=static_cast<double>(c.size());
        if(c==u"\\")c+=GetCS();else if(c==u"{")++braces;else if(c==u"}"){if(braces==0)throwTex(u"ExtraCloseLooking",u"Extra close brace while looking for %1",std::array<Property,1>{Property(property_host::debugText(token))});--braces;}
        if(braces==0&&c==token)return string_.substr(static_cast<std::size_t>(j),static_cast<std::size_t>(k-j));
    }
    throwTex(u"TokenNotFoundForCommand",u"Could not find %1 for %2",std::array<Property,2>{Property(property_host::debugText(token)),currentCS_});
}
mml::NodeValue TexParser::ParseArg(std::u16string name){const Property arg=GetArgument(name);return TexParser::make(property_host::string(arg),requireStack(stack).env(),requireConfiguration(*this))->mml();}
mml::NodeValue TexParser::ParseUpTo(std::u16string name,std::u16string token){const auto arg=GetUpTo(name,token);return TexParser::make(arg,requireStack(stack).env(),requireConfiguration(*this))->mml();}
Property TexParser::GetDelimiterArg(std::u16string name){
    const Property c=UnitUtil::instance().trimSpaces(GetArgument(name));
    if(c==Property("")||c==Property(property_host::debugText(u"")))return PropertyNull{};
    const std::u16string text=propText(c);
    if(contains(HandlerType.get("DELIMITER"),text))return Property(property_host::debugText(text));
    throwTex(u"MissingOrUnrecognizedDelim",u"Missing or unrecognized delimiter for %1",std::array<Property,1>{currentCS_});
}
bool TexParser::GetStar(){const bool star=GetNext()==u"*";if(star)++i;return star;}
mml::NodeValue TexParser::create(std::u16string kind,FunctionArgs rest){
    const mml::NodeValue node=requireConfiguration(*this).nodeFactoryOwner().create(property_host::debugText(kind),rest);
    if(node.requireNode("isToken")->isToken()&&node.requireNode("attributes")->attributes()->hasExplicit("mathvariant")){
        const Property value=node.requireNode("attributes")->attributes()->get("mathvariant");
        const std::u16string text=propText(value);if(!text.empty()&&text.front()==u'-')node.requireNode("setProperty")->setProperty("ignore-variant",true);
    }
    return node;
}
std::u16string TexParser::trimTex(std::u16string tex){
    const std::u16string original=tex;
    auto out=trim_source(std::move(tex));
    // Source regexp: (?:^|[^\\])(?:\\\\)*\\\s+$, using JS whitespace.
    const auto whitespace=[](char16_t c){return c==u' '||c==u'\t'||c==u'\n'||c==u'\r'||c==u'\v'||c==u'\f'||c==0x00A0||c==0x1680||(c>=0x2000&&c<=0x200A)||c==0x2028||c==0x2029||c==0x202F||c==0x205F||c==0x3000||c==0xFEFF;};
    std::size_t i=original.size();while(i>0&&whitespace(original[i-1]))--i;
    if(i==original.size()||i==0)return out;
    std::size_t slashCount=0;while(i>0&&original[i-1]==u'\\'){--i;++slashCount;}
    if(slashCount==0||slashCount%2==0)return out;
    out.push_back(u' ');return out;
}

void TexParser::updateResult(std::u16string input,double old){
    const mml::NodeValue node=requireStack(stack).Prev(true);
    if(!node.isNode())return;
    mml::MmlNode* target=node.requireNode("attributes");
    auto attributes=target->attributes();
    const Property latex=attributes->get("data-latex");
    const Property existing=attributes->get("data-latex-item");
    if(!existing.missing()){
        if(!property_host::truthy(latex)){
            if(input==u"}"||existing==Property("}"))composeBraces(node);
            else attributes->set("data-latex",existing);
        }
        return;
    }
    if(old<saveI)old=saveI;
    const std::u16string expression=trimTex(old!=i?string_slice(string_,old,i):input);
    if(expression.empty()||Property(property_host::debugText(expression))==latex||(input==u"\\"&&expression==u"\\"))return;
    if(expression==u"_"||expression==u"^"){node.requireNode("setProperty")->setProperty("sub-sup",Property(property_host::debugText(expression)));return;}
    const Property operation=node.requireNode("getProperty")->getProperty("sub-sup");
    const auto children=node.requireNode("childNodes")->childNodes;
    if(operation==Property("^")){
        if(children&&children->size()>2&&children->get(2).isNode()){
            const mml::NodeValue superscript=children->get(2);auto attrs=superscript.requireNode("attributes")->attributes();
            if(expression==u"}")composeBraces(superscript);else if(!attrs->hasExplicit("data-latex"))attrs->set("data-latex",Property(property_host::debugText(expression)));
        }
        if(children&&children->size()>1&&children->get(1).isNode()){
            const Property sub=children->get(1).requireNode("attributes")->attributes()->get("data-latex");
            composeLatex(node,u"_"+propText(sub)+u"^",0,2);
        }else composeLatex(node,u"^",0,2);
        return;
    }
    if(operation==Property("_")){
        if(children&&children->size()>1&&children->get(1).isNode()){
            const mml::NodeValue subscript=children->get(1);auto attrs=subscript.requireNode("attributes")->attributes();
            if(expression==u"}")composeBraces(subscript);else if(!attrs->hasExplicit("data-latex"))attrs->set("data-latex",Property(property_host::debugText(expression)));
        }
        if(children&&children->size()>2&&children->get(2).isNode()){
            const Property sup=children->get(2).requireNode("attributes")->attributes()->get("data-latex");
            composeLatex(node,u"^"+propText(sup)+u"_",0,1);
        }else composeLatex(node,u"_",0,1);
        return;
    }
    if(expression==u"}"){composeBraces(node);return;}
    attributes->set("data-latex",Property(property_host::debugText(expression)));
}
void TexParser::composeLatex(mml::NodeValue node,std::u16string comp,double pos1,double pos2){
    const auto children=node.requireNode("childNodes")->childNodes;
    if(!children||pos1<0||pos2<0||pos1>=static_cast<double>(children->size())||pos2>=static_cast<double>(children->size()))return;
    const auto left=children->get(static_cast<std::size_t>(pos1));const auto right=children->get(static_cast<std::size_t>(pos2));
    if(!left.isNode()||!right.isNode())return;
    const Property l=left.requireNode("attributes")->attributes()->get("data-latex");
    const Property r=right.requireNode("attributes")->attributes()->get("data-latex");
    // Source uses `(left || '') + comp + right`: only the left operand gets
    // truthiness fallback; the right operand follows ordinary JS string concat.
    const std::u16string leftText=property_host::truthy(l)?propText(l):std::u16string{};
    node.requireNode("attributes")->attributes()->set("data-latex",Property(property_host::debugText(leftText+comp+propText(r))));
}
void TexParser::composeBraces(mml::NodeValue atom){
    const std::u16string content=composeBracedContent(atom);
    atom.requireNode("attributes")->attributes()->set("data-latex",Property(property_host::debugText(u"{"+content+u"}")));
}
std::u16string TexParser::composeBracedContent(mml::NodeValue atom){
    const auto children=atom.requireNode("childNodes")->childNodes;
    if(!children||children->empty()||!children->get(0).isNode())return {};
    const auto content=children->get(0).requireNode("childNodes")->childNodes;
    if(!content)return {};
    std::u16string expression;
    for(std::size_t index=0;index<content->size();++index){
        const auto child=content->get(index);if(!child.isNode())continue;
        // Source uses `child?.attributes?.get(LATEX) || ""`: empty-node
        // implementations legitimately have no Attributes object.  Preserve
        // that optional read instead of dereferencing the null native view.
        const auto attributes=child.requireNode("attributes")->attributes();
        const Property latex=attributes?attributes->get("data-latex"):Property(Missing{});
        if(!property_host::truthy(latex))continue;
        const std::u16string text=propText(latex);
        const auto alpha=[](char16_t c){return(c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z');};
        if(!expression.empty()&&alpha(expression.back())&&alpha(text.front()))expression.push_back(u' ');
        expression+=text;
    }
    return expression;
}

} // namespace folia::math::tex
