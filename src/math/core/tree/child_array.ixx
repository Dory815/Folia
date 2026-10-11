// Root-frozen Source Array binding for factory-borrowed mathematical nodes.
export module folia.math.core.tree.child_array;
export import folia.math.core.tree.property;
export import folia.math.util.option_types;
import std;
export namespace folia::math::tree {
enum class ChildKind { Undefined, Null, Node };
template<class Product> class ChildValue {
    std::variant<Missing,PropertyNull,Product*> value_ = Missing{};
public:
    ChildValue()=default;
    ChildValue(Missing):value_(Missing{}){}
    ChildValue(PropertyNull):value_(PropertyNull{}){}
    ChildValue(std::nullptr_t):value_(PropertyNull{}){}
    ChildValue(Product* node):value_(node?decltype(value_)(node):decltype(value_)(PropertyNull{})){}
    ChildKind kind()const {
        if(std::holds_alternative<Missing>(value_))return ChildKind::Undefined;
        if(std::holds_alternative<PropertyNull>(value_))return ChildKind::Null;
        return ChildKind::Node;
    }
    bool isNode()const{return kind()==ChildKind::Node;}
    // Explicit truthiness/RTTI projection only, never an implicit pointer cast.
    Product* nodeOrNull()const{auto p=std::get_if<Product*>(&value_);return p?*p:nullptr;}
    Product* requireNode(std::string_view member="native node")const {
        if(auto* p=nodeOrNull())return p;
        const auto message=std::string("Cannot read properties of ")+(kind()==ChildKind::Null?"null":"undefined")+" (reading '"+std::string(member)+"')";
        throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
    }
    Product* requireNodeForWrite(std::string_view member)const {
        if(auto* p=nodeOrNull())return p;
        const auto message=std::string("Cannot set properties of ")+(kind()==ChildKind::Null?"null":"undefined")+" (setting '"+std::string(member)+"')";
        throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
    }
    bool operator==(const ChildValue&)const=default;
};
template<class Product> class ChildArray : public std::enable_shared_from_this<ChildArray<Product>> {
public:
    using Value=ChildValue<Product>;
    using Handle=std::shared_ptr<ChildArray>;
    using Each=std::function<void(Value,std::size_t,ChildArray&)>;
private:
    [[noreturn]] static void rangeError(std::u16string message) {
        throw options::Exception("RangeError", std::move(message));
    }
    [[noreturn]] static void typeError(std::u16string message) {
        throw options::Exception("TypeError", std::move(message));
    }
    // Work in unsigned magnitudes so INT64_MIN and lengths wider than int64_t
    // remain well-defined under the signed index binding.
    static std::size_t relativeIndex(std::int64_t index, std::size_t length) {
        if (index >= 0) {
            const auto positive = static_cast<std::uint64_t>(index);
            return positive >= length ? length : static_cast<std::size_t>(positive);
        }
        const auto magnitude = static_cast<std::uint64_t>(-(index + 1)) + 1;
        return magnitude >= length ? 0 : length - static_cast<std::size_t>(magnitude);
    }
    static std::size_t checkedLength(std::size_t length, std::size_t max,
                                     std::u16string_view operation) {
        if (length > max) {
            std::u16string message(operation);
            message += u" exceeds the native array length limit";
            rangeError(std::move(message));
        }
        return length;
    }
    std::vector<std::optional<Value>> slots_;
    Handle prototype_;
public:
    ChildArray()=default;
    ChildArray(std::initializer_list<Value> values) {
        slots_.reserve(values.size());
        for (const auto& value : values) slots_.emplace_back(value);
    }
    ChildArray(const ChildArray&)=delete;ChildArray&operator=(const ChildArray&)=delete;
    ChildArray(ChildArray&&)=delete;ChildArray&operator=(ChildArray&&)=delete;
    std::size_t size()const{return slots_.size();}
    bool empty()const{return slots_.empty();}
    bool hasOwn(std::size_t index)const {
        return index < slots_.size() && slots_[index].has_value();
    }
    bool has(std::size_t index)const {
        if (hasOwn(index)) return true;
        return prototype_ && prototype_->has(index);
    }
    Value get(std::size_t index)const {
        if (hasOwn(index)) return *slots_[index];
        if (prototype_) return prototype_->get(index);
        return Value(Missing{});
    }
    void set(std::size_t index,Value value) {
        checkedLength(index, slots_.max_size() - 1, u"Array index");
        if (index >= slots_.size()) slots_.resize(index + 1);
        slots_[index] = std::move(value);
    }
    void erase(std::size_t index) {
        if (index < slots_.size()) slots_[index].reset();
    }
    void resize(std::size_t length) {
        checkedLength(length, slots_.max_size(), u"Array length");
        slots_.resize(length);
    }
    std::size_t push_back(Value value) {
        if (slots_.size() >= slots_.max_size()) rangeError(u"Array length exceeds the native array length limit");
        slots_.emplace_back(std::move(value));
        return slots_.size();
    }
    Value pop_back() {
        if (slots_.empty()) return Value(Missing{});
        const Value value = get(slots_.size() - 1);
        slots_.pop_back();
        return value;
    }
    Handle slice(std::int64_t start=0,std::optional<std::int64_t> end = {})const {
        const std::size_t length = slots_.size();
        const std::size_t first = relativeIndex(start, length);
        const std::size_t last = end ? relativeIndex(*end, length) : length;
        auto result = std::make_shared<ChildArray>();
        const std::size_t resultLength = last > first ? last - first : 0;
        result->resize(resultLength);
        for (std::size_t offset = 0; offset < resultLength; ++offset) {
            const std::size_t sourceIndex = first + offset;
            if (has(sourceIndex)) result->set(offset, get(sourceIndex));
        }
        return result;
    }
    Handle splice(std::int64_t start,std::optional<std::size_t> count = {},std::span<const Value> added = {}) {
        const std::size_t length = slots_.size();
        const std::size_t first = relativeIndex(start, length);
        const std::size_t available = length - first;
        const std::size_t removedCount = count ? std::min(*count, available) : available;
        const std::size_t shortenedLength = length - removedCount;
        if (added.size() > slots_.max_size() - shortenedLength)
            rangeError(u"Array length exceeds the native array length limit");
        const std::size_t finalLength = shortenedLength + added.size();

        auto removed = std::make_shared<ChildArray>();
        removed->resize(removedCount);
        for (std::size_t offset = 0; offset < removedCount; ++offset) {
            const std::size_t sourceIndex = first + offset;
            if (has(sourceIndex)) removed->set(offset, get(sourceIndex));
        }

        if (added.size() < removedCount) {
            std::size_t sourceIndex = first + removedCount;
            std::size_t targetIndex = first + added.size();
            while (sourceIndex < length) {
                if (has(sourceIndex)) set(targetIndex, get(sourceIndex));
                else erase(targetIndex);
                ++sourceIndex;
                ++targetIndex;
            }
        } else if (added.size() > removedCount) {
            std::size_t sourceIndex = length - removedCount;
            while (sourceIndex > first) {
                const std::size_t from = sourceIndex + removedCount - 1;
                const std::size_t to = sourceIndex + added.size() - 1;
                if (has(from)) set(to, get(from));
                else erase(to);
                --sourceIndex;
            }
        }

        for (std::size_t offset = 0; offset < added.size(); ++offset) {
            set(first + offset, added[offset]);
        }
        resize(finalLength);
        return removed;
    }
    std::optional<std::size_t> indexOf(Value value,std::int64_t fromIndex=0)const {
        const std::size_t length = slots_.size();
        const std::size_t first = relativeIndex(fromIndex, length);
        for (std::size_t index = first; index < length; ++index) {
            if (has(index) && get(index) == value) return index;
        }
        return {};
    }
    void forEach(const Each& callback) {
        if (!callback) typeError(u"Array callback is not callable");
        const auto keepAlive = this->weak_from_this().lock();
        (void)keepAlive;
        const std::size_t length = slots_.size();
        for (std::size_t index = 0; index < length; ++index) {
            if (has(index)) callback(get(index), index, *this);
        }
    }
    void setPrototype(Handle prototype) {
        for (auto current = prototype; current; current = current->prototype_) {
            if (current.get() == this) {
                typeError(u"Cyclic array prototype value");
            }
        }
        prototype_ = std::move(prototype);
    }
    Handle prototype()const{return prototype_;}
};
}
