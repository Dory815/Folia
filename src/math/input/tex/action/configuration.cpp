// Native port of MathJax 4.1.3 input/tex/action/ActionConfiguration.ts.
module folia.math.input.tex.action.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
TexParser& parser(ParserContext& value){
    auto* result=dynamic_cast<TexParser*>(&value);
    if(!result)throw Exception("TypeError",u"Action method requires a TexParser");
    return *result;
}
Stack& stack(TexParser& value){
    const auto* result=std::get_if<std::shared_ptr<Stack>>(&value.stack);
    if(!result||!*result)property_host::typeError("Cannot read properties of undefined (reading 'env')");
    return **result;
}
std::u16string text(const Property& value){return property_host::string(value);}
mml::Children children(std::span<const mml::NodeValue> values){
    auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(const auto& value:values)result->push_back(value);
    return result;
}
mml::NodeValue maction(TexParser& p,mml::Children nodes,std::string action){
    const std::array<std::any,3> args{std::any(Property("maction")),std::any(std::move(nodes)),
        std::any(Property(PropertyList{{"actiontype",Property(std::move(action))}}))};
    return p.create(u"node",FunctionArgs(args));
}
FunctionRef toggle(){
    return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parser(context);const Property name=std::get<Property>(atom);
        std::vector<mml::NodeValue> rows;
        while(true){
            const Property argument=p.GetArgument(text(name));
            const std::u16string source=text(argument);
            if(source==u"\\endtoggle")break;
            ParseOptions* configuration=nullptr;
            if(const auto* value=std::get_if<ParseOptions*>(&p.configuration))configuration=*value;
            if(!configuration)property_host::typeError("Cannot read properties of undefined (reading 'configuration')");
            rows.push_back(TexParser::make(source,stack(p).env(),*configuration)->mml());
        }
        p.Push(maction(p,children(rows),"toggle"));return Property(Missing{});
    });
}
FunctionRef mathTip(){
    return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parser(context);const std::u16string name=text(std::get<Property>(atom));
        const mml::NodeValue math=p.ParseArg(name),tip=p.ParseArg(name);
        const std::array<mml::NodeValue,2> values{math,tip};
        p.Push(maction(p,children(values),"tooltip"));return Property(Missing{});
    });
}
Value object(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){
    const ObjectRef out=cx.object();for(const auto& [name,value]:fields)cx.set(out,key(name),value);return Value(out);
}
Value array(Context& cx,std::initializer_list<Value> values){
    const ObjectRef out=cx.array(values.size());std::size_t i=0;
    for(const auto& value:values){const auto digits=std::to_string(i++);cx.set(out,key(std::u16string(digits.begin(),digits.end())),value);}
    return Value(out);
}
}
ConfigurationRef ActionConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"action");
    if(const auto* existing=std::get_if<ConfigurationRef>(&loaded);existing&&*existing)return *existing;
    const FunctionRef toggleMethod=toggle(),tipMethod=mathTip();
    for(const auto& [body,name]:std::array{
            std::pair{toggleMethod,std::u16string_view(u"Toggle")},
            std::pair{tipMethod,std::u16string_view(u"Mathtip")}}){
        const auto bound=BaseMethods::bind(runtime,body,std::u16string(name));
        const auto* identity=std::get_if<ObjectRef>(&bound.data);
        if(!identity)throw Exception("TypeError",u"Action method has no Context identity");
        runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    }
    const Property methods(PropertyList{{"toggle",Property(toggleMethod)},{"mathtip",Property(tipMethod)},
        {"Macro",property_host::get(runtime.baseMethods,"Macro")}});
    const PropertyList entries{{"toggle",Property("toggle")},{"mathtip",Property("mathtip")},
        {"texttip",Property(PropertyArray{Property("Macro"),Property("\\mathtip{#1}{\\text{#2}}"),Property(2)})}};
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("action-macros"),Property(entries),methods);
    Context& cx=runtime.context();
    const Value macroNames=array(cx,{Value(u"action-macros")});
    const Value handler=object(cx,{{u"macro",macroNames}});
    return Configuration::create(runtime,u"action",object(cx,{{u"handler",handler}}));
}
}
