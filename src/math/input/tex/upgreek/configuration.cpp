// MathJax 4.1.3 input/tex/upgreek/UpgreekConfiguration.ts; Apache-2.0.
module folia.math.input.tex.upgreek.configuration;
import std;

namespace folia::math::tex {
DispatchOutcome mathchar0miNormal(TexParser &parser, TokenInput const &input) {
    auto entry = std::get_if<Token const *>(&input.value);
    if (!entry || !*entry)
        throw std::invalid_argument("Upright Greek requires a character token");
    auto const &token = **entry;
    NodeDefinition definition;
    if (token.attributes)
        definition.attributes = *token.attributes;
    definition.attributes.set(u"mathvariant", std::u16string(u"normal"));
    const auto text =
        token.character ? std::optional<std::u16string_view>(*token.character) : std::nullopt;
    parser.Push(&parser.createToken(mml::NodeKind::mi, text, std::move(definition)));
    return NoParseResult{};
}
ConfigurationRef UpgreekConfiguration::initializeModule(ConfigurationHandler &registry,
                                                        MapHandler &maps) {
    maps.registerMap(std::make_shared<CharacterMap>(
        CharacterMapInit{{u"upgreek", TokenMapParser{&mathchar0miNormal}},
                         {Token(u"upalpha", u"\u03B1"),      Token(u"upbeta", u"\u03B2"),
                          Token(u"upgamma", u"\u03B3"),      Token(u"updelta", u"\u03B4"),
                          Token(u"upepsilon", u"\u03F5"),    Token(u"upzeta", u"\u03B6"),
                          Token(u"upeta", u"\u03B7"),        Token(u"uptheta", u"\u03B8"),
                          Token(u"upiota", u"\u03B9"),       Token(u"upkappa", u"\u03BA"),
                          Token(u"uplambda", u"\u03BB"),     Token(u"upmu", u"\u03BC"),
                          Token(u"upnu", u"\u03BD"),         Token(u"upxi", u"\u03BE"),
                          Token(u"upomicron", u"\u03BF"),    Token(u"uppi", u"\u03C0"),
                          Token(u"uprho", u"\u03C1"),        Token(u"upsigma", u"\u03C3"),
                          Token(u"uptau", u"\u03C4"),        Token(u"upupsilon", u"\u03C5"),
                          Token(u"upphi", u"\u03D5"),        Token(u"upchi", u"\u03C7"),
                          Token(u"uppsi", u"\u03C8"),        Token(u"upomega", u"\u03C9"),
                          Token(u"upvarepsilon", u"\u03B5"), Token(u"upvartheta", u"\u03D1"),
                          Token(u"upvarpi", u"\u03D6"),      Token(u"upvarrho", u"\u03F1"),
                          Token(u"upvarsigma", u"\u03C2"),   Token(u"upvarphi", u"\u03C6"),
                          Token(u"Upgamma", u"\u0393"),      Token(u"Updelta", u"\u0394"),
                          Token(u"Uptheta", u"\u0398"),      Token(u"Uplambda", u"\u039B"),
                          Token(u"Upxi", u"\u039E"),         Token(u"Uppi", u"\u03A0"),
                          Token(u"Upsigma", u"\u03A3"),      Token(u"Upupsilon", u"\u03A5"),
                          Token(u"Upphi", u"\u03A6"),        Token(u"Uppsi", u"\u03A8"),
                          Token(u"Upomega", u"\u03A9")}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"upgreek"}}};
    return Configuration::create(registry, u"upgreek", std::move(definition));
}
} // namespace folia::math::tex
