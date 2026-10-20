// MathJax 4.1.3 input/tex/html/HtmlConfiguration.ts; Apache-2.0.
module folia.math.input.tex.html.configuration;
import std;

namespace folia::math::tex {
ConfigurationRef HtmlConfiguration::initializeModule(ConfigurationHandler &registry,
                                                     MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"html_macros", std::nullopt},
                     {Macro(u"data", &HtmlMethods::Data), Macro(u"href", &HtmlMethods::Href),
                      Macro(u"class", &HtmlMethods::Class), Macro(u"style", &HtmlMethods::Style),
                      Macro(u"cssId", &HtmlMethods::Id)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"html_macros"}}};
    return Configuration::create(registry, u"html", std::move(definition));
}
} // namespace folia::math::tex