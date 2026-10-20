// MathJax 4.1.3 input/tex/bbm/BbmConfiguration.ts; Apache-2.0.
module folia.math.input.tex.bbm.configuration;
import std;
namespace folia::math::tex {
DispatchOutcome BbmMethods::ChooseFont(TexParser &parser, MacroInvocation const &invocation) {
    const auto supplied = std::get<std::span<TexArgument const>>(invocation.arguments);
    const std::array<TexArgument, 1> arguments{
        supplied[parser.options().bbm->bold.value_or(false) ? 1 : 0]};
    return BaseMethods::MathFont(parser,
                                 MacroInvocation{invocation.input_token, invocation.dispatch_token,
                                                 std::span<TexArgument const>(arguments)});
}
DispatchOutcome BbmMethods::ChangeBold(TexParser &parser, MacroInvocation const &invocation) {
    parser.options().bbm->bold = parser.GetArgument(invocation.dispatch_token).value() == u"bold";
    return NoParseResult{};
}
ConfigurationRef BbmConfiguration::initializeModule(ConfigurationHandler &registry,
                                                    MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"bbm", std::nullopt},
        {Macro(u"mathversion", &BbmMethods::ChangeBold),
         Macro(u"mathbbm", &BbmMethods::ChooseFont,
               {std::u16string(u"-bbm-normal"), std::u16string(u"-bbm-bold")}),
         Macro(u"mathbbmss", &BbmMethods::ChooseFont,
               {std::u16string(u"-bbm-sans-serif"), std::u16string(u"-bbm-sans-serif-bold")}),
         Macro(u"mathbbmtt", BbmMethods::MathFont, {std::u16string(u"-bbm-monospace")})}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"bbm"}}};
    definition.options.bbm = BbmOptions{.bold = false};
    return Configuration::create(registry, u"bbm", std::move(definition));
}
} // namespace folia::math::tex
