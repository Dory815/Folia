/*************************************************************
 *
 * Copyright (c) 2017-2026 The MathJax Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Native source port of MathJax 4.1.3 ts/util/lengths.ts.
 * Source SHA256: 213f65e9a8fa6d30bda4df4a6c0cc3a1e8d50567267539d85a1f19c1c20cea73
 *************************************************************/
export module folia.math.util.lengths;
import std;
import folia.math.util.constants;
import folia.math.core.tree.property;
import folia.math.util.js_number;

namespace folia::math::lengths_detail {

struct PrefixMatch {
    bool matched = false;
    bool has_number = false;
    std::string_view number;
    std::string_view unit;
};

// Decode one UTF-8 scalar for ECMAScript's regexp \s character set. Invalid
// input bytes are treated as ordinary non-whitespace characters.
std::pair<char32_t, std::size_t> codepoint_at(std::string_view text, std::size_t i) {
    const auto first = static_cast<unsigned char>(text[i]);
    if (first < 0x80) return {first, 1};
    auto continuation = [&](std::size_t offset) {
        return i + offset < text.size() &&
               (static_cast<unsigned char>(text[i + offset]) & 0xC0) == 0x80;
    };
    if (first >= 0xC2 && first <= 0xDF && continuation(1)) {
        return {static_cast<char32_t>(((first & 0x1F) << 6) |
                                      (static_cast<unsigned char>(text[i + 1]) & 0x3F)), 2};
    }
    if (first >= 0xE0 && first <= 0xEF && continuation(1) && continuation(2)) {
        const auto second = static_cast<unsigned char>(text[i + 1]);
        const auto third = static_cast<unsigned char>(text[i + 2]);
        if ((first != 0xE0 || second >= 0xA0) && (first != 0xED || second < 0xA0)) {
            return {static_cast<char32_t>(((first & 0x0F) << 12) |
                                          ((second & 0x3F) << 6) | (third & 0x3F)), 3};
        }
    }
    if (first >= 0xF0 && first <= 0xF4 && continuation(1) && continuation(2) && continuation(3)) {
        const auto second = static_cast<unsigned char>(text[i + 1]);
        const auto third = static_cast<unsigned char>(text[i + 2]);
        const auto fourth = static_cast<unsigned char>(text[i + 3]);
        if ((first != 0xF0 || second >= 0x90) && (first != 0xF4 || second < 0x90)) {
            return {static_cast<char32_t>(((first & 0x07) << 18) |
                                          ((second & 0x3F) << 12) |
                                          ((third & 0x3F) << 6) | (fourth & 0x3F)), 4};
        }
    }
    return {first, 1};
}

bool ecmascript_space(char32_t c) {
    return (c >= 0x0009 && c <= 0x000D) || c == 0x0020 || c == 0x00A0 ||
           c == 0x1680 || (c >= 0x2000 && c <= 0x200A) || c == 0x2028 ||
           c == 0x2029 || c == 0x202F || c == 0x205F || c == 0x3000 ||
           c == 0xFEFF;
}

bool digit(char c) { return c >= '0' && c <= '9'; }

PrefixMatch match_length_prefix(std::string_view text) {
    PrefixMatch result;
    std::size_t i = 0;
    while (i < text.size()) {
        const auto [cp, count] = codepoint_at(text, i);
        if (!ecmascript_space(cp)) break;
        i += count;
    }
    const std::size_t after_space = i;

    std::size_t number_begin = i;
    if (i < text.size() && (text[i] == '+' || text[i] == '-')) ++i;
    const std::size_t integer_begin = i;
    while (i < text.size() && digit(text[i])) ++i;
    const bool has_integer = i != integer_begin;
    bool has_fraction = false;
    if (i < text.size() && text[i] == '.') {
        const std::size_t fraction_begin = i + 1;
        std::size_t fraction_end = fraction_begin;
        while (fraction_end < text.size() && digit(text[fraction_end])) ++fraction_end;
        if (fraction_end != fraction_begin) {
            i = fraction_end;
            has_fraction = true;
        } else if (has_integer) {
            ++i; // The regexp accepts digits followed by a bare decimal point.
        }
    }
    if (has_integer || has_fraction) {
        result.has_number = true;
        result.number = text.substr(number_begin, i - number_begin);
    } else {
        i = after_space;
    }

    constexpr std::string_view units[] = {"pt", "em", "ex", "mu", "px", "pc", "in", "mm", "cm", "%"};
    for (const auto unit : units) {
        if (text.substr(i).starts_with(unit)) {
            result.unit = unit;
            i += unit.size();
            break;
        }
    }
    result.matched = i != 0;
    return result;
}

// parseFloat's input is already restricted by the source regexp. from_chars
// gives locale-independent correctly-rounded conversion for that decimal.
double parse_decimal(std::string_view number) {
    if (number.empty()) return 1;
    bool positive_sign = number.front() == '+';
    if (positive_sign) number.remove_prefix(1);
    std::string normalized(number);
    if (!normalized.empty() && normalized.back() == '.') normalized.push_back('0');
    double value = 0;
    const auto [end, error] = std::from_chars(normalized.data(), normalized.data() + normalized.size(),
                                               value, std::chars_format::fixed);
    if (error == std::errc{} && end == normalized.data() + normalized.size()) return value;
    if (error != std::errc::result_out_of_range) return value;

    // The only out-of-range cases for this no-exponent grammar are overflow
    // and underflow. Determine the decimal order without rounding arithmetic.
    const bool negative = !positive_sign && !number.empty() && number.front() == '-';
    const std::string_view magnitude = negative ? number.substr(1) : number;
    const std::size_t dot = magnitude.find('.');
    const std::size_t integer_digits = dot == std::string_view::npos ? magnitude.size() : dot;
    std::size_t digit_index = 0;
    std::size_t first_nonzero = std::string_view::npos;
    for (char c : magnitude) {
        if (c == '.') continue;
        if (first_nonzero == std::string_view::npos && c != '0') first_nonzero = digit_index;
        ++digit_index;
    }
    const auto sign = negative ? -1.0 : 1.0;
    if (first_nonzero == std::string_view::npos) return std::copysign(0.0, sign);
    const auto decimal_order = static_cast<std::int64_t>(integer_digits) -
                               static_cast<std::int64_t>(first_nonzero) - 1;
    return decimal_order >= 0 ? std::copysign(std::numeric_limits<double>::infinity(), sign)
                              : std::copysign(0.0, sign);
}

void trim_decimal_zeros(std::string& value) {
    while (!value.empty() && value.back() == '0') value.pop_back();
    if (!value.empty() && value.back() == '.') value.pop_back();
}

} // namespace folia::math::lengths_detail

