// MathJax 4.1.3 input/tex/bboldx/BboldxMethods.ts; Apache-2.0.
module folia.math.input.tex.bboldx.methods;
import folia.math.input.tex.textmacros.methods;
import std;

namespace folia::math::tex {
namespace {
std::u16string getBbxFont(TexParser &parser, std::u16string_view normal, std::u16string_view light,
                          std::u16string_view bfbb) {
    auto const &options = parser.options().bboldx.value();
    return std::u16string(options.bfbb.value_or(false)    ? bfbb
                          : options.light.value_or(false) ? light
                                                          : normal);
}
DispatchOutcome character(TexParser &parser, TokenInput const &input, bool bold, bool delimiter) {
    auto const &token = *std::get<Token const *>(input.value);
    auto font = bold ? getBbxFont(parser, u"-bboldx-bold", u"-bboldx", u"-bboldx-bold")
                     : getBbxFont(parser, u"-bboldx", u"-bboldx-light", u"-bboldx-bold");
    NodeDefinition definition;
    if (delimiter)
        definition.attributes.set(u"stretchy", false);
    definition.attributes.set(u"mathvariant", std::move(font));
    const auto text =
        token.character ? std::optional<std::u16string_view>(*token.character) : std::nullopt;
    parser.Push(&parser.createToken(delimiter ? mml::NodeKind::mo : mml::NodeKind::mi, text,
                                    std::move(definition)));
    return NoParseResult{};
}
} // namespace
DispatchOutcome BboldxMethods::ChooseFont(TexParser &parser, MacroInvocation const &invocation) {
    auto arguments = std::get<std::span<TexArgument const>>(invocation.arguments);
    const std::array<TexArgument, 1> selected{
        getBbxFont(parser, std::get<std::u16string>(arguments[0]),
                   std::get<std::u16string>(arguments[1]), std::get<std::u16string>(arguments[2]))};
    return BaseMethods::MathFont(parser,
                                 MacroInvocation{invocation.input_token, invocation.dispatch_token,
                                                 std::span<TexArgument const>(selected)});
}
DispatchOutcome BboldxMethods::ChooseTextFont(TexParser &parser,
                                              MacroInvocation const &invocation) {
    auto arguments = std::get<std::span<TexArgument const>>(invocation.arguments);
    const std::array<TexArgument, 1> selected{
        getBbxFont(parser, std::get<std::u16string>(arguments[0]),
                   std::get<std::u16string>(arguments[1]), std::get<std::u16string>(arguments[2]))};
    return TextMacrosMethods::TextFont(
        parser, MacroInvocation{invocation.input_token, invocation.dispatch_token,
                                std::span<TexArgument const>(selected)});
}
DispatchOutcome BboldxMethods::mathchar0miNormal(TexParser &parser, TokenInput const &input) {
    return character(parser, input, false, false);
}
DispatchOutcome BboldxMethods::delimiterNormal(TexParser &parser, TokenInput const &input) {
    return character(parser, input, false, true);
}
DispatchOutcome BboldxMethods::mathchar0miBold(TexParser &parser, TokenInput const &input) {
    return character(parser, input, true, false);
}
DispatchOutcome BboldxMethods::delimiterBold(TexParser &parser, TokenInput const &input) {
    return character(parser, input, true, true);
}
} // namespace folia::math::tex
