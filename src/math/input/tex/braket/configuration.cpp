// Native port of MathJax 4.1.3 input/tex/braket/BraketConfiguration.ts.
module folia.math.input.tex.braket.configuration;
import folia.math.input.tex.braket.items;
import folia.math.input.tex.braket.mappings;
import folia.math.input.tex.handler_types;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
Value record(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const auto o=cx.object();for(const auto&[name,value]:fields)cx.set(o,key(name),value);return Value(o);}
Value array(Context& cx,std::initializer_list<Value> fields){const auto o=cx.array(fields.size());std::size_t i=0;for(const auto& v:fields){auto n=std::to_string(i++);cx.set(o,key(std::u16string(n.begin(),n.end())),v);}return Value(o);}
}
ConfigurationRef BraketConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"braket");
    if(const auto* prior=std::get_if<ConfigurationRef>(&loaded);prior&&*prior)return *prior;
    auto& cx=runtime.context();const auto type=BraketItem::initializeModule(runtime);
    const auto constructor=runtime.itemClassIdentities.find(type);
    if(!constructor)throw options::Exception("Unimplemented",u"BraketItem constructor identity was not registered");
    BraketMappings::initializeModule(runtime);
    const auto handler=record(cx,{{u"character",array(cx,{Value(u"Braket-characters")})},
                                   {u"macro",array(cx,{Value(u"Braket-macros")})}});
    const auto items=record(cx,{{u"braket",Value(*constructor)}});
    return Configuration::create(runtime,u"braket",record(cx,{{u"handler",handler},{u"items",items},{u"priority",Value(3.0)}}));
}
}
