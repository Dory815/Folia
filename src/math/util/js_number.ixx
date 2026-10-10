// Host adaptation of ECMAScript Number string conversion and toFixed.
// The formatting rules follow ECMAScript; this module does not embed a JS runtime.
export module folia.math.util.js_number;
import std;

namespace {

static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559 &&
              std::numeric_limits<double>::digits == 53 &&
              std::numeric_limits<double>::max_exponent == 1024,
              "folia.math.util.js_number requires IEC 559 binary64 double");

class BigUnsigned {
public:
    explicit BigUnsigned(std::uint64_t value = 0) {
        if (value != 0) {
            limbs_.push_back(static_cast<std::uint32_t>(value));
            const auto high = static_cast<std::uint32_t>(value >> 32);
            if (high != 0) limbs_.push_back(high);
        }
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
            if (bits != 0 && index + 1 < limbs_.size()) {
                value |= static_cast<std::uint64_t>(limbs_[index + 1]) << 32;
            }
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
            auto chunk = std::to_string(chunks[index]);
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

std::string format_shortest(double value) {
    if (std::isnan(value)) return "NaN";
    if (std::isinf(value)) return std::signbit(value) ? "-Infinity" : "Infinity";
    if (value == 0) return "0";

    const bool negative = std::signbit(value);
    const double magnitude = std::abs(value);
    std::array<char, 64> buffer{};
    const auto converted = std::to_chars(buffer.data(), buffer.data() + buffer.size(), magnitude, std::chars_format::general);
    if (converted.ec != std::errc{}) throw std::runtime_error("shortest double conversion failed");
    std::string_view raw(buffer.data(), static_cast<std::size_t>(converted.ptr - buffer.data()));

    const auto exponent_at = raw.find_first_of("eE");
    const auto mantissa = raw.substr(0, exponent_at);
    int explicit_exponent = 0;
    if (exponent_at != std::string_view::npos) {
        auto exponent_text = raw.substr(exponent_at + 1);
        if (!exponent_text.empty() && exponent_text.front() == '+') exponent_text.remove_prefix(1);
        const auto parsed = std::from_chars(exponent_text.data(), exponent_text.data() + exponent_text.size(), explicit_exponent);
        if (parsed.ec != std::errc{}) throw std::runtime_error("shortest double exponent parse failed");
    }

    const auto decimal_at = mantissa.find('.');
    int decimal_position = static_cast<int>(decimal_at == std::string_view::npos ? mantissa.size() : decimal_at) + explicit_exponent;
    std::string digits;
    digits.reserve(mantissa.size());
    for (const auto character : mantissa) {
        if (character != '.') digits.push_back(character);
    }
    const auto first = digits.find_first_not_of('0');
    if (first == std::string::npos) return "0";
    decimal_position -= static_cast<int>(first);
    digits.erase(0, first);
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
            result.append(digits.data() + decimal_position, digits.size() - static_cast<std::size_t>(decimal_position));
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

std::string format_fixed_magnitude(std::uint64_t significand, int binary_exponent, unsigned digits) {
    BigUnsigned integer(significand);
    for (unsigned i = 0; i < digits; ++i) integer.multiply_small(5);

    const int twos = binary_exponent + static_cast<int>(digits);
    if (twos >= 0) {
        integer.shift_left(static_cast<unsigned>(twos));
    } else {
        const auto denominator_power = static_cast<unsigned>(-twos);
        const bool round_up = denominator_power != 0 && integer.bit(denominator_power - 1);
        integer.shift_right(denominator_power);
        if (round_up) integer.increment();
    }

    auto result = integer.decimal();
    if (digits == 0) return result;
    if (result.size() <= digits) result.insert(0, static_cast<std::size_t>(digits + 1) - result.size(), '0');
    result.insert(result.size() - digits, 1, '.');
    return result;
}

} // namespace

export namespace folia::math::js_number {

std::string toString(double value) {
    return format_shortest(value);
}

std::string toFixed(double value, unsigned digits = 0) {
    if (digits > 100) throw std::out_of_range("toFixed digits must be between 0 and 100");
    if (!std::isfinite(value)) return format_shortest(value);
    if (value == 0) {
        if (digits == 0) return "0";
        return "0." + std::string(digits, '0');
    }
    if (std::abs(value) >= 1e21) return format_shortest(value);

    const auto bits = std::bit_cast<std::uint64_t>(std::abs(value));
    const auto exponent_field = static_cast<unsigned>((bits >> 52) & 0x7ffU);
    const auto fraction = bits & ((std::uint64_t{1} << 52) - 1);
    const std::uint64_t significand = exponent_field == 0 ? fraction : fraction | (std::uint64_t{1} << 52);
    const int binary_exponent = exponent_field == 0 ? -1074 : static_cast<int>(exponent_field) - 1023 - 52;

    auto result = format_fixed_magnitude(significand, binary_exponent, digits);
    if (std::signbit(value)) result.insert(result.begin(), '-');
    return result;
}

} // namespace folia::math::js_number
