module folia.math.input.tex.handler_context_bindings;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
using namespace options;
namespace {
[[noreturn]] void typeError(std::u16string message){throw Exception("TypeError",std::move(message));}
Value argument(std::span<const Value> values,std::size_t index){
    return index<values.size()?values[index]:Value(Undefined{});
}

class ContextHandlerMapStorage final:public SubHandlers::Storage {
    MapStorageRef source_;
    std::function<Property(const Value&)> keyFromValue_;
    std::function<Value(const Property&)> keyToValue_;
    std::function<SubHandlerRef(const Value&)> handlerFromValue_;
    std::function<Value(const SubHandlerRef&)> handlerToValue_;
public:
    ContextHandlerMapStorage(MapStorageRef source,
        std::function<Property(const Value&)> keyFromValue,
        std::function<Value(const Property&)> keyToValue,
        std::function<SubHandlerRef(const Value&)> handlerFromValue,
        std::function<Value(const SubHandlerRef&)> handlerToValue)
        :source_(std::move(source)),keyFromValue_(std::move(keyFromValue)),
         keyToValue_(std::move(keyToValue)),handlerFromValue_(std::move(handlerFromValue)),
         handlerToValue_(std::move(handlerToValue)){
        if(!source_||!keyFromValue_||!keyToValue_||!handlerFromValue_||!handlerToValue_)
            typeError(u"SubHandlers Map storage requires actual owner converters");
    }
    const void* identity()const noexcept override{return source_->identity();}
    std::size_t size()const noexcept override{return source_->size();}
    std::size_t slotCount()const noexcept override{return source_->slotCount();}
    std::optional<Entry> entryAt(std::size_t slot)const override{
        const auto entry=source_->entryAt(slot);
        if(!entry)return std::nullopt;
        return Entry{keyFromValue_(entry->first),handlerFromValue_(entry->second)};
    }
    std::optional<SubHandlerRef> get(const Property& key)const override{
        const Value valueKey=keyToValue_(key);
        if(!source_->has(valueKey))return std::nullopt;
        return handlerFromValue_(source_->get(valueKey));
    }
    bool has(const Property& key)const override{return source_->has(keyToValue_(key));}
    void set(Property key,SubHandlerRef value)override{
        source_->set(keyToValue_(key),handlerToValue_(value));
    }
    bool erase(const Property& key)override{return source_->erase(keyToValue_(key));}
    void clear()noexcept override{source_->clear();}
    const MapStorageRef& sourceMap()const noexcept{return source_;}
};
}

struct SubHandlerContextBindings::State:std::enable_shared_from_this<State>{
    Context& context;
    ContextPropertyBindings& properties;
    MapContextBindings& maps;
    TokenMapContextBindings& tokenMaps;
    FunctionListContextBindings& functionLists;
    OptionFunctionBindings& functions;
    MapHandler& registry;
    std::weak_ptr<void> registryLifetime;
    std::weak_ptr<std::atomic_bool> executionLease;
    PrioritizedListContextBindings& prioritized;
    ObjectRef subHandlerCtor{},subHandlerProto{},subHandlersCtor{},subHandlersProto{};
    SymbolRef fallbackSymbol{};

    struct HandlerEntry{std::weak_ptr<SubHandler> owner;ObjectRef object{};};
    struct HandlerListEntry{std::weak_ptr<SubHandlers> owner;ObjectRef object{};};
    struct ConfigurationEntry{
        std::weak_ptr<SubHandler::Configuration> owner;
        ObjectRef object{};
        std::vector<std::pair<std::weak_ptr<PrioritizedListItem<TokenMapRef>>,ObjectRef>> records;
        std::vector<std::pair<PrioritizedList< TokenMapRef>::StorageRef,ObjectRef>> arrays;
    };
    std::vector<HandlerEntry> handlers;
    std::vector<HandlerListEntry> handlerLists;
    std::vector<ConfigurationEntry> configurations;

