// Source-owned package assembly from AmsCdConfiguration.ts.
module folia.math.input.tex.amscd.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.parser;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::size_t i=0;for(const auto& value:values){const auto digits=std::to_string(i++);cx.set(Value(out),key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
Value object(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){Value out(cx.object());for(const auto& [name,value]:fields)cx.set(out,key(name),value);return out;}
}
ConfigurationRef AmsCdConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"amscd");if(const auto* found=std::get_if<ConfigurationRef>(&loaded);found&&*found)return *found;
    const auto methods=AmsCdMethods::create(runtime);AmsCdMappings::initializeModule(runtime,methods);Context& cx=runtime.context();
    const Value handler=object(cx,{{u"character",array(cx,{Value(u"amscd_special")})},{u"macro",array(cx,{Value(u"amscd_macros")})},{u"environment",array(cx,{Value(u"amscd_environment")})}});
    const Value amscd=object(cx,{{u"colspace",Value(u"5pt")},{u"rowspace",Value(u"5pt")},{u"harrowsize",Value(u"2.75em")},{u"varrowsize",Value(u"1.75em")},{u"hideHorizontalLabels",Value(false)}});
    return Configuration::create(runtime,u"amscd",object(cx,{{u"handler",handler},{u"options",object(cx,{{u"amscd",amscd}})}}));
}
}
