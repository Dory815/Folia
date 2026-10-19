// Owner for the live AbstractFactory nodeMap field.
// The default-node vector is only the constructor input. After construction,
// this Value-keyed stable-slot Map is the one mutable owner.
export module folia.math.core.tree.factory_class_map_owner;
export import folia.math.core.tree.factory_types;
export import folia.math.util.ordered_map_storage;
import folia.math.util.property_host;
import std;

export namespace folia::math::tree {
struct FactoryMapSameValueZero {
    bool operator()(const options::Value& left,const options::Value& right)const {
        const auto* a=std::get_if<double>(&left.data);
        const auto* b=std::get_if<double>(&right.data);
        if(a&&b)return *a==*b||(std::isnan(*a)&&std::isnan(*b));
        return left==right;
    }
};

template<class Product> struct FactoryClassMapValue {
    std::variant<FactoryClassRef<Product>,options::Value> value;
    static FactoryClassMapValue native(FactoryClassRef<Product> type){return {std::move(type)};}
    static FactoryClassMapValue source(options::Value raw){return {std::move(raw)};}
    const FactoryClassRef<Product>* nativeClass()const noexcept{
        return std::get_if<FactoryClassRef<Product>>(&value);
    }
    const options::Value* sourceValue()const noexcept{
        return std::get_if<options::Value>(&value);
    }
};

template<class Product>
class FactoryClassMapAccess:public OrderedMapAccess<options::Value,FactoryClassMapValue<Product>> {
public:
    virtual std::optional<options::Value> keyAt(std::size_t slot)const=0;
    virtual std::optional<options::Value> sourceValue()const{return std::nullopt;}
    virtual void sourceValue(options::Value){}
    virtual std::vector<options::Value> keysValues()const{
        std::vector<options::Value> result;
        result.reserve(this->size());
        for(std::size_t slot=0;slot<this->slotCount();++slot)if(auto key=keyAt(slot))result.push_back(std::move(*key));
        return result;
    }
    virtual FactoryClassRef<Product> resolveValue(const FactoryClassMapValue<Product>& value)const{
        if(const auto* native=value.nativeClass())return *native;
        throw options::Exception("TypeError",u"nodeMap entry is not a registered factory class");
    }
    virtual FactoryClassRef<Product> resolveClass(const options::Value& key)const{
        const auto value=this->get(key);
        if(!value)return {};
        return resolveValue(*value);
    }
};

template<class Product>
class FactoryClassMapOwner final:public FactoryClassMapAccess<Product> {
    using Storage=OrderedMapStorage<options::Value,FactoryClassMapValue<Product>,FactoryMapSameValueZero>;
    using Access=OrderedMapAccess<options::Value,FactoryClassMapValue<Product>>;
    std::shared_ptr<Storage> storage_=std::make_shared<Storage>();
public:
    using Entry=typename Access::Entry;
    FactoryClassMapOwner()=default;
    explicit FactoryClassMapOwner(FactoryClassMap<Product> initial){
        // FactoryClassMap is the initial Object.keys-style input. Its order is
        // preserved as inserted; live mutations use only this stable-slot Map.
        for(auto& [kind,type]:initial)
            storage_->set(options::Value(property_host::text(kind)),FactoryClassMapValue<Product>::native(std::move(type)));
    }
    const void* identity()const noexcept override{return storage_->identity();}
    std::size_t size()const noexcept override{return storage_->size();}
    std::size_t slotCount()const noexcept override{return storage_->slotCount();}
    std::optional<Entry> entryAt(std::size_t slot)const override{return storage_->entryAt(slot);}
    std::optional<options::Value> keyAt(std::size_t slot)const override{
        const auto entry=storage_->entryAt(slot);
        return entry?std::optional<options::Value>(entry->key):std::nullopt;
    }
    std::optional<FactoryClassMapValue<Product>> get(const options::Value& key)const override{
        return storage_->get(key);
    }
    bool has(const options::Value& key)const override{return storage_->has(key);}
    void set(options::Value key,FactoryClassMapValue<Product> value)override{
        storage_->set(std::move(key),std::move(value));
    }
    bool erase(const options::Value& key)override{return storage_->erase(key);}
    void clear()noexcept override{storage_->clear();}

    FactoryClassRef<Product> getKind(std::string_view kind)const{
        return this->resolveClass(options::Value(property_host::text(kind)));
    }
    void setKind(std::string_view kind,FactoryClassRef<Product> type){
        storage_->set(options::Value(property_host::text(kind)),FactoryClassMapValue<Product>::native(std::move(type)));
    }
    bool eraseKind(std::string_view kind){
        return storage_->erase(options::Value(property_host::text(kind)));
    }
};
}
