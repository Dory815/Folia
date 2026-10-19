// Native port of MathJax 4.1.3 input/tex/color/ColorMethods.ts.
module folia.math.input.tex.color.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.color.util;
import folia.math.input.tex.parser;
import folia.math.input.tex.node_util;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import folia.math.util.js_number;
import std;
namespace folia::math::tex {
using namespace options;
namespace {
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& value){auto* p=dynamic_cast<TexParser*>(&value);if(!p)property_host::typeError("Color method requires a TexParser");return *p;}
ParseOptions& parseOptions(TexParser& p){const auto* v=std::get_if<ParseOptions*>(&p.configuration);if(!v||!*v)property_host::typeError("Cannot read properties of null or undefined (reading 'packageData')");return **v;}
Stack& stack(TexParser& p){const auto v=p.stack;const auto* s=std::get_if<std::shared_ptr<Stack>>(&v);if(!s||!*s)property_host::typeError("Cannot read properties of null or undefined (reading 'env')");return **s;}
StackItemFactory& items(TexParser& p){const auto v=p.itemFactory();const auto* f=std::get_if<std::shared_ptr<StackItemFactory>>(&v);if(!f||!*f)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");return **f;}
std::u16string text(const Property& value){return property_host::string(value);}
ObjectRef modelObject(TexParser& p){
    auto& config=parseOptions(p);const auto* data=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&config.packageData);
    if(!data||!*data)property_host::typeError("Cannot read properties of null or undefined (reading 'get')");
    const auto color=(*data)->get(u"color");if(!color)property_host::typeError("Cannot read properties of undefined (reading 'model')");
    const auto* package=std::get_if<ObjectRef>(&color->data);if(!package)property_host::typeError("Cannot read properties of null or undefined (reading 'model')");
    const auto modelValue=config.runtime().context().get(Value(*package),key(u"model"));
    const auto* identity=std::get_if<ObjectRef>(&modelValue.data);if(!identity)property_host::typeError("Color model is not an object");
    const auto* actual=std::any_cast<std::shared_ptr<ColorModel>>(&config.runtime().context().payload(*identity));
    if(!actual||!*actual)throw Exception("TypeError",u"Color model identity is not registered");return *identity;
}
std::u16string getColor(TexParser& p,std::u16string_view colorModel,std::u16string_view definition){
    auto& cx=parseOptions(p).runtime().context();const auto identity=modelObject(p);const auto modelValue=Value(identity);const auto method=cx.get(modelValue,key(u"getColor"));
    const auto* fn=std::get_if<ObjectRef>(&method.data);if(!fn||cx.kind(*fn)!=ObjectKind::function)throw Exception("TypeError",u"colorModel.getColor is not a function");
    const std::array<Value,2> args{Value(std::u16string(colorModel)),Value(std::u16string(definition))};
    const auto result=cx.call(*fn,Value(identity),args);if(const auto* value=std::get_if<std::u16string>(&result.data))return *value;
    throw Exception("TypeError",u"ColorModel.getColor did not return a string");
}
FunctionRef colorMethod(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(context);const auto name=text(std::get<Property>(atom));const auto modelName=p.GetBrackets(name,Property(""));const auto definition=p.GetArgument(name);
    const auto color=property_host::debugText(getColor(p,text(modelName),text(definition)));
    auto* style=items(p).itemFactory().create("style",{});if(!style)property_host::typeError("Cannot read properties of null (reading 'setProperties')");
    style->setProperties(StackProperty(Property(PropertyList{{"styles",Property(PropertyList{{"mathcolor",color}})}})));
    auto env=PropertyList::record(stack(p).env());if(!env)property_host::typeError("Cannot set properties of null or undefined");env->setOwn("color",color);
    p.Push(items(p).reference(style));return Property(Missing{});
});}
FunctionRef textColorMethod(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(context);const auto name=text(std::get<Property>(atom));const auto modelName=p.GetBrackets(name,Property(""));const auto definition=p.GetArgument(name);
    const auto color=property_host::debugText(getColor(p,text(modelName),text(definition)));
    auto env=PropertyList::record(stack(p).env());if(!env)property_host::typeError("Cannot set properties of null or undefined");const auto old=env->get("color");env->setOwn("color",color);
    const auto math=p.ParseArg(name);
    if(property_host::truthy(old))env->setOwn("color",old);else env->erase("color");
    auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(math);
    const std::array<std::any,3> args{std::any(Property("mstyle")),std::any(children),std::any(Property(PropertyList{{"mathcolor",color}}))};
    p.Push(p.create(u"node",FunctionArgs(args)));return Property(Missing{});
});}
FunctionRef defineColorMethod(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(context);const auto name=text(std::get<Property>(atom));const auto cname=text(p.GetArgument(name)),modelName=text(p.GetArgument(name)),definition=text(p.GetArgument(name));
    auto& cx=parseOptions(p).runtime().context();const auto identity=modelObject(p);const auto method=cx.get(Value(identity),key(u"defineColor"));const auto* fn=std::get_if<ObjectRef>(&method.data);
    if(!fn||cx.kind(*fn)!=ObjectKind::function)throw Exception("TypeError",u"colorModel.defineColor is not a function");
    const std::array<Value,3> args{Value(modelName),Value(cname),Value(definition)};(void)cx.call(*fn,Value(identity),args);
    auto* nullItem=items(p).itemFactory().create("null",{});if(!nullItem)property_host::typeError("Cannot read properties of null (reading 'isKind')");p.Push(items(p).reference(nullItem));return Property(Missing{});
});}
Property padding(const Property& colorPadding){
    const auto pad=u"+"+text(colorPadding);const auto utf=property_host::debugText(text(colorPadding));
    const auto unit=std::regex_replace(utf,std::regex(R"(^.*?([a-z]*)$)"),"$1");
    const auto numberString=property_host::debugText(text(colorPadding));const double n=std::stod(numberString.substr(0,numberString.find_first_not_of("0123456789.+-")));
    const auto twice=js_number::toString(2*n);const auto suffix=property_host::text(unit);
    return Property(PropertyList{{"width",property_host::debugText(u"+"+property_host::text(twice)+suffix)},
        {"height",property_host::debugText(pad)},{"depth",property_host::debugText(pad)},{"lspace",colorPadding}});
}
Property option(TexParser& p,std::u16string_view keyName){const auto value=parseOptions(p).options;if(std::holds_alternative<ObjectRef>(value.data)){auto& runtime=parseOptions(p).runtime();const auto color=runtime.context().get(value,key(u"color"));return runtime.properties.from(runtime.context().get(color,key(keyName)));}return Missing{};}
mml::NodeValue padded(TexParser& p,mml::Children math,Property attrs,const Property& colorPadding){
    const std::array<std::any,3> args{std::any(Property("mpadded")),std::any(std::move(math)),std::any(std::move(attrs))};
    auto result=p.create(u"node",FunctionArgs(args));NodeUtil::instance().setProperties(result,Property(PropertyList::record(padding(colorPadding)).value()));return result;
}
FunctionRef colorBox(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(context);const auto name=text(std::get<Property>(atom));const auto modelName=text(p.GetBrackets(name,Property("")));const auto colorDef=p.GetArgument(name);const auto mathSource=text(p.GetArgument(name));
    auto math=ParseUtil::internalMath(p,mathSource);const auto color=property_host::debugText(getColor(p,modelName,text(colorDef)));
    const auto bg=Property(PropertyList{{"mathbackground",color}});p.Push(padded(p,math,bg,option(p,u"padding")));return Property(Missing{});
});}
FunctionRef fColorBox(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(context);const auto name=text(std::get<Property>(atom));const auto frameModel=p.GetBrackets(name,Property(""));const auto frameDef=p.GetArgument(name);const auto colorModel=p.GetBrackets(name,frameModel);const auto colorDef=p.GetArgument(name);const auto math=ParseUtil::internalMath(p,text(p.GetArgument(name)));
    const auto background=getColor(p,text(colorModel),text(colorDef));const auto frame=getColor(p,text(frameModel),text(frameDef));
    const auto border=property_host::string(option(p,u"borderWidth"));const auto attrs=Property(PropertyList{{"mathbackground",property_host::debugText(background)},{"style",property_host::debugText(u"border: "+border+u" solid "+frame)}});
    p.Push(padded(p,math,attrs,option(p,u"padding")));return Property(Missing{});
});}

}
Property ColorMethods::initializeModule(ParserRuntime& runtime){
    const std::array methods{
        std::pair{colorMethod(),std::u16string_view(u"Color")},
        std::pair{textColorMethod(),std::u16string_view(u"TextColor")},
        std::pair{defineColorMethod(),std::u16string_view(u"DefineColor")},
        std::pair{colorBox(),std::u16string_view(u"ColorBox")},
        std::pair{fColorBox(),std::u16string_view(u"FColorBox")}};
    PropertyList commands;
    for(const auto& [method,name]:methods){
        const auto bound=BaseMethods::bind(runtime,method,std::u16string(name));
        const auto* identity=std::get_if<ObjectRef>(&bound.data);
        if(!identity)throw Exception("TypeError",u"Color method has no Context identity");
        runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
        commands.setOwn(property_host::debugText(name),Property(method));
    }
    return Property(std::move(commands));
}
}
