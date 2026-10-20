// MathJax 4.1.3 input/tex/unicode/UnicodeConfiguration.ts; Apache-2.0.
module folia.math.input.tex.unicode.configuration;

import folia.math.input.tex.base_configuration;
import folia.math.input.tex.unit_util;
import folia.math.util.entities;
import folia.math.util.js_numeric_input;
import folia.math.util.string;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {

class UnicodeCommandMap final : public CommandMap {
  public:
    std::unordered_map<char32_t, std::u16string> fonts;

    UnicodeCommandMap()
        : CommandMap(CommandMapInit{
              MacroMapInit{{u"unicode", std::nullopt},
                           {Macro(u"unicode", &UnicodeMethods::Unicode),
                            Macro(u"U", &UnicodeMethods::RawUnicode),
                            Macro(u"char", &UnicodeMethods::Char)}}}) {}
};

[[noreturn]] void error(TexParser &parser, std::string id, std::u16string message) {
    const std::array args{parser.currentCS()};
    throw TexError(std::move(id), std::move(message), args);
}

std::u16string fromCodePoint(double value) {
    const std::array values{value};
    return string_util::unicodeString(values);
}

std::u16string_view trim(std::u16string_view text) {
    while (!text.empty() && js_input::isWhitespace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && js_input::isWhitespace(text.back()))
        text.remove_suffix(1);
    return text;
}

bool lineTerminator(char32_t value) {
    return value == 10 || value == 13 || value == 0x2028 || value == 0x2029;
}

bool whitespace(char32_t value) {
    return value <= 0xffff && js_input::isWhitespace(static_cast<char16_t>(value));
}

std::optional<std::pair<std::u16string, std::size_t>> numberChar(
    std::u16string_view text, std::size_t start, std::size_t limit,
    js_input::IntegerRadix radix) {
    auto end = start;
    while (end < text.size() && end - start < limit) {
        const auto value = text[end];
        auto digit = value >= u'0' && value <=
                         (radix == js_input::IntegerRadix::Octal ? u'7' : u'9');
        if (radix == js_input::IntegerRadix::Hex)
            digit = digit || (value >= u'A' && value <= u'F');
        if (!digit)
            break;
        ++end;
    }
    if (end == start)
        return std::nullopt;
    auto character = fromCodePoint(js_input::parseInt(text.substr(start, end - start), radix));
    if (end < text.size() && text[end] == u' ')
        ++end;
    return std::pair{std::move(character), end};
}

std::shared_ptr<UnicodeCommandMap> unicodeMap(TexParser &parser) {
    auto map = std::dynamic_pointer_cast<UnicodeCommandMap>(
        parser.configuration.maps().getMap(u"unicode"));
    if (!map)
        throw std::logic_error("Unicode requires its configured command map");
    return map;
}

} // namespace

DispatchOutcome UnicodeMethods::Unicode(TexParser &parser, MacroInvocation const &invocation) {
    const auto header = parser.GetBrackets(invocation.dispatch_token);
    std::u16string font;
    if (header && !header->empty()) {
        auto compact = *header;
        std::erase(compact, u' ');
        regexp_host::RegExp dimensions(
            u"^(\\d+(\\.\\d*)?|\\.\\d+),(\\d+(\\.\\d*)?|\\.\\d+)$");
        font = dimensions.exec(compact)
                   ? parser.GetBrackets(invocation.dispatch_token).value_or(u"")
                   : *header;
    }
    if (font.find(u';') != std::u16string::npos)
        error(parser, "BadFont", u"Font name for %1 can't contain semicolons");

    auto number = UnitUtil::instance().trimSpaces(
        parser.GetArgument(invocation.dispatch_token).value());
    if (number.starts_with(u"0x"))
        number.replace(0, 2, u"x");
    regexp_host::RegExp numeric(u"^(x[0-9A-Fa-f]+|[0-9]+)$");
    if (!numeric.exec(number))
        error(parser, "BadUnicode", u"Argument to %1 must be a number");

    auto text = entities::numeric(number);
    const auto code = static_cast<char32_t>(
        number.front() == u'x'
            ? js_input::parseInt(std::u16string_view(number).substr(1),
                                 js_input::IntegerRadix::Hex)
            : js_input::parseInt(number));
    auto map = unicodeMap(parser);
    if (font.empty()) {
        if (auto found = map->fonts.find(code); found != map->fonts.end())
            font = found->second;
    }

    NodeDefinition definition;
    const auto &variant = parser.stack.env().font;
    if (!font.empty()) {
        map->fonts.insert_or_assign(code, font);
        definition.attributes.set(u"fontfamily", font);
        if (variant && !variant->empty()) {
            if (variant->find(u"bold") != std::u16string::npos)
                definition.attributes.set(u"fontweight", u"bold");
            if (variant->find(u"italic") != std::u16string::npos ||
                variant->find(u"-mathit") != std::u16string::npos)
                definition.attributes.set(u"fontstyle", u"italic");
        }
    } else if (variant && !variant->empty()) {
        definition.attributes.set(u"mathvariant", *variant);
    }

    auto &node = parser.createToken(mml::NodeKind::mtext, text, std::move(definition));
    node.properties().tex.unicode = true;
    parser.Push(&node);
    return NoParseResult{};
}

