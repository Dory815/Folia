// Context identity adapters for the existing FunctionList and its live storage.
module folia.math.util.function_list_bindings;
import folia.math.util.property_host;
import folia.math.util.prioritized_list_source_operations;
import std;

namespace folia::math {
using namespace options;

namespace {
std::u16string indexName(std::uint32_t index){
    const auto text=std::to_string(index);
    return std::u16string(text.begin(),text.end());
}

Value argument(std::span<const Value> args,std::size_t index){
    return index<args.size()?args[index]:Value(Undefined{});
}

[[noreturn]] void typeError(std::u16string message){throw Exception("TypeError",std::move(message));}

}

struct FunctionListContextBindings::State:std::enable_shared_from_this<State>{
    Context& context;
    OptionFunctionBindings& functions;
    ContextPropertyBindings properties;
    PrioritizedListContextBindings prioritized;
    std::weak_ptr<std::atomic_bool> executionLease;
    std::shared_ptr<PromiseContextBindings> promises;
    async::QueueRef asyncQueue;
    ObjectRef ctor{};
    ObjectRef proto{};
    ObjectRef baseCtor{};
    ObjectRef baseProto{};

    struct StorageEntry{
        FunctionList::StorageRef storage;
        ObjectRef array{};
        std::weak_ptr<PrioritizedListContextArray<FunctionValue>> view;
    };
    struct ItemEntry{
        std::weak_ptr<FunctionListItem> item;
        ObjectRef object{};
    };
    struct ListEntry{
        std::weak_ptr<FunctionList> list;
        ObjectRef object{};
    };
    std::vector<StorageEntry> storages;
    std::vector<ItemEntry> items;
    std::vector<ListEntry> lists;
    std::vector<std::pair<std::exception_ptr,ObjectRef>> asyncErrors;
    std::vector<FunctionListContextBindings::NativeArgumentEncoder> nativeArgumentEncoders;

    Value argumentValue(const std::any& argument){
        ensureActive();
        if(const auto* value=std::any_cast<Value>(&argument))return *value;
        if(const auto* value=std::any_cast<bool>(&argument))return Value(*value);
        if(const auto* value=std::any_cast<double>(&argument))return Value(*value);
        if(const auto* value=std::any_cast<std::u16string>(&argument))return Value(*value);
        if(const auto* value=std::any_cast<std::string>(&argument))return properties.to(Property(*value));
        if(const auto* value=std::any_cast<Property>(&argument))return properties.to(*value);
        if(const auto* value=std::any_cast<FunctionValue>(&argument))return properties.to(functions::toProperty(*value));
        if(const auto* value=std::any_cast<ObjectRef>(&argument))return Value(*value);
        if(const auto* value=std::any_cast<SymbolRef>(&argument))return Value(*value);
        for(const auto& encoder:nativeArgumentEncoders)if(encoder)if(auto value=encoder(argument))return *value;
        typeError(u"FunctionList argument has no canonical Context Value binding");
    }

