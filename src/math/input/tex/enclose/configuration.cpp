// MathJax 4.1.3 input/tex/enclose/EncloseConfiguration.ts; Apache-2.0.
module folia.math.input.tex.enclose.configuration;
import std;

namespace folia::math::tex {
std::array<KeyValueOption, 7> const ENCLOSE_OPTIONS{{{u"data-arrowhead", std::nullopt},
                                                     {u"color", std::nullopt},
                                                     {u"mathcolor", std::nullopt},
                                                     {u"background", std::nullopt},
                                                     {u"mathbackground", std::nullopt},
                                                     {u"data-padding", std::nullopt},
                                                     {u"data-thickness", std::nullopt}}};

DispatchOutcome EncloseMethods::Enclose(TexParser &parser, MacroInvocation const &invocation) {
    auto notation = parser.GetArgument(invocation.dispatch_token).value();
    std::ranges::replace(notation, u',', u' ');
    auto attributes = parser.GetBrackets(invocation.dispatch_token, u"").value();
    auto math = parser.ParseArg(invocation.dispatch_token);
    NodeDefinition definition(
        parse_util::keyvalOptions(attributes, std::span<KeyValueOption const>(ENCLOSE_OPTIONS)));
    definition.attributes.set(u"notation", std::move(notation));
    const std::array children{math};
    parser.Push(&parser.create(mml::NodeKind::menclose, children, std::move(definition)));
    return NoParseResult{};
}
ConfigurationRef EncloseConfiguration::initializeModule(ConfigurationHandler &registry,
                                                        MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"enclose", std::nullopt}, {Macro(u"enclose", &EncloseMethods::Enclose)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"enclose"}}};
    return Configuration::create(registry, u"enclose", std::move(definition));
}
} // namespace folia::math::tex
