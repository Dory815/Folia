// Native storage adaptation for MathJax core/Tree/Node.ts PropertyList.
// MathJax source is Apache-2.0; see docs/math-port/architecture.md for provenance.
export module folia.math.core.tree.property;
import std;
import folia.math.util.option_types;

export namespace folia::math {
struct Missing { bool operator==(const Missing&) const = default; };
struct PropertyNull { bool operator==(const PropertyNull&) const = default; };
// Object.prototype values are observable on malformed attribute names, but are
// not callable math services. Keep their identities, rather than stringify them.
enum class BuiltinProperty {
    constructor, defineGetter, defineSetter, hasOwnProperty, lookupGetter,
    lookupSetter, isPrototypeOf, propertyIsEnumerable, toString, valueOf,
    toLocaleString, prototype, arrayConstructor, arrayToString, arrayJoin, arrayPrototype,
    numberPrototype, numberConstructor, numberToString, numberValueOf,
    functionPrototype, functionConstructor, functionCall, functionApply,
    functionBind, functionToString
};
struct PropertyStorage;
class PropertyList;
class PropertyArray;
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
    Property(const PropertyArray& v);
    bool missing() const { return std::holds_alternative<Missing>(value); }
    bool operator==(const Property&) const = default;
};
struct PropertyStorage {
    std::vector<std::pair<std::string, Property>> entries;
    std::shared_ptr<PropertyStorage> prototype;
    bool objectPrototype = true;
    bool arrayPrototype = false;
    bool array = false;
    std::vector<Property> arrayValues;
    std::vector<bool> arrayPresent;
};

// Copies are handles to the same dictionary, like JS object references. Own
// entries are separate from the prototype; deletion never hides inherited data.
class PropertyList {
    friend struct Property;
    using Storage = PropertyStorage;
    std::shared_ptr<Storage> storage_ = std::make_shared<Storage>();
    explicit PropertyList(std::shared_ptr<Storage> storage):storage_(std::move(storage)){}
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
            if(p->array){
                if(name=="length")return static_cast<double>(p->arrayValues.size());
                if(auto i=arrayIndex(name);i&&*i<p->arrayValues.size()&&p->arrayPresent[*i])return p->arrayValues[*i];
            }
            ordinary = p->objectPrototype;
            if(!p->prototype&&p->arrayPrototype&&ordinary){
                if(name=="length")return 0;
                if(name=="constructor")return BuiltinProperty::arrayConstructor;
                if(name=="toString")return BuiltinProperty::arrayToString;
                if(name=="join")return BuiltinProperty::arrayJoin;
            }
        }
        if (!ordinary) return Missing{};
        if (name == "__proto__" && storage->prototype) {
            Property value;
            value.value = storage->prototype;
            return value;
        }
        if(name=="__proto__"&&storage->arrayPrototype)return BuiltinProperty::arrayPrototype;
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
        if (auto p = std::get_if<std::shared_ptr<Storage>>(&value.value); p && *p && !(*p)->array) {
            return PropertyList(*p);
        }
        return {};
    }
    static std::optional<PropertyList> record(const Property& value){
        if(auto p=std::get_if<std::shared_ptr<Storage>>(&value.value);p&&*p)return PropertyList(*p);return {};
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
        if(storage_->array){if(name=="length")return static_cast<double>(storage_->arrayValues.size());if(auto i=arrayIndex(name);i&&*i<storage_->arrayValues.size()&&storage_->arrayPresent[*i])return storage_->arrayValues[*i];}
        for (const auto& [key, value] : storage_->entries) if (key == name) return value;
        return Missing{};
    }
    bool hasOwn(std::string_view name) const {
        if(storage_->array){if(name=="length")return true;if(auto i=arrayIndex(name);i&&*i<storage_->arrayValues.size()&&storage_->arrayPresent[*i])return true;}
        for (const auto& [key, value] : storage_->entries) if (key == name) return true;
        return false;
    }
    bool has(std::string_view name) const {
        bool ordinary = true;
        for (auto p = storage_; p; p = p->prototype)
        {
            for (const auto& [key, value] : p->entries) if (key == name) return true;
            if(p->array){
                if(name=="length")return true;
                if(auto i=arrayIndex(name);i&&*i<p->arrayValues.size()&&p->arrayPresent[*i])return true;
            }
            if(!p->prototype&&p->objectPrototype&&p->arrayPrototype&&(name=="length"||name=="join"||name=="toString"||name=="constructor"))return true;
            ordinary = p->objectPrototype;
        }
        return ordinary && !builtin(name).missing();
    }
    void setOwn(std::string_view name, Property value) {
        if(storage_->array){
            if(name=="length"){
                auto n=std::get_if<double>(&value.value);
                if(!n||!std::isfinite(*n)||*n<0||*n>0xFFFFFFFFULL||std::trunc(*n)!=*n)throw options::Exception("RangeError",u"Invalid array length");
                storage_->arrayValues.resize(static_cast<std::size_t>(*n));storage_->arrayPresent.resize(static_cast<std::size_t>(*n),false);return;
            }
            if(auto i=arrayIndex(name)){
                if(*i>=storage_->arrayValues.size()){storage_->arrayValues.resize(std::size_t(*i)+1);storage_->arrayPresent.resize(std::size_t(*i)+1,false);}
                storage_->arrayValues[*i]=std::move(value);storage_->arrayPresent[*i]=true;return;
            }
        }
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
                    storage_->arrayPrototype = false;
                } else if (value.value == Property::Value(BuiltinProperty::prototype)) {
                    storage_->prototype.reset();
                    storage_->objectPrototype = true;
                    storage_->arrayPrototype = false;
                } else if (value.value == Property::Value(BuiltinProperty::arrayPrototype)) {
                    storage_->prototype.reset();
                    storage_->objectPrototype = true;
                    storage_->arrayPrototype = true;
                } else if (std::holds_alternative<PropertyNull>(value.value)) {
                    storage_->prototype.reset();
                    storage_->objectPrototype = false;
                    storage_->arrayPrototype = false;
                }
                return;
            }
        }
        setOwn(name, std::move(value));
    }
    void erase(std::string_view name) {
        if(storage_->array){if(name=="length")return;if(auto i=arrayIndex(name);i&&*i<storage_->arrayValues.size()){storage_->arrayValues[*i]=Missing{};storage_->arrayPresent[*i]=false;}}
        std::erase_if(storage_->entries, [&](const auto& entry) { return entry.first == name; });
    }
    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        if(storage_->array)for(std::size_t i=0;i<storage_->arrayValues.size();++i)if(storage_->arrayPresent[i])result.push_back(std::to_string(i));
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

