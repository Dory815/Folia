// Native port of MathJax 4.1.3 input/tex/html/HtmlConfiguration.ts.
module folia.math.input.tex.html.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.html.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const auto digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const ObjectRef out=cx.object();for(const auto&[name,value]:fields)cx.set(Value(out),key(name),value);return Value(out);}
}
ConfigurationRef HtmlConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"html");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const Property methods=HtmlMethods::initializeModule(runtime);PropertyList commands;
    for(const auto& [keyName,sourceName]:std::array<std::pair<const char*,const char16_t*>,5>{{
            {"data",u"Data"},{"href",u"Href"},{"class",u"Class"},{"style",u"Style"},{"cssId",u"Id"}}}){
        const Property method=property_host::get(methods,keyName);
        const auto* function=std::get_if<FunctionRef>(&method.value);
        if(!function||!*function)throw Exception("TypeError",u"HTML source method is not callable");
        const Value identity=BaseMethods::bind(runtime,*function,std::u16string(sourceName));
        const auto* object=std::get_if<ObjectRef>(&identity.data);
        if(!object)throw Exception("TypeError",u"HTML source method has no Context identity");
        runtime.context().define(*object,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
        commands.setOwn(keyName,method);
    }
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("html_macros"),Property(commands));
    auto& cx=runtime.context();const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"html_macros")})}});
    return Configuration::create(runtime,u"html",record(cx,{{u"handler",handlers}}));
}
}
