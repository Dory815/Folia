// Native host for MathJax 4.1.3 TokenMap.ts argument spread semantics.
// Apache-2.0; see src/math/LICENSE-MathJax.txt.
module folia.math.util.argument_expansion;

import folia.math.util.property_host;
import folia.math.util.utf16;
import folia.math.util.option_types;
import std;

namespace {

constexpr double max_safe_integer = 9007199254740991.0;
constexpr double max_array_length = 4294967295.0;

[[noreturn]] void invalid_array_length() {
    throw folia::math::options::Exception("RangeError", u"Invalid array length");
}

std::uint64_t to_length(const folia::math::Property& value) {
    const double number = folia::math::property_host::number(value);
    if (std::isnan(number) || number <= 0) return 0;
    if (number >= max_safe_integer) return 9007199254740991ULL;
    return static_cast<std::uint64_t>(std::trunc(number));
}

std::string iterator_string(char32_t point) {
    std::u16string units;
    if (point <= 0xFFFF) {
        // Includes an isolated surrogate, which remains a lossless internal
        // WTF-8 string when converted by debugText.
        units.push_back(static_cast<char16_t>(point));
    } else {
        const auto scalar = static_cast<std::uint32_t>(point) - 0x10000;
        units.push_back(static_cast<char16_t>(0xD800 + (scalar >> 10)));
        units.push_back(static_cast<char16_t>(0xDC00 + (scalar & 0x3FF)));
    }
    return folia::math::property_host::debugText(units);
}

} // namespace

std::vector<folia::math::Property> folia::math::arguments::expand(
    const folia::math::Property& input) {
    using namespace folia::math;

    // __read reads Symbol.iterator before the spread helper constructs its
    // destination. In the accepted Property domain, nullish input cannot
    // supply that property access.
    if (property_host::nullish(input)) {
        property_host::typeError("Cannot convert null or undefined to object");
    }

    if (const auto* string = std::get_if<std::string>(&input.value)) {
        const auto units = property_host::text(*string);
        const auto points = utf16::codePoints(units);
        std::vector<Property> result;
        result.reserve(points.size());
        for (const auto point : points) result.emplace_back(iterator_string(point));
        return result;
    }

    if (const auto array = PropertyArray::array(input)) {
        const auto length = static_cast<std::uint64_t>(array->size());
        if (static_cast<double>(length) > max_array_length) invalid_array_length();
        std::vector<Property> result;
        result.reserve(static_cast<std::size_t>(length));
        // Array's iterator performs Get for every index. This observes
        // inherited numeric properties and yields Missing for an unfilled
        // hole, while nested Property handles retain their identity.
        for (std::uint64_t index = 0; index < length; ++index) {
            result.push_back(array->get(static_cast<std::size_t>(index)));
        }
        return result;
    }

    // With no Symbol.iterator in the supported non-array Property domain,
    // TypeScript's __read returns the value and __spreadArray falls through
    // to Array.prototype.slice.call(from, 0). Slice obtains ToLength before
    // ArrayCreate and indexed Get operations.
    const std::uint64_t length = to_length(property_host::get(input, "length"));
    if (static_cast<double>(length) > max_array_length) invalid_array_length();

    std::vector<Property> result;
    result.reserve(static_cast<std::size_t>(length));
    for (std::uint64_t index = 0; index < length; ++index) {
        result.push_back(property_host::get(input, std::to_string(index)));
    }
    return result;
}
