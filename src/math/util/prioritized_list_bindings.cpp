module folia.math.util.prioritized_list_bindings;
import std;

namespace folia::math {
using namespace options;
namespace {
[[noreturn]] void incompatible(){
    throw Exception("TypeError",u"PrioritizedList method called on incompatible receiver");
}
Value arg(std::span<const Value> values,std::size_t index){
    return index<values.size()?values[index]:Value(Undefined{});
}
std::u16string indexKey(std::uint32_t index){
    const auto digits=std::to_string(index);
    return {digits.begin(),digits.end()};
}
}

struct PrioritizedListContextBindings::State:std::enable_shared_from_this<State>{
    Context& context;
    ObjectRef ctor{};
    ObjectRef proto{};
    double defaultPriorityValue=5;
    std::weak_ptr<std::atomic_bool> executionLease;
    struct Entry{
        ObjectRef object{};
        std::weak_ptr<void> lifetime;
        Operations operations;
    };
    std::vector<Entry> owners;

    State(Context& context,std::weak_ptr<std::atomic_bool> executionLease)
        :context(context),executionLease(std::move(executionLease)){ensureActive();}

    void ensureActive()const{
        const auto active=executionLease.lock();
        if(!active||!active->load(std::memory_order_acquire))
            throw Exception("TypeError",u"PrioritizedList runtime services have expired");
    }

    Entry* entry(Value receiver){
        ensureActive();
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object||context.kind(*object)!=ObjectKind::instance)incompatible();
        for(auto& item:owners)if(item.object==*object){
            if(item.lifetime.expired())incompatible();
            return &item;
        }
        incompatible();
    }