    State(Context& context,ContextPropertyBindings& properties,MapContextBindings& maps,
          TokenMapContextBindings& tokenMaps,FunctionListContextBindings& functionLists,
          OptionFunctionBindings& functions,MapHandler& registry,
          std::weak_ptr<std::atomic_bool> executionLease)
        :context(context),properties(properties),maps(maps),tokenMaps(tokenMaps),functionLists(functionLists),
         functions(functions),registry(registry),registryLifetime(registry.lifetimeToken()),
         executionLease(std::move(executionLease)),
         prioritized(functionLists.prioritizedLists()){
        if(&properties.context()!=&context||&maps.context()!=&context||
           &tokenMaps.context()!=&context||&functions.context()!=&context)
            typeError(u"SubHandler graph bindings require one shared Context");
    }

    void ensureRegistry()const{
        const auto lease=executionLease.lock();
        if(!lease||!lease->load(std::memory_order_acquire))typeError(u"SubHandler runtime has ended");
        if(registryLifetime.expired())typeError(u"MapHandler registry owner has ended");
    }

    void defineMethod(ObjectRef prototype,std::u16string name,std::uint32_t arity,NativeCall body){
        const ObjectRef method=context.function(name,std::move(body));
        context.define(method,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
        // MapHandler.ts emits anonymous function expressions through direct
        // prototype assignment, so their own name is empty and the prototype
        // properties are enumerable.
        context.define(method,Key(u"name"),Descriptor::data(Value(u""),false,false,true));
        context.define(prototype,Key(std::move(name)),Descriptor::data(Value(method),true,true,true));
    }

    ObjectRef createInstance(ObjectRef constructor){return context.instance(constructor);}

    void initialize(){
        ensureRegistry();
        const auto self=shared_from_this();
        subHandlerCtor=context.nativeConstructor(u"SubHandler",[self](Context&,std::span<const Value>)->ObjectRef{
            self->ensureRegistry();return self->handlerObject(std::make_shared<SubHandler>(self->registry));
        },[](Context&,Value,std::span<const Value>)->Value{typeError(u"Class constructor SubHandler cannot be invoked without 'new'");});
        subHandlerProto=std::get<ObjectRef>(context.get(Value(subHandlerCtor),Key(u"prototype")).data);
        context.define(subHandlerCtor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        context.define(subHandlerCtor,Key(u"prototype"),Descriptor::data(Value(subHandlerProto),true,false,false));
        const Value fallbackValue=tokenMaps.parseResultToValue(SubHandler::FALLBACK);
        fallbackSymbol=std::get<SymbolRef>(fallbackValue.data);
        context.define(subHandlerCtor,Key(u"FALLBACK"),Descriptor::data(fallbackValue,true,true,true));

        subHandlersCtor=context.nativeConstructor(u"SubHandlers",[self](Context&,std::span<const Value>)->ObjectRef{
            self->ensureRegistry();
            auto owner=std::make_shared<SubHandlers>(self->registry);
            const ObjectRef object=self->context.instance(self->subHandlersCtor);
            self->context.setPayload(object,owner);
            return self->bindHandlerList(owner,object);
        },[](Context&,Value,std::span<const Value>)->Value{typeError(u"Class constructor SubHandlers cannot be invoked without 'new'");});
        subHandlersProto=std::get<ObjectRef>(context.get(Value(subHandlersCtor),Key(u"prototype")).data);
        context.define(subHandlersCtor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        context.define(subHandlersCtor,Key(u"prototype"),Descriptor::data(Value(subHandlersProto),true,false,false));
        installSubHandlerMethods();
        installSubHandlersMethods();
    }

    ObjectRef handlerObject(const SubHandlerRef& owner){
        ensureRegistry();
        if(!owner) return {};
        for(auto& entry:handlers)if(auto known=entry.owner.lock();known==owner)return entry.object;
        const ObjectRef object=createInstance(subHandlerCtor);
        context.setPayload(object,owner);
        const std::weak_ptr<State> weakSelf=shared_from_this();
        const std::weak_ptr<SubHandler> weakOwner=owner;
        auto configurationField=std::make_shared<NativeDataField>(owner,
            [weakSelf,weakOwner]{
                const auto self=weakSelf.lock();const auto handler=weakOwner.lock();
                if(!self||!handler)typeError(u"SubHandler owner has expired");
                self->ensureRegistry();
                return Value(self->bindConfiguration(handler->configurationOwner()));
            },
            [weakSelf,weakOwner](Value value){
                const auto self=weakSelf.lock();const auto handler=weakOwner.lock();
                if(!self||!handler)typeError(u"SubHandler owner has expired");
                self->ensureRegistry();
                const auto* identity=std::get_if<ObjectRef>(&value.data);
                if(!identity)typeError(u"SubHandler._configuration must be a PrioritizedList");
                const auto replacement=self->resolveConfiguration(*identity);
                if(!replacement)typeError(u"SubHandler._configuration is not a registered PrioritizedList");
                handler->configurationOwner(replacement);
            });
        context.bindData(object,Key(u"_configuration"),std::move(configurationField));
        (void)functionLists.bind(owner->fallbackOwner());
        auto fallbackField=std::make_shared<NativeDataField>(owner,
            [weakSelf,weakOwner]{
                const auto self=weakSelf.lock();const auto handler=weakOwner.lock();
                if(!self||!handler)typeError(u"SubHandler owner has expired");
                self->ensureRegistry();
                return Value(self->functionLists.bind(handler->fallbackOwner()));
            },
            [weakSelf,weakOwner](Value value){
                const auto self=weakSelf.lock();const auto handler=weakOwner.lock();
                if(!self||!handler)typeError(u"SubHandler owner has expired");
                self->ensureRegistry();
                const auto* identity=std::get_if<ObjectRef>(&value.data);
                if(!identity)typeError(u"SubHandler._fallback must be a FunctionList");
                const auto replacement=self->functionLists.resolve(*identity);
                if(!replacement)typeError(u"SubHandler._fallback is not a registered FunctionList");
                handler->fallbackOwner(replacement);
            });
        context.bindData(object,Key(u"_fallback"),std::move(fallbackField));
        context.setPrototype(object,subHandlerProto);
        handlers.push_back({owner,object});
        return object;
    }

    ObjectRef bindConfiguration(const SubHandler::ConfigurationRef& owner){
        ensureRegistry();
        if(!owner)typeError(u"SubHandler configuration owner is null");
        for(auto& entry:configurations)if(auto known=entry.owner.lock();known==owner)return entry.object;
        const ObjectRef object=context.instance(prioritized.constructor());
        context.setPayload(object,owner);
        const auto index=configurations.size();
        configurations.push_back({owner,object,{},{}});
        const std::weak_ptr<State> weakSelf=shared_from_this();
        const std::weak_ptr<SubHandler::Configuration> weakOwner=owner;
        PrioritizedListContextBindings::Operations operations;
        operations.add=[weakSelf,weakOwner](std::span<const Value> args){
            const auto self=weakSelf.lock();const auto list=weakOwner.lock();
            if(!self||!list)typeError(u"PrioritizedList owner has expired");
            self->ensureRegistry();
            const Value mapValue=argument(args,0);
            const auto* mapObject=std::get_if<ObjectRef>(&mapValue.data);
            if(!mapObject)typeError(u"PrioritizedList<TokenMap>.add requires a TokenMap object");
            auto map=self->tokenMaps.resolveTokenMap(*mapObject);
            if(!map)typeError(u"PrioritizedList<TokenMap>.add received an unbound TokenMap");
            const Value priorityValue=argument(args,1);
            const double priority=std::holds_alternative<Undefined>(priorityValue.data)
                ?self->prioritized.defaultPriority():self->context.toNumber(priorityValue);
            return Value(self->tokenMaps.bind(list->add(map,priority)));
        };
        operations.remove=[weakSelf,weakOwner,object](std::span<const Value> args){
            const auto self=weakSelf.lock();const auto list=weakOwner.lock();
            if(!self||!list)typeError(u"PrioritizedList owner has expired");
            self->ensureRegistry();
            const Value mapValue=argument(args,0);
            const auto* mapObject=std::get_if<ObjectRef>(&mapValue.data);
            if(!mapObject)typeError(u"PrioritizedList<TokenMap>.remove requires a TokenMap object");
            auto map=self->tokenMaps.resolveTokenMap(*mapObject);
            if(!map)typeError(u"PrioritizedList<TokenMap>.remove received an unbound TokenMap");
            list->remove(map);return Value(object);
        };
        operations.items=[weakSelf,weakOwner,index]{
            const auto self=weakSelf.lock();const auto list=weakOwner.lock();
            if(!self||!list)typeError(u"PrioritizedList owner has expired");
            self->ensureRegistry();
            return Value(self->configurationArray(index,list->storageRef()));
        };
        operations.setItems=[weakSelf,weakOwner](Value value){
            const auto self=weakSelf.lock();const auto list=weakOwner.lock();
            if(!self||!list)typeError(u"PrioritizedList owner has expired");
            self->ensureRegistry();
            const auto* array=std::get_if<ObjectRef>(&value.data);
            if(!array||self->context.kind(*array)!=ObjectKind::array)
                typeError(u"PrioritizedList.items must be a bound array");
            auto storage=self->configurationStorage(*array);
            if(!storage)typeError(u"PrioritizedList.items array is not a registered TokenMap array");
            list->rebindStorage(std::move(storage));
        };
        auto items=std::make_shared<NativeDataField>(owner,
            [weakSelf,weakOwner,index]{
                const auto self=weakSelf.lock();const auto list=weakOwner.lock();
                if(!self||!list)typeError(u"PrioritizedList owner has expired");
                return Value(self->configurationArray(index,list->storageRef()));
            },
            [set=operations.setItems](Value value)mutable{set(std::move(value));});
        context.bindData(object,Key(u"items"),std::move(items));
        prioritized.registerOwner(object,owner,std::move(operations));
        return object;
    }

    SubHandler::ConfigurationRef resolveConfiguration(ObjectRef object)const{
        for(const auto& entry:configurations)if(entry.object==object)if(auto owner=entry.owner.lock())return owner;
        return {};
    }

    ObjectRef configurationRecord(std::size_t listIndex,
        const std::shared_ptr<PrioritizedListItem<TokenMapRef>>& record){
        if(!record)typeError(u"PrioritizedList<TokenMap> record is null");
        auto& cache=configurations.at(listIndex).records;
        for(auto& entry:cache)if(auto known=entry.first.lock();known==record)return entry.second;
        const ObjectRef object=context.object();
        context.setPayload(object,record);
        auto self=shared_from_this();
        auto item=std::make_shared<NativeDataField>(record,
            [self,record]{self->ensureRegistry();return record->item?Value(self->tokenMaps.bind(record->item)):Value(Null{});},
            [self,record](Value value){
                self->ensureRegistry();
                if(std::holds_alternative<Null>(value.data)){record->item.reset();return;}
                const auto* map=std::get_if<ObjectRef>(&value.data);
                if(!map)typeError(u"PrioritizedList item must be a TokenMap");
                record->item=self->tokenMaps.resolveTokenMap(*map);
                if(!record->item)typeError(u"PrioritizedList item is not a bound TokenMap");
            });
        auto priority=std::make_shared<NativeDataField>(record,
            [self,record]{self->ensureRegistry();return Value(record->priority);},
            [self,record](Value value){
                self->ensureRegistry();
                if(const auto* number=std::get_if<double>(&value.data))record->priority=*number;
                else typeError(u"PrioritizedList item priority must be a Number");
            });
        context.bindData(object,Key(u"item"),std::move(item));
        context.bindData(object,Key(u"priority"),std::move(priority));
        cache.push_back({record,object});
        return object;
    }

    struct ConfigurationArray final:NativeIndexedView{
        std::weak_ptr<State> state;
        std::size_t listIndex;
        PrioritizedList<TokenMapRef>::StorageRef storage;
        bool lengthWritable=true;
        ConfigurationArray(std::shared_ptr<State> state,std::size_t listIndex,
            PrioritizedList<TokenMapRef>::StorageRef storage)
            :state(std::move(state)),listIndex(listIndex),storage(std::move(storage)){}
        std::shared_ptr<State> owner()const{
            auto result=state.lock();if(!result)typeError(u"PrioritizedList Context bindings have expired");return result;
        }
        std::uint32_t length()const override{
            owner()->ensureRegistry();
            if(!storage||storage->size()>std::numeric_limits<std::uint32_t>::max())
                throw Exception("RangeError",u"PrioritizedList.items has an invalid length");
            return static_cast<std::uint32_t>(storage->size());
        }
        Descriptor lengthDescriptor()const override{return Descriptor::data(Value(static_cast<double>(length())),lengthWritable,false,false);}
        std::optional<Descriptor> indexDescriptor(std::uint32_t index)const override{
            if(index>=length()||!(*storage)[index])return {};
            return Descriptor::data(Value(owner()->configurationRecord(listIndex,(*storage)[index])));
        }
        std::vector<std::uint32_t> ownIndexKeys()const override{
            std::vector<std::uint32_t> result;
            for(std::uint32_t index=0;index<length();++index)if((*storage)[index])result.push_back(index);
            return result;
        }
        void defineIndex(std::uint32_t index,Descriptor descriptor)override{
            owner()->ensureRegistry();
            if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                typeError(u"PrioritizedList.items does not support custom index descriptors");
            const auto* object=std::get_if<ObjectRef>(&descriptor.value.data);
            if(!object)typeError(u"PrioritizedList.items requires a record object");
            const auto& payload=std::as_const(owner()->context).payload(*object);
            const auto* record=std::any_cast<std::shared_ptr<PrioritizedListItem<TokenMapRef>>>(&payload);
            if(!record||!*record)typeError(u"Foreign PrioritizedList record cannot enter this array");
            if(index>=length()){
                if(!lengthWritable)typeError(u"Array length is not writable");
                storage->resize(static_cast<std::size_t>(index)+1);
            }
            (*storage)[index]=*record;
        }
        void defineLength(Descriptor descriptor)override{
            owner()->ensureRegistry();
            if(descriptor.accessor||descriptor.enumerable||descriptor.configurable)
                typeError(u"Invalid PrioritizedList.items length descriptor");
            if(!lengthWritable&&descriptor.writable)typeError(u"Cannot redefine non-writable length");
            const auto* number=std::get_if<double>(&descriptor.value.data);
            if(!number||!std::isfinite(*number)||*number<0||*number>4294967295.0||std::trunc(*number)!=*number)
                throw Exception("RangeError",u"Invalid array length");
            storage->resize(static_cast<std::size_t>(*number));
            lengthWritable=descriptor.writable;
        }
        void deleteIndex(std::uint32_t index)override{owner()->ensureRegistry();if(index<length())(*storage)[index].reset();}
    };

    ObjectRef configurationArray(std::size_t listIndex,const PrioritizedList<TokenMapRef>::StorageRef& storage){
        auto& entry=configurations.at(listIndex);
        for(auto& array:entry.arrays)if(array.first==storage)return array.second;
        auto view=std::make_shared<ConfigurationArray>(shared_from_this(),listIndex,storage);
        const ObjectRef object=context.nativeArray(view);
        entry.arrays.push_back({storage,object});
        return object;
    }

    PrioritizedList<TokenMapRef>::StorageRef configurationStorage(ObjectRef object)const{
        for(const auto& list:configurations)for(const auto& array:list.arrays)if(array.second==object)return array.first;
        return {};
    }

    using HandlerMapProjection=ProjectedMapStorage<Property,SubHandlerRef,SubHandlers::HandlerKeyEqual>;
    std::shared_ptr<HandlerMapProjection> handlerMapProjection(const SubHandlersRef& owner){
        ensureRegistry();
        const std::weak_ptr<State> weakSelf=shared_from_this();
        return std::make_shared<HandlerMapProjection>(owner->sourceMapStorage(),
            [weakSelf](const Value& value){const auto self=weakSelf.lock();if(!self)typeError(u"SubHandler Context bindings have expired");self->ensureRegistry();return self->properties.from(value);},
            [weakSelf](const Property& value){const auto self=weakSelf.lock();if(!self)typeError(u"SubHandler Context bindings have expired");self->ensureRegistry();return self->properties.to(value);},
            [weakSelf](const Value& value){
                const auto self=weakSelf.lock();if(!self)typeError(u"SubHandler Context bindings have expired");self->ensureRegistry();
                if(std::holds_alternative<Null>(value.data))return SubHandlerRef{};
                const auto* identity=std::get_if<ObjectRef>(&value.data);
                if(!identity)typeError(u"SubHandlers.map values must be SubHandler objects");
                auto handler=self->resolveHandler(*identity);
                if(!handler)typeError(u"SubHandlers.map value is not a bound SubHandler");
                return handler;
            },
            [weakSelf](const SubHandlerRef& value){
                const auto self=weakSelf.lock();if(!self)typeError(u"SubHandler Context bindings have expired");self->ensureRegistry();
                return value?Value(self->handlerObject(value)):Value(Null{});
            });
    }

    MapStorageRef handlerMapStorage(const SubHandlersRef& owner){
        ensureRegistry();
        const auto storage=owner->sourceMapStorage();
        if(const auto adapter=std::dynamic_pointer_cast<ContextHandlerMapStorage>(storage))
            return adapter->sourceMap();
        return handlerMapProjection(owner);
    }

    ObjectRef bindHandlerList(const SubHandlersRef& owner,ObjectRef object){
        ensureRegistry();
        if(!owner)typeError(u"SubHandlers owner is null");
        const auto* payload=std::any_cast<SubHandlersRef>(&std::as_const(context).payload(object));
        if(!payload||payload->get()!=owner.get())typeError(u"SubHandlers identity does not retain the supplied owner");
        for(const auto& entry:handlerLists)if(auto known=entry.owner.lock();known==owner){
            if(entry.object!=object)typeError(u"SubHandlers owner already has another Context identity");
            return object;
        }
        const std::weak_ptr<State> weakSelf=shared_from_this();
        auto field=std::make_shared<NativeDataField>(owner,
            [weakSelf,owner]{
                const auto self=weakSelf.lock();if(!self)typeError(u"SubHandler Context bindings have expired");
                self->ensureRegistry();return Value(self->maps.bind(self->handlerMapStorage(owner)));
            },
            [weakSelf,owner](Value value){
                const auto self=weakSelf.lock();if(!self)typeError(u"SubHandler Context bindings have expired");
                self->ensureRegistry();
                const auto* object=std::get_if<ObjectRef>(&value.data);
                if(!object)
                    typeError(u"SubHandlers.map must be a native Map projection");
                auto source=self->maps.resolve(*object);
                if(!source)typeError(u"SubHandlers.map cannot bind a foreign Map store");
                const std::weak_ptr<State> weak= self;
                auto adapter=std::make_shared<ContextHandlerMapStorage>(std::move(source),
                    [weak](const Value& key){auto state=weak.lock();if(!state)typeError(u"SubHandler Context bindings have expired");state->ensureRegistry();return state->properties.from(key);},
                    [weak](const Property& key){auto state=weak.lock();if(!state)typeError(u"SubHandler Context bindings have expired");state->ensureRegistry();return state->properties.to(key);},
                    [weak](const Value& mapped){auto state=weak.lock();if(!state)typeError(u"SubHandler Context bindings have expired");state->ensureRegistry();if(std::holds_alternative<Null>(mapped.data))return SubHandlerRef{};const auto* id=std::get_if<ObjectRef>(&mapped.data);if(!id)typeError(u"SubHandlers.map values must be SubHandler objects");auto handler=state->resolveHandler(*id);if(!handler)typeError(u"SubHandlers.map value is not a bound SubHandler");return handler;},
                    [weak](const SubHandlerRef& mapped){auto state=weak.lock();if(!state)typeError(u"SubHandler Context bindings have expired");state->ensureRegistry();return mapped?Value(state->handlerObject(mapped)):Value(Null{});});
                owner->rebindSourceMapStorage(std::move(adapter));
            });
        context.bindData(object,Key(u"map"),std::move(field));
        context.setPrototype(object,subHandlersProto);
        handlerLists.push_back({owner,object});
        return object;
    }

    SubHandlerRef resolveHandler(ObjectRef object)const{
        for(const auto& entry:handlers)if(entry.object==object)if(auto owner=entry.owner.lock())return owner;
        const auto* owner=std::any_cast<SubHandlerRef>(&std::as_const(context).payload(object));
        if(owner&&*owner){
            for(const auto& entry:handlers)if(auto known=entry.owner.lock();known==*owner&&entry.object==object)return *owner;
        }
        return {};
    }

    SubHandlersRef resolveSubHandlers(ObjectRef object)const{
        for(const auto& entry:handlerLists)if(entry.object==object)if(auto owner=entry.owner.lock())return owner;
        return {};
    }

    SubHandlerRef requireHandler(Value receiver)const{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object||context.kind(*object)!=ObjectKind::instance)typeError(u"SubHandler method called on incompatible receiver");
        const auto owner=resolveHandler(*object);
        if(!owner)typeError(u"SubHandler method called on incompatible receiver");
        return owner;
    }
    SubHandlersRef requireHandlerList(Value receiver)const{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object)typeError(u"SubHandlers method called on incompatible receiver");
        auto owner=resolveSubHandlers(*object);
        if(!owner)typeError(u"SubHandlers method called on incompatible receiver");
        return owner;
    }
    double priorityArgument(std::span<const Value> args,std::size_t index)const{
        const Value value=argument(args,index);
            return std::holds_alternative<Undefined>(value.data)?prioritized.defaultPriority():context.toNumber(value);
    }
    void installSubHandlerMethods(){
        const auto self=shared_from_this();
        defineMethod(subHandlerProto,u"add",3,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandler(std::move(receiver));
            const Value fallback=argument(args,1);
            owner->add(self->context,argument(args,0),self->tokenMaps.parseMethodFromValue(fallback),self->priorityArgument(args,2));
            return Value(Undefined{});
        });
        defineMethod(subHandlerProto,u"remove",2,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandler(std::move(receiver));
            owner->remove(self->context,argument(args,0),self->tokenMaps.parseMethodFromValue(argument(args,1)));
            return Value(Undefined{});
        });
        defineMethod(subHandlerProto,u"parse",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandler(std::move(receiver));
            return self->tokenMaps.parseResultToValue(owner->parse(self->tokenMaps.parseInputFromValue(argument(args,0))));
        });
        defineMethod(subHandlerProto,u"lookup",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandler(std::move(receiver));
            return self->tokenMaps.tokenLookupToValue(owner->lookup(self->tokenMaps.fromValue(argument(args,0))));
        });
        defineMethod(subHandlerProto,u"contains",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandler(std::move(receiver));
            return Value(owner->contains(self->tokenMaps.fromValue(argument(args,0))));
        });
        defineMethod(subHandlerProto,u"toString",0,[self](Context&,Value receiver,std::span<const Value>){
            self->ensureRegistry();return Value(property_host::text(self->requireHandler(std::move(receiver))->toString()));
        });
        defineMethod(subHandlerProto,u"applicable",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();const auto map=self->requireHandler(std::move(receiver))->applicable(self->tokenMaps.fromValue(argument(args,0)));
            return map?Value(self->tokenMaps.bind(map)):Value(Null{});
        });
        defineMethod(subHandlerProto,u"retrieve",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();const auto map=self->requireHandler(std::move(receiver))->retrieve(self->tokenMaps.fromValue(argument(args,0)));
            return map?Value(self->tokenMaps.bind(map)):Value(Null{});
        });
        defineMethod(subHandlerProto,u"warn",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();
            auto owner=self->requireHandler(std::move(receiver));
            const Value primitive=self->context.toPrimitive(argument(args,0),PrimitiveHint::defaultHint);
            owner->sourceWarn(property_host::string(self->properties.from(primitive)));
            return Value(Undefined{});
        });
    }

    void installSubHandlersMethods(){
        const auto self=shared_from_this();
        defineMethod(subHandlersProto,u"add",2,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandlerList(std::move(receiver));
            owner->add(self->context,self->functions,argument(args,0),argument(args,1),self->priorityArgument(args,2));
            return Value(Undefined{});
        });
        defineMethod(subHandlersProto,u"remove",2,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandlerList(std::move(receiver));
            owner->remove(self->context,self->functions,argument(args,0),argument(args,1));
            return Value(Undefined{});
        });
        defineMethod(subHandlersProto,u"set",2,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandlerList(std::move(receiver));
            Property name=self->properties.from(argument(args,0));
            const Value handlerValue=argument(args,1);
            const auto* identity=std::get_if<ObjectRef>(&handlerValue.data);
            if(!identity)typeError(u"SubHandlers.set requires a SubHandler");
            auto handler=self->resolveHandler(*identity);if(!handler)typeError(u"SubHandlers.set received an unbound SubHandler");
            owner->set(std::move(name),std::move(handler));return Value(Undefined{});
        });
        defineMethod(subHandlersProto,u"get",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandlerList(std::move(receiver));
            const Property name=self->properties.from(argument(args,0));
            const auto handler=owner->get(name);return handler?Value(self->handlerObject(handler)):Value(Undefined{});
        });
        defineMethod(subHandlersProto,u"retrieve",1,[self](Context&,Value receiver,std::span<const Value> args){
            self->ensureRegistry();auto owner=self->requireHandlerList(std::move(receiver));
            const auto map=owner->retrieve(self->tokenMaps.fromValue(argument(args,0)));
            return map?Value(self->tokenMaps.bind(map)):Value(Null{});
        });
        defineMethod(subHandlersProto,u"keys",0,[self](Context&,Value receiver,std::span<const Value>)->Value{
            self->ensureRegistry();auto owner=self->requireHandlerList(std::move(receiver));
            auto iterator=std::make_shared<SubHandlers::KeyIterator>(owner->keys());
            const ObjectRef result=self->context.nativeIterator([iterator,self](Context& context)->Value{
                self->ensureRegistry();
                const auto next=iterator->nextProperty();
                const ObjectRef step=context.object();
                context.define(step,Key(u"value"),Descriptor::data(next?self->properties.to(*next):Value(Undefined{})));
                context.define(step,Key(u"done"),Descriptor::data(Value(!next)));
                return Value(step);
            });
            return Value(result);
        });
    }
        };