    struct ContextItems final:FunctionListItemsAccess{
        std::weak_ptr<State> owner;
        std::weak_ptr<FunctionList> list;
        Value array;
        Value receiver;
        FunctionList::StorageRef nativeStorage;
        ContextItems(std::weak_ptr<State> owner,Value array,FunctionList::StorageRef storage={},std::weak_ptr<FunctionList> list={},Value receiver=Undefined{})
            :owner(std::move(owner)),list(std::move(list)),array(std::move(array)),receiver(std::move(receiver)),nativeStorage(std::move(storage)){}
        FunctionList::StorageRef* nativeStorageSlot()override{return nativeStorage?&nativeStorage:nullptr;}
        std::shared_ptr<State> state()const{auto value=owner.lock();if(!value)typeError(u"FunctionList array owner has expired");value->ensureActive();return value;}
        Value currentArray()const{
            if(auto native=list.lock())if(auto current=native->itemsAccess())return current->identity();
            return array;
        }
        Value identity()const override{return array;}
        std::size_t length()override{
            auto self=state();const Value raw=self->context.get(array,Key(u"length"));const double n=self->context.toNumber(raw);
            if(!std::isfinite(n)||n<=0)return 0;if(n>static_cast<double>(std::numeric_limits<std::size_t>::max()))throw Exception("RangeError",u"Invalid FunctionList items length");return static_cast<std::size_t>(std::ceil(n));
        }
        bool doneAfterRead(std::size_t nextIndex)override{
            auto self=state();const Value raw=self->context.get(array,Key(u"length"));
            const double length=self->context.toNumber(raw);
            return static_cast<double>(nextIndex)>length;
        }
        bool indexBeforeLength(double index)override{
            auto self=state();const Value raw=self->context.get(array,Key(u"length"));
            return index<self->context.toNumber(raw);
        }
        FunctionListItemHandle read(std::size_t index)override{
            auto self=state();const auto key=indexName(static_cast<std::uint32_t>(index));
            if(nativeStorage){
                const Value value=self->context.get(array,Key(key));
                if(index>=nativeStorage->size()||!(*nativeStorage)[index])return FunctionListItemHandle{};
                auto item=(*nativeStorage)[index];
                if(!item->rawArraySlot)(void)self->itemObject(item);
                (void)value;
                return FunctionListItemHandle(std::move(item));
            }
            Value record=self->context.get(array,Key(key));
            const bool present=self->context.hasOwn(array,Key(key));
            std::weak_ptr<State> weak=self;
            FunctionListItemHandle::ContextSlot slot;
            slot.context=&self->context;slot.record=std::move(record);slot.lease=self->executionLease;slot.present=present;
            const auto slotRecord=slot.record;
            slot.readItem=[weak,slotRecord]()->FunctionListItemHandle::ItemValue{auto owner=weak.lock();if(!owner)typeError(u"FunctionList array owner has expired");owner->ensureActive();return owner->context.get(slotRecord,Key(u"item"));};
            slot.readPriority=[weak,slotRecord]()->Value{auto owner=weak.lock();if(!owner)typeError(u"FunctionList array owner has expired");owner->ensureActive();return owner->context.get(slotRecord,Key(u"priority"));};
            slot.writeItem=[weak,slotRecord](FunctionValue item){auto owner=weak.lock();if(!owner)typeError(u"FunctionList array owner has expired");owner->ensureActive();owner->context.set(slotRecord,Key(u"item"),owner->properties.to(functions::toProperty(item)));};
            slot.writePriority=[weak,slotRecord](Value priority){auto owner=weak.lock();if(!owner)typeError(u"FunctionList array owner has expired");owner->ensureActive();owner->context.set(slotRecord,Key(u"priority"),std::move(priority));};
            slot.convertArgument=[weak](const std::any& value){auto owner=weak.lock();if(!owner)typeError(u"FunctionList array owner has expired");return owner->argumentValue(value);};
            slot.encodeAux=[weak](const std::any& argument){auto owner=weak.lock();if(!owner)typeError(u"FunctionList array owner has expired");owner->ensureActive();if(const auto* property=std::any_cast<Property>(&argument))return owner->properties.to(*property);if(const auto* value=std::any_cast<Value>(&argument))return *value;typeError(u"FunctionList auxiliary value has no Context binding");};
            return FunctionListItemHandle(std::move(slot));
        }
        bool less(const Value& left,const Value& right)override{
            auto self=state();const Value a=self->context.toPrimitive(left,PrimitiveHint::numberHint),b=self->context.toPrimitive(right,PrimitiveHint::numberHint);
            if(const auto* x=std::get_if<std::u16string>(&a.data))if(const auto* y=std::get_if<std::u16string>(&b.data))return *x<*y;
            const double x=self->context.toNumber(a),y=self->context.toNumber(b);return !std::isnan(x)&&!std::isnan(y)&&x<y;
        }
        Value itemValue(const FunctionValue& item){auto self=state();return self->properties.to(functions::toProperty(item));}
        void insert(Value item,Value priority){
            auto self=state();
            (void)PrioritizedListSourceOperations::add(self->context,receiver,std::move(item),std::move(priority),
                [self](Value targetItems,Value sourceItem,Value sourcePriority)->Value{
                    const auto* target=std::get_if<ObjectRef>(&targetItems.data);
                    const auto storage=target?self->storageFor(*target):FunctionList::StorageRef{};
                    if(storage){
                        auto native=std::make_shared<FunctionListItem>(FunctionListItem{5,FunctionValue{Property(Missing{})}});
                        native->hasRawItem=true;native->rawItem=std::move(sourceItem);
                        native->hasRawPriority=true;native->rawPriority=std::move(sourcePriority);
                        return Value(self->itemObject(native));
                    }
                    const ObjectRef record=self->context.object();
                    self->context.set(Value(record),Key(u"item"),std::move(sourceItem));
                    self->context.set(Value(record),Key(u"priority"),std::move(sourcePriority));
                    return Value(record);
                });
        }
        FunctionValue add(FunctionValue item,double priority)override{const auto result=item;insert(itemValue(item),Value(priority));return result;}
        void remove(const FunctionValue& item)override{removeRaw(itemValue(item));}
        void addRaw(Value item,Value priority)override{insert(std::move(item),std::move(priority));}
        void removeRaw(Value target)override{
            auto self=state();
            (void)PrioritizedListSourceOperations::remove(self->context,receiver,std::move(target));
        }
        Value convertArgument(const std::any& argument)override{return state()->argumentValue(argument);}
        Value encodeAux(const std::any& argument)override{auto self=state();if(const auto* property=std::any_cast<Property>(&argument))return self->properties.to(*property);if(const auto* value=std::any_cast<Value>(&argument))return *value;typeError(u"FunctionList auxiliary value has no Context binding");}
        void setSlot(std::size_t index,Value value)override{auto self=state();self->context.set(currentArray(),Key(indexName(static_cast<std::uint32_t>(index))),std::move(value));}
        bool deleteSlot(std::size_t index)override{auto self=state();const auto target=self->context.toObject(currentArray());self->context.erase(target,Key(indexName(static_cast<std::uint32_t>(index))));return true;}
        void setLength(Value value)override{auto self=state();self->context.set(currentArray(),Key(u"length"),std::move(value));}
    };

