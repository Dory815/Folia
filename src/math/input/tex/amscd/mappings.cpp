// MathJax 4.1.3 input/tex/amscd/AmsCdMappings.ts; Apache-2.0.
module folia.math.input.tex.amscd.mappings;
import folia.math.input.tex.parse_methods;
import std;

namespace folia::math::tex {
void AmsCdMappings::initialize(MapHandler &maps) {
    maps.registerMap(std::make_shared<EnvironmentMap>(
        EnvironmentMapInit{MacroMapInit{{u"amscd_environment", std::nullopt},
                                        {Macro(u"CD", MacroMethod{&AmsCdMethods::CD}, {})}},
                           &ParseMethods::environment}));
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"amscd_macros", std::nullopt},
        {Macro(u"minCDarrowwidth", MacroMethod{&AmsCdMethods::minCDarrowwidth}, {}),
         Macro(u"minCDarrowheight", MacroMethod{&AmsCdMethods::minCDarrowheight}, {})}}}));
    maps.registerMap(std::make_shared<MacroMap>(MacroMapInit{
        {u"amscd_special", std::nullopt}, {Macro(u"@", MacroMethod{&AmsCdMethods::arrow}, {})}}));
}
} // namespace folia::math::tex