// Native array storage in the same Property graph as ordinary property objects.
// Numeric slots preserve holes versus own undefined; iteration reads live slots.
class PropertyArray {
    friend struct Property;
    std::shared_ptr<PropertyStorage> storage_=std::make_shared<PropertyStorage>();
    explicit PropertyArray(std::shared_ptr<PropertyStorage> storage):storage_(std::move(storage)){}
public:
    PropertyArray(){storage_->array=true;storage_->arrayPrototype=true;}
    PropertyArray(std::initializer_list<Property> values):PropertyArray(){for(const auto& p:values)push(p);}
    static std::optional<PropertyArray> array(const Property& p){
        if(auto v=std::get_if<std::shared_ptr<PropertyStorage>>(&p.value);v&&*v&&(*v)->array)return PropertyArray(*v);return {};
    }
    std::size_t size()const{return storage_->arrayValues.size();}
    bool has(std::size_t i)const{return i<size()&&storage_->arrayPresent[i];}
    Property get(std::size_t i)const{return record().get(std::to_string(i));}
    PropertyList record()const{return PropertyList::record(Property(*this)).value();}
    void resize(std::size_t length){
        if(length>0xFFFFFFFFULL)throw options::Exception("RangeError",u"Invalid array length");
        storage_->arrayValues.resize(length);storage_->arrayPresent.resize(length,false);
    }
    void set(std::size_t i,Property value){
        if(i>=0xFFFFFFFFULL)throw options::Exception("RangeError",u"Native array index exceeds index domain");
        if(i>=size())resize(i+1);storage_->arrayValues[i]=std::move(value);storage_->arrayPresent[i]=true;
    }
    void push(Property value){set(size(),std::move(value));}
    void erase(std::size_t i){if(i<size()){storage_->arrayValues[i]=Missing{};storage_->arrayPresent[i]=false;}}
    std::vector<std::string> keys()const{return record().keys();}
    PropertyArray spread()const{PropertyArray out;for(std::size_t i=0;i<size();++i)out.push(get(i));return out;}
};
inline Property::Property(const PropertyArray& v):value(v.storage_){}
}
