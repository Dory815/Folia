// MathJax 4.1.3 input/tex/newcommand/NewcommandMappings.ts; Apache-2.0.
module folia.math.input.tex.newcommand.mappings;
import std;
namespace folia::math::tex {
void NewcommandMappings::initialize(MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"Newcommand-macros", std::nullopt},
        {Macro(u"newcommand", &NewcommandMethods::NewCommand),
         Macro(u"renewcommand", &NewcommandMethods::NewCommand),
         Macro(u"newenvironment", &NewcommandMethods::NewEnvironment),
         Macro(u"renewenvironment", &NewcommandMethods::NewEnvironment),
         Macro(u"def", &NewcommandMethods::MacroDef), Macro(u"let", &NewcommandMethods::Let)}}}));
}
} // namespace folia::math::tex
