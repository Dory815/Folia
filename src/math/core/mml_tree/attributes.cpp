module folia.math.core.mml_tree.attributes;
import std;

namespace folia::math::mml {
const AttributeValue* AttributeSet::find(
    const std::vector<Entry>& entries, std::u16string_view name) noexcept {
    for (const auto& entry : entries) {
        if (entry.first == name) {
            return &entry.second;
        }
    }
    return nullptr;
}

void AttributeSet::put(
    std::vector<Entry>& entries, std::u16string name, AttributeValue value) {
    const auto found = std::find_if(entries.begin(), entries.end(), [&](const Entry& entry) {
        return entry.first == name;
    });
    if (found != entries.end()) {
        found->second = std::move(value);
        return;
    }
    entries.emplace_back(std::move(name), std::move(value));
}

void AttributeSet::set(std::u16string name, AttributeValue value) {
    put(own_, std::move(name), std::move(value));
}

bool AttributeSet::erase(std::u16string_view name) {
    const auto found = std::find_if(own_.begin(), own_.end(), [&](const Entry& entry) {
        return entry.first == name;
    });
    if (found == own_.end()) {
        return false;
    }
    own_.erase(found);
    return true;
}

const AttributeValue* AttributeSet::own(std::u16string_view name) const noexcept {
    return find(own_, name);
}

const AttributeValue* AttributeSet::defaultValue(std::u16string_view name) const noexcept {
    return find(defaults_, name);
}

const AttributeValue* AttributeSet::inherited(std::u16string_view name) const noexcept {
    return find(inherited_, name);
}

const AttributeValue* AttributeSet::lookup(std::u16string_view name) const noexcept {
    if (const auto* value = own(name)) {
        return value;
    }
    if (const auto* value = inherited(name)) {
        return value;
    }
    return defaultValue(name);
}

void AttributeSet::setDefault(std::u16string name, AttributeValue value) {
    put(defaults_, std::move(name), std::move(value));
}

void AttributeSet::setInherited(std::u16string name, AttributeValue value) {
    put(inherited_, std::move(name), std::move(value));
}
}
