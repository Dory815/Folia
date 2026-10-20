// MathJax 4.1.3 input/tex/units/UnitsConfiguration.ts; Apache-2.0.
module folia.math.input.tex.units.configuration;
import std;

namespace folia::math::tex {
DispatchOutcome UnitsMethods::Unit(TexParser &parser, MacroInvocation const &invocation) {
    const auto value = parser.GetBrackets(invocation.dispatch_token);
    const auto dimension = parser.GetArgument(invocation.dispatch_token).value();
    std::u16string macro = u"\\mathrm{" + dimension + u"}";
    if (value && !value->empty())
        macro = *value + (parser.options().units->loose.value_or(false) ? u"~" : u"\\,") + macro;
    parser.setString(macro + std::u16string(parser.string().substr(parser.position())));
    parser.setPosition(0);
    return NoParseResult{};
}
DispatchOutcome UnitsMethods::UnitFrac(TexParser &parser, MacroInvocation const &invocation) {
    const auto value = parser.GetBrackets(invocation.dispatch_token);
    const auto numerator = parser.GetArgument(invocation.dispatch_token).value();
    const auto denominator = parser.GetArgument(invocation.dispatch_token).value();
    std::u16string macro = u"\\nicefrac[\\mathrm]{" + numerator + u"}{" + denominator + u"}";
    if (value && !value->empty())
        macro = *value + (parser.options().units->loose.value_or(false) ? u"~" : u"\\,") + macro;
    parser.setString(macro + std::u16string(parser.string().substr(parser.position())));
    parser.setPosition(0);
    return NoParseResult{};
}
DispatchOutcome UnitsMethods::NiceFrac(TexParser &parser, MacroInvocation const &invocation) {
    const auto font = parser.GetBrackets(invocation.dispatch_token, u"\\mathrm").value();
    const auto numerator = parser.GetArgument(invocation.dispatch_token).value();
    const auto denominator = parser.GetArgument(invocation.dispatch_token).value();
    auto num = TexParser(font + u"{" + numerator + u"}", ParserEnvironmentSeed{parser.stack.env()},
                         parser.configuration)
                   .mml();
    auto den = TexParser(font + u"{" + denominator + u"}",
                         ParserEnvironmentSeed{parser.stack.env()}, parser.configuration)
                   .mml();
    NodeDefinition definition;
    if (!parser.options().units->ugly.value_or(false))
        definition.attributes.set(u"bevelled", true);
    const std::array children{num, den};
    parser.Push(&parser.create(mml::NodeKind::mfrac, children, std::move(definition)));
    return NoParseResult{};
}
ConfigurationRef UnitsConfiguration::initializeModule(ConfigurationHandler &registry,
                                                      MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"units", std::nullopt},
        {Macro(u"units", &UnitsMethods::Unit), Macro(u"unitfrac", &UnitsMethods::UnitFrac),
         Macro(u"nicefrac", &UnitsMethods::NiceFrac)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"units"}}};
    definition.options.units = UnitsOptions{.loose = false, .ugly = false};
    return Configuration::create(registry, u"units", std::move(definition));
}
} // namespace folia::math::tex
