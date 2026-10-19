// Per-Context ECMAScript Map objects over one insertion-ordered storage.
export module folia.math.util.map_bindings;
export import folia.math.util.option_types;
export import folia.math.util.ordered_map_storage;
import std;

export namespace folia::math::options {
struct MapEntry {
    Value key;
    Value value;
    bool live=true;
};
class MapStorage {
public:
    struct EntriesView {
        const MapStorage* storage=nullptr;
        class const_iterator {
            const MapStorage* storage_=nullptr;
            mutable std::size_t slot_=0;
            bool end_sentinel_=false;
            void skip()const{
                while(storage_&&slot_<storage_->slotCount()&&!storage_->entryAt(slot_))++slot_;
            }
            friend struct EntriesView;
            const_iterator(const MapStorage* storage,std::size_t slot,bool endSentinel)
                :storage_(storage),slot_(slot),end_sentinel_(endSentinel){skip();}
        public:
            using iterator_category=std::input_iterator_tag;
            using value_type=MapEntry;
            using difference_type=std::ptrdiff_t;
            using pointer=void;
            using reference=MapEntry;
            const_iterator()=default;
            reference operator*()const{
                if(!storage_)throw std::out_of_range("Map entries iterator has no storage");
                const auto entry=storage_->entryAt(slot_);
                if(!entry)throw std::out_of_range("Map entries iterator is not dereferenceable");
                return {entry->first,entry->second,true};
            }
            const_iterator& operator++(){++slot_;skip();return *this;}
            const_iterator operator++(int){auto previous=*this;++*this;return previous;}
            bool operator==(const const_iterator& other)const{
                if(storage_!=other.storage_)return false;
                if(end_sentinel_&&other.end_sentinel_)return true;
                if(end_sentinel_){other.skip();return !storage_||other.slot_>=storage_->slotCount();}
                if(other.end_sentinel_){skip();return !storage_||slot_>=storage_->slotCount();}
                skip();other.skip();return slot_==other.slot_;
            }
        };
        std::size_t size()const noexcept;
        MapEntry operator[](std::size_t slot)const;
        const_iterator begin()const{return const_iterator(storage,0,false);}
        const_iterator end()const{return const_iterator(storage,0,true);}
    };
    virtual ~MapStorage()=default;
    virtual const void* identity()const noexcept=0;
    virtual std::size_t size()const noexcept=0;
    virtual std::size_t slotCount()const noexcept=0;
    virtual std::optional<std::pair<Value,Value>> entryAt(std::size_t slot)const=0;
    // Raw non-observing payload edge enumeration. Implementations unable to
    // prove completeness return false; Context then pins the Map owner.
    virtual bool visitContextEdges(const ContextEdgeVisitor&)const{return false;}
    virtual Value get(const Value& key)const=0;
    virtual bool has(const Value& key)const=0;
    virtual void set(Value key,Value value)=0;
    virtual bool erase(const Value& key)=0;
    virtual void clear()noexcept=0;
    EntriesView entries()const noexcept{return {this};}
};
using MapStorageRef=std::shared_ptr<MapStorage>;

// A live, type-erased view over an existing native ordered map. The adapter
// owns no entries: conversion happens at each read or write boundary.
template<class Key,class Mapped,class Equal=std::equal_to<Key>>
class ProjectedMapStorage final:public MapStorage {
public:
    using NativeStorage=folia::math::OrderedMapAccess<Key,Mapped>;
    using KeyFromValue=std::function<Key(const Value&)>;
    using KeyToValue=std::function<Value(const Key&)>;
    using MappedFromValue=std::function<Mapped(const Value&)>;
    using MappedToValue=std::function<Value(const Mapped&)>;
    using NormalizeKey=std::function<Key(Key)>;
private:
    std::shared_ptr<NativeStorage> storage_;
    KeyFromValue keyFromValue_;
    KeyToValue keyToValue_;
    MappedFromValue mappedFromValue_;
    MappedToValue mappedToValue_;
    NormalizeKey normalizeKey_;
public:
    ProjectedMapStorage(std::shared_ptr<NativeStorage> storage,
                        KeyFromValue keyFromValue,KeyToValue keyToValue,
                        MappedFromValue mappedFromValue,MappedToValue mappedToValue,
                        NormalizeKey normalizeKey={})
        :storage_(std::move(storage)),keyFromValue_(std::move(keyFromValue)),
         keyToValue_(std::move(keyToValue)),mappedFromValue_(std::move(mappedFromValue)),
         mappedToValue_(std::move(mappedToValue)),normalizeKey_(std::move(normalizeKey)){
        if(!storage_)throw std::invalid_argument("Projected Map storage is null");
        if(!keyFromValue_||!keyToValue_||!mappedFromValue_||!mappedToValue_)
            throw std::invalid_argument("Projected Map converters must be provided");
    }
    const void* identity()const noexcept override{return storage_->identity();}
    std::shared_ptr<NativeStorage> nativeStorage()const noexcept{return storage_;}
    std::size_t size()const noexcept override{return storage_->size();}
    std::size_t slotCount()const noexcept override{return storage_->slotCount();}
    std::optional<std::pair<Value,Value>> entryAt(std::size_t slot)const override{
        auto entry=storage_->entryAt(slot);
        if(!entry)return std::nullopt;
        return std::pair<Value,Value>{keyToValue_(entry->key),mappedToValue_(entry->value)};
    }
    Value get(const Value& key)const override{
        const Key nativeKey=keyFromValue_(key);
        auto value=storage_->get(nativeKey);
        if(!value)return Value(Undefined{});
        // A converter can re-enter Context and mutate/reallocate the native
        // store, so never pass it a reference into a store slot.
        return mappedToValue_(*value);
    }
    bool has(const Value& key)const override{return storage_->has(keyFromValue_(key));}
    void set(Value key,Value value)override{
        Key nativeKey=keyFromValue_(key);
        if(normalizeKey_)nativeKey=normalizeKey_(std::move(nativeKey));
        storage_->set(std::move(nativeKey),mappedFromValue_(value));
    }
    bool erase(const Value& key)override{return storage_->erase(keyFromValue_(key));}
    void clear()noexcept override{storage_->clear();}
};
template<class Key,class Mapped,class Equal=std::equal_to<Key>>
using ProjectedMapStorageRef=std::shared_ptr<ProjectedMapStorage<Key,Mapped,Equal>>;

class ValueMapStorage final:public MapStorage {
    using NativeStorage=folia::math::OrderedMapStorage<Value,Value,std::function<bool(const Value&,const Value&)>>;
    std::shared_ptr<NativeStorage> storage_;
public:
    ValueMapStorage();
    const void* identity()const noexcept override;
    std::size_t size()const noexcept override;
    std::size_t slotCount()const noexcept override;
    std::optional<std::pair<Value,Value>> entryAt(std::size_t slot)const override;
    bool visitContextEdges(const ContextEdgeVisitor&)const override;
    Value get(const Value& key)const override;
    bool has(const Value& key)const override;
    void set(Value key,Value value)override;
    bool erase(const Value& key)override;
    void clear()noexcept override;
};

// Owns the actual Map constructor/prototype and binds each native storage to
// one Context object. Objects and iterators retain the original storage; no
// entry snapshots or copied registries are created at the Source boundary.
class MapContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    explicit MapContextBindings(Context& context);
    ~MapContextBindings();
    MapContextBindings(const MapContextBindings&)=delete;
    MapContextBindings& operator=(const MapContextBindings&)=delete;
    Context& context()const;
    ObjectRef constructor()const;
    ObjectRef prototype()const;
    MapStorageRef createStorage()const;
    ObjectRef bind(const MapStorageRef& storage);
    ObjectRef create(Value iterable=Undefined{});
    MapStorageRef resolve(ObjectRef object)const;
    Value get(ObjectRef object,Value key)const;
    bool has(ObjectRef object,Value key)const;
    ObjectRef set(ObjectRef object,Value key,Value value)const;
    bool erase(ObjectRef object,Value key)const;
    void clear(ObjectRef object)const;
    std::size_t size(ObjectRef object)const;
    std::size_t canonicalStorageCountForDiagnostics()const noexcept;
};
}