    State(Context& context,OptionFunctionBindings& functions,std::weak_ptr<std::atomic_bool> executionLease,async::QueueRef asyncQueue)
        :context(context),functions(functions),properties(context,functions),prioritized(context,executionLease),
         executionLease(std::move(executionLease)),asyncQueue(std::move(asyncQueue)){
        if(&functions.context()!=&context)typeError(u"FunctionList bindings require the same Context");
        if(!this->asyncQueue)typeError(u"FunctionList bindings require the runtime Promise queue");
        ensureActive();
    }

    void ensureActive()const{
        const auto active=executionLease.lock();
        if(!active||!active->load(std::memory_order_acquire))
            typeError(u"FunctionList runtime services have expired");
    }

    Value valueFromAsync(const std::any& value){
        if(const auto* raw=std::any_cast<Value>(&value))return *raw;
        if(const auto* boolean=std::any_cast<bool>(&value))return Value(*boolean);
        if(const auto* number=std::any_cast<double>(&value))return Value(*number);
        if(const auto* text=std::any_cast<std::u16string>(&value))return Value(*text);
        if(const auto* promise=std::any_cast<async::PromiseRef>(&value);promise&&*promise)return Value(promises->wrap(*promise));
        if(const auto* thrown=std::any_cast<async::ThrownValue>(&value))return valueFromAsync(thrown->value);
        if(const auto* exception=std::any_cast<std::exception_ptr>(&value);exception&&*exception){
            for(const auto& [known,object]:asyncErrors)if(known==*exception)return Value(object);
            std::string name="Error";std::u16string message=u"Native promise rejected";
            try{std::rethrow_exception(*exception);}
            catch(const Exception& error){name=error.name;message=error.message;}
            catch(const std::exception& error){message=property_host::text(error.what());}
            catch(...){}
            const auto object=context.object();context.setPayload(object,*exception);
            context.set(Value(object),Key(u"name"),Value(property_host::text(name)));
            context.set(Value(object),Key(u"message"),Value(std::move(message)));
            asyncErrors.emplace_back(*exception,object);return Value(object);
        }
        if(value.has_value())throw Exception("Unimplemented",u"Native promise value has no Context binding");
        return Value(Undefined{});
    }
    std::any asyncFromValue(Value value){
        if(const auto* object=std::get_if<ObjectRef>(&value.data))
            if(auto promise=promises->unwrap(value))return std::any(std::move(promise));
        if(const auto* boolean=std::get_if<bool>(&value.data))return std::any(*boolean);
        if(const auto* number=std::get_if<double>(&value.data))return std::any(*number);
        if(const auto* text=std::get_if<std::u16string>(&value.data))return std::any(*text);
        return std::any(std::move(value));
    }
    ObjectRef wrapPromise(async::PromiseRef promise){ensureActive();return promises->wrap(std::move(promise));}

    std::shared_ptr<FunctionListItem> itemFor(ObjectRef object)const{
        ensureActive();
        for(const auto& entry:items)if(entry.object==object){
            if(auto item=entry.item.lock())return item;
            break;
        }
        return {};
    }

