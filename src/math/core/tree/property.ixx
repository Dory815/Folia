// Native storage adaptation for MathJax core/Tree/Node.ts PropertyList.
// MathJax source is Apache-2.0; see docs/math-port/architecture.md for provenance.
export module folia.math.core.tree.property;
import std;

export namespace folia::math {
struct Missing { bool operator==(const Missing&) const = default; };
struct PropertyNull { bool operator==(const PropertyNull&) const = default; };
// Object.prototype values are observable on malformed attribute names, but are
// not callable math services. Keep their identities, rather than stringify them.
enum class BuiltinProperty {
    constructor, defineGetter, defineSetter, hasOwnProperty, lookupGetter,
    lookupSetter, isPrototypeOf, propertyIsEnumerable, toString, valueOf,
    toLocaleString, prototype
};
struct PropertyStorage;
class PropertyList;
struct Property {
    using Value = std::variant<Missing, PropertyNull, bool, double, std::string, BuiltinProperty,
                               std::shared_ptr<PropertyStorage>>;
    Value value = Missing{};
    Property() = default;
    Property(Missing v) : value(v) {}
    Property(PropertyNull v) : value(v) {}
    Property(bool v) : value(v) {}
    Property(double v) : value(v) {}
    Property(int v) : value(double(v)) {}
    Property(const char* v) : value(std::string(v)) {}
    Property(std::string v) : value(std::move(v)) {}
    Property(BuiltinProperty v) : value(v) {}
    Property(const PropertyList& v);
    bool missing() const { return std::holds_alternative<Missing>(value); }
    bool operator==(const Property&) const = default;
};
struct PropertyStorage {
    std::vector<std::pair<std::string, Property>> entries;
    std::shared_ptr<PropertyStorage> prototype;
    bool objectPrototype = true;
};

// Copies are handles to the same dictionary, like JS object references. Own
// entries are separate from the prototype; deletion never hides inherited data.
class PropertyList {
    friend struct Property;
    using Storage = PropertyStorage;
    std::shared_ptr<Storage> storage_ = std::make_shared<Storage>();
    static Property builtin(std::string_view name) {
        static const std::pair<std::string_view, BuiltinProperty> values[] = {
            {"constructor", BuiltinProperty::constructor},
            {"__defineGetter__", BuiltinProperty::defineGetter},
            {"__defineSetter__", BuiltinProperty::defineSetter},
            {"hasOwnProperty", BuiltinProperty::hasOwnProperty},
            {"__lookupGetter__", BuiltinProperty::lookupGetter},
            {"__lookupSetter__", BuiltinProperty::lookupSetter},
            {"isPrototypeOf", BuiltinProperty::isPrototypeOf},
            {"propertyIsEnumerable", BuiltinProperty::propertyIsEnumerable},
            {"toString", BuiltinProperty::toString},
            {"valueOf", BuiltinProperty::valueOf},
            {"toLocaleString", BuiltinProperty::toLocaleString},
            {"__proto__", BuiltinProperty::prototype}
        };
        for (auto [key, value] : values) if (key == name) return value;
        return Missing{};
    }
    static Property resolve(const std::shared_ptr<Storage>& storage, std::string_view name) {
        bool ordinary = true;
        for (auto p = storage; p; p = p->prototype)
        {
            for (const auto& [key, value] : p->entries) if (key == name) return value;
            ordinary = p->objectPrototype;
        }
        if (!ordinary) return Missing{};
        if (name == "__proto__" && storage->prototype) {
            Property value;
            value.value = storage->prototype;
            return value;
        }
        return builtin(name);
    }
    static std::optional<std::uint32_t> arrayIndex(std::string_view key) {
        if (key.empty() || (key.size() > 1 && key.front() == '0')) return {};
        std::uint32_t n{};
        auto [p, e] = std::from_chars(key.data(), key.data() + key.size(), n);
        if (e != std::errc{} || p != key.data() + key.size() || n == std::numeric_limits<std::uint32_t>::max()) return {};
        return n;
    }
public:
    PropertyList() = default;
    static std::optional<PropertyList> object(const Property& value) {
        if (auto p = std::get_if<std::shared_ptr<Storage>>(&value.value); p && *p) {
            PropertyList result; result.storage_ = *p; return result;
        }
        return {};
    }
    PropertyList(std::initializer_list<std::pair<std::string, Property>> entries) {
        for (const auto& [key, value] : entries) setOwn(key, value);
    }
    static PropertyList withPrototype(const PropertyList& parent) {
        PropertyList result;
        result.storage_->prototype = parent.storage_;
        return result;
    }
    Property get(std::string_view name) const { return resolve(storage_, name); }
    Property getOwn(std::string_view name) const {
        for (const auto& [key, value] : storage_->entries) if (key == name) return value;
        return Missing{};
    }
    bool hasOwn(std::string_view name) const {
        for (const auto& [key, value] : storage_->entries) if (key == name) return true;
        return false;
    }
    bool has(std::string_view name) const {
        bool ordinary = true;
        for (auto p = storage_; p; p = p->prototype)
        {
            for (const auto& [key, value] : p->entries) if (key == name) return true;
            ordinary = p->objectPrototype;
        }
        return ordinary && !builtin(name).missing();
    }
    void setOwn(std::string_view name, Property value) {
        for (auto& [key, old] : storage_->entries) if (key == name) {
            old = std::move(value); return;
        }
        storage_->entries.emplace_back(name, std::move(value));
    }
    void set(std::string_view name, Property value) {
        // The inherited Object.prototype __proto__ setter ignores scalar values.
        // An own data property on any layer instead allows a new own property.
        if (name == "__proto__") {
            bool data = false;
            bool ordinary = true;
            for (auto p = storage_; p; p = p->prototype)
            {
                for (const auto& [key, v] : p->entries) if (key == name) data = true;
                ordinary = p->objectPrototype;
            }
            if (!data && ordinary) {
                if (auto parent = std::get_if<std::shared_ptr<Storage>>(&value.value)) {
                    for (auto p = *parent; p; p = p->prototype)
                        if (p == storage_) throw std::invalid_argument("Cyclic __proto__ value");
                    storage_->prototype = *parent;
                    storage_->objectPrototype = true;
                } else if (value.value == Property::Value(BuiltinProperty::prototype)) {
                    storage_->prototype.reset();
                    storage_->objectPrototype = true;
                } else if (std::holds_alternative<PropertyNull>(value.value)) {
                    storage_->prototype.reset();
                    storage_->objectPrototype = false;
                }
                return;
            }
        }
        setOwn(name, std::move(value));
    }
    void erase(std::string_view name) {
        std::erase_if(storage_->entries, [&](const auto& entry) { return entry.first == name; });
    }
    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        for (const auto& [key, value] : storage_->entries) result.push_back(key);
        std::stable_sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
            auto i = arrayIndex(a), j = arrayIndex(b);
            if (i && j) return *i < *j;
            return bool(i) && !j;
        });
        return result;
    }
    void assign(const PropertyList& source) {
        for (const auto& key : source.keys()) set(key, source.getOwn(key));
    }
};
inline Property::Property(const PropertyList& v) : value(v.storage_) {}
}
