module folia.math.core.mml_tree.attributes;
import std;

namespace folia::math::mml {
namespace {
constexpr std::u16string_view inheritSentinel = u"_inherit_";
}

const AttributeValue* AttributeSet::find(
    const std::vector<Entry>& entries,
    std::u16string_view name) noexcept {
    const auto found = std::find_if(
        entries.begin(), entries.end(),
        [name](const Entry& entry) { return entry.first == name; });
    return found == entries.end() ? nullptr : &found->second;
}

void AttributeSet::put(
    std::vector<Entry>& entries,
    std::u16string name,
    AttributeValue value) {
    const auto found = std::find_if(
        entries.begin(), entries.end(),
        [&name](const Entry& entry) { return entry.first == name; });
    if (found != entries.end()) {
        found->second = std::move(value);
        return;
    }
    entries.emplace_back(std::move(name), std::move(value));
}

void AttributeSet::set(std::u16string name, AttributeValue value) {
    put(own_, std::move(name), std::move(value));
}

void AttributeSet::setInherited(std::u16string name, AttributeValue value) {
    put(inherited_, std::move(name), std::move(value));
}

void AttributeSet::setDefault(std::u16string name, AttributeValue value) {
    put(defaults_, std::move(name), std::move(value));
}

void AttributeSet::setGlobalDefault(std::u16string name, AttributeValue value) {
    put(globalDefaults_, std::move(name), std::move(value));
}

bool AttributeSet::erase(std::u16string_view name) {
    const auto found = std::find_if(
        own_.begin(), own_.end(),
        [name](const Entry& entry) { return entry.first == name; });
    if (found == own_.end()) {
        return false;
    }
    own_.erase(found);
    return true;
}

const AttributeValue* AttributeSet::own(std::u16string_view name) const noexcept {
    return find(own_, name);
}

const AttributeValue* AttributeSet::inherited(std::u16string_view name) const noexcept {
    return find(inherited_, name);
}

const AttributeValue* AttributeSet::defaultValue(std::u16string_view name) const noexcept {
    return find(defaults_, name);
}

const AttributeValue* AttributeSet::globalDefault(std::u16string_view name) const noexcept {
    return find(globalDefaults_, name);
}

const AttributeValue* AttributeSet::effectiveDefault(std::u16string_view name) const noexcept {
    if (const auto* value = defaultValue(name)) {
        return value;
    }
    return globalDefault(name);
}

const AttributeValue* AttributeSet::effectiveInherited(
    std::u16string_view name) const noexcept {
    if (const auto* value = inherited(name)) {
        return value;
    }
    return effectiveDefault(name);
}

const AttributeValue* AttributeSet::lookup(std::u16string_view name) const noexcept {
    const AttributeValue* value = own(name);
    if (!value) {
        value = inherited(name);
    }
    if (!value) {
        value = defaultValue(name);
    }
    if (!value) {
        value = globalDefault(name);
    }

    if (const auto* stringValue = value ? std::get_if<std::u16string>(value) : nullptr;
        stringValue && *stringValue == inheritSentinel) {
        return globalDefault(name);
    }
    return value;
}

bool AttributeSet::isSet(std::u16string_view name) const noexcept {
    return own(name) != nullptr || inherited(name) != nullptr;
}

bool AttributeSet::hasDefault(std::u16string_view name) const noexcept {
    return defaultValue(name) != nullptr || globalDefault(name) != nullptr;
}
}