    std::shared_ptr<FunctionListItem> rawSlotRecord(Value value){
        auto record=std::make_shared<FunctionListItem>(FunctionListItem{5,FunctionValue{Property(Missing{})}});
        record->rawArraySlot=true;record->rawSlotValue=value;
        const std::weak_ptr<State> weak=shared_from_this();
        auto binding=std::make_shared<PrioritizedListItemContextBinding<FunctionValue>>();
        binding->context=&context;binding->record=value;binding->lease=executionLease;
        binding->readItem=[weak,value]{auto self=weak.lock();if(!self)typeError(u"FunctionList slot Context has expired");self->ensureActive();return self->context.get(value,Key(u"item"));};
        binding->readPriority=[weak,value]{auto self=weak.lock();if(!self)typeError(u"FunctionList slot Context has expired");self->ensureActive();return self->context.get(value,Key(u"priority"));};
        binding->writeItem=[weak,value](FunctionValue item){auto self=weak.lock();if(!self)typeError(u"FunctionList slot Context has expired");self->ensureActive();self->context.set(value,Key(u"item"),self->properties.to(functions::toProperty(item)));};
        binding->writePriority=[weak,value](Value priority){auto self=weak.lock();if(!self)typeError(u"FunctionList slot Context has expired");self->ensureActive();self->context.set(value,Key(u"priority"),std::move(priority));};
        binding->invoke=[weak,value](std::span<const Value> args){auto self=weak.lock();if(!self)typeError(u"FunctionList slot Context has expired");self->ensureActive();const auto callable=self->context.get(value,Key(u"item"));return self->context.callMember(callable,value,args,u"FunctionList item.item");};
        binding->convertArgument=[weak](const std::any& argument){auto self=weak.lock();if(!self)typeError(u"FunctionList slot Context has expired");return self->argumentValue(argument);};
        binding->encodeAux=[weak](const std::any& argument){auto self=weak.lock();if(!self)typeError(u"FunctionList slot Context has expired");self->ensureActive();if(const auto* v=std::any_cast<Value>(&argument))return *v;if(const auto* v=std::any_cast<Property>(&argument))return self->properties.to(*v);typeError(u"FunctionList auxiliary value has no Context binding");};
        record->contextBinding=std::move(binding);
        return record;
    }

