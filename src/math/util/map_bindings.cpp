// ECMAScript Map and Map Iterator objects for one options::Context.
module folia.math.util.map_bindings;
import std;
import folia.math.util.js_number;
import folia.math.util.property_host;

namespace folia::math::options {
namespace {
Value arg(std::span<const Value> args,std::size_t index){
    return index<args.size()?args[index]:Value(Undefined{});
}
[[noreturn]] void typeError(std::u16string message){throw Exception("TypeError",std::move(message));}
ObjectRef requireCallable(Context& context,const Value& value,std::u16string_view expression){
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object||context.kind(*object)!=ObjectKind::function)
        typeError(std::u16string(expression)+u" is not a function");
    return *object;
}
void forEachMapIterable(Context& context,Value iterable,
                        const std::function<void(Value,Value)>& body){
    const Value method=context.get(iterable,Key(context.intrinsics().symbolIterator));
    if(std::holds_alternative<Undefined>(method.data)||std::holds_alternative<Null>(method.data))
        typeError(u"Value is not iterable");
    const ObjectRef iteratorMethod=requireCallable(context,method,u"Map iterator method");
    const Value iterator=context.call(iteratorMethod,iterable);
    const auto* iteratorObject=std::get_if<ObjectRef>(&iterator.data);
    if(!iteratorObject)typeError(u"Iterator method did not return an object");
    const Value next=context.get(iterator,Key(u"next"));
    const ObjectRef nextMethod=requireCallable(context,next,u"Iterator next");
    auto closeOnEntryFailure=[&](){
        const auto original=std::current_exception();
        try{
            const Value close=context.get(iterator,Key(u"return"));
            if(!std::holds_alternative<Undefined>(close.data)&&!std::holds_alternative<Null>(close.data)){
                const ObjectRef closeMethod=requireCallable(context,close,u"Iterator return");
                (void)context.call(closeMethod,iterator);
            }
        }catch(...){ }
        std::rethrow_exception(original);
    };
    for(;;){
        // IteratorStep/IteratorValue abrupt completions propagate directly;
        // AddEntriesFromIterable only closes the outer iterator after it has
        // obtained a value and an entry/key/value/adder operation fails.
        const Value step=context.call(nextMethod,iterator);
        if(!std::holds_alternative<ObjectRef>(step.data))typeError(u"Iterator result is not an object");
        if(truthy(context.get(step,Key(u"done"))))return;
        const Value entry=context.get(step,Key(u"value"));
        try{
            const auto* pair=std::get_if<ObjectRef>(&entry.data);
            if(!pair){
                std::u16string display;
                if(std::holds_alternative<Undefined>(entry.data))display=u"undefined";
                else if(std::holds_alternative<Null>(entry.data))display=u"null";
                else if(const auto* text=std::get_if<std::u16string>(&entry.data))display=*text;
                else if(const auto* boolean=std::get_if<bool>(&entry.data))display=*boolean?u"true":u"false";
                else if(const auto* number=std::get_if<double>(&entry.data))display=property_host::text(js_number::toString(*number));
                else if(std::holds_alternative<SymbolRef>(entry.data))display=u"Symbol()";
                else display=u"[object Object]";
                typeError(u"Iterator value "+display+u" is not an entry object");
            }
            const Value key=context.get(entry,Key(u"0"));
            const Value value=context.get(entry,Key(u"1"));
            body(key,value);
        }catch(...){
            closeOnEntryFailure();
        }
    }
}
bool sameValueZero(const Value& left,const Value& right){
    if(left.data.index()!=right.data.index())return false;
    if(const auto* number=std::get_if<double>(&left.data)){
        const double other=std::get<double>(right.data);
        return *number==other||(std::isnan(*number)&&std::isnan(other));
    }
    return left==right;
}
struct IteratorState{
    MapStorageRef map;
    std::size_t cursor=0;
    enum class Kind{keys,values,entries}kind=Kind::entries;
    bool done=false;
};
class MapStorageOwnerTrace final:public NativeOwnerTrace {
    MapStorageRef owner_;
public:
    explicit MapStorageOwnerTrace(MapStorageRef owner):owner_(std::move(owner)){}
    bool visitContextEdges(const ContextEdgeVisitor& visitor)const override{
        return owner_&&owner_->visitContextEdges(visitor);
    }
};
}

ValueMapStorage::ValueMapStorage()
    :storage_(std::make_shared<NativeStorage>([](const Value& left,const Value& right){
        return sameValueZero(left,right);
    })){}
