// Primitive ECMAScript numeric string host operations used by pinned MathJax.
// This is a native host adaptation, not an implementation of MathJax algorithms.
export module folia.math.util.js_numeric_input;
import std;

export namespace folia::math::js_input {
enum class IntegerRadix { Automatic = 0, Octal = 8, Decimal = 10, Hex = 16 };

double number(std::u16string_view text);
double parseInt(std::u16string_view text,
                IntegerRadix radix = IntegerRadix::Automatic);
}

namespace {

static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559 &&
              std::numeric_limits<double>::digits == 53 &&
              std::numeric_limits<double>::max_exponent == 1024,
              "folia.math.util.js_numeric_input requires IEC 559 binary64 double");

bool is_space(char16_t c) {
    switch (c) {
    case 0x0009: case 0x000A: case 0x000B: case 0x000C: case 0x000D:
    case 0x0020: case 0x00A0: case 0x1680:
    case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004:
    case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009:
    case 0x200A: case 0x2028: case 0x2029: case 0x202F: case 0x205F:
    case 0x3000: case 0xFEFF:
        return true;
    default:
        return false;
    }
}

std::u16string_view trim(std::u16string_view text) {
    while (!text.empty() && is_space(text.front())) text.remove_prefix(1);
    while (!text.empty() && is_space(text.back())) text.remove_suffix(1);
    return text;
}

std::u16string_view trim_start(std::u16string_view text) {
    while (!text.empty() && is_space(text.front())) text.remove_prefix(1);
    return text;
}

int digit_value(char16_t c) {
    if (c >= u'0' && c <= u'9') return static_cast<int>(c - u'0');
    if (c >= u'a' && c <= u'z') return static_cast<int>(c - u'a') + 10;
    if (c >= u'A' && c <= u'Z') return static_cast<int>(c - u'A') + 10;
    return -1;
}

class BigUnsigned {
public:
    void multiply_add(unsigned radix, unsigned digit) {
        std::uint64_t carry = digit;
        for (auto& limb : limbs_) {
            const auto value = static_cast<std::uint64_t>(limb) * radix + carry;
            limb = static_cast<std::uint32_t>(value);
            carry = value >> 32;
        }
        if (carry != 0) limbs_.push_back(static_cast<std::uint32_t>(carry));
        else if (limbs_.empty() && digit != 0) limbs_.push_back(digit);
    }

    [[nodiscard]] bool empty() const { return limbs_.empty(); }

    [[nodiscard]] std::size_t bit_length() const {
        if (limbs_.empty()) return 0;
        return (limbs_.size() - 1) * 32 +
            (32 - std::countl_zero(limbs_.back()));
    }

    [[nodiscard]] bool bit(std::size_t index) const {
        const auto word = index / 32;
        return word < limbs_.size() && ((limbs_[word] >> (index % 32)) & 1U) != 0;
    }

    [[nodiscard]] bool any_low_bits(std::size_t count) const {
        const auto full_words = count / 32;
        for (std::size_t i = 0; i < std::min(full_words, limbs_.size()); ++i) {
            if (limbs_[i] != 0) return true;
        }
        const auto remaining = count % 32;
        if (full_words < limbs_.size() && remaining != 0) {
            const auto mask = (std::uint32_t{1} << remaining) - 1U;
            if ((limbs_[full_words] & mask) != 0) return true;
        }
        return false;
    }

    [[nodiscard]] std::uint64_t shifted_low(std::size_t shift, unsigned count) const {
        std::uint64_t result = 0;
        for (unsigned i = 0; i < count; ++i) {
            if (bit(shift + i)) result |= std::uint64_t{1} << i;
        }
        return result;
    }

    [[nodiscard]] std::uint64_t to_uint64() const {
        std::uint64_t result = limbs_.empty() ? 0 : limbs_[0];
        if (limbs_.size() > 1) result |= static_cast<std::uint64_t>(limbs_[1]) << 32;
        return result;
    }

