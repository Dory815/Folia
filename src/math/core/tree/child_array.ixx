// Root-frozen Source Array binding for factory-borrowed mathematical nodes.
export module folia.math.core.tree.child_array;
export import folia.math.core.tree.property;
export import folia.math.util.option_types;
import std;
export namespace folia::math::tree {
enum class ChildKind { Undefined, Null, Node };
template<class Product> class ChildValue;
template<class Product> class ChildArray;
template<class Product> using TreeRawVisitor=std::function<void(ChildValue<Product>,const std::any&)>;
// Source-backed nodes retain the exact Value and borrow behavior through the
// Context that owns it. This protocol contains no tree algorithms or storage;
// ContextTreeNodeBindings implements it by resolving the current property and
// method on that original object at every operation.
template<class Product> class TreeNodeAccess {
public:
    virtual ~TreeNodeAccess()=default;
    virtual options::Value sourceValue()const=0;
    virtual Product* nativeProduct()const noexcept=0;
    virtual options::Value get(std::u16string_view key)const=0;
    virtual void set(std::u16string_view key,options::Value value)const=0;
    // Invoke a callable value already selected by Source Get, preserving the
    // exact receiver without looking the member up a second time.
    virtual options::Value invoke(options::Value callable,
        std::span<const options::Value> arguments={})const=0;
    // Primitive operations on exact raw values read from this Context. These
    // keep an operation-time provider/lease when a native algorithm follows a
    // foreign value returned by Get.
    virtual options::Value getValue(const options::Value& receiver,
        std::u16string_view key)const=0;
    virtual void setValue(const options::Value& receiver,std::u16string_view key,
        options::Value value)const=0;
    // Source assignment with an arbitrary property-key Value. This keeps
    // Symbol keys intact instead of routing them through a string-only native
    // property API.
    virtual void setPropertyValue(const options::Value& receiver,
        const options::Value& key,options::Value value)const=0;
    virtual options::Value invokeWithThis(options::Value callable,
        const options::Value& receiver,std::span<const options::Value> arguments={})const=0;
    virtual std::shared_ptr<ChildArray<Product>> arrayView(options::Value value)const=0;
    virtual void forOfValue(const options::Value& value,
        const std::function<void(options::Value)>& callback)const=0;
    virtual double toNumber(const options::Value& value)const=0;
    // JavaScript ToPropertyKey (string hint, with Symbols preserved).
    virtual Property toPropertyKey(const options::Value& value)const=0;
    virtual bool truthyValue(const options::Value& value)const=0;
    virtual bool sameValue(const options::Value& left,const options::Value& right)const=0;
    virtual std::string stringValue(const options::Value& value)const=0;
    virtual options::Value copyEnumerableValue(const options::Value& value)const=0;
    virtual std::vector<std::u16string> ownEnumerableStringKeys(const options::Value& value)const=0;
    virtual options::Value makeCallback(
        const std::function<options::Value(std::span<const options::Value>)>& body)const=0;
    virtual ChildValue<Product> childValue(options::Value value)const=0;
    virtual Property propertyValue(options::Value value)const=0;
    virtual options::Value call(std::u16string_view key,
        std::span<const options::Value> arguments={})const=0;
    virtual std::shared_ptr<ChildArray<Product>> children()const=0;
    virtual bool sameValue(const TreeNodeAccess<Product>& other)const=0;
    virtual bool truthy()const=0;
    virtual ChildValue<Product> parent()const=0;
    virtual void parent(ChildValue<Product> value)const=0;
    virtual bool isKind(std::string_view kind)const=0;
    virtual ChildValue<Product> copy()const=0;
    virtual std::any walkTree(const std::function<void(Product*,const std::any&)>&,
                              const std::any& data={})const=0;
    virtual std::any walkTreeRaw(const TreeRawVisitor<Product>&,const std::any& data={},
                                 const std::any& visitorIdentity={})const=0;
    virtual std::string toString()const=0;
};
template<class Product> using TreeNodeAccessRef=std::shared_ptr<TreeNodeAccess<Product>>;
template<class Product> class ChildValue {
    std::variant<Missing,PropertyNull,Product*,TreeNodeAccessRef<Product>> value_ = Missing{};
public:
    ChildValue()=default;
    ChildValue(Missing):value_(Missing{}){}
    ChildValue(PropertyNull):value_(PropertyNull{}){}
    ChildValue(std::nullptr_t):value_(PropertyNull{}){}
    ChildValue(Product* node):value_(node?decltype(value_)(node):decltype(value_)(PropertyNull{})){}
    ChildValue(TreeNodeAccessRef<Product> source)
        :value_(source?decltype(value_)(std::move(source)):decltype(value_)(Missing{})){}
    static ChildValue source(TreeNodeAccessRef<Product> access){return ChildValue(std::move(access));}
    ChildKind kind()const {
        if(std::holds_alternative<Missing>(value_))return ChildKind::Undefined;
        if(std::holds_alternative<PropertyNull>(value_))return ChildKind::Null;
        return ChildKind::Node;
    }
    bool isNode()const{
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return *access&&(*access)->truthy();
        return kind()==ChildKind::Node;
    }
    bool isSourceBacked()const{return std::holds_alternative<TreeNodeAccessRef<Product>>(value_);}
    TreeNodeAccessRef<Product> sourceAccess()const{
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return *access;
        return {};
    }
    options::Value sourceValue()const{
        if(const auto* node=std::get_if<Product*>(&value_)){
            if constexpr(requires(const Product& product){product.sourceValue();})
                return (*node)->sourceValue();
            throw options::Exception("Unimplemented",u"Native tree node has no Context identity");
        }
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return (*access)->sourceValue();
        if(std::holds_alternative<PropertyNull>(value_))return options::Null{};
        return options::Undefined{};
    }
    bool truthy()const{
        if(const auto* node=std::get_if<Product*>(&value_))return *node!=nullptr;
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return *access&&(*access)->truthy();
        return false;
    }
    ChildValue getParent()const{
        if(const auto* node=std::get_if<Product*>(&value_))return (*node)->getParent();
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return (*access)->parent();
        const auto message=std::string("Cannot read properties of ")+
            (kind()==ChildKind::Null?"null":"undefined")+" (reading 'parent')";
        throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
    }
    void setParent(ChildValue value)const{
        if(const auto* node=std::get_if<Product*>(&value_)){(*node)->setParent(std::move(value));return;}
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_)){(*access)->parent(std::move(value));return;}
        const auto message=std::string("Cannot set properties of ")+
            (kind()==ChildKind::Null?"null":"undefined")+" (setting 'parent')";
        throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
    }
    ChildValue copy()const{
        if(const auto* node=std::get_if<Product*>(&value_))return (*node)->copy();
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return (*access)->copy();
        const auto message=std::string("Cannot read properties of ")+
            (kind()==ChildKind::Null?"null":"undefined")+" (reading 'copy')";
        throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
    }
    std::any walkTree(const std::function<void(Product*,const std::any&)>& visit,const std::any& data={})const{
        if(const auto* node=std::get_if<Product*>(&value_))return (*node)->walkTree(visit,data);
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return (*access)->walkTree(visit,data);
        const auto message=std::string("Cannot read properties of ")+
            (kind()==ChildKind::Null?"null":"undefined")+" (reading 'walkTree')";
        throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
    }
    std::any walkTreeRaw(const TreeRawVisitor<Product>& visit,const std::any& data={},
                         const std::any& visitorIdentity={})const{
        if(const auto* node=std::get_if<Product*>(&value_))return (*node)->walkTreeRaw(visit,data,visitorIdentity);
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return (*access)->walkTreeRaw(visit,data,visitorIdentity);
        const auto message=std::string("Cannot read properties of ")+
            (kind()==ChildKind::Null?"null":"undefined")+" (reading 'walkTree')";
        throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
    }
    std::string toString()const{
        if(const auto* node=std::get_if<Product*>(&value_)){
            if constexpr(requires(const Product& product){{product.toString()}->std::convertible_to<std::string>;})
                return (*node)->toString();
            throw options::Exception("TypeError",u"Native tree value has no string conversion");
        }
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return (*access)->toString();
        return {};
    }
    std::string joinString()const{
        if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return (*access)->toString();
        if(const auto* node=std::get_if<Product*>(&value_)){
            if constexpr(requires(const Product& product){{product.toString()}->std::convertible_to<std::string>;})
                return (*node)->toString();
            throw options::Exception("TypeError",u"Native tree value has no string conversion");
        }
        return {};
    }
    // Explicit truthiness/RTTI projection only, never an implicit pointer cast.
    Product* nodeOrNull()const{
        if(auto* p=std::get_if<Product*>(&value_))return *p;
        if(auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))return *access?(*access)->nativeProduct():nullptr;
        return nullptr;
    }
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
    bool operator==(const ChildValue& other)const{
        if(value_.index()!=other.value_.index()){
            // A native node and its live Context view name the same object.
            // Null/undefined and unrelated foreign objects keep their own
            // identities; nativeProduct performs no Source coercion.
            if(const auto* node=std::get_if<Product*>(&value_))
                if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&other.value_))
                    return *node&&*access&&(*access)->nativeProduct()==*node;
            if(const auto* access=std::get_if<TreeNodeAccessRef<Product>>(&value_))
                if(const auto* node=std::get_if<Product*>(&other.value_))
                    return *node&&*access&&(*access)->nativeProduct()==*node;
            return false;
        }
        if(const auto* lhs=std::get_if<TreeNodeAccessRef<Product>>(&value_))
            return *lhs&&std::get<TreeNodeAccessRef<Product>>(other.value_)&&
                (*lhs)->sameValue(*std::get<TreeNodeAccessRef<Product>>(other.value_));
        return value_==other.value_;
    }
};
template<class Product> class ChildArray : public std::enable_shared_from_this<ChildArray<Product>> {
public:
    virtual ~ChildArray()=default;
    using Value=ChildValue<Product>;
    using Handle=std::shared_ptr<ChildArray>;
    using Each=std::function<void(Value,std::size_t,ChildArray&)>;
    using ForOf=std::function<void(Value)>;
    virtual const void* identity()const noexcept{return this;}
    virtual std::optional<options::Value> sourceValue()const{return {};}
    // Exact Source array-like length for operations whose branch uses the raw
    // property (truthiness/strict equality) rather than ToLength.
    virtual std::optional<options::Value> rawLength()const{return {};}
    virtual std::string joinedString()const{
        const auto owner=this->identity();
        if(std::ranges::find(joinedOwners_,owner)!=joinedOwners_.end())return {};
        joinedOwners_.push_back(owner);
        struct Exit{std::vector<const void*>& values;~Exit(){values.pop_back();}} exit{joinedOwners_};
        const auto length=size();std::string result;
        for(std::size_t index=0;index<length;++index){if(index)result.push_back(',');result+=get(index).joinString();}
        return result;
    }
private:
    inline static thread_local std::vector<const void*> joinedOwners_;
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
    std::vector<std::pair<std::string,Value>> named_;
    Handle prototype_;
    static std::optional<std::uint32_t> propertyIndex(std::string_view key) {
        if (key.empty() || (key.size()>1 && key.front()=='0')) return {};
        std::uint64_t value=0;
        const auto [end,error]=std::from_chars(key.data(),key.data()+key.size(),value);
        if(error!=std::errc{} || end!=key.data()+key.size() || value>=4294967295ULL) return {};
        return static_cast<std::uint32_t>(value);
    }
public:
    ChildArray()=default;
    ChildArray(std::initializer_list<Value> values) {
        slots_.reserve(values.size());
        for (const auto& value : values) slots_.emplace_back(value);
    }
    ChildArray(const ChildArray&)=delete;ChildArray&operator=(const ChildArray&)=delete;
    ChildArray(ChildArray&&)=delete;ChildArray&operator=(ChildArray&&)=delete;
    virtual std::size_t size()const{return slots_.size();}
    virtual bool empty()const{return size()==0;}
    // Node.ts branches on the raw `children.length` truthiness before it
    // dynamically calls setChildren; this is distinct from ToLength/size for
    // foreign array-like receivers.
    virtual bool lengthTruthy()const{return !empty();}
    virtual bool hasOwn(std::size_t index)const {
        return index < slots_.size() && slots_[index].has_value();
    }
    virtual bool has(std::size_t index)const {
        if (hasOwn(index)) return true;
        return prototype_ && prototype_->has(index);
    }
    virtual Value get(std::size_t index)const {
        if (hasOwn(index)) return *slots_[index];
        if (prototype_) return prototype_->get(index);
        return Value(Missing{});
    }
    virtual void set(std::size_t index,Value value) {
        checkedLength(index, slots_.max_size() - 1, u"Array index");
        if (index >= slots_.size()) slots_.resize(index + 1);
        slots_[index] = std::move(value);
    }
    virtual void erase(std::size_t index) {
        if (index < slots_.size()) slots_[index].reset();
    }
    // Node-valued property access. Non-index keys are ordinary named slots;
    // they neither extend array length nor participate in indexed slice/pop.
    // Scalar length and Array/Object intrinsics belong to their typed hosts.
    bool hasOwnProperty(std::string_view key)const {
        if(auto index=propertyIndex(key))return hasOwn(*index);
        return std::ranges::any_of(named_,[&](const auto& entry){return entry.first==key;});
    }
    Value getProperty(std::string_view key)const {
        if(auto index=propertyIndex(key))return get(*index);
        const auto found=std::ranges::find_if(named_,[&](const auto& entry){return entry.first==key;});
        if(found!=named_.end())return found->second;
        return prototype_?prototype_->getProperty(key):Value(Missing{});
    }
    void setProperty(std::string key,Value value) {
        if(auto index=propertyIndex(key)){set(*index,std::move(value));return;}
        const auto found=std::ranges::find_if(named_,[&](const auto& entry){return entry.first==key;});
        if(found==named_.end())named_.emplace_back(std::move(key),std::move(value));
        else found->second=std::move(value);
    }
    void eraseProperty(std::string_view key) {
        if(auto index=propertyIndex(key)){erase(*index);return;}
        std::erase_if(named_,[&](const auto& entry){return entry.first==key;});
    }
    void resize(std::size_t length) {
        checkedLength(length, slots_.max_size(), u"Array length");
        slots_.resize(length);
    }
    virtual std::size_t push_back(Value value) {
        if (slots_.size() >= slots_.max_size()) rangeError(u"Array length exceeds the native array length limit");
        slots_.emplace_back(std::move(value));
        return slots_.size();
    }
    virtual Value pop_back() {
        if (slots_.empty()) return Value(Missing{});
        const Value value = get(slots_.size() - 1);
        slots_.pop_back();
        return value;
    }
    virtual Handle slice(std::int64_t start=0,std::optional<std::int64_t> end = {})const {
        const std::size_t length = size();
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
    virtual Handle sliceNumber(double start)const {
        const auto length=size();
        // ECMAScript ToIntegerOrInfinity, followed by relative-index clamping.
        const double integer=std::isnan(start)?0:std::trunc(start);
        const double relative=integer<0?std::max(static_cast<double>(length)+integer,0.0)
                                      :std::min(integer,static_cast<double>(length));
        const auto first=static_cast<std::size_t>(relative);
        auto result=std::make_shared<ChildArray>();result->resize(length-first);
        for(std::size_t index=first;index<length;++index)
            if(has(index))result->set(index-first,get(index));
        return result;
    }
    virtual Handle splice(std::int64_t start,std::optional<std::size_t> count = {},std::span<const Value> added = {}) {
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
    virtual std::optional<std::size_t> indexOf(Value value,std::int64_t fromIndex=0)const {
        const std::size_t length = slots_.size();
        const std::size_t first = relativeIndex(fromIndex, length);
        for (std::size_t index = first; index < length; ++index) {
            if (has(index) && get(index) == value) return index;
        }
        return {};
    }
    virtual void forEach(const Each& callback) {
        if (!callback) typeError(u"Array callback is not callable");
        const auto keepAlive = this->weak_from_this().lock();
        (void)keepAlive;
        const std::size_t length = slots_.size();
        for (std::size_t index = 0; index < length; ++index) {
            if (has(index)) callback(get(index), index, *this);
        }
    }
    // Array iteration used by the Source Tree algorithms is not Array#forEach:
    // holes yield undefined and the iterator observes live length as it runs.
    virtual void forOf(const ForOf& callback)const{
        if(!callback)typeError(u"Tree iterator callback is not callable");
        for(std::size_t index=0;index<size();++index)callback(get(index));
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
