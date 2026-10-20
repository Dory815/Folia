// MathJax 4.1.3 input/tex/dsfont/DsfontConfiguration.ts; Apache-2.0.
module folia.math.input.tex.dsfont.configuration;
import folia.math.input.tex.base_methods;
import std;
namespace folia::math::tex {
DispatchOutcome DsfontConfiguration::ChooseFont(TexParser &parser,
                                                MacroInvocation const &invocation) {
    const std::array<TexArgument, 1> arguments{
        std::u16string(parser.options().dsfont->sans.value_or(false) ? u"-ds-ss" : u"-ds-rm")};
    return BaseMethods::MathFont(parser,
                                 MacroInvocation{invocation.input_token, invocation.dispatch_token,
                                                 std::span<TexArgument const>(arguments)});
}
ConfigurationRef DsfontConfiguration::initializeModule(ConfigurationHandler &registry,
                                                       MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(
        CommandMapInit{MacroMapInit{{u"dsfont", std::nullopt}, {Macro(u"mathds", &ChooseFont)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"dsfont"}}};
    definition.options.dsfont = DsfontOptions{.sans = false};
    return Configuration::create(registry, u"dsfont", std::move(definition));
}
} // namespace folia::math::tex
