// MathJax 4.1.3 input/tex/cancel/CancelConfiguration.ts; Apache-2.0.
module folia.math.input.tex.cancel.configuration;
import folia.math.input.tex.enclose.configuration;
import std;

namespace folia::math::tex {
DispatchOutcome CancelMethods::Cancel(TexParser &parser, MacroInvocation const &invocation) {
    auto attributes = parser.GetBrackets(invocation.dispatch_token, u"").value();
    auto math = parser.ParseArg(invocation.dispatch_token);
    NodeDefinition definition(
        parse_util::keyvalOptions(attributes, std::span<KeyValueOption const>(ENCLOSE_OPTIONS)));
    auto args = std::get<std::span<TexArgument const>>(invocation.arguments);
    definition.attributes.set(u"notation", std::get<std::u16string>(args.front()));
    const std::array children{math};
    parser.Push(&parser.create(mml::NodeKind::menclose, children, std::move(definition)));
    return NoParseResult{};
}
DispatchOutcome CancelMethods::CancelTo(TexParser &parser, MacroInvocation const &invocation) {
    auto attributes = parser.GetBrackets(invocation.dispatch_token, u"").value();
    auto value = parser.ParseArg(invocation.dispatch_token);
    auto math = parser.ParseArg(invocation.dispatch_token);
    NodeDefinition definition(
        parse_util::keyvalOptions(attributes, std::span<KeyValueOption const>(ENCLOSE_OPTIONS)));
    definition.attributes.set(u"notation", u"updiagonalstrike updiagonalarrow northeastarrow");
    NodeDefinition padded;
    padded.attributes.set(u"depth", u"-.1em");
    padded.attributes.set(u"height", u"+.1em");
    padded.attributes.set(u"voffset", u".1em");
    const std::array valueChildren{value};
    auto &script = parser.create(mml::NodeKind::mpadded, valueChildren, std::move(padded));
    const std::array mathChildren{math};
    auto &enclosed = parser.create(mml::NodeKind::menclose, mathChildren, std::move(definition));
    const std::array children{&enclosed, &script};
    parser.Push(&parser.create(mml::NodeKind::msup, children));
    return NoParseResult{};
}
ConfigurationRef CancelConfiguration::initializeModule(ConfigurationHandler &registry,
                                                       MapHandler &maps) {
    if (!registry.get(u"enclose"))
        EncloseConfiguration::initializeModule(registry, maps);
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"cancel", std::nullopt},
        {Macro(u"cancel", &CancelMethods::Cancel, {std::u16string(u"updiagonalstrike")}),
         Macro(u"bcancel", &CancelMethods::Cancel, {std::u16string(u"downdiagonalstrike")}),
         Macro(u"xcancel", &CancelMethods::Cancel,
               {std::u16string(u"updiagonalstrike downdiagonalstrike")}),
         Macro(u"cancelto", &CancelMethods::CancelTo)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"cancel"}}};
    return Configuration::create(registry, u"cancel", std::move(definition));
}
} // namespace folia::math::tex
