// MathJax 4.1.3 input/tex/color/ColorMethods.ts; Apache-2.0.
module folia.math.input.tex.color.methods;
import folia.math.input.tex.parse_util;
import folia.math.util.js_number;
import std;

namespace folia::math::tex {
namespace {
ColorModel &model(TexParser &parser) {
    auto current = parser.configuration.packageData.colorModel();
    if (!current)
        throw std::logic_error("Color package has not been configured");
    return *current;
}
std::u16string required(TexParser &parser, std::u16string_view name) {
    return parser.GetArgument(name).value();
}
std::u16string numberText(double value) {
    auto text = js_number::toString(value);
    return {text.begin(), text.end()};
}
double paddingNumber(std::u16string_view source) {
    // Only the numeric prefix of the Source padding helper, with decimal
    // grammar and locale-independent conversion. No hexadecimal conversion.
    auto match = regexp_host::RegExp(
                     u"^[+]?(?:Infinity|(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?)")
                     .exec(std::u16string(source));
    if (!match)
        return std::numeric_limits<double>::quiet_NaN();
    auto value = match->at(0).value();
    if (value.starts_with(u"+"))
        value.erase(0, 1);
    if (value == u"Infinity")
        return std::numeric_limits<double>::infinity();
    std::string ascii(value.begin(), value.end());
    double result{};
    auto [end, error] = std::from_chars(ascii.data(), ascii.data() + ascii.size(), result);
    if (error == std::errc{} && end == ascii.data() + ascii.size())
        return result;
    if (error != std::errc::result_out_of_range)
        throw std::logic_error("Validated padding number failed conversion");
    auto exponentAt = ascii.find_first_of("eE");
    auto mantissa = std::string_view(ascii).substr(0, exponentAt);
    long long exponent{};
    if (exponentAt != ascii.npos) {
        auto digits = std::string_view(ascii).substr(exponentAt + 1);
        bool negative = digits.starts_with('-');
        if (digits.starts_with('-') || digits.starts_with('+'))
            digits.remove_prefix(1);
        for (auto digit : digits)
            exponent = std::min(1000000ll, exponent * 10 + digit - '0');
        if (negative)
            exponent = -exponent;
    }
    auto dot = mantissa.find('.');
    long long before = static_cast<long long>(dot == mantissa.npos ? mantissa.size() : dot);
    long long position{};
    for (auto digit : mantissa) {
        if (digit == '.')
            continue;
        if (digit != '0')
            return before - position - 1 + exponent < 0 ? 0
                                                        : std::numeric_limits<double>::infinity();
        ++position;
    }
    return 0;
}
} // namespace
NodeDefinition padding(std::u16string_view colorPadding) {
    auto pad = u"+" + std::u16string(colorPadding);
    auto unitMatch = regexp_host::RegExp(u"^.*?([a-z]*)$").exec(std::u16string(colorPadding));
    auto unit = std::u16string(colorPadding);
    if (unitMatch) {
        unit.replace(unitMatch->index, unitMatch->at(0)->size(), unitMatch->at(1).value());
    }
    NodeDefinition definition;
    definition.attributes.set(u"width", u"+" + numberText(2 * paddingNumber(pad)) + unit);
    definition.attributes.set(u"height", pad);
    definition.attributes.set(u"depth", pad);
    definition.attributes.set(u"lspace", std::u16string(colorPadding));
    return definition;
}
DispatchOutcome ColorMethods::Color(TexParser &parser, MacroInvocation const &invocation) {
    auto colorModel = parser.GetBrackets(invocation.dispatch_token, u"").value();
    auto definition = required(parser, invocation.dispatch_token);
    auto color = model(parser).getColor(colorModel, definition);
    auto style = parser.itemFactory().create(StackKind::style);
    dynamic_cast<StyleItem &>(*style).styles.set(u"mathcolor", color);
    parser.stack.env().color = color;
    parser.Push(std::move(style));
    return NoParseResult{};
}
DispatchOutcome ColorMethods::TextColor(TexParser &parser, MacroInvocation const &invocation) {
    auto colorModel = parser.GetBrackets(invocation.dispatch_token, u"").value();
    auto definition = required(parser, invocation.dispatch_token);
    auto color = model(parser).getColor(colorModel, definition);
    mml::MmlNode *math{};
    {
        auto &environment = parser.stack.env();
        struct Restore {
            TexEnvironment &environment;
            std::optional<std::u16string> old;
            ~Restore() {
                environment.color = old && !old->empty() ? old : std::nullopt;
            }
        } restore{environment, environment.color};
        environment.color = color;
        math = parser.ParseArg(invocation.dispatch_token);
    }
    NodeDefinition attributes;
    attributes.attributes.set(u"mathcolor", color);
    const std::array children{math};
    parser.Push(&parser.create(mml::NodeKind::mstyle, children, std::move(attributes)));
    return NoParseResult{};
}
DispatchOutcome ColorMethods::DefineColor(TexParser &parser, MacroInvocation const &invocation) {
    auto name = required(parser, invocation.dispatch_token);
    auto colorModel = required(parser, invocation.dispatch_token);
    auto definition = required(parser, invocation.dispatch_token);
    model(parser).defineColor(colorModel, name, definition);
    parser.Push(parser.itemFactory().create(StackKind::null_item));
    return NoParseResult{};
}
DispatchOutcome ColorMethods::ColorBox(TexParser &parser, MacroInvocation const &invocation) {
    auto colorModel = parser.GetBrackets(invocation.dispatch_token, u"").value();
    auto definition = required(parser, invocation.dispatch_token);
    auto math = parse_util::internalMath(parser, required(parser, invocation.dispatch_token));
    NodeDefinition attributes;
    attributes.attributes.set(u"mathbackground", model(parser).getColor(colorModel, definition));
    auto &node = parser.create(mml::NodeKind::mpadded, math, std::move(attributes));
    NodeUtil::setProperties(node, padding(parser.options().color->padding.value()));
    parser.Push(&node);
    return NoParseResult{};
}
DispatchOutcome ColorMethods::FColorBox(TexParser &parser, MacroInvocation const &invocation) {
    auto frameModel = parser.GetBrackets(invocation.dispatch_token, u"").value();
    auto frameName = required(parser, invocation.dispatch_token);
    auto colorModel = parser.GetBrackets(invocation.dispatch_token, frameModel).value();
    auto colorName = required(parser, invocation.dispatch_token);
    auto math = parse_util::internalMath(parser, required(parser, invocation.dispatch_token));
    auto const &options = parser.options().color.value();
    NodeDefinition attributes;
    attributes.attributes.set(u"mathbackground", model(parser).getColor(colorModel, colorName));
    attributes.attributes.set(u"style", u"border: " + options.borderWidth.value() + u" solid " +
                                            model(parser).getColor(frameModel, frameName));
    auto &node = parser.create(mml::NodeKind::mpadded, math, std::move(attributes));
    NodeUtil::setProperties(node, padding(options.padding.value()));
    parser.Push(&node);
    return NoParseResult{};
}
} // namespace folia::math::tex