    ObjectRef itemObject(const std::shared_ptr<FunctionListItem>& item){
        ensureActive();
        if(!item)typeError(u"FunctionList item record is null");
        for(auto& entry:items)if(auto known=entry.item.lock();known==item)return entry.object;
        if(item->contextBinding){const auto* object=std::get_if<ObjectRef>(&item->contextBinding->record.data);if(object)return *object;}
        if(const auto function=functions::reference(item->item);function&&!functions.find(function)){
            OptionCallBinding binding;
            binding.receiver=[](Context& context,Value receiver)->FunctionReceiver{
                if(const auto* object=std::get_if<ObjectRef>(&receiver.data)){
                    const auto& payload=context.payload(*object);
                    if(const auto* native=std::any_cast<NativeObjectRef>(&payload))return *native;
                    if(const auto* item=std::any_cast<std::shared_ptr<FunctionListItem>>(&payload))return std::static_pointer_cast<NativeObject>(*item);
                }
                return receiver;
            };
            binding.arguments=[](Context& context,std::span<const Value> args){
                std::vector<std::any> result;result.reserve(args.size());
                for(const auto& value:args){
                    if(const auto* object=std::get_if<ObjectRef>(&value.data)){
                        const auto& payload=context.payload(*object);
                        if(payload.has_value())result.push_back(payload);
                        else result.emplace_back(value);
                    }else result.emplace_back(value);
                }
                return result;
            };
            const auto self=shared_from_this();
            binding.result=[self](Context&,const std::any& result)->Value{
                self->ensureActive();
                if(!result.has_value()||std::any_cast<std::monostate>(&result))return Undefined{};
                if(const auto* value=std::any_cast<Value>(&result))return *value;
                if(const auto* property=std::any_cast<Property>(&result))return self->properties.to(*property);
                if(const auto* value=std::any_cast<bool>(&result))return *value;
                if(const auto* value=std::any_cast<double>(&result))return *value;
                if(const auto* value=std::any_cast<std::u16string>(&result))return *value;
                if(const auto* value=std::any_cast<std::string>(&result))return property_host::text(*value);
                if(const auto* function=std::any_cast<FunctionRef>(&result))return self->properties.to(Property(*function));
                throw Exception("Unimplemented",u"Native FunctionList result has no Context primitive binding");
            };
            functions.bind(function,std::move(binding),u"native filter");
        }
        const auto object=context.object();
        context.setPayload(object,std::static_pointer_cast<NativeObject>(item));
        const std::weak_ptr<State> weak=shared_from_this();
        auto record=item;
        auto itemField=std::make_shared<NativeDataField>(record,
            [weak,record]{auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();return record->hasRawItem?record->rawItem:self->properties.to(functions::toProperty(record->item));},
            [weak,record](Value value){auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();record->rawItem=std::move(value);record->hasRawItem=true;});
        auto priorityField=std::make_shared<NativeDataField>(record,
            [weak,record]{auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();return record->hasRawPriority?record->rawPriority:Value(record->priority);},
            [weak,record](Value value){auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();record->rawPriority=std::move(value);record->hasRawPriority=true;});
        context.bindData(object,Key(u"item"),std::move(itemField));
        context.bindData(object,Key(u"priority"),std::move(priorityField));
        auto binding=std::make_shared<PrioritizedListItemContextBinding<FunctionValue>>();
        binding->context=&context;binding->record=Value(object);binding->lease=executionLease;
        binding->readItem=[weak,record]{auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();return record->hasRawItem?record->rawItem:self->properties.to(functions::toProperty(record->item));};
        binding->readPriority=[weak,record]{auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();return record->hasRawPriority?record->rawPriority:Value(record->priority);};
        binding->writeItem=[weak,record](FunctionValue value){auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();record->item=std::move(value);record->hasRawItem=false;};
        binding->writePriority=[weak,record](Value value){auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();record->rawPriority=std::move(value);record->hasRawPriority=true;};
        binding->invoke=[weak,object](std::span<const Value> args){auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();const Value callable=self->context.get(Value(object),Key(u"item"));return self->context.callMember(callable,Value(object),args,u"FunctionList item.item");};
        binding->convertArgument=[weak](const std::any& argument){auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");return self->argumentValue(argument);};
        binding->encodeAux=[weak](const std::any& argument){auto self=weak.lock();if(!self)typeError(u"FunctionList record Context has expired");self->ensureActive();if(const auto* value=std::any_cast<Value>(&argument))return *value;if(const auto* value=std::any_cast<Property>(&argument))return self->properties.to(*value);typeError(u"FunctionList auxiliary value has no Context binding");};
        item->contextBinding=std::move(binding);
        items.push_back({item,object});
        return object;
    }

    ObjectRef storageObject(const FunctionList::StorageRef& storage){
        ensureActive();
        if(!storage)typeError(u"FunctionList item array is null");
        for(auto& entry:storages)if(entry.storage==storage){
            if(auto existing=entry.view.lock())return entry.array;
        }
        const std::weak_ptr<State> weak=shared_from_this();
        PrioritizedListContextArray<FunctionValue>::Callbacks callbacks;
        callbacks.ensureActive=[weak]{
            auto state=weak.lock();if(!state)typeError(u"FunctionList storage Context has expired");state->ensureActive();
        };
        callbacks.recordObject=[weak](const std::shared_ptr<FunctionListItem>& item){
            auto state=weak.lock();if(!state)typeError(u"FunctionList storage Context has expired");state->ensureActive();return state->itemObject(item);
        };
        callbacks.cellFromValue=[weak](const Value& value){
            auto state=weak.lock();if(!state)typeError(u"FunctionList storage Context has expired");state->ensureActive();
            const auto* object=std::get_if<ObjectRef>(&value.data);
            return object?state->itemFor(*object):std::shared_ptr<FunctionListItem>{};
        };
        callbacks.rawCell=[weak](Value value){
            auto state=weak.lock();if(!state)typeError(u"FunctionList storage Context has expired");state->ensureActive();return state->rawSlotRecord(std::move(value));
        };
        auto [array,provider]=makePrioritizedListContextArray<FunctionValue>(context,storage,std::move(callbacks));
        storages.push_back({storage,array,provider});
        return array;
    }

    FunctionList::StorageRef storageFor(ObjectRef object)const{
        ensureActive();
        for(const auto& entry:storages)if(entry.array==object)return entry.storage;
        return {};
    }

    std::shared_ptr<FunctionList> listFor(Value receiver)const{
        ensureActive();
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object||context.kind(*object)!=ObjectKind::instance)typeError(u"FunctionList method called on incompatible receiver");
        for(const auto& entry:lists)if(entry.object==*object)if(auto list=entry.list.lock())return list;
        typeError(u"FunctionList method called on incompatible receiver");
    }