    [[nodiscard]] double to_double() const {
        const auto length = bit_length();
        if (length == 0) return 0.0;

        std::size_t shift = length > 53 ? length - 53 : 0;
        std::uint64_t significand = length > 53
            ? shifted_low(shift, 53)
            : to_uint64() << (53 - static_cast<unsigned>(length));
        if (length > 53) {
            const bool guard = bit(shift - 1);
            const bool sticky = any_low_bits(shift - 1);
            if (guard && (sticky || (significand & 1U) != 0)) {
                ++significand;
                if (significand == (std::uint64_t{1} << 53)) {
                    significand >>= 1;
                    ++shift;
                }
            }
        }

        const auto exponent = length > 53
            ? static_cast<std::int64_t>(shift) + 52
            : static_cast<std::int64_t>(length) - 1;
        if (exponent > 1023) return std::numeric_limits<double>::infinity();
        const auto exponent_field = static_cast<std::uint64_t>(exponent + 1023);
        const auto fraction = significand - (std::uint64_t{1} << 52);
        return std::bit_cast<double>((exponent_field << 52) | fraction);
    }

private:
    std::vector<std::uint32_t> limbs_;
};

double parse_unsigned_integer(std::u16string_view text, unsigned radix) {
    BigUnsigned value;
    for (const auto c : text) {
        const int digit = digit_value(c);
        if (digit < 0 || static_cast<unsigned>(digit) >= radix)
            return std::numeric_limits<double>::quiet_NaN();
        value.multiply_add(radix, static_cast<unsigned>(digit));
    }
    return value.to_double();
}

bool decimal_grammar(std::u16string_view text, std::size_t& first_nonzero,
                     std::size_t& digits_before_dot, bool& exponent_negative,
                     std::size_t& exponent_magnitude) {
    std::size_t i = 0;
    if (i < text.size() && (text[i] == u'+' || text[i] == u'-')) ++i;
    digits_before_dot = 0;
    std::size_t digit_count = 0;
    first_nonzero = std::u16string_view::npos;
    while (i < text.size() && text[i] >= u'0' && text[i] <= u'9') {
        if (text[i] != u'0' && first_nonzero == std::u16string_view::npos)
            first_nonzero = digit_count;
        ++digit_count;
        ++digits_before_dot;
        ++i;
    }
    if (i < text.size() && text[i] == u'.') {
        ++i;
        while (i < text.size() && text[i] >= u'0' && text[i] <= u'9') {
            if (text[i] != u'0' && first_nonzero == std::u16string_view::npos)
                first_nonzero = digit_count;
            ++digit_count;
            ++i;
        }
    }
    if (digit_count == 0) return false;

    exponent_negative = false;
    exponent_magnitude = 0;
    if (i < text.size() && (text[i] == u'e' || text[i] == u'E')) {
        ++i;
        if (i < text.size() && (text[i] == u'+' || text[i] == u'-')) {
            exponent_negative = text[i] == u'-';
            ++i;
        }
        const auto exponent_start = i;
        constexpr auto margin = std::size_t{4096};
        constexpr auto size_limit = std::numeric_limits<std::size_t>::max();
        const auto cap = text.size() > size_limit - margin
            ? size_limit
            : text.size() + margin;
        while (i < text.size() && text[i] >= u'0' && text[i] <= u'9') {
            const auto digit = static_cast<std::size_t>(text[i] - u'0');
            if (exponent_magnitude > (cap - digit) / 10) {
                exponent_magnitude = cap;
            } else {
                exponent_magnitude = exponent_magnitude * 10 + digit;
            }
            ++i;
        }
        if (i == exponent_start) return false;
    }
    if (i != text.size()) return false;
    return true;
}

bool decimal_magnitude_is_negative(bool exponent_negative,
                                   std::size_t exponent_magnitude,
                                   std::size_t digits_before_dot,
                                   std::size_t first_nonzero) {
    bool offset_negative = false;
    std::size_t offset_magnitude = 0;
    if (digits_before_dot > first_nonzero) {
        const auto difference = digits_before_dot - first_nonzero;
        if (difference > 1) offset_magnitude = difference - 1;
    } else {
        offset_negative = true;
        const auto difference = first_nonzero - digits_before_dot;
        offset_magnitude = difference == std::numeric_limits<std::size_t>::max()
            ? difference
            : difference + 1;
    }

    if (exponent_magnitude == 0) return offset_negative && offset_magnitude != 0;
    if (offset_magnitude == 0) return exponent_negative;
    if (exponent_negative == offset_negative) return exponent_negative;
    if (exponent_magnitude == offset_magnitude) return false;
    return exponent_magnitude > offset_magnitude ? exponent_negative : offset_negative;
}

}