const void* ValueMapStorage::identity()const noexcept{return storage_.get();}
std::size_t ValueMapStorage::size()const noexcept{return storage_->size();}
std::size_t ValueMapStorage::slotCount()const noexcept{return storage_->slotCount();}
std::optional<std::pair<Value,Value>> ValueMapStorage::entryAt(std::size_t slot)const{
    auto entry=storage_->entryAt(slot);
    if(!entry)return std::nullopt;
    return std::pair<Value,Value>{entry->key,entry->value};
}
bool ValueMapStorage::visitContextEdges(const ContextEdgeVisitor& visitor)const{
    for(std::size_t slot=0;slot<storage_->slotCount();++slot){
        const auto entry=storage_->entryAt(slot);
        if(!entry)continue;
        visitor(entry->key);visitor(entry->value);
    }
    return true;
}
Value ValueMapStorage::get(const Value& key)const{
    const auto* value=storage_->find(key);
    return value?*value:Value(Undefined{});
}
bool ValueMapStorage::has(const Value& key)const{return storage_->has(key);}
void ValueMapStorage::set(Value key,Value value){
    if(const auto* number=std::get_if<double>(&key.data);number&&*number==0)key=Value(0.0);
    storage_->set(std::move(key),std::move(value));
}
bool ValueMapStorage::erase(const Value& key){return storage_->erase(key);}
void ValueMapStorage::clear()noexcept{storage_->clear();}

std::size_t MapStorage::EntriesView::size()const noexcept{return storage?storage->slotCount():0;}
MapEntry MapStorage::EntriesView::operator[](std::size_t slot)const{
    if(!storage)return {Value(Undefined{}),Value(Undefined{}),false};
    auto entry=storage->entryAt(slot);
    return entry?MapEntry{std::move(entry->first),std::move(entry->second),true}
                :MapEntry{Value(Undefined{}),Value(Undefined{}),false};
}

struct MapContextBindings::State:std::enable_shared_from_this<State>{
    Context& context;
    ObjectRef ctor{},proto{};
    struct Instance{const void* identity=nullptr;MapStorageRef storage;ObjectRef object{};};
    std::vector<Instance> instances;

    explicit State(Context& context):context(context){}

