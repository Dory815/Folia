// Source-owned Newcommand command map registration.
module folia.math.input.tex.newcommand.mappings;
import folia.math.input.tex.newcommand.methods;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void NewcommandMappings::initializeModule(ParserRuntime& runtime){
    const Property methods=NewcommandMethods::initializeModule(runtime);
    PropertyList map;
    for(const auto name:{u"newcommand",u"renewcommand"})map.setOwn(property_host::debugText(name),property_host::get(methods,property_host::debugText(u"NewCommand")));
    map.setOwn(property_host::debugText(u"newenvironment"),property_host::get(methods,property_host::debugText(u"NewEnvironment")));
    map.setOwn(property_host::debugText(u"renewenvironment"),property_host::get(methods,property_host::debugText(u"NewEnvironment")));
    map.setOwn(property_host::debugText(u"def"),property_host::get(methods,property_host::debugText(u"MacroDef")));
    map.setOwn(property_host::debugText(u"let"),property_host::get(methods,property_host::debugText(u"Let")));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("Newcommand-macros"),Property(map));
}
}
