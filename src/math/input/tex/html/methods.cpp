// MathJax 4.1.3 input/tex/html/HtmlMethods.ts; Apache-2.0.
module folia.math.input.tex.html.methods;
import folia.math.input.tex.node_util;
import folia.math.input.tex.parse_util;
import folia.math.util.js_number;
import std;

namespace folia::math::tex {
namespace {
mml::MmlNode &GetArgumentMML(TexParser &parser, std::u16string_view name) {
    auto arg = parser.ParseArg(name);
    if (!arg->isInferred())
        return *arg;
    auto &row = parser.create(mml::NodeKind::mrow);
    const std::vector<mml::MmlNode *> children(arg->children().begin(), arg->children().end());
    row.setChildren(children);
    NodeUtil::copyAttributes(*arg, row);
    return row;
}
bool isLegalAttributeName(std::u16string_view name) {
    for (std::size_t i = 0; i < name.size(); ++i) {
        const auto c = name[i];
        if (c <= 0x1f || (c >= 0x7f && c <= 0x9f) || c == u' ' || c == u'"' || c == u'\'' ||
            c == u'>' || c == u'/' || c == u'=')
            return false;
        if (c >= 0xd800 && c <= 0xdbff && i + 1 < name.size() && name[i + 1] >= 0xdc00 &&
            name[i + 1] <= 0xdfff) {
            const auto point = 0x10000 + ((c - 0xd800) << 10) + (name[++i] - 0xdc00);
            if ((point & 0xffff) >= 0xfffe)
                return false;
        } else if ((c >= 0xfdd0 && c <= 0xfdef) || c == 0xfffe || c == 0xffff) {
            return false;
        }
    }
    return true;
}
std::u16string priorText(mml::MmlNode const &node, std::u16string_view key) {
    auto value = node.attributes().lookup(key);
    if (!value || std::holds_alternative<mml::AttributeNull>(*value))
        return {};
    if (auto text = std::get_if<std::u16string>(value))
        return *text;
    if (auto boolean = std::get_if<bool>(value))
        return *boolean ? u"true" : u"";
    auto number = std::get<double>(*value);
    if (number == 0 || std::isnan(number))
        return {};
    auto text = js_number::toString(number);
    return {text.begin(), text.end()};
}
} // namespace
DispatchOutcome HtmlMethods::Data(TexParser &parser, MacroInvocation const &invocation) {
    const auto dataset = parser.GetArgument(invocation.dispatch_token).value();
    auto &arg = GetArgumentMML(parser, invocation.dispatch_token);
    auto data = parse_util::keyvalOptions(dataset);
    for (auto const &[key, value] : data.ownEntries()) {
        const auto attribute = u"data-" + key;
        if (!isLegalAttributeName(key))
            throw TexError("InvalidHTMLAttr", u"Invalid HTML attribute: %1",
                           std::array<std::u16string, 1>{attribute});
        arg.attributes().set(attribute, value);
    }
    parser.Push(&arg);
    return NoParseResult{};
}
DispatchOutcome HtmlMethods::Href(TexParser &parser, MacroInvocation const &invocation) {
    const auto url = parser.GetArgument(invocation.dispatch_token).value();
    auto &arg = GetArgumentMML(parser, invocation.dispatch_token);
    arg.attributes().set(u"href", url);
    parser.Push(&arg);
    return NoParseResult{};
}
DispatchOutcome HtmlMethods::Class(TexParser &parser, MacroInvocation const &invocation) {
    auto text = parser.GetArgument(invocation.dispatch_token).value();
    auto &arg = GetArgumentMML(parser, invocation.dispatch_token);
    const auto prior = priorText(arg, u"class");
    if (!prior.empty())
        text = prior + u" " + text;
    arg.attributes().set(u"class", std::move(text));
    parser.Push(&arg);
    return NoParseResult{};
}
DispatchOutcome HtmlMethods::Style(TexParser &parser, MacroInvocation const &invocation) {
    auto style = parser.GetArgument(invocation.dispatch_token).value();
    auto &arg = GetArgumentMML(parser, invocation.dispatch_token);
    auto prior = priorText(arg, u"style");
    if (!prior.empty()) {
        if (!std::holds_alternative<std::u16string>(*arg.attributes().lookup(u"style")))
            throw std::invalid_argument("A style attribute must contain text");
        // Preserve the original index: Source examines the prior style using
        // the incoming style's length, before appending its separator.
        const auto index = style.empty() ? std::u16string::npos : style.size() - 1;
        if (index >= prior.size() || prior[index] != u';')
            prior += u';';
        style = prior + u" " + style;
    }
    arg.attributes().set(u"style", std::move(style));
    parser.Push(&arg);
    return NoParseResult{};
}
DispatchOutcome HtmlMethods::Id(TexParser &parser, MacroInvocation const &invocation) {
    const auto id = parser.GetArgument(invocation.dispatch_token).value();
    auto &arg = GetArgumentMML(parser, invocation.dispatch_token);
    arg.attributes().set(u"id", id);
    parser.Push(&arg);
    return NoParseResult{};
}
} // namespace folia::math::tex