SubHandlerContextBindings::SubHandlerContextBindings(Context& context,ContextPropertyBindings& properties,
    MapContextBindings& maps,TokenMapContextBindings& tokenMaps,FunctionListContextBindings& functionLists,
    OptionFunctionBindings& functions,MapHandler& registry)
    :ownedExecutionLease_(std::make_shared<std::atomic_bool>(true)),
     state_(std::make_shared<State>(context,properties,maps,tokenMaps,functionLists,functions,registry,ownedExecutionLease_)){
    state_->initialize();
}
SubHandlerContextBindings::SubHandlerContextBindings(Context& context,ContextPropertyBindings& properties,
    MapContextBindings& maps,TokenMapContextBindings& tokenMaps,FunctionListContextBindings& functionLists,
    OptionFunctionBindings& functions,MapHandler& registry,std::weak_ptr<std::atomic_bool> executionLease)
    :state_(std::make_shared<State>(context,properties,maps,tokenMaps,functionLists,functions,registry,std::move(executionLease))){
    state_->initialize();
}
SubHandlerContextBindings::~SubHandlerContextBindings(){if(ownedExecutionLease_)ownedExecutionLease_->store(false,std::memory_order_release);}
ObjectRef SubHandlerContextBindings::bind(const SubHandlersRef& owner,ObjectRef existingIdentity){
    state_->ensureRegistry();
    return state_->bindHandlerList(owner,existingIdentity);
}
ObjectRef SubHandlerContextBindings::bind(const SubHandlerRef& owner){state_->ensureRegistry();return state_->handlerObject(owner);}
ObjectRef SubHandlerContextBindings::subHandlersConstructor()const{state_->ensureRegistry();return state_->subHandlersCtor;}
ObjectRef SubHandlerContextBindings::subHandlersPrototype()const{state_->ensureRegistry();return state_->subHandlersProto;}
ObjectRef SubHandlerContextBindings::subHandlerConstructor()const{state_->ensureRegistry();return state_->subHandlerCtor;}
ObjectRef SubHandlerContextBindings::subHandlerPrototype()const{state_->ensureRegistry();return state_->subHandlerProto;}
SubHandlersRef SubHandlerContextBindings::resolveSubHandlers(ObjectRef object)const{
    state_->ensureRegistry();
    for(const auto& entry:state_->handlerLists)if(entry.object==object)if(auto owner=entry.owner.lock())return owner;
    return {};
}
SubHandlerRef SubHandlerContextBindings::resolveSubHandler(ObjectRef object)const{state_->ensureRegistry();return state_->resolveHandler(object);}
Context& SubHandlerContextBindings::context()const{state_->ensureRegistry();return state_->context;}
}