    ObjectRef bindList(const std::shared_ptr<FunctionList>& list){
        ensureActive();
        if(!list)typeError(u"FunctionList instance is null");
        for(auto& entry:lists)if(auto known=entry.list.lock();known==list)return entry.object;
        const auto object=context.instance(ctor);
        context.setPayload(object,list);
        // Context access wraps the existing cells. The array view, typed
        // storage references, and iterators all continue to share this owner.
        if(auto active=list->itemsAccess();active&&active->nativeStorageSlot()){
            const auto storage=list->storageRef();if(!storage)typeError(u"FunctionList native seed has no storage");
            const auto array=storageObject(storage);
            list->bindItemsAccess(std::make_shared<ContextItems>(shared_from_this(),Value(array),storage,list,Value(object)));
        }
        auto owner=list;
        const auto self=shared_from_this();
        auto itemsField=std::make_shared<NativeDataField>(owner,
            [self,owner]{self->ensureActive();if(auto access=owner->itemsAccess();access&&!access->nativeStorageSlot())return access->identity();return Value(self->storageObject(owner->storageRef()));},
            [self,owner,object](Value value){
                self->ensureActive();
                const auto* array=std::get_if<ObjectRef>(&value.data);
                auto storage=array&&self->context.kind(*array)==ObjectKind::array?self->storageFor(*array):FunctionList::StorageRef{};
                if(storage){if(auto current=owner->itemsAccess();current&&current->identity()==value)return;owner->bindItemsAccess(std::make_shared<ContextItems>(self,value,std::move(storage),owner,Value(object)));}
                else owner->bindItemsAccess(std::make_shared<ContextItems>(self,value,FunctionList::StorageRef{},owner,Value(object)));
            });
        context.bindData(object,Key(u"items"),std::move(itemsField));
        lists.push_back({list,object});
        PrioritizedListContextBindings::Operations operations;
        const std::weak_ptr<FunctionList> weakList=list;
        const std::weak_ptr<State> weakState=shared_from_this();
        operations.add=[weakState,weakList](std::span<const Value> args){
            const auto self=weakState.lock();
            const auto list=weakList.lock();
            if(!list||!self)typeError(u"FunctionList owner has expired");
            self->ensureActive();
            const auto value=argument(args,0);
            const auto priority=argument(args,1);
            auto access=list->itemsAccess();
            if(!access)typeError(u"FunctionList has no active items owner");
            Value p=std::holds_alternative<Undefined>(priority.data)?Value(self->prioritized.defaultPriority()):priority;
            access->addRaw(value,std::move(p));return value;
        };
        operations.remove=[weakState,weakList,object](std::span<const Value> args){
            const auto self=weakState.lock();
            const auto list=weakList.lock();
            if(!list||!self)typeError(u"FunctionList owner has expired");
            self->ensureActive();
            const Value value=argument(args,0);if(auto access=list->itemsAccess()){access->removeRaw(value);return Value(object);}
            return Value(object);
        };
        operations.items=[weakState,weakList]{
            const auto self=weakState.lock();
            const auto list=weakList.lock();
            if(!list||!self)typeError(u"FunctionList owner has expired");
            self->ensureActive();
            if(auto access=list->itemsAccess())return access->identity();
            return Value(self->storageObject(list->storageRef()));
        };
        operations.setItems=[weakState,weakList,object](Value value){
            const auto self=weakState.lock();
            const auto list=weakList.lock();
            if(!list||!self)typeError(u"FunctionList owner has expired");
            self->ensureActive();
            const auto* array=std::get_if<ObjectRef>(&value.data);
            auto storage=array&&self->context.kind(*array)==ObjectKind::array?self->storageFor(*array):FunctionList::StorageRef{};
            if(storage){if(auto access=list->itemsAccess();access&&access->identity()==value)return;list->bindItemsAccess(std::make_shared<ContextItems>(self,value,std::move(storage),list,Value(object)));}
            else list->bindItemsAccess(std::make_shared<ContextItems>(self,std::move(value),FunctionList::StorageRef{},list,Value(object)));
        };
        prioritized.registerOwner(object,list,std::move(operations));
        return object;
    }

