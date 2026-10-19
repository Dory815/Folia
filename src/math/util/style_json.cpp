// MathJax 4.1.3 util/StyleJson.ts; Apache-2.0.
module folia.math.util.style_json;
import folia.math.util.property_host;
import std;

namespace folia::math::style_json {
namespace {
bool isObject(const Property& value) {
    if (PropertyList::record(value) || PropertyArray::array(value)) return true;
    return std::holds_alternative<FunctionRef>(value.value);
}

std::u16string ownString(const Property& value, std::string_view key) {
    return property_host::string(property_host::get(value, key));
}
}

StyleJsonSheet::StyleJsonSheet(Property styles) {
    addStyles(styles);
}

std::u16string StyleJsonSheet::cssText() const {
    return getStyleString();
}

void StyleJsonSheet::addStyles(const Property& input) {
    if (!property_host::truthy(input)) return;
    const auto keys = property_host::ownKeys(input);
    for (const auto& selector : keys) {
        Property target = styles_.get(selector);
        if (!property_host::truthy(target)) {
            PropertyList created;
            styles_.set(selector, Property(created));
            target = Property(created);
        }
        const Property source = property_host::get(input, selector);
        if (property_host::nullish(source)) continue; // Object.assign ignores nullish sources.
        const auto properties = property_host::ownKeys(source);
        // Object.assign boxes a primitive target; writes are not retained by
        // the stylesheet. Record targets are the Source's ordinary data path.
        if (auto record = PropertyList::record(target)) {
            for (const auto& property : properties)
                record->set(property, property_host::get(source, property));
        }
    }
}

void StyleJsonSheet::removeStyles(std::span<const std::u16string> selectors) {
    for (const auto& selector : selectors)
        styles_.erase(property_host::debugText(selector));
}

void StyleJsonSheet::clear() {
    styles_ = StyleJson{};
}

std::u16string StyleJsonSheet::getStyleString() const {
    const auto rules = getStyleRules();
    std::u16string result;
    for (std::size_t i = 0; i < rules.size(); ++i) {
        if (i) result += u"\n\n";
        result += rules[i];
    }
    return result;
}

std::vector<std::u16string> StyleJsonSheet::getStyleRules(
    Property styles, std::u16string_view spaces) const {
    if (styles.missing()) styles = Property(styles_);
    const auto selectors = property_host::ownKeys(styles);
    std::vector<std::u16string> defs(selectors.size());
    std::size_t i = 0;
    for (const auto& selector : selectors) {
        const Property data = property_host::get(styles, selector);
        std::u16string def(spaces);
        def += property_host::text(selector);
        def += u" {\n";
        def += getStyleDefString(data, spaces);
        def += u"\n";
        def += spaces;
        def += u"}";
        defs[i++] = std::move(def);
    }
    return defs;
}

std::u16string StyleJsonSheet::getStyleDefString(
    const Property& input, std::u16string_view spaces) const {
    const auto properties = property_host::ownKeys(input);
    std::vector<std::u16string> values(properties.size());
    std::size_t i = 0;
    for (const auto& property : properties) {
        const Property value = property_host::get(input, property);
        if (isObject(value)) {
            PropertyList nested;
            nested.set(property, value);
            const auto rules = getStyleRules(Property(nested), std::u16string(spaces) + u"  ");
            std::u16string joined;
            for (std::size_t j = 0; j < rules.size(); ++j) {
                if (j) joined += u"\n" + std::u16string(spaces);
                joined += rules[j];
            }
            values[i++] = std::u16string(spaces) + joined;
        } else {
            values[i++] = u"  " + std::u16string(spaces) + property_host::text(property) +
                u": " + property_host::string(value) + u";";
        }
    }
    std::u16string result;
    for (std::size_t j = 0; j < values.size(); ++j) {
        if (j) result += u"\n" + std::u16string(spaces);
        result += values[j];
    }
    return result;
}
}