namespace folia::math::js_input {

double number(std::u16string_view input) {
    auto text = trim(input);
    if (text.empty()) return 0.0;

    bool negative = false;
    bool had_sign = false;
    if (text.front() == u'+' || text.front() == u'-') {
        had_sign = true;
        negative = text.front() == u'-';
    }
    auto unsigned_text = text;
    if (had_sign) unsigned_text.remove_prefix(1);
    if (unsigned_text.empty()) return std::numeric_limits<double>::quiet_NaN();
    if (unsigned_text == u"Infinity") {
        const auto infinity = std::numeric_limits<double>::infinity();
        return negative ? -infinity : infinity;
    }

    if (!had_sign && !text.empty() && text.front() == u'0' && text.size() >= 2) {
        unsigned radix = 0;
        if (text[1] == u'x' || text[1] == u'X') radix = 16;
        else if (text[1] == u'o' || text[1] == u'O') radix = 8;
        else if (text[1] == u'b' || text[1] == u'B') radix = 2;
        if (radix != 0) {
            if (text.size() == 2) return std::numeric_limits<double>::quiet_NaN();
            return parse_unsigned_integer(text.substr(2), radix);
        }
    }

    std::size_t first_nonzero = 0;
    std::size_t digits_before_dot = 0;
    bool exponent_negative = false;
    std::size_t exponent_magnitude = 0;
    if (!decimal_grammar(text, first_nonzero, digits_before_dot,
                         exponent_negative, exponent_magnitude))
        return std::numeric_limits<double>::quiet_NaN();
    if (first_nonzero == std::u16string_view::npos)
        return std::copysign(0.0, negative ? -1.0 : 1.0);

    const bool sign = negative;
    if (text.front() == u'+') text.remove_prefix(1);
    std::string ascii;
    ascii.reserve(text.size());
    for (const auto c : text) {
        if (c > 0x7f) return std::numeric_limits<double>::quiet_NaN();
        ascii.push_back(static_cast<char>(c));
    }
    double result = 0.0;
    const auto converted = std::from_chars(ascii.data(), ascii.data() + ascii.size(),
                                           result, std::chars_format::general);
    if (converted.ec == std::errc{} && converted.ptr == ascii.data() + ascii.size())
        return result;
    if (converted.ec == std::errc::result_out_of_range &&
        converted.ptr == ascii.data() + ascii.size()) {
        if (decimal_magnitude_is_negative(exponent_negative, exponent_magnitude,
                                          digits_before_dot, first_nonzero))
            return std::copysign(0.0, sign ? -1.0 : 1.0);
        return std::copysign(std::numeric_limits<double>::infinity(), sign ? -1.0 : 1.0);
    }
    return std::numeric_limits<double>::quiet_NaN();
}

double parseInt(std::u16string_view input, IntegerRadix requested) {
    unsigned radix = 0;
    switch (requested) {
    case IntegerRadix::Automatic: radix = 0; break;
    case IntegerRadix::Octal: radix = 8; break;
    case IntegerRadix::Decimal: radix = 10; break;
    case IntegerRadix::Hex: radix = 16; break;
    default: throw std::invalid_argument("unsupported parseInt radix");
    }

    auto text = trim_start(input);
    bool negative = false;
    if (!text.empty() && (text.front() == u'+' || text.front() == u'-')) {
        negative = text.front() == u'-';
        text.remove_prefix(1);
    }
    if (radix == 0) {
        radix = 10;
        if (text.size() >= 2 && text[0] == u'0' &&
            (text[1] == u'x' || text[1] == u'X')) {
            radix = 16;
            text.remove_prefix(2);
        }
    } else if (radix == 16 && text.size() >= 2 && text[0] == u'0' &&
               (text[1] == u'x' || text[1] == u'X')) {
        text.remove_prefix(2);
    }

    std::size_t digits = 0;
    BigUnsigned value;
    for (const auto c : text) {
        const int digit = digit_value(c);
        if (digit < 0 || static_cast<unsigned>(digit) >= radix) break;
        value.multiply_add(radix, static_cast<unsigned>(digit));
        ++digits;
    }
    if (digits == 0) return std::numeric_limits<double>::quiet_NaN();
    const double result = value.to_double();
    return negative ? std::copysign(result, -1.0) : result;
}

}
