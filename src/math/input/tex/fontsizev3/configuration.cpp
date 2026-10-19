// Native port of MathJax 4.1.3 input/tex/fontsizev3/FontSizeV3Configuration.ts.
module folia.math.input.tex.fontsizev3.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> values){Value out(cx.object());for(const auto& [name,value]:values)cx.set(out,key(name),value);return out;}
}
ConfigurationRef FontSizeV3Configuration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"fontsizev3");if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    const Property setSize=BaseMethods::get(runtime,u"SetSize");
    PropertyList commands;
    const auto add=[&](std::string_view name,double scale){commands.setOwn(name,Property(PropertyArray{setSize,Property(scale)}));};
    add("tiny",0.5);add("Tiny",0.6);add("scriptsize",0.7);add("small",0.85);add("normalsize",1.0);add("large",1.2);add("Large",1.44);add("LARGE",1.73);add("huge",2.07);add("Huge",2.49);
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("fontsizev3-macros"),Property(commands));
    auto& cx=runtime.context();const Value handlers=record(cx,{{u"macro",array(cx,{Value(u"fontsizev3-macros")})}});
    return Configuration::create(runtime,u"fontsizev3",record(cx,{{u"handler",handlers}}));
}
}