    void installMethod(std::u16string name,std::uint32_t arity,NativeCall call){
        const auto method=context.function(name,std::move(call));
        context.define(method,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
        context.define(proto,Key(std::move(name)),Descriptor::data(Value(method),true,true,true));
    }

    void initialize(){
        const auto self=shared_from_this();
        ctor=context.nativeConstructor(u"PrioritizedList",
            [self](Context&,std::span<const Value>)->ObjectRef{return self->makeValueList();});
        proto=std::get<ObjectRef>(context.get(Value(ctor),Key(u"prototype")).data);
        context.define(ctor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        context.define(ctor,Key(u"prototype"),Descriptor::data(Value(proto),true,false,false));
        const auto defaultPriorityOwner=shared_from_this();
        context.bindData(ctor,Key(u"DEFAULTPRIORITY"),std::make_shared<NativeDataField>(defaultPriorityOwner,
            [self]{self->ensureActive();return Value(self->defaultPriorityValue);},
            [self](Value value){
                self->ensureActive();
                const auto* number=std::get_if<double>(&value.data);
                if(!number)throw Exception("TypeError",u"PrioritizedList.DEFAULTPRIORITY must be a Number");
                self->defaultPriorityValue=*number;
            }));
        installMethod(u"add",1,[self](Context&,Value receiver,std::span<const Value> args){
            const auto* entry=self->entry(receiver);
            return entry->operations.add(args);
        });
        installMethod(u"remove",1,[self](Context&,Value receiver,std::span<const Value> args){
            const auto* entry=self->entry(receiver);
            return entry->operations.remove(args);
        });
        const auto iterator=context.function(u"[Symbol.iterator]",[self](Context& cx,Value receiver,std::span<const Value>)->Value{
            self->ensureActive();
            const auto* entry=self->entry(receiver);
            const auto array=entry->operations.items();
            auto index=std::make_shared<std::uint64_t>(0);
            const auto result=cx.object();
            const std::weak_ptr<State> weakSelf=self;
            const auto next=cx.function(u"next",[array,index,weakSelf](Context& inner,Value,std::span<const Value>)->Value{
                const auto owner=weakSelf.lock();
                if(!owner)throw Exception("TypeError",u"PrioritizedList runtime services have expired");
                owner->ensureActive();
                const auto current=(*index)++;
                const auto digits=std::to_string(current);
                const Value value=inner.get(array,Key(std::u16string(digits.begin(),digits.end())));
                const Value length=inner.get(array,Key(u"length"));
                const bool done=static_cast<double>(*index)>inner.toNumber(length);
                const auto step=inner.object();
                inner.define(step,Key(u"value"),Descriptor::data(value));
                inner.define(step,Key(u"done"),Descriptor::data(Value(done)));
                return Value(step);
            });
            cx.define(next,Key(u"length"),Descriptor::data(Value(0),false,false,true));
            cx.define(result,Key(u"next"),Descriptor::data(Value(next)));
            return Value(result);
        });
        context.define(iterator,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    context.define(proto,Key(context.intrinsics().symbolIterator),Descriptor::data(Value(iterator),true,true,true));
    }

    struct ValueListState;
    struct ValueArray final:NativeIndexedView{
        std::shared_ptr<ValueListState> state;
        PrioritizedList<Value>::StorageRef storage;
        bool lengthWritable=true;
        ValueArray(std::shared_ptr<ValueListState> state,PrioritizedList<Value>::StorageRef storage)
            :state(std::move(state)),storage(std::move(storage)){}
        std::uint32_t length()const override;
        Descriptor lengthDescriptor()const override;
        std::optional<Descriptor> indexDescriptor(std::uint32_t index)const override;
        std::vector<std::uint32_t> ownIndexKeys()const override;
        void defineIndex(std::uint32_t index,Descriptor descriptor)override;
        void defineLength(Descriptor descriptor)override;
        void deleteIndex(std::uint32_t index)override;
    };
    struct ValueListState:std::enable_shared_from_this<ValueListState>{
        std::shared_ptr<State> binding;
        std::shared_ptr<PrioritizedList<Value>> owner;
        ObjectRef object{};
        std::vector<std::pair<std::weak_ptr<PrioritizedListItem<Value>>,ObjectRef>> records;
        std::vector<std::pair<PrioritizedList<Value>::StorageRef,ObjectRef>> arrays;
        ValueListState(std::shared_ptr<State> binding,std::shared_ptr<PrioritizedList<Value>> owner)
            :binding(std::move(binding)),owner(std::move(owner)){}
        ObjectRef record(const std::shared_ptr<PrioritizedListItem<Value>>& item){
            binding->ensureActive();
            for(auto& entry:records)if(auto known=entry.first.lock();known==item)return entry.second;
            const ObjectRef value=binding->context.object();
            binding->context.setPayload(value,item);
            auto keep=item;
            binding->context.bindData(value,Key(u"item"),std::make_shared<NativeDataField>(keep,
                [self=shared_from_this(),keep]{self->binding->ensureActive();return keep->item;},
                [self=shared_from_this(),keep](Value next){self->binding->ensureActive();keep->item=std::move(next);}));
            binding->context.bindData(value,Key(u"priority"),std::make_shared<NativeDataField>(keep,
                [self=shared_from_this(),keep]{self->binding->ensureActive();return Value(keep->priority);},
                [self=shared_from_this(),keep](Value next){
                    self->binding->ensureActive();
                    const auto* number=std::get_if<double>(&next.data);
                    if(!number)throw Exception("TypeError",u"PrioritizedList item priority must be a Number");
                    keep->priority=*number;
                }));
            records.push_back({item,value});
            return value;
        }
        ObjectRef array(const PrioritizedList<Value>::StorageRef& storage){
            binding->ensureActive();
            for(const auto& entry:arrays)if(entry.first==storage)return entry.second;
            auto view=std::make_shared<ValueArray>(shared_from_this(),storage);
            const ObjectRef value=binding->context.nativeArray(view);
            arrays.push_back({storage,value});
            return value;
        }
    };

    ObjectRef makeValueList(){
        ensureActive();
        auto owner=std::make_shared<PrioritizedList<Value>>();
        auto list=std::make_shared<ValueListState>(shared_from_this(),owner);
        list->object=context.instance(ctor);
        context.setPayload(list->object,owner);
        auto self=shared_from_this();
        auto items=std::make_shared<NativeDataField>(owner,
            [list]{list->binding->ensureActive();return Value(list->array(list->owner->storageRef()));},
            [list](Value value){
                list->binding->ensureActive();
                const auto* object=std::get_if<ObjectRef>(&value.data);
                if(!object||list->binding->context.kind(*object)!=ObjectKind::array)
                    throw Exception("TypeError",u"PrioritizedList.items must be a bound array");
                for(const auto& entry:list->arrays)if(entry.second==*object){list->owner->rebindStorage(entry.first);return;}
                throw Exception("TypeError",u"PrioritizedList.items array is not registered to this owner");
            });
        context.bindData(list->object,Key(u"items"),std::move(items));
        Operations operations;
        const std::weak_ptr<ValueListState> weakList=list;
        const std::weak_ptr<State> weakState=shared_from_this();
        operations.add=[weakList,weakState](std::span<const Value> args){
            const auto list=weakList.lock();const auto self=weakState.lock();
            if(!list||!self)throw Exception("TypeError",u"PrioritizedList owner has expired");
            self->ensureActive();
            const Value item=arg(args,0);
            const double priority=std::holds_alternative<Undefined>(arg(args,1).data)
                ?self->defaultPriorityValue:self->context.toNumber(arg(args,1));
            return list->owner->add(item,priority);
        };
        operations.remove=[weakList](std::span<const Value> args){
            const auto list=weakList.lock();
            if(!list)throw Exception("TypeError",u"PrioritizedList owner has expired");
            list->binding->ensureActive();
            list->owner->remove(arg(args,0));
            return Value(list->object);
        };
        operations.items=[weakList]{
            const auto list=weakList.lock();
            if(!list)throw Exception("TypeError",u"PrioritizedList owner has expired");
            list->binding->ensureActive();
            return Value(list->array(list->owner->storageRef()));
        };
        operations.setItems=[weakList](Value value){
            const auto list=weakList.lock();
            if(!list)throw Exception("TypeError",u"PrioritizedList owner has expired");
            list->binding->ensureActive();
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object||list->binding->context.kind(*object)!=ObjectKind::array)
                throw Exception("TypeError",u"PrioritizedList.items must be a bound array");
            for(const auto& entry:list->arrays)if(entry.second==*object){list->owner->rebindStorage(entry.first);return;}
            throw Exception("TypeError",u"PrioritizedList.items array is not registered to this owner");
        };
        owners.push_back({list->object,owner,std::move(operations)});
        return list->object;
    }
};

std::uint32_t PrioritizedListContextBindings::State::ValueArray::length()const{
    state->binding->ensureActive();
    if(!storage||storage->size()>std::numeric_limits<std::uint32_t>::max())
        throw Exception("RangeError",u"PrioritizedList.items has an invalid length");
    return static_cast<std::uint32_t>(storage->size());
}
Descriptor PrioritizedListContextBindings::State::ValueArray::lengthDescriptor()const{
    return Descriptor::data(Value(static_cast<double>(length())),lengthWritable,false,false);
}
std::optional<Descriptor> PrioritizedListContextBindings::State::ValueArray::indexDescriptor(std::uint32_t index)const{
    state->binding->ensureActive();
    if(index>=length()||!(*storage)[index])return {};
    return Descriptor::data(Value(state->record((*storage)[index])));
}
std::vector<std::uint32_t> PrioritizedListContextBindings::State::ValueArray::ownIndexKeys()const{
    state->binding->ensureActive();
    std::vector<std::uint32_t> result;
    for(std::uint32_t i=0;i<length();++i)if((*storage)[i])result.push_back(i);
    return result;
}
void PrioritizedListContextBindings::State::ValueArray::defineIndex(std::uint32_t index,Descriptor descriptor){
    state->binding->ensureActive();
    if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
        throw Exception("TypeError",u"PrioritizedList.items does not support custom index descriptors");
    const auto* object=std::get_if<ObjectRef>(&descriptor.value.data);
    if(!object)throw Exception("TypeError",u"PrioritizedList.items requires a record object");
    const auto& payload=std::as_const(state->binding->context).payload(*object);
    const auto* record=std::any_cast<std::shared_ptr<PrioritizedListItem<Value>>>(&payload);
    if(!record||!*record)throw Exception("TypeError",u"Foreign PrioritizedList record cannot enter this list");
    if(index>=length()){
        if(!lengthWritable)throw Exception("TypeError",u"Array length is not writable");
        storage->resize(static_cast<std::size_t>(index)+1);
    }
    (*storage)[index]=*record;
}
void PrioritizedListContextBindings::State::ValueArray::defineLength(Descriptor descriptor){
    state->binding->ensureActive();
    if(descriptor.accessor||descriptor.enumerable||descriptor.configurable)
        throw Exception("TypeError",u"Invalid PrioritizedList.items length descriptor");
    if(!lengthWritable&&descriptor.writable)throw Exception("TypeError",u"Cannot redefine non-writable length");
    const auto* number=std::get_if<double>(&descriptor.value.data);
    if(!number||!std::isfinite(*number)||*number<0||*number>4294967295.0||std::trunc(*number)!=*number)
        throw Exception("RangeError",u"Invalid array length");
    storage->resize(static_cast<std::size_t>(*number));
    lengthWritable=descriptor.writable;
}
void PrioritizedListContextBindings::State::ValueArray::deleteIndex(std::uint32_t index){
    state->binding->ensureActive();
    if(index<length())(*storage)[index].reset();
}

PrioritizedListContextBindings::PrioritizedListContextBindings(Context& context)
    :ownedExecutionLease_(std::make_shared<std::atomic_bool>(true)),
     state_(std::make_shared<State>(context,ownedExecutionLease_)){state_->initialize();}
PrioritizedListContextBindings::PrioritizedListContextBindings(Context& context,std::weak_ptr<std::atomic_bool> executionLease)
    :state_(std::make_shared<State>(context,std::move(executionLease))){state_->initialize();}
PrioritizedListContextBindings::~PrioritizedListContextBindings(){
    if(ownedExecutionLease_)ownedExecutionLease_->store(false,std::memory_order_release);
}
Context& PrioritizedListContextBindings::context()const{state_->ensureActive();return state_->context;}
ObjectRef PrioritizedListContextBindings::constructor()const{state_->ensureActive();return state_->ctor;}
ObjectRef PrioritizedListContextBindings::prototype()const{state_->ensureActive();return state_->proto;}
double PrioritizedListContextBindings::defaultPriority()const{state_->ensureActive();return state_->defaultPriorityValue;}
void PrioritizedListContextBindings::defaultPriority(double value){state_->ensureActive();state_->defaultPriorityValue=value;}
void PrioritizedListContextBindings::registerOwner(ObjectRef object,std::weak_ptr<void> lifetime,Operations operations){
    state_->ensureActive();
    if(lifetime.expired())throw Exception("TypeError",u"PrioritizedList owner has expired");
    for(auto& entry:state_->owners)if(entry.object==object){entry.lifetime=std::move(lifetime);entry.operations=std::move(operations);return;}
    state_->owners.push_back({object,std::move(lifetime),std::move(operations)});
}
bool PrioritizedListContextBindings::owns(ObjectRef object)const{
    state_->ensureActive();
    for(const auto& entry:state_->owners)if(entry.object==object&&!entry.lifetime.expired())return true;
    return false;
}
std::optional<ObjectRef> PrioritizedListContextBindings::findOwner(const std::shared_ptr<void>& owner)const{
    state_->ensureActive();
    if(!owner)return {};
    for(const auto& entry:state_->owners)if(const auto live=entry.lifetime.lock();live&&live.get()==owner.get()&&!live.owner_before(owner)&&!owner.owner_before(live))return entry.object;
    return {};
}
Value PrioritizedListContextBindings::add(Value receiver,std::span<const Value> args)const{state_->ensureActive();return state_->entry(std::move(receiver))->operations.add(args);}
Value PrioritizedListContextBindings::remove(Value receiver,std::span<const Value> args)const{state_->ensureActive();return state_->entry(std::move(receiver))->operations.remove(args);}
}
