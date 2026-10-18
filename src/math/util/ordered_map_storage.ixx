// One insertion-ordered map storage shared by native owners and Context Map
// projections. Deleted slots release their values while retaining positions
// for live iterators; reinsertion appends a new slot.
export module folia.math.util.ordered_map_storage;
import std;

export namespace folia::math {
template<class Key,class Value>
class OrderedMapAccess {
public:
    struct Entry{Key key;Value value;};
    class const_iterator {
        const OrderedMapAccess* owner_=nullptr;
        mutable std::size_t slot_=0;
        bool end_sentinel_=false;
        void skip()const{
            while(owner_&&slot_<owner_->slotCount()&&!owner_->entryAt(slot_))++slot_;
        }
        friend class OrderedMapAccess;
        const_iterator(const OrderedMapAccess* owner,std::size_t slot,bool endSentinel)
            :owner_(owner),slot_(slot),end_sentinel_(endSentinel){skip();}
    public:
        using iterator_category=std::input_iterator_tag;
        using value_type=Entry;
        using difference_type=std::ptrdiff_t;
        using pointer=void;
        using reference=Entry;
        const_iterator()=default;
        reference operator*()const{
            if(!owner_)throw std::out_of_range("OrderedMapAccess iterator has no owner");
            auto entry=owner_->entryAt(slot_);
            if(!entry)throw std::out_of_range("OrderedMapAccess iterator is not dereferenceable");
            return std::move(*entry);
        }
        const_iterator& operator++(){++slot_;skip();return *this;}
        const_iterator operator++(int){auto previous=*this;++*this;return previous;}
        bool operator==(const const_iterator& other)const{
            if(owner_!=other.owner_)return false;
            if(end_sentinel_&&other.end_sentinel_)return true;
            if(end_sentinel_){other.skip();return !owner_||other.slot_>=owner_->slotCount();}
            if(other.end_sentinel_){skip();return !owner_||slot_>=owner_->slotCount();}
            skip();other.skip();
            return slot_==other.slot_;
        }
    };
    virtual ~OrderedMapAccess()=default;
    virtual const void* identity()const noexcept=0;
    virtual std::size_t size()const noexcept=0;
    virtual std::size_t slotCount()const noexcept=0;
    virtual std::optional<Entry> entryAt(std::size_t slot)const=0;
    // Non-observing raw-owner enumeration. Unknown implementations remain
    // incomplete rather than invoking getters to discover hidden edges.
    virtual bool visitRawEntries(const std::function<void(const Key&,const Value&)>&)const{return false;}
    virtual std::optional<Value> get(const Key& key)const=0;
    virtual bool has(const Key& key)const=0;
    virtual void set(Key key,Value value)=0;
    virtual bool erase(const Key& key)=0;
    virtual void clear()noexcept=0;
    const_iterator begin()const{return const_iterator(this,0,false);}
    const_iterator end()const{return const_iterator(this,0,true);}
};

template<class Key, class Value, class Equal = std::equal_to<Key>>
class OrderedMapStorage:public OrderedMapAccess<Key,Value> {
public:
    using Entry=typename OrderedMapAccess<Key,Value>::Entry;

private:
    std::vector<std::optional<Entry>> slots_;
    std::size_t size_ = 0;
    [[no_unique_address]] Equal equal_{};

public:
    OrderedMapStorage()=default;
    explicit OrderedMapStorage(Equal equal):equal_(std::move(equal)){}

    const void* identity()const noexcept override{return this;}
    std::size_t size() const noexcept override { return size_; }
    std::size_t slotCount() const noexcept override { return slots_.size(); }

    std::optional<Entry> entryAt(std::size_t slot) const override {
        if (slot >= slots_.size() || !slots_[slot]) return std::nullopt;
        return *slots_[slot];
    }
    bool visitRawEntries(const std::function<void(const Key&,const Value&)>& visitor)const override{
        for(const auto& slot:slots_)if(slot)visitor(slot->key,slot->value);
        return true;
    }

    const Value* find(const Key& key) const {
        for (const auto& slot : slots_) {
            if (slot && equal_(slot->key, key)) return &slot->value;
        }
        return nullptr;
    }

    Value* find(const Key& key) {
        for (auto& slot : slots_) {
            if (slot && equal_(slot->key, key)) return &slot->value;
        }
        return nullptr;
    }

    std::optional<Value> get(const Key& key)const override{
        if(const auto* value=find(key))return *value;
        return std::nullopt;
    }

    bool has(const Key& key) const override { return find(key) != nullptr; }

    void set(Key key, Value value) override {
        for (auto& slot : slots_) {
            if (slot && equal_(slot->key, key)) {
                slot->value = std::move(value);
                return;
            }
        }
        slots_.emplace_back(Entry{std::move(key), std::move(value)});
        ++size_;
    }

    bool erase(const Key& key) override {
        for (auto& slot : slots_) {
            if (slot && equal_(slot->key, key)) {
                slot.reset();
                --size_;
                return true;
            }
        }
        return false;
    }

    void clear() noexcept override {
        for (auto& slot : slots_) slot.reset();
        size_ = 0;
    }
};
}
