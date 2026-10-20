// MathJax 4.1.3 input/tex/gensymb/GensymbConfiguration.ts; Apache-2.0.
module folia.math.input.tex.gensymb.configuration;
import std;

namespace folia::math::tex {
DispatchOutcome mathcharUnit(TexParser &parser, TokenInput const &input) {
    auto entry = std::get_if<Token const *>(&input.value);
    if (!entry || !*entry)
        throw std::invalid_argument("A unit requires a character token");
    auto const &token = **entry;
    NodeDefinition definition;
    if (token.attributes)
        definition.attributes = *token.attributes;
    definition.attributes.set(u"mathvariant", std::u16string(u"normal"));
    definition.attributes.set(u"class", std::u16string(u"MathML-Unit"));
    const auto text =
        token.character ? std::optional<std::u16string_view>(*token.character) : std::nullopt;
    parser.Push(&parser.createToken(mml::NodeKind::mi, text, std::move(definition)));
    return NoParseResult{};
}
ConfigurationRef GensymbConfiguration::initializeModule(ConfigurationHandler &registry,
                                                        MapHandler &maps) {
    maps.registerMap(std::make_shared<CharacterMap>(CharacterMapInit{
        {u"gensymb-symbols", TokenMapParser{&mathcharUnit}},
        {Token(u"ohm", u"\u2126"), Token(u"degree", u"\u00B0"), Token(u"celsius", u"\u2103"),
         Token(u"perthousand", u"\u2030"), Token(u"micro", u"\u00B5")}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"gensymb-symbols"}}};
    return Configuration::create(registry, u"gensymb", std::move(definition));
}
} // namespace folia::math::tex