export namespace folia::math::lengths {

inline const std::array<std::pair<std::string_view, double>, 4> UNITS{{
    {"px", 1}, {"in", 96}, {"cm", 96 / 2.54}, {"mm", 96 / 25.4}
}};

inline const std::array<std::pair<std::string_view, double>, 5> RELUNITS{{
    {"em", 1}, {"ex", .431}, {"pt", 1.0 / 10}, {"pc", 12.0 / 10}, {"mu", 1.0 / 18}
}};

inline const std::array<std::pair<std::string_view, double>, 21> MATHSPACE{{
    {"veryverythinmathspace", 1.0 / 18},
    {"verythinmathspace", 2.0 / 18},
    {"thinmathspace", 3.0 / 18},
    {"mediummathspace", 4.0 / 18},
    {"thickmathspace", 5.0 / 18},
    {"verythickmathspace", 6.0 / 18},
    {"veryverythickmathspace", 7.0 / 18},
    {"negativeveryverythinmathspace", -1.0 / 18},
    {"negativeverythinmathspace", -2.0 / 18},
    {"negativethinmathspace", -3.0 / 18},
    {"negativemediummathspace", -4.0 / 18},
    {"negativethickmathspace", -5.0 / 18},
    {"negativeverythickmathspace", -6.0 / 18},
    {"negativeveryverythickmathspace", -7.0 / 18},
    {"thin", .04}, {"medium", .06}, {"thick", .1},
    {"normal", 1}, {"big", 2}, {"small", 1 / std::sqrt(2.0)},
    {"infinity", BIGDIMEN}
}};

Property length2em(std::string_view length, double size = 0, double scale = 1, double em = 16) {
    if (length.empty()) return Property(size);

    for (const auto& [name, value] : MATHSPACE) {
        if (length == name) return Property(value);
    }
    // A plain JS object inherits Object.prototype; those properties are
    // truthy and are returned verbatim before any regex parsing occurs.
    const Property inherited = PropertyList{}.get(length);
    if (!inherited.missing()) return inherited;

    const auto match = lengths_detail::match_length_prefix(length);
    if (!match.matched) return Property(size);
    const double m = match.has_number ? lengths_detail::parse_decimal(match.number) : 1;
    for (const auto& [unit, factor] : UNITS) {
        if (match.unit == unit) return Property((m * factor) / em / scale);
    }
    for (const auto& [unit, factor] : RELUNITS) {
        if (match.unit == unit) return Property(m * factor);
    }
    if (match.unit == "%") return Property((m / 100) * size);
    return Property(m * size);
}

Property length2em(double length, double size = 0, double scale = 1, double em = 16) {
    return length2em(js_number::toString(length), size, scale, em);
}

std::string percent(double m) {
    std::string value = js_number::toFixed(100 * m, 1);
    lengths_detail::trim_decimal_zeros(value);
    return value + "%";
}

std::string em(double m) {
    if (std::abs(m) < 0.001) return "0";
    std::string value = js_number::toFixed(m, 3);
    lengths_detail::trim_decimal_zeros(value);
    return value + "em";
}

std::string px(double m, double M = -BIGDIMEN, double em_size = 16) {
    m *= em_size;
    if (M && m < M) m = M;
    if (std::abs(m) < 0.1) return "0";
    std::string value = js_number::toFixed(m, 1);
    if (value.size() >= 2 && value.ends_with(".0")) value.resize(value.size() - 2);
    return value + "px";
}

} // namespace folia::math::lengths