    void validateValue(const Value& value)const{
        if(const auto* object=std::get_if<ObjectRef>(&value.data))(void)context.kind(*object);
        if(const auto* symbol=std::get_if<SymbolRef>(&value.data))(void)context.symbolString(*symbol);
    }
    MapStorageRef require(Value receiver)const{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object||context.kind(*object)!=ObjectKind::instance)
            typeError(u"Method Map.prototype called on incompatible receiver");
        for(const auto& entry:instances)if(entry.object==*object)return entry.storage;
        typeError(u"Method Map.prototype called on incompatible receiver");
    }
    ObjectRef bindStorage(MapStorageRef storage){
        if(!storage)typeError(u"Map storage is null");
        for(std::size_t index=0;index<storage->slotCount();++index){
            if(const auto entry=storage->entryAt(index)){validateValue(entry->first);validateValue(entry->second);}
        }
        for(const auto& entry:instances)if(entry.identity==storage->identity())return entry.object;
        const ObjectRef object=context.instance(ctor,storage);
        context.registerNativeOwnerTrace(object,NativeOwnerPart::payload,
            std::make_shared<MapStorageOwnerTrace>(storage));
        instances.push_back({storage->identity(),std::move(storage),object});
        return object;
    }
    ObjectRef makeIterator(MapStorageRef storage,IteratorState::Kind kind){
        auto state=std::make_shared<IteratorState>();state->map=std::move(storage);state->kind=kind;
        return context.nativeIterator([state](Context& cx)->Value{
            Value value=Undefined{};bool done=state->done;
            if(!done){
                std::optional<std::pair<Value,Value>> entry;
                while(state->cursor<state->map->slotCount()&&!entry)
                    entry=state->map->entryAt(state->cursor++);
                if(!entry){
                    state->done=true;state->map.reset();done=true;
                }else{
                    if(state->kind==IteratorState::Kind::keys)value=entry->first;
                    else if(state->kind==IteratorState::Kind::values)value=entry->second;
                    else{
                        const ObjectRef pair=cx.array(2);
                        cx.set(Value(pair),Key(u"0"),entry->first);cx.set(Value(pair),Key(u"1"),entry->second);value=pair;
                    }
                }
            }
            const ObjectRef result=cx.object();
            cx.define(result,Key(u"value"),Descriptor::data(value));
            cx.define(result,Key(u"done"),Descriptor::data(Value(done)));
            return Value(result);
        });
    }
    void method(ObjectRef target,std::u16string name,std::uint32_t arity,NativeCall call){
        ObjectRef fn=context.function(name,std::move(call));
        context.define(fn,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
        context.define(target,Key(std::move(name)),Descriptor::data(Value(fn),true,false,true));
    }
    void initialize(){
        const auto self=shared_from_this();
        ctor=context.nativeConstructor(u"Map",
            [self](Context&,std::span<const Value> args)->ObjectRef{
                const auto object=self->bindStorage(std::make_shared<ValueMapStorage>());
                const Value iterable=arg(args,0);
                if(std::holds_alternative<Undefined>(iterable.data)||std::holds_alternative<Null>(iterable.data))return object;
                // Map obtains the adder once before acquiring the iterator.
                const Value adder=self->context.get(Value(object),Key(u"set"));
                const auto* adderObject=std::get_if<ObjectRef>(&adder.data);
                if(!adderObject||self->context.kind(*adderObject)!=ObjectKind::function)
                    typeError(u"Map.prototype.set is not a function");
                forEachMapIterable(self->context,iterable,[&](Value key,Value value){
                    const std::array<Value,2> args{key,value};
                    (void)self->context.call(*adderObject,Value(object),args);
                });
                return object;
            },
            [](Context&,Value,std::span<const Value>)->Value{
                typeError(u"Constructor Map requires 'new'");
            });
        context.define(ctor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        proto=std::get<ObjectRef>(context.get(Value(ctor),Key(u"prototype")).data);
        // The built-in Map constructor exposes a non-writable, non-enumerable,
        // non-configurable prototype property.
        context.define(ctor,Key(u"prototype"),Descriptor::data(Value(proto),false,false,false));
        context.define(proto,Key(u"constructor"),Descriptor::data(Value(ctor),true,false,true));

        method(proto,u"get",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto storage=self->require(receiver);const Value key=arg(args,0);self->validateValue(key);return storage->get(key);
        });
        method(proto,u"has",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto storage=self->require(receiver);const Value key=arg(args,0);self->validateValue(key);return Value(storage->has(key));
        });
        method(proto,u"set",2,[self](Context&,Value receiver,std::span<const Value> args){
            auto storage=self->require(receiver);Value key=arg(args,0),value=arg(args,1);
            self->validateValue(key);self->validateValue(value);storage->set(std::move(key),std::move(value));return receiver;
        });
        method(proto,u"delete",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto storage=self->require(receiver);const Value key=arg(args,0);self->validateValue(key);return Value(storage->erase(key));
        });
        method(proto,u"clear",0,[self](Context&,Value receiver,std::span<const Value>){self->require(receiver)->clear();return Value(Undefined{});});
        method(proto,u"forEach",1,[self](Context&,Value receiver,std::span<const Value> args){
            const auto storage=self->require(receiver);const Value callback=arg(args,0);
            const auto* function=std::get_if<ObjectRef>(&callback.data);
            if(!function||self->context.kind(*function)!=ObjectKind::function)
                typeError(u"Map.prototype.forEach callback must be a function");
            const Value thisArg=arg(args,1);
            for(std::size_t index=0;index<storage->slotCount();++index){
                const auto entry=storage->entryAt(index);if(!entry)continue;
                const std::array<Value,3> callArgs{entry->second,entry->first,receiver};
                (void)self->context.call(*function,thisArg,callArgs);
            }
            return Value(Undefined{});
        });
        const auto iteratorMethod=[self](IteratorState::Kind kind){
            return [self,kind](Context&,Value receiver,std::span<const Value>)->Value{
                return Value(self->makeIterator(self->require(receiver),kind));
            };
        };
        method(proto,u"entries",0,iteratorMethod(IteratorState::Kind::entries));
        method(proto,u"keys",0,iteratorMethod(IteratorState::Kind::keys));
        method(proto,u"values",0,iteratorMethod(IteratorState::Kind::values));
        const Value entries=context.get(Value(proto),Key(u"entries"));
        context.define(proto,Key(context.intrinsics().symbolIterator),Descriptor::data(entries,true,false,true));

        const auto sizeGetter=context.function(u"get size",[self](Context&,Value receiver,std::span<const Value>){
            return Value(static_cast<double>(self->require(receiver)->size()));
        });
        context.define(sizeGetter,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        context.define(proto,Key(u"size"),Descriptor::access(sizeGetter,std::nullopt,false,true));

    }
};

MapContextBindings::MapContextBindings(Context& context):state_(std::make_shared<State>(context)){state_->initialize();}
MapContextBindings::~MapContextBindings()=default;
Context& MapContextBindings::context()const{return state_->context;}
ObjectRef MapContextBindings::constructor()const{return state_->ctor;}
ObjectRef MapContextBindings::prototype()const{return state_->proto;}
MapStorageRef MapContextBindings::createStorage()const{return std::make_shared<ValueMapStorage>();}
ObjectRef MapContextBindings::bind(const MapStorageRef& storage){return state_->bindStorage(storage);}
ObjectRef MapContextBindings::create(Value iterable){
    const std::array<Value,1> args{std::move(iterable)};
    return state_->context.construct(state_->ctor,args);
}
MapStorageRef MapContextBindings::resolve(ObjectRef object)const{return state_->require(Value(object));}
Value MapContextBindings::get(ObjectRef object,Value key)const{state_->validateValue(key);return resolve(object)->get(key);}
bool MapContextBindings::has(ObjectRef object,Value key)const{state_->validateValue(key);return resolve(object)->has(key);}
ObjectRef MapContextBindings::set(ObjectRef object,Value key,Value value)const{state_->validateValue(key);state_->validateValue(value);resolve(object)->set(std::move(key),std::move(value));return object;}
bool MapContextBindings::erase(ObjectRef object,Value key)const{state_->validateValue(key);return resolve(object)->erase(key);}
void MapContextBindings::clear(ObjectRef object)const{resolve(object)->clear();}
std::size_t MapContextBindings::size(ObjectRef object)const{return resolve(object)->size();}
}
