// MathJax 4.1.3 input/tex/UnitUtil.ts source algorithms; Apache-2.0.
module folia.math.input.tex.unit_util;
import folia.math.util.lengths;
import std;

namespace folia::math::tex {
namespace {
// Only called for the original dimension numeric capture (after comma
// replacement) or the ASCII value emitted by em(). No general value conversion.
double numericDimension(std::u16string_view text) {
    std::string number(text.begin(), text.end());
    if (!number.empty() && number.front() == '+') number.erase(0, 1);
    if (number == "Infinity") return std::numeric_limits<double>::infinity();
    if (number == "-Infinity") return -std::numeric_limits<double>::infinity();
    if (number == "NaN") return std::numeric_limits<double>::quiet_NaN();
    double value{};
    const auto [end, error] = std::from_chars(
        number.data(), number.data() + number.size(), value, std::chars_format::general);
    if (error == std::errc{} && end != number.data()) return value;
    if (error != std::errc::result_out_of_range)
        return std::numeric_limits<double>::quiet_NaN();

    // from_chars leaves value unchanged for overflow and underflow. Classify the
    // original fixed-decimal capture by its first nonzero digit, retaining -0.
    // em() never emits an out-of-range exponent: its input is already a double.
    const bool negative = !number.empty() && number.front() == '-';
    std::string_view magnitude(number);
    if (negative) magnitude.remove_prefix(1);
    const auto dot = magnitude.find('.');
    const auto integer_digits = dot == magnitude.npos ? magnitude.size() : dot;
    std::size_t digit_index{};
    auto first_nonzero = magnitude.npos;
    for (const auto digit : magnitude) {
        if (digit == '.') continue;
        if (first_nonzero == magnitude.npos && digit != '0') first_nonzero = digit_index;
        ++digit_index;
    }
    const double limit = first_nonzero != magnitude.npos && first_nonzero < integer_digits
        ? std::numeric_limits<double>::infinity() : 0.0;
    return std::copysign(limit, negative ? -1.0 : 1.0);
}

DimensionMatch muReplace(UnitUtil& util, DimensionMatch matched) {
    if (matched.unit != u"mu") return matched;
    const auto factor = util.UNIT_CASES.get(matched.unit).value_or(
        std::numeric_limits<double>::quiet_NaN());
    auto em = util.em(factor * numericDimension(matched.value));
    em.resize(em.size() - 2);
    matched.value.assign(em.begin(), em.end());
    matched.unit = u"em";
    return matched;
}

bool whitespace(char16_t c) noexcept {
    return (c >= 0x0009 && c <= 0x000D) || c == 0x0020 || c == 0x00A0 ||
        c == 0x1680 || (c >= 0x2000 && c <= 0x200A) || c == 0x2028 ||
        c == 0x2029 || c == 0x202F || c == 0x205F || c == 0x3000 || c == 0xFEFF;
}
}

UnitMap::UnitMap(UnitEntries entries) {
    for (auto& [name, value] : entries) {
        const auto found = std::ranges::find_if(map_, [&](auto const& entry) {
            return entry.first == name;
        });
        if (found == map_.end()) map_.emplace_back(std::move(name), value);
        else found->second = value;
    }
    updateDimen();
}

void UnitMap::updateDimen() {
    unit = u"(";
    bool first = true;
    for (auto const& [name, value] : map_) {
        (void)value;
        if (!first) unit.push_back(u'|');
        unit += name; // Source uses raw names, including regex metacharacters.
        first = false;
    }
    unit.push_back(u')');
    dimenEnd = std::make_unique<regexp_host::RegExp>(u"^\\s*" + num + u"\\s*" + unit + u"\\s*$");
    dimenRest = std::make_unique<regexp_host::RegExp>(u"^\\s*" + num + u"\\s*" + unit + u" ?");
}

UnitMap* UnitMap::set(std::u16string name, double ems) {
    const auto found = std::ranges::find_if(map_, [&](auto const& entry) {
        return entry.first == name;
    });
    if (found == map_.end()) map_.emplace_back(std::move(name), ems);
    else found->second = ems;
    updateDimen();
    return this;
}

std::optional<double> UnitMap::get(std::u16string_view name) const {
    const auto found = std::ranges::find_if(map_, [&](auto const& entry) {
        return entry.first == name;
    });
    if (found != map_.end() && found->second != 0 && !std::isnan(found->second))
        return found->second;
    const auto point = std::ranges::find_if(map_, [](auto const& entry) {
        return entry.first == u"pt";
    });
    return point == map_.end() ? std::nullopt : std::optional<double>(point->second);
}

bool UnitMap::erase(std::u16string_view name) {
    const auto found = std::ranges::find_if(map_, [&](auto const& entry) {
        return entry.first == name;
    });
    if (found == map_.end()) return false;
    map_.erase(found);
    updateDimen();
    return true;
}

UnitMap defaultUnitCases() {
    constexpr double emPerInch = 7.2, pxPerInch = 72;
    return UnitMap(UnitEntries{{u"em", 1}, {u"ex", .43}, {u"pt", 1. / 10},
        {u"pc", 1.2}, {u"px", emPerInch / pxPerInch}, {u"in", emPerInch},
        {u"cm", emPerInch / 2.54}, {u"mm", emPerInch / 25.4}, {u"mu", 1. / 18}});
}

UnitUtil::UnitUtil() : UNIT_CASES(defaultUnitCases()) {}
UnitUtil& UnitUtil::instance() { static UnitUtil actual; return actual; }

Dimension UnitUtil::matchDimen(std::u16string_view dim, bool rest) {
    auto& pattern = rest ? *UNIT_CASES.dimenRest : *UNIT_CASES.dimenEnd;
    const auto matched = pattern.exec(std::u16string(dim));
    if (!matched) return std::nullopt;
    auto value = matched->at(1).value();
    if (const auto comma = value.find(u','); comma != value.npos) value[comma] = u'.';
    return muReplace(*this, DimensionMatch{std::move(value),
        matched->at(4).value(), matched->at(0).value().size()});
}

double UnitUtil::dimen2em(std::u16string_view dim) {
    const auto matched = matchDimen(dim);
    const auto value = matched && !matched->value.empty()
        ? numericDimension(matched->value) : 1.0;
    // An unmatched Source dimension passes null as its unit, which takes the
    // point fallback rather than looking up an actual empty-string unit key.
    const auto factor = UNIT_CASES.get(matched ? std::u16string_view(matched->unit) : std::u16string_view(u"pt"));
    return factor.value_or(std::numeric_limits<double>::quiet_NaN()) * value;
}

std::string UnitUtil::em(double value) {
    if (std::abs(value) < .0006) return "0em";
    auto fixed = lengths::format_decimal(value, 3);
    // Source /\.?0+$/ also trims exponent zeros on values >= 1e21.
    const auto old_size = fixed.size();
    while (!fixed.empty() && fixed.back() == '0') fixed.pop_back();
    if (fixed.size() != old_size && !fixed.empty() && fixed.back() == '.') fixed.pop_back();
    return fixed + "em";
}

std::u16string UnitUtil::trimSpaces(std::u16string_view text) {
    std::size_t first{}, last = text.size();
    while (first < last && whitespace(text[first])) ++first;
    while (last > first && whitespace(text[last - 1])) --last;
    std::u16string trimmed(text.substr(first, last - first));
    if (!trimmed.empty() && trimmed.back() == u'\\' && !text.empty() && text.back() == u' ')
        trimmed.push_back(u' ');
    return trimmed;
}
}
