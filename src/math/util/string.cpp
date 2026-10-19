module folia.math.util.string;
import folia.math.util.utf16;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;

// Ported from MathJax 4.1.3 ts/util/string.ts (Apache-2.0).
// Copyright (c) 2017-2026 The MathJax Consortium; see LICENSE-MathJax.txt.
namespace folia::math::string_util {
namespace {

bool is_line_terminator(char16_t unit) {
    return unit == u'\n' || unit == u'\r' || unit == 0x2028 || unit == 0x2029;
}

int hex_value(char16_t unit) {
    if (unit >= u'0' && unit <= u'9')
        return static_cast<int>(unit - u'0');
    if (unit >= u'a' && unit <= u'f')
        return static_cast<int>(unit - u'a') + 10;
    if (unit >= u'A' && unit <= u'F')
        return static_cast<int>(unit - u'A') + 10;
    return -1;
}

bool is_hex(char16_t unit) {
    return hex_value(unit) >= 0;
}

std::u16string code_point_string(std::uint32_t point) {
    std::u16string result;
    if (point <= 0xFFFF) {
        result.push_back(static_cast<char16_t>(point));
    } else {
        point -= 0x10000;
        result.push_back(static_cast<char16_t>(0xD800 + (point >> 10)));
        result.push_back(static_cast<char16_t>(0xDC00 + (point & 0x3FF)));
    }
    return result;
}

[[noreturn]] void invalid_code_point(double point) {
    std::u16string message = u"Invalid code point ";
    if (std::isnan(point)) {
        message += u"NaN";
    } else if (std::isinf(point)) {
        message += std::signbit(point) ? u"-Infinity" : u"Infinity";
    } else {
        for (const auto byte : folia::math::js_number::toString(point))
            message.push_back(static_cast<char16_t>(byte));
    }
    throw std::range_error(folia::math::utf16::encode(message));
}

std::uint32_t parse_code_point(std::u16string_view digits) {
    const auto value =
        folia::math::js_input::parseInt(digits, folia::math::js_input::IntegerRadix::Hex);
    if (!std::isfinite(value) || value < 0 || value > 0x10FFFF || std::trunc(value) != value) {
        invalid_code_point(value);
    }
    return static_cast<std::uint32_t>(value);
}

} // namespace

double sortLength(std::u16string_view a, std::u16string_view b) {
    if (a.size() != b.size())
        return static_cast<double>(b.size()) - static_cast<double>(a.size());
    if (a == b)
        return 0;
    return a < b ? -1 : 1;
}

std::u16string quotePattern(std::u16string_view text) {
    constexpr std::u16string_view quoted = u"^$(){}.+*?-|[]:\\";
    std::u16string result;
    result.reserve(text.size() * 2);
    for (const auto unit : text) {
        if (quoted.find(unit) != std::u16string_view::npos)
            result.push_back(u'\\');
        result.push_back(unit);
    }
    return result;
}

std::vector<double> unicodeChars(std::u16string_view text) {
    const auto points = folia::math::utf16::codePoints(text);
    std::vector<double> result;
    result.reserve(points.size());
    for (const auto point : points)
        result.push_back(static_cast<double>(point));
    return result;
}

std::u16string unicodeString(std::span<const double> data) {
    std::u16string result;
    for (const auto point : data) {
        if (!std::isfinite(point) || point < 0 || point > 0x10FFFF || std::trunc(point) != point) {
            invalid_code_point(point);
        }
        result += code_point_string(static_cast<std::uint32_t>(point));
    }
    return result;
}

bool isPercent(std::u16string_view text) {
    while (!text.empty() && folia::math::js_input::isWhitespace(text.back()))
        text.remove_suffix(1);
    return !text.empty() && text.back() == u'%';
}

std::vector<std::u16string> split(std::u16string_view text) {
    while (!text.empty() && folia::math::js_input::isWhitespace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && folia::math::js_input::isWhitespace(text.back()))
        text.remove_suffix(1);
    if (text.empty())
        return {std::u16string{}};

    std::vector<std::u16string> result;
    std::size_t cursor = 0;
    while (cursor < text.size()) {
        while (cursor < text.size() && folia::math::js_input::isWhitespace(text[cursor]))
            ++cursor;
        if (cursor == text.size())
            break;
        const auto start = cursor;
        while (cursor < text.size() && !folia::math::js_input::isWhitespace(text[cursor]))
            ++cursor;
        result.emplace_back(text.substr(start, cursor - start));
    }
    return result;
}

std::u16string replaceUnicode(std::u16string_view text) {
    std::u16string result;
    result.reserve(text.size());
    for (std::size_t i = 0; i < text.size();) {
        if (text[i] != u'\\' || i + 1 >= text.size() || is_line_terminator(text[i + 1])) {
            result.push_back(text[i++]);
            continue;
        }

        if (text[i + 1] == u'\\') {
            result.push_back(u'\\');
            i += 2;
            continue;
        }

        // First regex alternative: \U plus exactly four hex units, or a
        // braced one-to-six-digit form with ECMAScript whitespace.
        if (text[i + 1] == u'U') {
            const auto after_u = i + 2;
            if (text.size() - after_u >= 4 && is_hex(text[after_u]) && is_hex(text[after_u + 1]) &&
                is_hex(text[after_u + 2]) && is_hex(text[after_u + 3])) {
                const auto digits = text.substr(after_u, 4);
                result += code_point_string(parse_code_point(digits));
                i = after_u + 4;
                continue;
            }
            if (after_u < text.size() && text[after_u] == u'{') {
                auto cursor = after_u + 1;
                while (cursor < text.size() && folia::math::js_input::isWhitespace(text[cursor]))
                    ++cursor;
                const auto digits_start = cursor;
                while (cursor < text.size() && cursor - digits_start < 6 && is_hex(text[cursor]))
                    ++cursor;
                const auto digits_end = cursor;
                while (cursor < text.size() && folia::math::js_input::isWhitespace(text[cursor]))
                    ++cursor;
                if (digits_end != digits_start && cursor < text.size() && text[cursor] == u'}') {
                    result += code_point_string(
                        parse_code_point(text.substr(digits_start, digits_end - digits_start)));
                    i = cursor + 1;
                    continue;
                }
            }
        }

        // Fallback regex alternative \.: consumes one UTF-16 unit. Only \\\\ is
        // accepted by the callback; other escapes convert NaN via fromCodePoint.
        invalid_code_point(std::numeric_limits<double>::quiet_NaN());
    }
    return result;
}

std::u16string toEntity(std::u16string_view text) {
    const auto point = folia::math::utf16::codePointAt(text, 0);
    if (!point) {
        throw std::invalid_argument("Cannot read properties of undefined (reading 'toString')");
    }
    constexpr char16_t digits[] = u"0123456789ABCDEF";
    auto value = static_cast<std::uint32_t>(*point);
    std::u16string hexadecimal;
    do {
        hexadecimal.push_back(digits[value & 0xF]);
        value >>= 4;
    } while (value != 0);
    std::reverse(hexadecimal.begin(), hexadecimal.end());
    return std::u16string(u"&#x") + hexadecimal + u";";
}

} // namespace folia::math::string_util
