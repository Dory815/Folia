// Source-owned assembly for CasesConfiguration.ts.
module folia.math.input.tex.cases.configuration;
import folia.math.input.tex.cases.items;
import folia.math.input.tex.cases.methods;
import folia.math.input.tex.cases.tags;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.token_map;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view s){return Key(std::u16string(s));}
Value array(Context& cx,std::initializer_list<Value> values){const ObjectRef out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;for(const auto& value:values){const std::string index=std::to_string(i++);cx.set(Value(out),key(std::u16string(index.begin(),index.end())),value);}return Value(out);}
Value object(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const ObjectRef out=cx.object();for(const auto& [name,value]:fields)cx.set(Value(out),key(name),value);return Value(out);}
}
ConfigurationRef CasesConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"cases");
    if(const auto* current=std::get_if<ConfigurationRef>(&loaded);current&&*current)return *current;
    auto& cx=runtime.context();
    const Property methodTable=CasesMethods::initializeModule(runtime);
    const ParseMethod environmentParser=functions::fromProperty(property_host::get(methodTable,"environment"));
    const PropertyArray numcases{Property("NumCases"),Property("cases")};
    const PropertyArray subnumcases{Property("NumCases"),Property("cases")};
    const Property environmentMap(PropertyList{{"numcases",Property(numcases)},{"subnumcases",Property(subnumcases)}});
    (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property("cases-env"),environmentParser,environmentMap,methodTable);
    const Property characterMap(PropertyList{{"&",Property("Entry")}});
    (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property("cases-macros"),characterMap,methodTable);

    const auto itemClass=CasesBeginItem::initializeModule(runtime);
    const auto itemCtor=runtime.itemClassIdentities.find(itemClass);
    if(!itemCtor)throw Exception("Unimplemented",u"CasesBeginItem constructor identity was not registered");

    const auto ams=runtime.configurationHandler.get(u"ams");
    const auto* amsConfiguration=std::get_if<ConfigurationRef>(&ams);
    if(!amsConfiguration||!*amsConfiguration)throw Exception("TypeError",u"AMS configuration is unavailable");
    const Value amsTags=cx.get((*amsConfiguration)->field(u"tags"),key(u"ams"));
    const auto* amsCtor=std::get_if<ObjectRef>(&amsTags.data);
    if(!amsCtor)throw Exception("TypeError",u"AmsTags constructor is unavailable");
    const TagsClassRef casesTags=TagsFactory::registerSubclass(runtime,u"CasesTags",
        [&runtime](){return std::make_shared<CasesTags>(runtime);},*amsCtor,
        [](Tags& value){if(!dynamic_cast<CasesTags*>(&value))throw Exception("TypeError",u"CasesTags initializer received another class");});
    const auto tagCtor=runtime.tagClassIdentities.find(casesTags);
    if(!tagCtor)throw Exception("Unimplemented",u"CasesTags constructor identity was not registered");

    const Value handlers=object(cx,{{u"environment",array(cx,{Value(u"cases-env")})},
                                    {u"character",array(cx,{Value(u"cases-macros")})}});
    const Value items=object(cx,{{u"cases-begin",Value(*itemCtor)}});
    const Value tags=object(cx,{{u"cases",Value(*tagCtor)}});
    return Configuration::create(runtime,u"cases",object(cx,{{u"handler",handlers},{u"items",items},{u"tags",tags}}));
}
}
