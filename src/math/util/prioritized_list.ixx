/*************************************************************
 *
 *  Copyright (c) 2017-2026 The MathJax Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// Native port of MathJax 4.1.3 ts/util/PrioritizedList.ts.
export module folia.math.util.prioritized_list;
export import folia.math.util.object_types;
export import folia.math.util.options;
import std;

namespace folia::math::detail {
    inline double prioritized_list_default_priority = 5;
}

export namespace folia::math {

template<class T>
struct PrioritizedListItemContextBinding {
    options::Context* context=nullptr;
    options::Value record;
    std::weak_ptr<std::atomic_bool> lease;
    std::function<options::Value()> readItem,readPriority;
    std::function<void(T)> writeItem;
    std::function<void(options::Value)> writePriority;
    std::function<options::Value(std::span<const options::Value>)> invoke;
    std::function<options::Value(const std::any&)> convertArgument,encodeAux;
};

template<class T>
struct PrioritizedListItem : NativeObject {
    double priority;
    T item;
    bool hasRawItem=false,hasRawPriority=false;
    options::Value rawItem,rawPriority;
    bool rawArraySlot=false;
    options::Value rawSlotValue;
    std::shared_ptr<PrioritizedListItemContextBinding<T>> contextBinding;
    PrioritizedListItem()=default;
    PrioritizedListItem(double priority,T item)
        : priority(priority),item(std::move(item)) {}
};

template<class T> class PrioritizedListItemsAccess;
template<class T> using PrioritizedListItemsAccessRef=std::shared_ptr<PrioritizedListItemsAccess<T>>;
template<class T> class PrioritizedListItemHandle;

template<class T>
class PrioritizedListItemsAccess {
public:
    using Storage=std::vector<std::shared_ptr<PrioritizedListItem<T>>>;
    using StorageRef=std::shared_ptr<Storage>;
    virtual ~PrioritizedListItemsAccess()=default;
    virtual options::Value identity()const{return options::Undefined{};}
    virtual std::size_t length()=0;
    // Iterator.next reads an indexed value first, increments its cursor, then
    // asks this owner whether that incremented index is past live length.
    virtual bool doneAfterRead(std::size_t nextIndex){return static_cast<double>(nextIndex)>static_cast<double>(length());}
    virtual bool indexBeforeLength(double index){return index<static_cast<double>(length());}
    virtual PrioritizedListItemHandle<T> read(std::size_t)=0;
    virtual T add(T,double)=0;
    virtual void remove(const T&)=0;
    virtual void addRaw(options::Value,options::Value){throw std::logic_error("raw add is unsupported by this prioritized-list owner");}
    virtual void removeRaw(options::Value){throw std::logic_error("raw remove is unsupported by this prioritized-list owner");}
    virtual options::Value convertArgument(const std::any&){throw std::logic_error("argument conversion is unsupported by this prioritized-list owner");}
    virtual options::Value encodeAux(const std::any&){throw std::logic_error("auxiliary conversion is unsupported by this prioritized-list owner");}
    virtual bool less(const options::Value& left,const options::Value& right){
        if(const auto* a=std::get_if<double>(&left.data)){if(const auto* b=std::get_if<double>(&right.data))return *a<*b;}
        throw std::logic_error("prioritized-list owner has no Source comparison operation");
    }
    virtual StorageRef* nativeStorageSlot(){return nullptr;}
    virtual void setSlot(std::size_t,options::Value){throw std::logic_error("indexed write is unsupported by this owner");}
    virtual bool deleteSlot(std::size_t){throw std::logic_error("indexed delete is unsupported by this owner");}
    virtual void setLength(options::Value){throw std::logic_error("length write is unsupported by this owner");}
};

template<class T>
class PrioritizedListItemHandle {
public:
    using NativeRef=std::shared_ptr<PrioritizedListItem<T>>;
    using Identity=std::variant<NativeRef,options::Value>;
    using ItemValue=std::variant<T,options::Value>;
    struct ContextSlot {
        options::Context* context=nullptr;
        options::Value record;
        std::weak_ptr<std::atomic_bool> lease;
        bool present=true;
        std::function<ItemValue()> readItem;
        std::function<options::Value()> readPriority;
        std::function<void(T)> writeItem;
        std::function<void(options::Value)> writePriority;
        std::function<options::Value(std::span<const options::Value>)> invoke;
        std::function<options::Value(const std::any&)> convertArgument;
        std::function<options::Value(const std::any&)> encodeAux;
    };
    using Storage=std::variant<NativeRef,ContextSlot>;
    PrioritizedListItemHandle()=default;
    PrioritizedListItemHandle(NativeRef value):storage_(std::move(value)){}
    PrioritizedListItemHandle(ContextSlot value):storage_(std::move(value)){}
    bool native()const noexcept{return std::holds_alternative<NativeRef>(storage_);}
    NativeRef nativeRecord()const{return std::get<NativeRef>(storage_);}
    const Storage& storage()const noexcept{return storage_;}
    bool contextBacked()const{return native()?bool(nativeRecord()&&nativeRecord()->contextBinding):false;}
    bool present()const{return native()?bool(std::get<NativeRef>(storage_)):std::get<ContextSlot>(storage_).present;}
    Identity recordIdentity()const{if(native()){auto item=nativeRecord();if(!item)throw std::logic_error("null prioritized-list record");if(item->rawArraySlot)return item->rawSlotValue;if(item->contextBinding)return item->contextBinding->record;return item;}validate();return std::get<ContextSlot>(storage_).record;}
    ItemValue readItem()const{if(native()){auto item=nativeRecord();if(!item)throw std::logic_error("null prioritized-list record");if(item->contextBinding)return item->contextBinding->readItem();if(item->hasRawItem)return item->rawItem;return item->item;}validate();const auto&slot=std::get<ContextSlot>(storage_);return slot.readItem?slot.readItem():options::Undefined{};}
    options::Value readPriorityValue()const{if(native()){auto item=nativeRecord();if(!item)throw std::logic_error("null prioritized-list record");if(item->contextBinding)return item->contextBinding->readPriority();return item->hasRawPriority?item->rawPriority:options::Value(item->priority);}validate();const auto&slot=std::get<ContextSlot>(storage_);return slot.readPriority?slot.readPriority():options::Value(options::Undefined{});}
    double readPriority()const{if(native()){auto item=nativeRecord();if(!item)throw std::logic_error("null prioritized-list record");if(item->contextBinding)return item->contextBinding->context->toNumber(item->contextBinding->readPriority());if(item->hasRawPriority){if(const auto* number=std::get_if<double>(&item->rawPriority.data))return *number;throw std::logic_error("raw priority requires Context conversion");}return item->priority;}validate();const auto&slot=std::get<ContextSlot>(storage_);return slot.context->toNumber(readPriorityValue());}
    void writeItem(T value)const{if(native()){auto item=nativeRecord();if(!item)throw std::logic_error("null prioritized-list record");if(item->contextBinding){item->contextBinding->writeItem(std::move(value));return;}item->item=std::move(value);item->hasRawItem=false;return;}validate();const auto&slot=std::get<ContextSlot>(storage_);if(!slot.writeItem)throw std::logic_error("prioritized-list slot is read-only");slot.writeItem(std::move(value));}
    void writePriority(double value)const{if(native()){auto item=nativeRecord();if(!item)throw std::logic_error("null prioritized-list record");if(item->contextBinding){item->contextBinding->writePriority(options::Value(value));return;}item->priority=value;item->hasRawPriority=false;return;}validate();const auto&slot=std::get<ContextSlot>(storage_);if(!slot.writePriority)throw std::logic_error("prioritized-list slot is read-only");slot.writePriority(options::Value(value));}
    options::Context* context()const{if(native()){auto item=nativeRecord();return item&&item->contextBinding?item->contextBinding->context:nullptr;}validate();return std::get<ContextSlot>(storage_).context;}
    options::Value rawRecord()const{if(native())throw std::logic_error("native record has no raw Context identity");validate();return std::get<ContextSlot>(storage_).record;}
    options::Value invoke(std::span<const options::Value> args)const{if(native()&&nativeRecord()->contextBinding){auto& binding=*nativeRecord()->contextBinding;const auto active=binding.lease.lock();if(!active||!active->load(std::memory_order_acquire))throw options::Exception("TypeError",u"PrioritizedList Context slot runtime has expired");return binding.invoke(args);}validate();const auto&slot=std::get<ContextSlot>(storage_);if(slot.invoke)return slot.invoke(args);const auto callable=slot.context->get(slot.record,options::Key(u"item"));return slot.context->callMember(callable,slot.record,args,u"PrioritizedList item.item");}
    options::Value convertArgument(const std::any& value)const{if(native()&&nativeRecord()->contextBinding)return nativeRecord()->contextBinding->convertArgument(value);validate();const auto&slot=std::get<ContextSlot>(storage_);if(!slot.convertArgument)throw std::logic_error("prioritized-list slot has no argument converter");return slot.convertArgument(value);}
    options::Value encodeAux(const std::any& value)const{if(native()&&nativeRecord()->contextBinding)return nativeRecord()->contextBinding->encodeAux(value);validate();const auto&slot=std::get<ContextSlot>(storage_);if(!slot.encodeAux)throw std::logic_error("prioritized-list slot has no auxiliary converter");return slot.encodeAux(value);}
private:
    void validate()const{const auto&slot=std::get<ContextSlot>(storage_);const auto active=slot.lease.lock();if(!slot.context||!active||!active->load(std::memory_order_acquire))throw options::Exception("TypeError",u"PrioritizedList Context slot runtime has expired");}
    Storage storage_{NativeRef{}};
};

template<class T>
class PrioritizedList {
public:
    inline static double& DEFAULTPRIORITY = detail::prioritized_list_default_priority;
    using Storage=typename PrioritizedListItemsAccess<T>::Storage;
    using StorageRef=typename PrioritizedListItemsAccess<T>::StorageRef;
    using OwnerRef=PrioritizedListItemsAccessRef<T>;
    using ItemRef=PrioritizedListItemHandle<T>;

    class iterator {
    public:
        struct Step{ItemRef value;bool done=false;};
        using iterator_category=std::input_iterator_tag;
        using value_type=ItemRef;
        using difference_type=std::ptrdiff_t;
        using reference=ItemRef;
        iterator()=default;
        Step next(){if(!owner_)throw std::logic_error("increment of an unbound PrioritizedList iterator");const auto index=cursor_++;auto value=owner_->read(index);const bool done=owner_->doneAfterRead(cursor_);return{std::move(value),done};}
        ItemRef record()const{prepare();if(cached_->done)throw std::out_of_range("PrioritizedList iterator is done");return cached_->value;}
        reference operator*()const{return record();}
        iterator& operator++(){if(!cached_)cached_=next();cached_.reset();return *this;}
        iterator operator++(int){auto previous=*this;++*this;return previous;}
        bool operator==(const iterator& other)const{
            // End is a comparison against the iterator's captured owner, not
            // against the list's current owner. Source iterators retain the
            // original items array even after the list field is rebound.
            if(sentinel_&&other.sentinel_)return true;
            if(sentinel_){if(!other.owner_)return false;return other.isDone();}
            if(other.sentinel_){if(!owner_)return false;return isDone();}
            if(owner_!=other.owner_)return false;if(!owner_)return true;
            return cursor_==other.cursor_;
        }
        bool operator!=(const iterator& other)const{return !(*this==other);}
    private:
        friend class PrioritizedList;
        iterator(OwnerRef owner,std::size_t cursor,bool sentinel):owner_(std::move(owner)),cursor_(cursor),sentinel_(sentinel){}
        void prepare()const{if(!cached_)cached_=const_cast<iterator*>(this)->next();}
        bool isDone()const{prepare();return cached_->done;}
        OwnerRef owner_;std::size_t cursor_=0;bool sentinel_=false;mutable std::optional<Step> cached_;
    };

    PrioritizedList():owner_(std::make_shared<NativeItems>(std::make_shared<Storage>())){}
    [[nodiscard]] iterator begin(){return iterator(owner_,0,false);}
    [[nodiscard]] iterator end(){return iterator(owner_,0,true);}
    T add(T item,double priority=DEFAULTPRIORITY){return owner_->add(std::move(item),priority);}
    PrioritizedList& remove(const T& item){owner_->remove(item);return *this;}
    StorageRef storageRef()const{auto* slot=owner_->nativeStorageSlot();return slot?*slot:StorageRef{};}
    void rebindStorage(StorageRef storage){if(!storage)throw std::invalid_argument("PrioritizedList storage is null");owner_=std::make_shared<NativeItems>(std::move(storage));}
    void bindItemsAccess(OwnerRef owner){if(!owner)throw std::invalid_argument("PrioritizedList owner is null");owner_=std::move(owner);}
    OwnerRef itemsAccess()const{return owner_;}
    void addRaw(options::Value item,options::Value priority){owner_->addRaw(std::move(item),std::move(priority));}
    void removeRaw(options::Value item){owner_->removeRaw(std::move(item));}

protected:
    StorageRef& itemStorage(){auto* slot=owner_->nativeStorageSlot();if(!slot)throw std::logic_error("Context-backed PrioritizedList has no native storage vector");return *slot;}
    const StorageRef& itemStorage()const{auto* slot=owner_->nativeStorageSlot();if(!slot)throw std::logic_error("Context-backed PrioritizedList has no native storage vector");return *slot;}
private:
    class NativeItems final:public PrioritizedListItemsAccess<T>{
        StorageRef storage_;
    public:
        explicit NativeItems(StorageRef storage):storage_(std::move(storage)){}
        std::size_t length()override{if(!storage_)throw options::Exception("TypeError",u"PrioritizedList item owner has no storage");return storage_->size();}
        ItemRef read(std::size_t index)override{return index<length()?ItemRef((*storage_)[index]):ItemRef(typename ItemRef::NativeRef{});}
        T add(T item,double priority)override{
            std::size_t index=storage_->size();while(index>0&&priority<(*storage_)[index-1]->priority)--index;
            auto result=item;storage_->insert(storage_->begin()+static_cast<std::ptrdiff_t>(index),std::make_shared<PrioritizedListItem<T>>(PrioritizedListItem<T>{priority,std::move(item)}));return result;
        }
        void remove(const T& item)override{for(std::size_t i=storage_->size();i>0;--i)if((*storage_)[i-1]&&(*storage_)[i-1]->item==item){storage_->erase(storage_->begin()+static_cast<std::ptrdiff_t>(i-1));break;}}
        StorageRef* nativeStorageSlot()override{return &storage_;}
    };
    OwnerRef owner_;
};

}
