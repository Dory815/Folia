// Native port of MathJax 4.1.3 input/tex/color/ColorUtil.ts.
module folia.math.input.tex.color.util;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
using namespace options;
namespace {
Key key(std::u16string_view value){return Key(std::u16string(value));}
bool jsWhitespace(char16_t c){
    return c==0x0009||c==0x000A||c==0x000B||c==0x000C||c==0x000D||c==0x0020||
        c==0x00A0||c==0x1680||(c>=0x2000&&c<=0x200A)||c==0x2028||c==0x2029||
        c==0x202F||c==0x205F||c==0x3000||c==0xFEFF;
}
std::u16string_view trimJS(std::u16string_view value){
    while(!value.empty()&&jsWhitespace(value.front()))value.remove_prefix(1);
    while(!value.empty()&&jsWhitespace(value.back()))value.remove_suffix(1);
    return value;
}
std::vector<std::u16string> splitComma(std::u16string_view source){
    std::vector<std::u16string> parts;std::size_t start=0;
    while(start<=source.size()){
        const auto comma=source.find(u',',start);std::size_t a=start,b=comma==std::u16string_view::npos?source.size():comma;
        while(a<b&&jsWhitespace(source[a]))++a;
        while(b>a&&jsWhitespace(source[b-1]))--b;
        parts.emplace_back(source.substr(a,b-a));if(comma==std::u16string_view::npos)break;start=comma+1;
    }
    return parts;
}
double parseDecimal(std::u16string_view value,bool allowWhitespace=false){
    const std::u16string trimmed(allowWhitespace?trimJS(value):value);
    const auto text=property_host::debugText(trimmed);
    static const std::regex decimal(R"((\d+(\.\d*)?|\.\d+))");
    if(!std::regex_match(text,decimal))throw TexError(u"InvalidDecimalNumber",u"Invalid decimal number");
    // JS parseFloat returns +/-Infinity for a syntactically valid decimal
    // outside the finite range. std::stod throws instead, which changes the
    // Source TexError from ModelArg2 into a native range exception.
    return std::strtod(text.c_str(),nullptr);
}
std::string hex(double value){
    const auto integer=static_cast<unsigned>(value);std::ostringstream out;out<<std::hex<<std::nouppercase<<integer;
    auto result=out.str();if(result.size()<2)result.insert(result.begin(),'0');return result;
}
[[noreturn]] void rangeError(std::u16string_view model,std::u16string_view low,std::u16string_view high){
    const std::array<Property,3> rest{property_host::debugText(model),property_host::debugText(low),property_host::debugText(high)};
    throw TexError(u"ModelArg2",u"Color values for the %1 model must be between %2 and %3",rest);
}
}
std::u16string ColorModel::normalizeColor(std::u16string_view model,std::u16string_view definition)const{
    if(model.empty()||model==u"named"){
        if(definition.find(u';')!=std::u16string_view::npos)throw TexError(u"BadColorValue",u"Invalid color value");
        return std::u16string(definition);
    }
    if(model==u"rgb"){
        const auto parts=splitComma(definition);if(parts.size()!=3){const std::array<Property,1> rest{Property("rgb")};throw TexError(u"ModelArg1",u"Color values for the %1 model require 3 numbers",rest);}
        std::u16string result=u"#";
        for(const auto& part:parts){const double n=parseDecimal(part);if(n<0||n>1)rangeError(u"rgb",u"0",u"1");result+=property_host::text(hex(std::floor(n*255)));}
        return result;
    }
    if(model==u"RGB"){
        const auto parts=splitComma(definition);if(parts.size()!=3){const std::array<Property,1> rest{Property("RGB")};throw TexError(u"ModelArg1",u"Color values for the %1 model require 3 numbers",rest);}
        std::u16string result=u"#";
        for(const auto& part:parts){const auto text=property_host::debugText(part);if(!std::regex_match(text,std::regex(R"(\d+)")))throw TexError(u"InvalidNumber",u"Invalid number");const double n=std::strtod(text.c_str(),nullptr);if(n>255)rangeError(u"RGB",u"0",u"255");result+=property_host::text(hex(n));}
        return result;
    }
    if(model==u"gray"){
        const double n=parseDecimal(definition,true);if(n<0||n>1)rangeError(u"gray",u"0",u"1");const auto gray=property_host::text(hex(std::floor(n*255)));return u"#"+gray+gray+gray;
    }
    const std::array<Property,1> rest{property_host::debugText(model)};
    throw TexError(u"UndefinedColorModel",u"Color model '%1' not defined",rest);
}
namespace {
std::optional<std::u16string> userColor(Context& cx,Value model,std::u16string_view name){
    const Value colors=cx.get(model,key(u"userColors"));
    auto callMap=[&](std::u16string_view method,std::span<const Value> args)->Value{
        const Value fnValue=cx.get(colors,key(method));const auto* fn=std::get_if<ObjectRef>(&fnValue.data);
        if(!fn||cx.kind(*fn)!=ObjectKind::function)throw Exception("TypeError",u"ColorModel.userColors method is not a function");
        return cx.call(*fn,colors,args);
    };
    const std::array<Value,1> args{Value(std::u16string(name))};
    if(!truthy(callMap(u"has",args)))return {};
    const Value value=callMap(u"get",args);
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    throw Exception("TypeError",u"ColorModel user color is not a string");
}
ObjectRef colorSourceMethod(Context& cx,ObjectRef receiver,std::u16string_view name){
    const Value value=cx.get(Value(receiver),key(name));const auto* function=std::get_if<ObjectRef>(&value.data);
    if(!function||cx.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",std::u16string(u"ColorModel.")+std::u16string(name)+u" is not a function");
    return *function;
}

}
ColorUtilModule ColorUtil::initializeModule(ParserRuntime& runtime,std::shared_ptr<MapContextBindings> maps,ObjectRef colors){
    auto& cx=runtime.context();
    if(!maps)throw Exception("TypeError",u"ColorUtil requires its Map owner");
    // Source ColorModelProcessors is a module Map. Keep the actual Map object
    // as the dispatch authority: normalizeColor performs its live has/get and
    // calls the stored processor instead of selecting a C++ branch by model.
    const ObjectRef processors=maps->create();
    for(const std::u16string_view model:{u"rgb",u"RGB",u"gray"}){
        const auto processor=cx.function(u"",[model](Context&,Value,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"Color processor requires a definition");
            const auto* definition=std::get_if<std::u16string>(&args[0].data);
            if(!definition)throw Exception("TypeError",u"Color processor definition must be a string");
            return Value(ColorModel{}.normalizeColor(model,*definition));
        });
        cx.define(processor,key(u"length"),Descriptor::data(Value(1.0),false,false,true));
        (void)maps->set(processors,Value(std::u16string(model)),Value(processor));
    }
    auto constructorRef=std::make_shared<ObjectRef>();
    *constructorRef=cx.nativeConstructor(u"ColorModel",
        [constructorRef,maps](Context& context,std::span<const Value>)->ObjectRef{
            const ObjectRef identity=context.instance(*constructorRef,std::make_shared<ColorModel>());
            context.set(identity,Key(u"userColors"),Value(maps->create()));
            return identity;
        },
        [](Context&,Value,std::span<const Value>)->Value{throw Exception("TypeError",u"Class constructor ColorModel cannot be invoked without 'new'");});
    const ObjectRef prototype=std::get<ObjectRef>(cx.get(Value(*constructorRef),key(u"prototype")).data);
    cx.define(prototype,key(u"constructor"),Descriptor::data(Value(*constructorRef),true,false,true));
    const auto normalizeFn=cx.function(u"",[processors,maps](Context& c,Value receiver,std::span<const Value> args)->Value{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"ColorModel method receiver is not an object");
        const auto* model=std::any_cast<std::shared_ptr<ColorModel>>(&std::as_const(c).payload(*object));if(!model||!*model)throw Exception("TypeError",u"ColorModel method called on incompatible receiver");
        if(args.size()<2)throw Exception("TypeError",u"ColorModel.normalizeColor requires model and definition");
        const auto* name=std::get_if<std::u16string>(&args[0].data);const auto* def=std::get_if<std::u16string>(&args[1].data);if(!name||!def)throw Exception("TypeError",u"ColorModel arguments must be strings");
        if(name->empty()||*name==u"named"){
            if(def->find(u';')!=std::u16string::npos)throw TexError(u"BadColorValue",u"Invalid color value");
            return Value(*def);
        }
        const Value keyValue(*name);
        if(!maps->has(processors,keyValue)){const std::array<Property,1> rest{Property(property_host::debugText(*name))};throw TexError(u"UndefinedColorModel",u"Color model '%1' not defined",rest);}
        const Value processor=maps->get(processors,keyValue);const auto* function=std::get_if<ObjectRef>(&processor.data);
        if(!function||c.kind(*function)!=ObjectKind::function)throw Exception("TypeError",u"ColorModel processor is not a function");
        const std::array<Value,1> processorArgs{args[1]};return c.call(*function,Value(Undefined{}),processorArgs);
    });
    const auto getColorFn=cx.function(u"",[](Context& c,Value receiver,std::span<const Value> args)->Value{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"ColorModel method receiver is not an object");
        const auto* model=std::any_cast<std::shared_ptr<ColorModel>>(&std::as_const(c).payload(*object));if(!model||!*model)throw Exception("TypeError",u"ColorModel method called on incompatible receiver");
        if(args.size()<2)throw Exception("TypeError",u"ColorModel.getColor requires model and definition");
        const auto* name=std::get_if<std::u16string>(&args[0].data);const auto* def=std::get_if<std::u16string>(&args[1].data);if(!name||!def)throw Exception("TypeError",u"ColorModel arguments must be strings");
        const bool named=name->empty()||*name==u"named";
        const std::array<Value,2> normalizeArgs{args[0],args[1]};
        const std::array<Value,1> namedArgs{args[1]};
        const auto fn=colorSourceMethod(c,*object,named?u"getColorByName":u"normalizeColor");
        return c.call(fn,receiver,named?std::span<const Value>(namedArgs):std::span<const Value>(normalizeArgs));
    });
    const auto getByNameFn=cx.function(u"",[colors](Context& c,Value receiver,std::span<const Value> args)->Value{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"ColorModel method receiver is not an object");
        const auto* model=std::any_cast<std::shared_ptr<ColorModel>>(&std::as_const(c).payload(*object));if(!model||!*model)throw Exception("TypeError",u"ColorModel method called on incompatible receiver");
        if(args.empty())throw Exception("TypeError",u"ColorModel.getColorByName requires a name");
        const auto* name=std::get_if<std::u16string>(&args[0].data);if(!name)throw Exception("TypeError",u"Color name must be a string");
        if(const auto user=userColor(c,receiver,*name))return Value(*user);
        auto mapCall=[&](std::u16string_view method)->Value{
            const Value fnValue=c.get(Value(colors),key(method));const auto* fn=std::get_if<ObjectRef>(&fnValue.data);
            if(!fn||c.kind(*fn)!=ObjectKind::function)throw Exception("TypeError",u"Color constants Map method is not a function");
            return c.call(*fn,Value(colors),args);
        };
        if(truthy(mapCall(u"has")))return mapCall(u"get");
        if(name->find(u';')!=std::u16string::npos)throw TexError(u"BadColorValue",u"Invalid color value");
        return Value(*name);
    });
    const auto defineFn=cx.function(u"",[](Context& c,Value receiver,std::span<const Value> args)->Value{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"ColorModel method receiver is not an object");
        const auto* model=std::any_cast<std::shared_ptr<ColorModel>>(&std::as_const(c).payload(*object));if(!model||!*model)throw Exception("TypeError",u"ColorModel method called on incompatible receiver");
        if(args.size()<3)throw Exception("TypeError",u"ColorModel.defineColor requires model, name and definition");
        const auto* m=std::get_if<std::u16string>(&args[0].data);const auto* n=std::get_if<std::u16string>(&args[1].data);const auto* d=std::get_if<std::u16string>(&args[2].data);if(!m||!n||!d)throw Exception("TypeError",u"ColorModel arguments must be strings");
        const std::array<Value,2> normalizeArgs{args[0],args[2]};
        const auto normalize=colorSourceMethod(c,*object,u"normalizeColor");
        const Value normalized=c.call(normalize,receiver,normalizeArgs);
        const Value receiverValue=*object;const Value colors=c.get(receiverValue,Key(u"userColors"));
        const Value setValue=c.get(colors,Key(u"set"));const auto* set=std::get_if<ObjectRef>(&setValue.data);
        if(!set||c.kind(*set)!=ObjectKind::function)throw Exception("TypeError",u"ColorModel.userColors.set is not a function");
        const std::array<Value,2> setArgs{args[1],normalized};(void)c.call(*set,colors,setArgs);return Value(Undefined{});
    });
    cx.define(normalizeFn,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    cx.define(getByNameFn,key(u"length"),Descriptor::data(Value(1.0),false,false,true));
    cx.define(getColorFn,key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    cx.define(defineFn,key(u"length"),Descriptor::data(Value(3.0),false,false,true));
    cx.define(prototype,key(u"normalizeColor"),Descriptor::data(Value(normalizeFn),true,true,true));
    cx.define(prototype,key(u"getColor"),Descriptor::data(Value(getColorFn),true,true,true));
    cx.define(prototype,key(u"getColorByName"),Descriptor::data(Value(getByNameFn),true,true,true));
    cx.define(prototype,key(u"defineColor"),Descriptor::data(Value(defineFn),true,true,true));
    return ColorUtilModule{*constructorRef,processors,colors,std::move(maps)};
}

}
