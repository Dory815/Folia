// MathJax 4.1.3 input/tex/ParseMethods.ts; Apache-2.0.
module folia.math.input.tex.parse_methods;
import folia.math.input.tex.parse_util;
import std;

namespace folia::math::tex {
namespace {
Token const &token(TokenInput const &input) {
    auto value = std::get<Token const *>(input.value);
    if (!value)
        throw std::invalid_argument("Character method requires a Token");
    return *value;
}
std::u16string_view character(Token const &value) {
    if (!value.character)
        throw std::invalid_argument("Character method requires character text");
    return *value.character;
}
// Both methods use String.match, not stateful RegExp.exec. Global matches
// always start at zero and leave lastIndex zero; sticky-only matches use it.
std::optional<std::u16string> firstMatch(regexp_host::RegExp &expression,
                                         std::u16string_view text) {
    const bool global = expression.flags().find(u'g') != std::u16string::npos;
    if (global)
        expression.last_index = 0;
    auto match = expression.exec(std::u16string(text));
    if (global)
        expression.last_index = 0;
    return match ? match->at(0) : std::nullopt;
}
NodeDefinition variant(std::u16string value) {
    NodeDefinition result;
    result.attributes.set(u"mathvariant", std::move(value));
    return result;
}
DispatchOutcome push(TexParser &parser, mml::NodeKind kind, std::u16string_view text,
                     NodeDefinition definition) {
    parser.Push(&parser.createToken(kind, text, std::move(definition)));
    return NoParseResult{};
}
} // namespace
DispatchOutcome ParseMethods::variable(TexParser &parser, TokenInput const &input) {
    std::u16string text(std::get<std::u16string_view>(input.value));
    auto definition = parse_util::getFontDef(parser);
    auto const &env = parser.stack.env();
    if (env.multi_letter_identifiers && *env.multi_letter_identifiers &&
        (!env.font || !env.font->empty())) {
        auto match =
            firstMatch(**env.multi_letter_identifiers,
                       parser.string().substr(parser.position() ? parser.position() - 1 : 0));
        if (match && !match->empty())
            text = std::move(*match);
        parser.setPosition(parser.position() + text.size() - 1);
        auto font = definition.attributes.own(u"mathvariant");
        if (font && std::get<std::u16string>(*font) == u"normal" &&
            env.no_auto_op.value_or(false) && text.size() > 1)
            definition.properties.tex.auto_operator = false;
    }
    if (!definition.attributes.own(u"mathvariant") && parse_util::isLatinOrGreekChar(text)) {
        auto style = parser.configuration.mathStyle(text, false);
        if (!style.empty())
            definition.attributes.set(u"mathvariant", std::move(style));
    }
    return push(parser, mml::NodeKind::mi, text, std::move(definition));
}
DispatchOutcome ParseMethods::digit(TexParser &parser, TokenInput const &) {
    auto const &pattern = parser.options().numberPattern;
    if (!pattern || !*pattern)
        throw std::logic_error("Digit parsing requires numberPattern");
    auto start = parser.position()         ? parser.position() - 1
                 : parser.string().empty() ? 0
                                           : parser.string().size() - 1;
    auto match = firstMatch(**pattern, parser.string().substr(start));
    if (!match)
        return ContinueDispatch{};
    auto text = *match;
    std::erase_if(text, [](char16_t c) { return c == u'{' || c == u'}'; });
    auto &node = parser.createToken(mml::NodeKind::mn, text, parse_util::getFontDef(parser));
    parser.setPosition(parser.position() + match->size() - 1);
    parser.Push(&node);
    return Handled{};
}
DispatchOutcome ParseMethods::controlSequence(TexParser &parser, TokenInput const &) {
    auto name = parser.GetCS();
    parser.parse(HandlerKind::macro, ParseInput{parser, std::move(name)});
    return NoParseResult{};
}
DispatchOutcome ParseMethods::lcGreek(TexParser &parser, TokenInput const &input) {
    auto text = character(token(input));
    auto style = parser.configuration.mathStyle(text, false);
    return push(parser, mml::NodeKind::mi, text,
                variant(style.empty() ? u"italic" : std::move(style)));
}
DispatchOutcome ParseMethods::ucGreek(TexParser &parser, TokenInput const &input) {
    auto text = character(token(input));
    auto const &font = parser.stack.env().font;
    auto style = font && !font->empty() ? *font : parser.configuration.mathStyle(text, true);
    return push(parser, mml::NodeKind::mi, text,
                variant(style.empty() ? u"normal" : std::move(style)));
}
DispatchOutcome ParseMethods::mathchar0mi(TexParser &parser, TokenInput const &input) {
    auto const &value = token(input);
    auto definition = value.attributes ? NodeDefinition{*value.attributes} : variant(u"italic");
    return push(parser, mml::NodeKind::mi, character(value), std::move(definition));
}
DispatchOutcome ParseMethods::mathchar0mo(TexParser &parser, TokenInput const &input) {
    auto const &value = token(input);
    NodeDefinition definition{value.attributes.value_or(mml::AttributeSet{})};
    definition.attributes.set(u"stretchy", false);
    auto &node = parser.createToken(mml::NodeKind::mo, character(value), std::move(definition));
    node.properties().tex.fix_stretchy = true;
    parser.configuration.addNode(u"fixStretchy", node);
    parser.Push(&node);
    return NoParseResult{};
}
DispatchOutcome ParseMethods::mathchar7(TexParser &parser, TokenInput const &input) {
    auto const &value = token(input);
    auto definition = value.attributes ? NodeDefinition{*value.attributes} : variant(u"normal");
    auto const &font = parser.stack.env().font;
    if (font && !font->empty())
        definition.attributes.set(u"mathvariant", *font);
    return push(parser, mml::NodeKind::mi, character(value), std::move(definition));
}
DispatchOutcome ParseMethods::delimiter(TexParser &parser, TokenInput const &input) {
    auto const &value = token(input);
    NodeDefinition definition;
    definition.attributes.set(u"fence", false);
    definition.attributes.set(u"stretchy", false);
    if (value.attributes)
        for (auto const &[key, attribute] : value.attributes->ownEntries())
            definition.attributes.set(key, attribute);
    const auto text = value.character ? std::optional<std::u16string_view>{*value.character} : std::nullopt;
    auto &node = parser.createToken(mml::NodeKind::mo, text, std::move(definition));
    if (text && *text == u"|")
        node.properties().tex.keep_attributes = u"stretchy";
    parser.Push(&node);
    return NoParseResult{};
}
DispatchOutcome ParseMethods::environment(TexParser &parser, std::u16string_view name,
                                          EnvironmentBodyHandler body,
                                          std::span<TexArgument const> arguments) {
    auto begin = parser.itemFactory().create(StackKind::begin);
    dynamic_cast<BeginItem &>(*begin).name = name;
    parser.Push(body(parser, begin, arguments.subspan(std::min<std::size_t>(1, arguments.size()))));
    return NoParseResult{};
}
} // namespace folia::math::tex
