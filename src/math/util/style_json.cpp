// MathJax 4.1.3 util/StyleJson.ts; Apache-2.0.
module folia.math.util.style_json;
import folia.math.util.js_number;
import std;

namespace folia::math::style_json {
namespace {

const std::vector<StyleEntry> *group(const StyleEntry &entry) {
    return std::get_if<std::vector<StyleEntry>>(&entry.value);
}

std::u16string join(const std::vector<std::u16string> &values, std::u16string_view separator) {
    std::u16string result;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i)
            result += separator;
        result += values[i];
    }
    return result;
}

std::u16string scalar_text(const StyleEntry &entry) {
    if (const auto *text = std::get_if<std::u16string>(&entry.value))
        return *text;
    if (const auto *number = std::get_if<double>(&entry.value)) {
        const auto ascii = folia::math::js_number::toString(*number);
        std::u16string result;
        result.reserve(ascii.size());
        for (const unsigned char byte : ascii)
            result.push_back(static_cast<char16_t>(byte));
        return result;
    }
    throw std::invalid_argument("StyleJson group cannot be rendered as a CSS scalar");
}

} // namespace

StyleJsonSheet::StyleJsonSheet(StyleJson styles) {
    addStyles(styles);
}

std::u16string StyleJsonSheet::cssText() const {
    return getStyleString();
}

void StyleJsonSheet::addStyles(const StyleJson &input) {
    for (const auto &selector : input) {
        const auto *incoming = group(selector);
        if (!incoming) {
            throw std::invalid_argument("StyleJson selector value must be a style group");
        }
        auto found = std::find_if(styles_.begin(), styles_.end(), [&](const StyleEntry &entry) {
            return entry.name == selector.name;
        });
        if (found == styles_.end()) {
            styles_.push_back(selector);
            continue;
        }
        auto *target = std::get_if<std::vector<StyleEntry>>(&found->value);
        if (!target) {
            // A valid stylesheet keeps each selector's outer value a group.
            // Repairing an invalid stored state would hide a broken owner invariant.
            throw std::logic_error("StyleJson stored selector is not a style group");
        }
        for (const auto &property : *incoming) {
            const auto existing =
                std::find_if(target->begin(), target->end(),
                             [&](const StyleEntry &entry) { return entry.name == property.name; });
            if (existing == target->end())
                target->push_back(property);
            else
                existing->value = property.value;
        }
    }
}

void StyleJsonSheet::removeStyles(std::span<const std::u16string> selectors) {
    for (const auto &selector : selectors) {
        styles_.erase(
            std::remove_if(styles_.begin(), styles_.end(),
                           [&](const StyleEntry &entry) { return entry.name == selector; }),
            styles_.end());
    }
}

void StyleJsonSheet::clear() {
    styles_.clear();
}

std::u16string StyleJsonSheet::getStyleString() const {
    const auto rules = getStyleRules();
    std::u16string result;
    for (std::size_t i = 0; i < rules.size(); ++i) {
        if (i)
            result += u"\n\n";
        result += rules[i];
    }
    return result;
}

std::vector<std::u16string> StyleJsonSheet::getStyleRules(std::u16string_view spaces) const {
    return getStyleRules(styles_, spaces);
}

std::vector<std::u16string> StyleJsonSheet::getStyleRules(const StyleJson &styles,
                                                          std::u16string_view spaces) const {
    std::vector<std::u16string> rules;
    rules.reserve(styles.size());
    for (const auto &selector : styles) {
        const auto *data = group(selector);
        if (!data) {
            throw std::invalid_argument("StyleJson selector value must be a style group");
        }
        std::u16string definition(spaces);
        definition += selector.name;
        definition += u" {\n";
        definition += getStyleDefString(*data, spaces);
        definition += u"\n";
        definition += spaces;
        definition += u"}";
        rules.push_back(std::move(definition));
    }
    return rules;
}

std::u16string StyleJsonSheet::getStyleDefString(const StyleJson &styles,
                                                 std::u16string_view spaces) const {
    std::vector<std::u16string> values;
    values.reserve(styles.size());
    for (const auto &property : styles) {
        if (const auto *nested = group(property)) {
            std::u16string joinedPrefix(spaces);
            const std::u16string childSpaces = std::u16string(spaces) + u"  ";
            const StyleJson childRule{StyleEntry{property.name, *nested}};
            const auto nestedRules = getStyleRules(childRule, childSpaces);
            values.push_back(joinedPrefix +
                             join(nestedRules, std::u16string(u"\n") + std::u16string(spaces)));
        } else {
            std::u16string declaration = u"  ";
            declaration += spaces;
            declaration += property.name;
            declaration += u": ";
            declaration += scalar_text(property);
            declaration += u";";
            values.push_back(std::move(declaration));
        }
    }
    return join(values, std::u16string(u"\n") + std::u16string(spaces));
}

} // namespace folia::math::style_json
