module folia.math.util.lengths;
import std;
import folia.math.util.constants;

namespace folia::math::lengths_detail {
struct PrefixMatch {
    bool matched = false;
    bool has_number = false;
    std::string_view number;
    std::string_view unit;
};

std::pair<char32_t, std::size_t> codepoint_at(std::string_view text, std::size_t i) {
    const auto first = static_cast<unsigned char>(text[i]);
    if (first < 0x80) return {first, 1};

    const auto continuation = [&](std::size_t offset) {
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
    if (first >= 0xF0 && first <= 0xF4 && continuation(1) &&
        continuation(2) && continuation(3)) {
        const auto second = static_cast<unsigned char>(text[i + 1]);
        const auto third = static_cast<unsigned char>(text[i + 2]);
        const auto fourth = static_cast<unsigned char>(text[i + 3]);
        if ((first != 0xF0 || second >= 0x90) && (first != 0xF4 || second < 0x90)) {
            return {static_cast<char32_t>(((first & 0x07) << 18) | ((second & 0x3F) << 12) |
                                          ((third & 0x3F) << 6) | (fourth & 0x3F)), 4};
        }
    }
    return {first, 1};
}

bool space(char32_t value) {
    return (value >= 0x0009 && value <= 0x000D) || value == 0x0020 || value == 0x00A0 ||
           value == 0x1680 || (value >= 0x2000 && value <= 0x200A) || value == 0x2028 ||
           value == 0x2029 || value == 0x202F || value == 0x205F || value == 0x3000 ||
           value == 0xFEFF;
}

bool digit(char value) { return value >= '0' && value <= '9'; }

PrefixMatch match(std::string_view text) {
    PrefixMatch result;
    std::size_t i = 0;
    while (i < text.size()) {
        const auto [value, count] = codepoint_at(text, i);
        if (!space(value)) break;
        i += count;
    }

    const auto after_space = i;
    const auto number_begin = i;
    if (i < text.size() && (text[i] == '+' || text[i] == '-')) ++i;
    const auto integer_begin = i;
    while (i < text.size() && digit(text[i])) ++i;
    const bool has_integer = i != integer_begin;
    bool has_fraction = false;
    if (i < text.size() && text[i] == '.') {
        const auto fraction_begin = i + 1;
        auto fraction_end = fraction_begin;
        while (fraction_end < text.size() && digit(text[fraction_end])) ++fraction_end;
        if (fraction_end != fraction_begin) {
            i = fraction_end;
            has_fraction = true;
        } else if (has_integer) {
            ++i;
        }
    }
    if (has_integer || has_fraction) {
        result.has_number = true;
        result.number = text.substr(number_begin, i - number_begin);
    } else {
        i = after_space;
    }

    constexpr std::string_view units[] = {
        "pt", "em", "ex", "mu", "px", "pc", "in", "mm", "cm", "%"
    };
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

double decimal(std::string_view text) {
    if (text.empty()) return 1;
    if (text.front() == '+') text.remove_prefix(1);

    std::string normalized(text);
    if (!normalized.empty() && normalized.back() == '.') normalized.push_back('0');
    double value = 0;
    const auto [end, error] = std::from_chars(normalized.data(),
                                               normalized.data() + normalized.size(), value,
                                               std::chars_format::fixed);
    if (error == std::errc{} && end == normalized.data() + normalized.size()) return value;
    if (error != std::errc::result_out_of_range) return value;

    const bool negative = !text.empty() && text.front() == '-';
    const auto magnitude = negative ? text.substr(1) : text;
    const auto dot = magnitude.find('.');
    const auto integer_digits = dot == std::string_view::npos ? magnitude.size() : dot;
    std::size_t digit_index = 0;
    auto first_nonzero = std::string_view::npos;
    for (const auto character : magnitude) {
        if (character == '.') continue;
        if (first_nonzero == std::string_view::npos && character != '0')
            first_nonzero = digit_index;
        ++digit_index;
    }
    const auto sign = negative ? -1.0 : 1.0;
    if (first_nonzero == std::string_view::npos) return std::copysign(0.0, sign);

    const auto decimal_order = static_cast<std::int64_t>(integer_digits) -
                               static_cast<std::int64_t>(first_nonzero) - 1;
    const auto limit = decimal_order >= 0 ? std::numeric_limits<double>::infinity() : 0.0;
    return std::copysign(limit, sign);
}
class BigUnsigned {
public:
    explicit BigUnsigned(std::uint64_t value) {
        if (value == 0) return;
        limbs_.push_back(static_cast<std::uint32_t>(value));
        const auto high = static_cast<std::uint32_t>(value >> 32);
        if (high != 0) limbs_.push_back(high);
    }

    void multiply_small(std::uint32_t factor) {
        std::uint64_t carry = 0;
        for (auto& limb : limbs_) {
            const auto product = static_cast<std::uint64_t>(limb) * factor + carry;
            limb = static_cast<std::uint32_t>(product);
            carry = product >> 32;
        }
        if (carry != 0) limbs_.push_back(static_cast<std::uint32_t>(carry));
    }

    void shift_left(unsigned count) {
        if (limbs_.empty() || count == 0) return;
        const auto words = count / 32;
        const auto bits = count % 32;
        std::vector<std::uint32_t> shifted(words, 0);
        shifted.reserve(words + limbs_.size() + 1);
        std::uint64_t carry = 0;
        for (const auto limb : limbs_) {
            const auto value = (static_cast<std::uint64_t>(limb) << bits) | carry;
            shifted.push_back(static_cast<std::uint32_t>(value));
            carry = value >> 32;
        }
        if (carry != 0) shifted.push_back(static_cast<std::uint32_t>(carry));
        limbs_ = std::move(shifted);
    }

    [[nodiscard]] bool bit(unsigned index) const {
        const auto word = static_cast<std::size_t>(index / 32);
        return word < limbs_.size() && ((limbs_[word] >> (index % 32)) & 1U) != 0;
    }

    void shift_right(unsigned count) {
        if (limbs_.empty() || count == 0) return;
        const auto words = static_cast<std::size_t>(count / 32);
        const auto bits = count % 32;
        if (words >= limbs_.size()) {
            limbs_.clear();
            return;
        }
        std::vector<std::uint32_t> shifted;
        shifted.reserve(limbs_.size() - words);
        for (auto index = words; index < limbs_.size(); ++index) {
            std::uint64_t value = limbs_[index];
            if (bits != 0 && index + 1 < limbs_.size())
                value |= static_cast<std::uint64_t>(limbs_[index + 1]) << 32;
            shifted.push_back(static_cast<std::uint32_t>(value >> bits));
        }
        limbs_ = std::move(shifted);
        normalize();
    }

    void increment() {
        std::uint64_t carry = 1;
        for (auto& limb : limbs_) {
            const auto value = static_cast<std::uint64_t>(limb) + carry;
            limb = static_cast<std::uint32_t>(value);
            carry = value >> 32;
            if (carry == 0) return;
        }
        if (carry != 0) limbs_.push_back(1);
    }

    [[nodiscard]] std::string decimal() const {
        if (limbs_.empty()) return "0";
        auto work = limbs_;
        std::vector<std::uint32_t> chunks;
        while (!work.empty()) {
            std::uint64_t remainder = 0;
            for (auto index = work.size(); index-- > 0;) {
                const auto value = (remainder << 32) | work[index];
                work[index] = static_cast<std::uint32_t>(value / 1'000'000'000ULL);
                remainder = value % 1'000'000'000ULL;
            }
            chunks.push_back(static_cast<std::uint32_t>(remainder));
            while (!work.empty() && work.back() == 0) work.pop_back();
        }
        std::string result = std::to_string(chunks.back());
        for (auto index = chunks.size() - 1; index-- > 0;) {
            const auto chunk = std::to_string(chunks[index]);
            result.append(9 - chunk.size(), '0');
            result += chunk;
        }
        return result;
    }

private:
    void normalize() {
        while (!limbs_.empty() && limbs_.back() == 0) limbs_.pop_back();
    }

    std::vector<std::uint32_t> limbs_;
};

std::string shortest(double value) {
    if (std::isnan(value)) return "NaN";
    if (std::isinf(value)) return std::signbit(value) ? "-Infinity" : "Infinity";
    if (value == 0) return "0";

    const bool negative = std::signbit(value);
    std::array<char, 64> buffer{};
    const auto converted = std::to_chars(buffer.data(), buffer.data() + buffer.size(),
                                         std::abs(value), std::chars_format::general);
    if (converted.ec != std::errc{}) return "0";
    const std::string_view raw(buffer.data(), static_cast<std::size_t>(converted.ptr - buffer.data()));
    const auto exponent_at = raw.find_first_of("eE");
    const auto mantissa = raw.substr(0, exponent_at);
    int explicit_exponent = 0;
    if (exponent_at != std::string_view::npos) {
        auto exponent_text = raw.substr(exponent_at + 1);
        if (!exponent_text.empty() && exponent_text.front() == '+') exponent_text.remove_prefix(1);
        const auto parsed = std::from_chars(exponent_text.data(),
                                            exponent_text.data() + exponent_text.size(),
                                            explicit_exponent);
        if (parsed.ec != std::errc{}) return "0";
    }
    const auto decimal_at = mantissa.find('.');
    int decimal_position = static_cast<int>(decimal_at == std::string_view::npos
                                                ? mantissa.size() : decimal_at) + explicit_exponent;
    std::string digits;
    for (const auto character : mantissa)
        if (character != '.') digits.push_back(character);
    const auto first_nonzero = digits.find_first_not_of('0');
    if (first_nonzero == std::string::npos) return "0";
    decimal_position -= static_cast<int>(first_nonzero);
    digits.erase(0, first_nonzero);
    while (digits.size() > 1 && digits.back() == '0') digits.pop_back();

    const int scientific_exponent = decimal_position - 1;
    std::string result;
    if (negative) result.push_back('-');
    if (scientific_exponent >= -6 && scientific_exponent < 21) {
        if (decimal_position <= 0) {
            result += "0.";
            result.append(static_cast<std::size_t>(-decimal_position), '0');
            result += digits;
        } else if (static_cast<std::size_t>(decimal_position) >= digits.size()) {
            result += digits;
            result.append(static_cast<std::size_t>(decimal_position) - digits.size(), '0');
        } else {
            result.append(digits.data(), static_cast<std::size_t>(decimal_position));
            result.push_back('.');
            result.append(digits.data() + decimal_position,
                          digits.size() - static_cast<std::size_t>(decimal_position));
        }
    } else {
        result.push_back(digits.front());
        if (digits.size() > 1) {
            result.push_back('.');
            result.append(digits.data() + 1, digits.size() - 1);
        }
        result.push_back('e');
        if (scientific_exponent >= 0) result.push_back('+');
        result += std::to_string(scientific_exponent);
    }
    return result;
}

std::string fixed(double value, unsigned precision) {
    if (precision > 100) throw std::out_of_range("decimal precision must be between 0 and 100");
    if (std::isnan(value)) return "NaN";
    if (std::isinf(value)) return std::signbit(value) ? "-Infinity" : "Infinity";
    if (value == 0) {
        return precision == 0 ? "0" : "0." + std::string(precision, '0');
    }
    if (std::abs(value) >= 1e21) return shortest(value);

    const auto bits = std::bit_cast<std::uint64_t>(std::abs(value));
    const auto exponent_field = static_cast<unsigned>((bits >> 52) & 0x7ffU);
    const auto fraction = bits & ((std::uint64_t{1} << 52) - 1);
    const auto significand = exponent_field == 0 ? fraction : fraction | (std::uint64_t{1} << 52);
    const auto binary_exponent = exponent_field == 0
                                     ? -1074 : static_cast<int>(exponent_field) - 1023 - 52;
    BigUnsigned integer(significand);
    for (unsigned index = 0; index < precision; ++index) integer.multiply_small(5);

    const auto shift = binary_exponent + static_cast<int>(precision);
    if (shift >= 0) {
        integer.shift_left(static_cast<unsigned>(shift));
    } else {
        const auto denominator_power = static_cast<unsigned>(-shift);
        const bool round_up = integer.bit(denominator_power - 1);
        integer.shift_right(denominator_power);
        if (round_up) integer.increment();
    }

    auto result = integer.decimal();
    if (precision != 0) {
        if (result.size() <= precision)
            result.insert(0, static_cast<std::size_t>(precision + 1) - result.size(), '0');
        result.insert(result.size() - precision, 1, '.');
    }
    if (std::signbit(value)) result.insert(result.begin(), '-');
    return result;
}
void trim(std::string& s) {
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
}
}
namespace folia::math::lengths {
namespace {
constexpr std::pair<std::string_view, double> named_spaces[] = {
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
    {"thin", .04},
    {"medium", .06},
    {"thick", .1},
    {"normal", 1},
    {"big", 2},
    {"small", 0.70710678118654752440},
    {"infinity", BIGDIMEN},
};

std::string number_string(double x) {
    return lengths_detail::shortest(x);
}
}
double length2em(std::string_view length, LengthContext const& context) {
    if (length.empty()) return context.size_em;
    for (const auto& [name, value] : named_spaces) {
        if (length == name) return value;
    }

    const auto match = lengths_detail::match(length);
    if (!match.matched) return context.size_em;
    const auto value = match.has_number ? lengths_detail::decimal(match.number) : 1.0;

    if (match.unit == "px")
        return value / context.em_pixels / context.scale;
    if (match.unit == "in")
        return value * context.pixels_per_inch / context.em_pixels / context.scale;
    if (match.unit == "cm")
        return value * (context.pixels_per_inch / 2.54) / context.em_pixels / context.scale;
    if (match.unit == "mm")
        return value * (context.pixels_per_inch / 25.4) / context.em_pixels / context.scale;
    if (match.unit == "em") return value;
    if (match.unit == "ex") return value * context.ex_em;
    if (match.unit == "mu") return value * context.mu_em;
    if (match.unit == "pt") return value * .1;
    if (match.unit == "pc") return value * 1.2;
    if (match.unit == "%") return (value / 100) * context.size_em;
    return value * context.size_em;
}

double length2em(double value, LengthContext const& context) {
    return length2em(number_string(value), context);
}

std::string format_decimal(double value, unsigned precision) {
    return lengths_detail::fixed(value, precision);
}

std::string percent(double value) {
    auto result = lengths_detail::fixed(100 * value, 1);
    lengths_detail::trim(result);
    return result + "%";
}

std::string em(double value) {
    if (std::abs(value) < .001) return "0";
    auto result = lengths_detail::fixed(value, 3);
    lengths_detail::trim(result);
    return result + "em";
}

std::string px(double value, double minimum_pixels, double em_pixels) {
    value *= em_pixels;
    if (minimum_pixels && value < minimum_pixels) value = minimum_pixels;
    if (std::abs(value) < .1) return "0";

    auto result = lengths_detail::fixed(value, 1);
    if (result.ends_with(".0")) result.resize(result.size() - 2);
    return result + "px";
}

} // namespace folia::math::lengths
