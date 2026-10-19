// Source-owned registration of AmsCdMappings.ts.
module folia.math.input.tex.amscd.mappings;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
Property methodMap(const AmsCdMethodSet& methods){return Property(PropertyList{{"CD",Property(methods.cd)},{"arrow",Property(methods.arrow)},{"cell",Property(methods.cell)},{"minCDarrowwidth",Property(methods.minWidth)},{"minCDarrowheight",Property(methods.minHeight)}});}
}
void AmsCdMappings::initializeModule(ParserRuntime& runtime,const AmsCdMethodSet& methods){
    const Property functionTable=methodMap(methods);
    const Property environment(PropertyList{{"CD",Property("CD")}});
    const Property commands(PropertyList{{"minCDarrowwidth",Property("minCDarrowwidth")},{"minCDarrowheight",Property("minCDarrowheight")}});
    const Property special(PropertyList{{"@",Property("arrow")}});
    const Property parseMethods=ParseMethods::initializeModule(runtime);
    const auto environmentParser=functions::fromProperty(property_host::get(parseMethods,"environment"));
    (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property("amscd_environment"),environmentParser,environment,functionTable);
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("amscd_macros"),commands,functionTable);
    (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property("amscd_special"),special,functionTable);
}
}
