// Source-owned AMS token-map registration.
module folia.math.input.tex.ams.mappings;
import folia.math.input.tex.parser;
import folia.math.input.tex.token_map;
import folia.math.input.tex.base_mappings;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Property object(std::initializer_list<std::pair<std::string,Property>> fields){PropertyList result;for(const auto& [name,value]:fields)result.setOwn(name,value);return Property(result);}
Property table(const std::map<std::string,Property>& values,std::string_view name){const auto found=values.find(std::string(name));if(found==values.end())throw Exception("Unimplemented",u"Missing pinned AMS mapping data");return found->second;}
ParseMethod method(const Property& methods,std::u16string_view name){return functions::fromProperty(property_host::get(methods,property_host::debugText(name)));}
}
#include "ams_mapping_data.inc"
void AmsMappings::initializeModule(ParserRuntime& runtime){
    std::map<std::string,Property> tables;appendPinnedAmsMappingTables(tables);
    const Property methods=runtime.amsMethods;
    const Property parseMethods=runtime.parseMethods;
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("AMSmath-mathchar0mo"),method(parseMethods,u"mathchar0mo"),table(tables,"AMSmath-mathchar0mo"));
    const auto regex=runtime.regexp().create(u"[-*]",u"i");
    (void)TokenMapHost::create<RegExpMap>(runtime.mapHandler,Property("AMSmath-operatorLetter"),method(methods,u"operatorLetter"),BaseMappings::regexpContext(runtime,Value(regex)));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("AMSmath-macros"),table(tables,"AMSmath-macros"),methods);
    (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property("AMSmath-environment"),method(parseMethods,u"environment"),table(tables,"AMSmath-environment"),methods);
    (void)TokenMapHost::create<DelimiterMap>(runtime.mapHandler,Property("AMSmath-delimiter"),method(parseMethods,u"delimiter"),table(tables,"AMSmath-delimiter"));
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("AMSsymbols-mathchar0mi"),method(parseMethods,u"mathchar0mi"),table(tables,"AMSsymbols-mathchar0mi"));
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("AMSsymbols-mathchar0mo"),method(parseMethods,u"mathchar0mo"),table(tables,"AMSsymbols-mathchar0mo"));
    (void)TokenMapHost::create<DelimiterMap>(runtime.mapHandler,Property("AMSsymbols-delimiter"),method(parseMethods,u"delimiter"),table(tables,"AMSsymbols-delimiter"));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("AMSsymbols-macros"),table(tables,"AMSsymbols-macros"),methods);
}
}
