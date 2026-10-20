// MathJax 4.1.3 input/tex/bbox/BboxConfiguration.ts; Apache-2.0.
module folia.math.input.tex.bbox.configuration;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;

namespace folia::math::tex {
namespace {
std::u16string trim(std::u16string_view value) {
    while (!value.empty() && js_input::isWhitespace(value.front()))
        value.remove_prefix(1);
    while (!value.empty() && js_input::isWhitespace(value.back()))
        value.remove_suffix(1);
    return std::u16string(value);
}
std::u16string BBoxStyle(std::u16string styles) {
    return styles;
}
std::u16string BBoxPadding(std::u16string pad) {
    return pad;
}
void duplicate(std::u16string_view property, std::u16string_view command) {
    throw TexError(
        "MultipleBBoxProperty", u"%1 specified twice in %2",
        std::array<std::u16string, 2>{std::u16string(property), std::u16string(command)});
}
mml::MmlNode &wrap(TexParser &parser, mml::NodeKind kind, mml::MmlNode &math,
                   mml::AttributeSet definition) {
    const std::array<mml::MmlNode *, 1> children{&math};
    NodeDefinition def;
    def.attributes = std::move(definition);
    return parser.create(kind, children, std::move(def));
}
} // namespace
DispatchOutcome BboxMethods::BBox(TexParser &parser, MacroInvocation const &invocation) {
    const auto options = parser.GetBrackets(invocation.dispatch_token, std::u16string{}).value();
    auto math = parser.ParseArg(invocation.dispatch_token);
    std::optional<mml::AttributeSet> padding;
    std::optional<std::u16string> background, style;
    std::size_t start = 0;
    do {
        const auto comma = options.find(u',', start);
        const auto part = trim(std::u16string_view(options).substr(
            start, comma == options.npos ? options.size() - start : comma - start));
        const auto match =
            regexp_host::RegExp(u"^(\\.\\d+|\\d+(\\.\\d*)?)(pt|em|ex|mu|px|in|cm|mm)$").exec(part);
        if (match) {
            if (padding)
                duplicate(u"Padding", invocation.dispatch_token);
            const auto number = match->at(1).value();
            const auto unit = match->at(3).value();
            const auto pad = BBoxPadding(number + unit);
            if (!pad.empty()) {
                padding.emplace();
                padding->set(u"height", u"+" + pad);
                padding->set(u"depth", u"+" + pad);
                padding->set(u"lspace", pad);
                const auto width = js_number::toString(2 * js_input::number(number));
                padding->set(u"width", u"+" + std::u16string(width.begin(), width.end()) + unit);
            }
        } else if (regexp_host::RegExp(u"^([a-z0-9]+|#[0-9a-f]{6}|#[0-9a-f]{3})$", u"i")
                       .exec(part)) {
            if (background)
                duplicate(u"Background", invocation.dispatch_token);
            background = part;
        } else if (regexp_host::RegExp(u"^[-a-z]+:", u"i").exec(part)) {
            if (style)
                duplicate(u"Style", invocation.dispatch_token);
            style = BBoxStyle(part);
        } else if (!part.empty()) {
            throw TexError("InvalidBBoxProperty",
                           u"\"%1\" doesn't look like a color, a padding dimension, or a style",
                           std::array<std::u16string, 1>{part});
        }
        if (comma == options.npos)
            break;
        start = comma + 1;
    } while (true);
    if (padding)
        math = &wrap(parser, mml::NodeKind::mpadded, *math, std::move(*padding));
    if (background || style) {
        mml::AttributeSet definition;
        if (background)
            definition.set(u"mathbackground", *background);
        if (style)
            definition.set(u"style", *style);
        math = &wrap(parser, mml::NodeKind::mstyle, *math, std::move(definition));
    }
    parser.Push(math);
    return NoParseResult{};
}
ConfigurationRef BboxConfiguration::initializeModule(ConfigurationHandler &registry,
                                                     MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"bbox", std::nullopt}, {Macro(u"bbox", &BboxMethods::BBox)}}}));
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"bbox"}}};
    return Configuration::create(registry, u"bbox", std::move(definition));
}
} // namespace folia::math::tex