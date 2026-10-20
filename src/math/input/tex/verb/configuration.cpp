// MathJax 4.1.3 input/tex/verb/VerbConfiguration.ts; Apache-2.0.
module folia.math.input.tex.verb.configuration;
import std;

namespace folia::math::tex {
DispatchOutcome VerbMethods::Verb(TexParser &parser, MacroInvocation const &invocation) {
    const auto delimiter = parser.GetNext();
    const auto start = parser.position() + 1;
    parser.setPosition(start);
    if (delimiter.empty()) {
        const std::array<std::u16string, 1> arguments{std::u16string(invocation.dispatch_token)};
        throw TexError("MissingArgFor", u"Missing argument for %1", arguments);
    }
    auto source = parser.string();
    auto position = parser.position();
    // Source charAt compares one UTF-16 unit with the complete GetNext result.
    while (position < source.size() && source.substr(position, 1) != delimiter)
        ++position;
    parser.setPosition(position);
    if (position == source.size()) {
        const std::array arguments{parser.currentCS()};
        throw TexError("NoClosingDelim", u"Can't find closing delimiter for %1", arguments);
    }
    auto content = std::u16string(source.substr(start, position - start));
    std::ranges::replace(content, u' ', u'\u00A0');
    parser.setPosition(position + 1);
    NodeDefinition definition;
    definition.attributes.set(u"mathvariant", u"monospace");
    parser.Push(&parser.createToken(mml::NodeKind::mtext, content, std::move(definition)));
    return NoParseResult{};
}
ConfigurationRef VerbConfiguration::initializeModule(ConfigurationHandler &registry,
                                                     MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"verb", std::nullopt}, {Macro(u"verb", &VerbMethods::Verb)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"verb"}}};
    return Configuration::create(registry, u"verb", std::move(definition));
}
} // namespace folia::math::tex
