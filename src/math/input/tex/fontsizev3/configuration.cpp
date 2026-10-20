// MathJax 4.1.3 input/tex/fontsizev3/FontSizeV3Configuration.ts; Apache-2.0.
module folia.math.input.tex.fontsizev3.configuration;
import folia.math.input.tex.base_methods;
import std;

namespace folia::math::tex {
ConfigurationRef FontSizeV3Configuration::initializeModule(ConfigurationHandler &registry,
                                                           MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"fontsizev3-macros", std::nullopt},
        {Macro(u"tiny", &BaseMethods::SetSize, {0.5}), Macro(u"Tiny", &BaseMethods::SetSize, {0.6}),
         Macro(u"scriptsize", &BaseMethods::SetSize, {0.7}),
         Macro(u"small", &BaseMethods::SetSize, {0.85}),
         Macro(u"normalsize", &BaseMethods::SetSize, {1.0}),
         Macro(u"large", &BaseMethods::SetSize, {1.2}),
         Macro(u"Large", &BaseMethods::SetSize, {1.44}),
         Macro(u"LARGE", &BaseMethods::SetSize, {1.73}),
         Macro(u"huge", &BaseMethods::SetSize, {2.07}),
         Macro(u"Huge", &BaseMethods::SetSize, {2.49})}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"fontsizev3-macros"}}};
    return Configuration::create(registry, u"fontsizev3", std::move(definition));
}
} // namespace folia::math::tex