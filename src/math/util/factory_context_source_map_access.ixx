// Candidate generic adapter for any Source receiver used as a writable
// factory nodeMap. It keeps the raw receiver Value and dispatches live Map
// methods only when a typed native operation consumes the field.
export module folia.math.util.factory_context_source_map_access;
export import folia.math.core.tree.factory_class_map_owner;
import std;

export namespace folia::math::tree {
template<class Product>
class SourceFactoryClassMapAccess final : public FactoryClassMapAccess<Product> {
public:
    using Mapped=FactoryClassMapValue<Product>;
    using Entry=typename OrderedMapAccess<options::Value,Mapped>::Entry;
    using Call=std::function<options::Value(const options::Value&,std::u16string_view,
                                             std::span<const options::Value>)>;
    using Keys=std::function<std::vector<options::Value>(const options::Value&)>;
    struct StableIdentity {
        const void* value=nullptr;
        std::shared_ptr<const void> owner;
    };
    using Identity=std::function<StableIdentity(const options::Value&)>;
    using EnsureActive=std::function<void()>;
    using Resolve=std::function<FactoryClassRef<Product>(const options::Value&)>;
    using ToSource=std::function<options::Value(const Mapped&)>;
private:
    options::Value receiver_;
    Call call_;
    Keys keys_;
    Identity identity_;
    StableIdentity stableIdentity_;
    EnsureActive ensureActive_;
    Resolve resolve_;
    ToSource toSource_;
    options::Value call(std::u16string_view method,
                        std::span<const options::Value> args={})const {
        ensureActive_();
        return call_(receiver_,method,args);
    }
    std::vector<options::Value> keys()const {
        ensureActive_();
        return keys_(receiver_);
    }
    options::Value valueFor(const Mapped& value)const {
        if(const auto* raw=value.sourceValue())return *raw;
        return toSource_(value);
    }
public:
    SourceFactoryClassMapAccess(options::Value receiver,Call call,Keys keys,
                                Identity identity,EnsureActive ensureActive,
                                Resolve resolve,ToSource toSource)
        :receiver_(std::move(receiver)),call_(std::move(call)),keys_(std::move(keys)),
         identity_(std::move(identity)),ensureActive_(std::move(ensureActive)),
         resolve_(std::move(resolve)),toSource_(std::move(toSource)){
        if(!call_||!keys_||!identity_||!ensureActive_||!resolve_||!toSource_)
            throw options::Exception("TypeError",u"Source nodeMap adapter requires live Context operations");
        ensureActive_();
        stableIdentity_=identity_(receiver_);
        if(!stableIdentity_.value)stableIdentity_.value=this;
    }
    const void* identity()const noexcept override {
        return stableIdentity_.value;
    }
    std::size_t size()const noexcept override {
        try{return keys().size();}catch(...){return 0;}
    }
    std::size_t slotCount()const noexcept override{return size();}
    std::vector<options::Value> keysValues()const override{return keys();}
    std::optional<options::Value> keyAt(std::size_t slot)const override {
        const auto entries=keys();
        if(slot>=entries.size())return std::nullopt;
        return entries[slot];
    }
    std::optional<Entry> entryAt(std::size_t slot)const override {
        const auto key=keyAt(slot);
        if(!key)return std::nullopt;
        const std::array<options::Value,1> args{*key};
        return Entry{*key,Mapped::source(call(u"get",args))};
    }
    std::optional<Mapped> get(const options::Value& key)const override {
        const std::array<options::Value,1> args{key};
        return Mapped::source(call(u"get",args));
    }
    bool has(const options::Value& key)const override {
        const std::array<options::Value,1> args{key};
        const auto result=call(u"has",args);
        const auto* boolean=std::get_if<bool>(&result.data);
        return boolean&&*boolean;
    }
    void set(options::Value key,Mapped value)override {
        const std::array<options::Value,2> args{std::move(key),valueFor(value)};
        (void)call(u"set",args);
    }
    bool erase(const options::Value& key)override {
        const std::array<options::Value,1> args{key};
        const auto result=call(u"delete",args);
        const auto* boolean=std::get_if<bool>(&result.data);
        return boolean&&*boolean;
    }
    void clear()noexcept override {try{(void)call(u"clear");}catch(...){} }
    FactoryClassRef<Product> resolveValue(const Mapped& mapped)const override {
        if(const auto* native=mapped.nativeClass())return *native;
        const auto* raw=mapped.sourceValue();
        if(!raw||std::holds_alternative<options::Undefined>(raw->data))return {};
        if(auto resolved=resolve_(*raw))return resolved;
        throw options::Exception("TypeError",u"nodeMap value is not a registered factory class");
    }
    FactoryClassRef<Product> resolveClass(const options::Value& key)const override {
        const auto mapped=get(key);
        return mapped?resolveValue(*mapped):FactoryClassRef<Product>{};
    }
    std::optional<options::Value> sourceValue()const override{return receiver_;}
    void sourceValue(options::Value value)override{
        ensureActive_();
        auto nextIdentity=identity_(value);
        if(!nextIdentity.value)nextIdentity.value=this;
        receiver_=std::move(value);
        stableIdentity_=std::move(nextIdentity);
    }
};
}
