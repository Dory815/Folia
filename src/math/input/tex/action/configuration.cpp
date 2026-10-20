// MathJax 4.1.3 input/tex/action/ActionConfiguration.ts; Apache-2.0.
module folia.math.input.tex.action.configuration;
import std;

namespace folia::math::tex {
DispatchOutcome ActionMethods::Toggle(TexParser &parser, MacroInvocation const &invocation) {
    std::vector<mml::MmlNode *> children;
    while (true) {
        auto argument = parser.GetArgument(invocation.dispatch_token).value();
        if (argument == u"\\endtoggle")
            break;
        TexParser child(std::move(argument), ParserEnvironmentSeed{parser.stack.env()},
                        parser.configuration);
        children.push_back(child.mml());
    }
    NodeDefinition definition;
    definition.attributes.set(u"actiontype", u"toggle");
    parser.Push(&parser.create(mml::NodeKind::maction, children, std::move(definition)));
    return NoParseResult{};
}
DispatchOutcome ActionMethods::Mathtip(TexParser &parser, MacroInvocation const &invocation) {
    auto argument = parser.ParseArg(invocation.dispatch_token);
    auto tip = parser.ParseArg(invocation.dispatch_token);
    const std::array children{argument, tip};
    NodeDefinition definition;
    definition.attributes.set(u"actiontype", u"tooltip");
    parser.Push(&parser.create(mml::NodeKind::maction, children, std::move(definition)));
    return NoParseResult{};
}
ConfigurationRef ActionConfiguration::initializeModule(ConfigurationHandler &registry,
                                                       MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"action-macros", std::nullopt},
        {Macro(u"toggle", &ActionMethods::Toggle), Macro(u"mathtip", &ActionMethods::Mathtip),
         Macro(u"texttip", ActionMethods::Macro,
               {std::u16string(u"\\mathtip{#1}{\\text{#2}}"), 2.0})}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"action-macros"}}};
    return Configuration::create(registry, u"action", std::move(definition));
}
} // namespace folia::math::tex
