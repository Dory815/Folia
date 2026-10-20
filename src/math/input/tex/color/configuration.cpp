// MathJax 4.1.3 input/tex/color/ColorConfiguration.ts; Apache-2.0.
module folia.math.input.tex.color.configuration;
import std;
namespace folia::math::tex {
namespace {
void configure(ParserConfiguration &, TeX &jax) {
    jax.session().enableColor() = ColorModel{};
}
} // namespace
ConfigurationRef ColorConfiguration::initializeModule(ConfigurationHandler &registry,
                                                      MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"color", std::nullopt},
        {Macro(u"color", &ColorMethods::Color), Macro(u"textcolor", &ColorMethods::TextColor),
         Macro(u"definecolor", &ColorMethods::DefineColor),
         Macro(u"colorbox", &ColorMethods::ColorBox),
         Macro(u"fcolorbox", &ColorMethods::FColorBox)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"color"}}};
    definition.config = ProcessorSpec<ConfigMethod>{configure};
    definition.options.color = ColorOptions{u"5px", u"2px"};
    return Configuration::create(registry, u"color", std::move(definition));
}
} // namespace folia::math::tex