    std::shared_ptr<FunctionList> create(Value definitions){
        ensureActive();
        auto list=std::make_shared<FunctionList>();
        (void)bindList(list); // Source instance fields exist before addList runs.
        if(truthy(definitions)){
            OptionFunctionDefinitions source(functions,definitions);
            list->addListWithDefault(source,prioritized.defaultPriority());
        }
        return list;
    }

    Value invokeItem(const FunctionList::ItemRef& handle,std::span<const std::any> nativeArgs){
        ensureActive();
        if(handle.contextBacked()){
            std::vector<Value> args;args.reserve(nativeArgs.size());
            for(const auto& arg:nativeArgs)args.push_back(argumentValue(arg));
            return handle.invoke(args);
        }
        if(!handle.native()){
            std::vector<Value> args;args.reserve(nativeArgs.size());
            for(const auto& arg:nativeArgs)args.push_back(argumentValue(arg));
            return handle.invoke(args);
        }
        const auto record=handle.nativeRecord();
        const auto object=itemObject(record);
        const Value callable=context.get(Value(object),Key(u"item"));
        std::vector<Value> args;
        args.reserve(nativeArgs.size());
        for(const auto& arg:nativeArgs)args.push_back(argumentValue(arg));
        return context.callMember(callable,Value(object),args,u"FunctionList item.item");
    }