DispatchOutcome UnicodeMethods::RawUnicode(TexParser &parser,
                                           MacroInvocation const &invocation) {
    const auto source = parser.GetArgument(invocation.dispatch_token).value();
    const auto hex = trim(source);
    if (hex.empty() || hex.size() > 6 ||
        !std::ranges::all_of(hex, [](char16_t value) {
            return (value >= u'0' && value <= u'9') || (value >= u'A' && value <= u'F');
        }))
        error(parser, "BadRawUnicode",
              u"Argument to %1 must a hexadecimal number with 1 to 6 digits");

    const auto code = js_input::parseInt(hex, js_input::IntegerRadix::Hex);
    auto character = fromCodePoint(code);
    character += parser.string().substr(parser.position());
    parser.setString(std::move(character));
    parser.setPosition(0);
    return NoParseResult{};
}

DispatchOutcome UnicodeMethods::Char(TexParser &parser, MacroInvocation const &) {
    const auto next = parser.GetNext();
    const auto text = parser.string().substr(parser.position());
    std::u16string character;
    std::size_t consumed = 0;
    std::optional<std::pair<std::u16string, std::size_t>> numeric;

    if (next == u"'") {
        numeric = numberChar(text, 1, 7, js_input::IntegerRadix::Octal);
    } else if (next == u"\"") {
        numeric = numberChar(text, 1, 6, js_input::IntegerRadix::Hex);
    } else if (next == u"`") {
        const auto code = utf16::codePointAt(text, 1);
        if (code) {
            const auto following = utf16::codePointAt(text, 2);
            if (*code == u'\\' && following && !whitespace(*following)) {
                parser.setPosition(parser.position() + 2);
                character = parser.GetCS();
                if (utf16::codePoints(character).size() > 1)
                    error(parser, "InvalidAlphanumeric",
                          u"Invalid alphanumeric constant for %1");
            } else if (!lineTerminator(*code)) {
                const auto width = *code > 0xffff ? 2u : 1u;
                character = text.substr(1, width);
                consumed = 1 + width;
            }
        }
    } else {
        numeric = numberChar(text, 0, 7, js_input::IntegerRadix::Decimal);
    }

    if (numeric) {
        character = std::move(numeric->first);
        consumed = numeric->second;
    }
    if (character.empty())
        error(parser, "MissingNumber", u"Missing numeric constant for %1");
    parser.setPosition(parser.position() + consumed);

    if (character.size() == 1 && character[0] >= u'0' && character[0] <= u'9')
        parser.Push(&parser.createToken(mml::NodeKind::mn, character));
    else if (std::ranges::any_of(character, [](char16_t value) {
                 return (value >= u'A' && value <= u'Z') || (value >= u'a' && value <= u'z');
             }))
        parser.Push(&parser.createToken(mml::NodeKind::mi, character));
    else
        Other(parser, character);
    return NoParseResult{};
}

ConfigurationRef UnicodeConfiguration::initializeModule(ConfigurationHandler &registry,
                                                         MapHandler &maps) {
    if (!std::dynamic_pointer_cast<UnicodeCommandMap>(maps.getMap(u"unicode")))
        maps.registerMap(std::make_shared<UnicodeCommandMap>());
    if (auto existing = registry.get(u"unicode"))
        return existing;
    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::macro, {u"unicode"}}};
    return Configuration::create(registry, u"unicode", std::move(definition));
}

} // namespace folia::math::tex