    void installMethod(std::u16string name,std::uint32_t arity,NativeCall body){
        const auto method=context.function(name,std::move(body));
        context.define(method,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
        // The pinned ES5 output assigns these prototype members directly,
        // which makes their descriptors enumerable.
        context.define(proto,Key(std::move(name)),Descriptor::data(Value(method),true,true,true));
    }

    void initialize(){
        const auto self=shared_from_this();
        const std::weak_ptr<State> weak=self;
        promises=std::make_shared<PromiseContextBindings>(context,asyncQueue,executionLease,
            [weak](const async::Value& value){const auto state=weak.lock();if(!state)typeError(u"Promise Context owner has expired");return state->valueFromAsync(value);},
            [weak](Value value){const auto state=weak.lock();if(!state)typeError(u"Promise Context owner has expired");return state->asyncFromValue(std::move(value));});
        promises->initialize();
        baseCtor=prioritized.constructor();
        baseProto=prioritized.prototype();
        ctor=context.nativeConstructor(u"FunctionList",
            [self](Context&,std::span<const Value> args)->ObjectRef{
                const auto& state=self;
                const Value definitions=args.empty()?Value(Null{}):args[0];
                const auto list=state->create(definitions);
                for(const auto& entry:state->lists)if(auto known=entry.list.lock();known==list)return entry.object;
                throw Exception("TypeError",u"FunctionList constructor failed to bind its instance");
            });
        // The pinned CJS constructor is emitted as function FunctionList(list)
        // with a default-argument check in its body, so Function.length is 1.
        context.define(ctor,Key(u"length"),Descriptor::data(Value(1),false,false,true));
        proto=std::get<ObjectRef>(context.get(Value(ctor),Key(u"prototype")).data);
        context.setPrototype(proto,baseProto);
        context.setPrototype(ctor,baseCtor);
        context.define(ctor,Key(u"prototype"),Descriptor::data(Value(proto),true,false,false));
        installMethod(u"addList",1,[self](Context&,Value receiver,std::span<const Value> args){
            const auto& state=self;
            auto list=state->listFor(receiver);const auto definitions=argument(args,0);
            OptionFunctionDefinitions source(state->functions,definitions);list->addListWithDefault(source,state->prioritized.defaultPriority());return Value(Undefined{});
        });
        installMethod(u"execute",0,[self](Context&,Value receiver,std::span<const Value> args){
            const auto& state=self;
            auto list=state->listFor(receiver);std::vector<std::any> nativeArgs;
            nativeArgs.reserve(args.size());for(const auto& value:args)nativeArgs.emplace_back(value);
            const bool result=list->executeUsing([&](const FunctionList::ItemRef& item){
                const Value returned=state->invokeItem(item,nativeArgs);
                return !(std::holds_alternative<bool>(returned.data)&&!std::get<bool>(returned.data));
            });
            return Value(result);
        });
        installMethod(u"asyncExecute",0,[self](Context&,Value receiver,std::span<const Value> args){
            if(!self)throw Exception("TypeError",u"FunctionList binding has expired");
            self->ensureActive();auto list=self->listFor(receiver);
            std::vector<std::any> nativeArgs;nativeArgs.reserve(args.size());for(const auto& value:args)nativeArgs.emplace_back(value);
            return Value(self->wrapPromise(list->asyncExecute(self->asyncQueue,nativeArgs)));
        });
    }
};

FunctionListContextBindings::FunctionListContextBindings(Context& context,OptionFunctionBindings& functions)
    :ownedExecutionLease_(std::make_shared<std::atomic_bool>(true)),
     state_(std::make_shared<State>(context,functions,ownedExecutionLease_,std::make_shared<async::JobQueue>())){state_->initialize();}
FunctionListContextBindings::FunctionListContextBindings(Context& context,OptionFunctionBindings& functions,
    std::weak_ptr<std::atomic_bool> executionLease)
    :state_(std::make_shared<State>(context,functions,std::move(executionLease),std::make_shared<async::JobQueue>())){state_->initialize();}
FunctionListContextBindings::FunctionListContextBindings(Context& context,OptionFunctionBindings& functions,
    std::weak_ptr<std::atomic_bool> executionLease,async::QueueRef queue)
    :state_(std::make_shared<State>(context,functions,std::move(executionLease),std::move(queue))){state_->initialize();}
FunctionListContextBindings::~FunctionListContextBindings(){
    if(ownedExecutionLease_)ownedExecutionLease_->store(false,std::memory_order_release);
}
ObjectRef FunctionListContextBindings::constructor()const{state_->ensureActive();return state_->ctor;}
ObjectRef FunctionListContextBindings::prototype()const{state_->ensureActive();return state_->proto;}
ObjectRef FunctionListContextBindings::bind(const std::shared_ptr<FunctionList>& list){state_->ensureActive();return state_->bindList(list);}
std::shared_ptr<FunctionList> FunctionListContextBindings::resolve(ObjectRef object)const{
    state_->ensureActive();
    for(const auto& entry:state_->lists)if(entry.object==object)if(auto list=entry.list.lock())return list;
    return {};
}
std::shared_ptr<FunctionList> FunctionListContextBindings::make(Value definitions){state_->ensureActive();return state_->create(std::move(definitions));}
ObjectRef FunctionListContextBindings::view(const FunctionList::StorageRef& storage){state_->ensureActive();return state_->storageObject(storage);}
void FunctionListContextBindings::addNativeArgumentEncoder(NativeArgumentEncoder encoder){state_->ensureActive();if(!encoder)typeError(u"FunctionList native argument encoder is empty");state_->nativeArgumentEncoders.push_back(std::move(encoder));}
PrioritizedListContextBindings& FunctionListContextBindings::prioritizedLists(){state_->ensureActive();return state_->prioritized;}
OptionFunctionBindings& FunctionListContextBindings::functions()const{state_->ensureActive();return state_->functions;}
Value FunctionListContextBindings::toValue(const Property& property)const{state_->ensureActive();return state_->properties.to(property);}
Property FunctionListContextBindings::fromValue(Value value){state_->ensureActive();return state_->properties.from(std::move(value));}
std::function<Value(const Property&)> FunctionListContextBindings::toValueConverter()const{
    state_->ensureActive();
    const auto state=state_;
    return [state](const Property& property){state->ensureActive();return state->properties.to(property);};
}
std::function<Property(Value)> FunctionListContextBindings::fromValueConverter()const{
    state_->ensureActive();
    const auto state=state_;
    return [state](Value value){state->ensureActive();return state->properties.from(std::move(value));};
}
std::function<ObjectRef(const std::shared_ptr<FunctionList>&)> FunctionListContextBindings::bindConverter()const{
    state_->ensureActive();
    const auto state=state_;
    return [state](const std::shared_ptr<FunctionList>& list){state->ensureActive();return state->bindList(list);};
}
std::function<std::shared_ptr<FunctionList>(ObjectRef)> FunctionListContextBindings::resolveConverter()const{
    state_->ensureActive();
    const auto state=state_;
    return [state](ObjectRef object){
        state->ensureActive();
        for(const auto& entry:state->lists)if(entry.object==object)if(auto list=entry.list.lock())return list;
        return std::shared_ptr<FunctionList>{};
    };
}
std::weak_ptr<std::atomic_bool> FunctionListContextBindings::executionLifetime()const{
    state_->ensureActive();
    return state_->executionLease;
}

}
