// Source-visible CommonWrapperFactory own fields backed by the actual factory
// and creator owners. Creator records and Map adapters remain live views; they
// do not maintain parallel copies of Source-visible state.
export module folia.math.output.common:factory_context_fields;
export import :wrapper_factory;
export import :wrapper_class_bindings;
export import folia.math.util.context_property_bindings;
export import folia.math.util.factory_context_field_views;
export import folia.math.util.factory_context_source_map_access;
export import folia.math.util.map_bindings;
import folia.math.util.property_host;
import std;

export namespace folia::math::output::common {
using namespace options;

// This narrow owner receives canonical class/function identity adapters from
// CommonFactoryContextBindings::State. It binds the real mutable fields on the
// existing factory; it never owns copies of nodeMap or node entries.
class CommonFactoryLiveFields : public std::enable_shared_from_this<CommonFactoryLiveFields> {
public:
    using FactoryOwner=tree::AbstractFactory<CommonWrapper>;
    using MapValue=tree::FactoryClassMapValue<CommonWrapper>;
    using ClassFromValue=std::function<MapValue(const Value&)>;
    using ClassToValue=std::function<Value(const MapValue&)>;
    using NodeFromIdentity=std::function<mml::MmlNode&(ObjectRef)>;
    using NodeToIdentity=std::function<ObjectRef(mml::MmlNode&)>;
    using WrapperFromIdentity=std::function<CommonWrapper&(ObjectRef)>;
    using WrapperToIdentity=std::function<ObjectRef(CommonWrapper&)>;

private:
    Context* context_;
    MapContextBindings* maps_;
    ContextPropertyBindings* properties_;
    std::weak_ptr<std::atomic_bool> executionLease_;
    ClassFromValue classFromValue_;
    ClassToValue classToValue_;
    NodeFromIdentity nodeFromIdentity_;
    NodeToIdentity nodeToIdentity_;
    WrapperFromIdentity wrapperFromIdentity_;
    WrapperToIdentity wrapperToIdentity_;
    std::vector<std::tuple<tree::FactoryCreatorRef<CommonWrapper>,FactoryOwner*,ObjectRef>> creatorObjects_;
    std::vector<std::pair<FactoryOwner*,ObjectRef>> nodeObjects_;

    // Stable context-local identity tokens for non-Map object receivers. Actual
    // Maps return their MapStorage identity; these tokens are only used when a
    // raw assigned object is consumed through Map-shaped methods.
    std::vector<std::pair<ObjectRef,std::shared_ptr<const std::uint8_t>>> sourceMapIdentities_;

    std::shared_ptr<tree::FactoryClassMapAccess<CommonWrapper>> sourceMapAccess(Value receiver){
        const auto self=shared_from_this();
        const std::weak_ptr<CommonFactoryLiveFields> weakSelf=self;
        const auto call=[weakSelf](const Value& target,std::u16string_view method,
                               std::span<const Value> arguments){
            const auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"Common factory field owner has been released");
            auto lease=self->requireExecution();(void)lease;
            const Value callable=self->context_->get(target,Key(std::u16string(method)));
            return self->context_->callMember(callable,target,arguments,method);
        };
        const auto keys=[weakSelf,call](const Value& target){
            const auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"Common factory field owner has been released");
            auto lease=self->requireExecution();(void)lease;
            const Value iterator=call(target,u"keys",{});
            std::vector<Value> result;
            self->context_->forEachIterable(iterator,[&](Value key){result.push_back(std::move(key));});
            return result;
        };
        const auto identity=[weakSelf](const Value& target)->tree::SourceFactoryClassMapAccess<CommonWrapper>::StableIdentity{
            const auto self=weakSelf.lock();
            if(!self)return {};
            auto lease=self->requireExecution();(void)lease;
            const auto* object=std::get_if<ObjectRef>(&target.data);
            if(!object)return {};
            try{
                auto storage=self->maps_->resolve(*object);
                return {storage->identity(),std::static_pointer_cast<const void>(std::move(storage))};
            }catch(...){}
            for(const auto& [known,token]:self->sourceMapIdentities_)
                if(known==*object)return {token.get(),token};
            self->sourceMapIdentities_.emplace_back(*object,std::make_shared<const std::uint8_t>(0));
            const auto& token=self->sourceMapIdentities_.back().second;
            return {token.get(),token};
        };
        const auto ensure=[weakSelf]{
            const auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"Common factory field owner has been released");
            auto lease=self->requireExecution();(void)lease;
        };
        const auto resolve=[weakSelf](const Value& raw){
            const auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"Common factory field owner has been released");
            auto lease=self->requireExecution();(void)lease;
            const auto mapped=self->classFromValue_(raw);
            if(const auto* native=mapped.nativeClass())return *native;
            return tree::FactoryClassRef<CommonWrapper>{};
        };
        const auto toSource=[weakSelf](const MapValue& mapped){
            const auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"Common factory field owner has been released");
            auto lease=self->requireExecution();(void)lease;
            return self->classToValue_(mapped);
        };
        return std::make_shared<tree::SourceFactoryClassMapAccess<CommonWrapper>>(
            std::move(receiver),call,keys,identity,ensure,resolve,toSource);
    }

    std::shared_ptr<std::atomic_bool> requireExecution()const {
        auto lease=executionLease_.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw Exception("TypeError",u"Common factory Context runtime has been released");
        return lease;
    }

    Value sourceValue(const std::any& value)const {
        if(const auto* raw=std::any_cast<Value>(&value))return *raw;
        if(const auto* node=std::any_cast<mml::MmlNode*>(&value))
            return *node?Value(nodeToIdentity_(**node)):Value(Null{});
        if(const auto* wrapper=std::any_cast<CommonWrapper*>(&value))
            return *wrapper?Value(wrapperToIdentity_(**wrapper)):Value(Null{});
        if(const auto* property=std::any_cast<Property>(&value))return properties_->to(*property);
        throw Exception("TypeError",u"Common factory argument has no Source value conversion");
    }

    static std::optional<std::uint32_t> arrayIndex(const Key& key){
        const auto* text=std::get_if<std::u16string>(&key.data);
        if(!text||text->empty()||(text->size()>1&&text->front()==u'0'))return {};
        std::uint64_t value=0;
        for(const char16_t c:*text){
            if(c<u'0'||c>u'9')return {};
            const auto digit=static_cast<unsigned>(c-u'0');
            constexpr auto maxIndex=std::numeric_limits<std::uint32_t>::max()-1;
            if(value>(maxIndex-digit)/10)return {};
            value=value*10+digit;
        }
        return static_cast<std::uint32_t>(value);
    }

    static std::vector<Key> creatorKeys(const tree::FactoryCreatorView<CommonWrapper>& owner){
        using Entry=std::pair<Key,tree::FactoryCreatorSlot<CommonWrapper>>;
        std::vector<const Entry*> indices,strings,symbols;
        indices.reserve(owner->entries.size());
        strings.reserve(owner->entries.size());
        symbols.reserve(owner->entries.size());
        for(const auto& entry:owner->entries){
            if(!std::holds_alternative<std::u16string>(entry.first.data))symbols.push_back(&entry);
            else if(arrayIndex(entry.first))indices.push_back(&entry);
            else strings.push_back(&entry);
        }
        std::stable_sort(indices.begin(),indices.end(),[](const auto* left,const auto* right){
            return *arrayIndex(left->first)<*arrayIndex(right->first);
        });
        std::vector<Key> keys;keys.reserve(owner->entries.size());
        for(const auto* entry:indices)keys.push_back(entry->first);
        for(const auto* entry:strings)keys.push_back(entry->first);
        for(const auto* entry:symbols)keys.push_back(entry->first);
        return keys;
    }

    std::vector<std::any> nativeArguments(std::span<const Value> source)const {
        std::vector<std::any> result;
        result.reserve(source.size());
        for(std::size_t index=0;index<source.size();++index){
            const auto& value=source[index];
            if(index==0){
                const auto* object=std::get_if<ObjectRef>(&value.data);
                if(!object)throw Exception("TypeError",u"Wrapper constructor argument is not an MmlNode");
                result.emplace_back(&nodeFromIdentity_(*object));
            }else if(index==1){
                if(std::holds_alternative<Undefined>(value.data)||std::holds_alternative<Null>(value.data))
                    result.emplace_back(static_cast<CommonWrapper*>(nullptr));
                else {
                    const auto* object=std::get_if<ObjectRef>(&value.data);
                    if(!object)throw Exception("TypeError",u"Wrapper parent is not an object");
                    result.emplace_back(&wrapperFromIdentity_(*object));
                }
            }else result.emplace_back(properties_->from(value));
        }
        return result;
    }

    ObjectRef creatorFunction(const tree::FactoryCreatorRef<CommonWrapper>& creator,
        const std::shared_ptr<FactoryOwner>& factoryOwner) {
        if(!creator)throw Exception("TypeError",u"Cannot expose a null Common factory creator");
        const auto found=std::find_if(creatorObjects_.begin(),creatorObjects_.end(),
            [&](const auto& entry){return std::get<0>(entry)==creator&&std::get<1>(entry)==factoryOwner.get();});
        if(found!=creatorObjects_.end())return std::get<2>(*found);
        const auto weak=std::weak_ptr<std::atomic_bool>(executionLease_);
        const auto self=shared_from_this();
        const auto function=context_->function(u"",[self,weak,creator,factoryOwner](Context& cx,Value,
            std::span<const Value> args)->Value{
            auto lease=weak.lock();
            if(!lease||!lease->load(std::memory_order_acquire))
                throw Exception("TypeError",u"Common factory Context runtime has been released");
            const auto native=self->nativeArguments(args);
            auto* product=creator->callWithServices(*factoryOwner,tree::FactoryArgs(native));
            if(!product)throw Exception("TypeError",u"Common factory creator returned no wrapper");
            return Value(self->wrapperToIdentity_(*product));
        });
        context_->define(function,Key(u"name"),Descriptor::data(Value(u""),false,false,true));
        context_->define(function,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        context_->payload(function)=creator;
        creatorObjects_.emplace_back(creator,factoryOwner.get(),function);
        return function;
    }

    tree::FactoryCreatorRef<CommonWrapper> creatorFromValue(const Value& value)const {
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object)return {};
        try{return std::any_cast<tree::FactoryCreatorRef<CommonWrapper>>(context_->payload(*object));}
        catch(const std::bad_any_cast&){return {};}
    }

    ObjectRef currentNodeObject(const std::shared_ptr<FactoryOwner>& factory) {
        if(const auto source=factory->creatorSourceObject())return *source;
        const auto found=std::find_if(nodeObjects_.begin(),nodeObjects_.end(),
            [&](const auto& entry){return entry.first==factory.get();});
        if(found!=nodeObjects_.end())return found->second;

        // Attach the Context record to the same FactoryCreatorObject retained
        // by creatorView(). Its per-key Descriptor cells are authoritative;
        // NativeRecordView does not maintain a second property dictionary.
        const auto self=shared_from_this();
        auto access=factory->creatorAccess();
        const auto native=access->nativeOwner();
        if(!native)throw Exception("TypeError",u"Common factory has no initial native creator owner");
        for(auto& [name,cell]:native->entries){
            (void)name;
            if(!cell.descriptor){
                const Value value=cell.creator?Value(creatorFunction(cell.creator,factory)):Value(Null{});
                cell.descriptor=Descriptor::data(value);
            }
        }
        std::optional<ObjectRef> prototype=context_->intrinsics().objectPrototype;
        if(native->prototypeAssigned){
            prototype.reset();
            if(native->prototypeCreator)
                prototype=creatorFunction(native->prototypeCreator,factory);
        }
        auto prototypeCell=std::make_shared<std::optional<ObjectRef>>(prototype);
        const auto runtimeWeak=std::weak_ptr<std::atomic_bool>(executionLease_);
        auto view=std::make_shared<tree::FactoryCreatorRecordView>(
            std::static_pointer_cast<void>(factory),
            [native]{return creatorKeys(native);},
            [self,native,runtimeWeak,factory](const Key& key)->std::optional<Descriptor>{
                auto found=std::find_if(native->entries.begin(),native->entries.end(),
                    [&](const auto& entry){return entry.first==key;});
                if(found==native->entries.end())return {};
                if(!found->second.descriptor){
                    auto lease=runtimeWeak.lock();
                    if(!lease||!lease->load(std::memory_order_acquire))
                        throw Exception("TypeError",u"Common factory Context runtime has been released");
                    const Value value=found->second.creator?
                        Value(self->creatorFunction(found->second.creator,factory)):Value(Null{});
                    found->second.descriptor=Descriptor::data(value);
                }
                return found->second.descriptor;
            },
            [native](const Key& key,Descriptor descriptor){
                const auto found=std::find_if(native->entries.begin(),native->entries.end(),
                    [&](const auto& entry){return entry.first==key;});
                if(found==native->entries.end())
                    native->entries.emplace_back(key,tree::FactoryCreatorSlot<CommonWrapper>{{},std::move(descriptor)});
                else found->second.descriptor=std::move(descriptor);
            },
            [native](const Key& key){
                std::erase_if(native->entries,[&](const auto& entry){return entry.first==key;});
            },
            [runtimeWeak,prototypeCell]() -> std::optional<ObjectRef>{
                auto lease=runtimeWeak.lock();
                if(!lease||!lease->load(std::memory_order_acquire))
                    throw Exception("TypeError",u"Common factory Context runtime has been released");
                return *prototypeCell;
            },
            [runtimeWeak,prototypeCell](std::optional<ObjectRef> prototype){
                auto lease=runtimeWeak.lock();
                if(!lease||!lease->load(std::memory_order_acquire))
                    throw Exception("TypeError",u"Common factory Context runtime has been released");
                *prototypeCell=prototype;
            },
            [native,runtimeWeak](const Key& key){
                auto lease=runtimeWeak.lock();
                if(!lease||!lease->load(std::memory_order_acquire))
                    throw Exception("TypeError",u"Common factory Context runtime has been released");
                const auto found=std::find_if(native->entries.begin(),native->entries.end(),
                    [&](const auto& entry){return entry.first==key;});
                return found!=native->entries.end()&&
                    (!found->second.descriptor||found->second.descriptor->enumerable);
            });
        const auto object=context_->nativeRecord(view,prototype);
        const auto nativeAccess=std::dynamic_pointer_cast<tree::FactoryCreatorObjectAccess<CommonWrapper>>(access);
        if(!nativeAccess)throw Exception("TypeError",u"Common factory creator access is not its canonical native owner");
        bindNativeCreatorBridge(nativeAccess,object,factory);
        native->prototypeCreator.reset();
        native->prototypeAssigned=false;
        nodeObjects_.emplace_back(factory.get(),object);
        return object;
    }

    void bindNativeCreatorBridge(
        const std::shared_ptr<tree::FactoryCreatorObjectAccess<CommonWrapper>>& access,ObjectRef object,
        const std::shared_ptr<FactoryOwner>& factoryOwner) {
        using Selection=tree::FactoryCreatorSelection<CommonWrapper>;
        using Prepared=tree::PreparedFactoryCreatorCall<CommonWrapper>;
        const auto weak=std::weak_ptr<std::atomic_bool>(executionLease_);
        const auto self=shared_from_this();
        auto require=[weak](){
            auto lease=weak.lock();
            if(!lease||!lease->load(std::memory_order_acquire))
                throw Exception("TypeError",u"Common factory Context runtime has been released");
            return lease;
        };
        tree::FactoryCreatorContextBridge<CommonWrapper> bridge;
        bridge.node=object;
        bridge.runtimeLease=executionLease_;
        bridge.ownerLease=std::static_pointer_cast<void>(factoryOwner);
        bridge.lookup=[self,require](ObjectRef receiver,const Key& key)->Selection{
            auto lease=require();(void)lease;
            const Value value=self->context_->get(Value(receiver),key);
            const auto* function=std::get_if<ObjectRef>(&value.data);
            if(function&&self->context_->kind(*function)==ObjectKind::function)
                return {tree::FactoryCreatorSelectionKind::creator,self->creatorFromValue(value),value};
            return {tree::FactoryCreatorSelectionKind::source_value,{},value};
        };
        bridge.prepareApply=[self,require](const Selection& selected)->Prepared{
            auto lease=require();(void)lease;
            return {selected,self->context_->get(selected.sourceValue,Key(u"apply"))};
        };
        bridge.invokePrepared=[self,require](const Prepared& selected,tree::FactoryArgs args)->CommonWrapper*{
            auto lease=require();(void)lease;
            const auto array=self->context_->array(static_cast<std::uint32_t>(args.size()));
            for(std::size_t index=0;index<args.size();++index){
                const auto key=std::to_string(index);
                self->context_->set(Value(array),Key(std::u16string(key.begin(),key.end())),self->sourceValue(args[index]));
            }
            const std::array<Value,2> applyArgs{Value(Undefined{}),Value(array)};
            const Value result=self->context_->callMember(selected.applyValue,selected.selection.sourceValue,
                applyArgs,u"node[kind].apply");
            const auto* wrapper=std::get_if<ObjectRef>(&result.data);
            if(!wrapper)throw Exception("TypeError",u"Common factory creator returned no wrapper object");
            return &self->wrapperFromIdentity_(*wrapper);
        };
        bridge.invokeDirect=[self,require](const Value& receiver,const Selection& selected,
                                           tree::FactoryArgs args)->CommonWrapper*{
            auto lease=require();(void)lease;
            const auto* function=std::get_if<ObjectRef>(&selected.sourceValue.data);
            if(!function)throw Exception("TypeError",u"this.node[kind] is not a function");
            std::vector<Value> values;values.reserve(args.size());
            for(const auto& argument:args)values.push_back(self->sourceValue(argument));
            const Value result=self->context_->call(*function,receiver,values);
            const auto* wrapper=std::get_if<ObjectRef>(&result.data);
            if(!wrapper)throw Exception("TypeError",u"Common factory creator returned no wrapper object");
            return &self->wrapperFromIdentity_(*wrapper);
        };
        const std::weak_ptr<FactoryOwner> weakFactory=factoryOwner;
        bridge.expose=[self,require,weakFactory](const tree::FactoryCreatorRef<CommonWrapper>& creator){
            auto lease=require();(void)lease;
            auto factory=weakFactory.lock();
            if(!factory)throw Exception("TypeError",u"Common factory owner has been released");
            return Value(self->creatorFunction(creator,factory));
        };
        bridge.ownKeys=[self,require](ObjectRef receiver){auto lease=require();(void)lease;return self->context_->ownKeys(Value(receiver));};
        bridge.prototype=[self,require](ObjectRef receiver){auto lease=require();(void)lease;return self->context_->prototype(receiver);};
        bridge.setPrototype=[self,require](ObjectRef receiver,std::optional<ObjectRef> prototype){
            auto lease=require();(void)lease;self->context_->setPrototype(receiver,prototype);};
        access->bindContextNode(std::move(bridge));
    }

    void installContextCreatorOwner(const std::shared_ptr<FactoryOwner>& factory,
                                    Value object) {
        const auto weak=std::weak_ptr<std::atomic_bool>(executionLease_);
        const auto self=shared_from_this();
        auto read=[self,weak](const Value& receiver,const Key& key){auto lease=weak.lock();if(!lease||!lease->load())throw Exception("TypeError",u"Common factory runtime has been released");return self->context_->get(receiver,key);};
        auto write=[self,weak](const Value& receiver,const Key& key,Value value){auto lease=weak.lock();if(!lease||!lease->load())throw Exception("TypeError",u"Common factory runtime has been released");self->context_->set(receiver,key,std::move(value));};
        auto objectOf=[](const Value& receiver){const auto* object=std::get_if<ObjectRef>(&receiver.data);if(!object)throw Exception("TypeError",u"Cannot convert undefined or null to object");return *object;};
        auto remove=[self,weak,objectOf](const Value& receiver,const Key& key){auto lease=weak.lock();if(!lease||!lease->load())throw Exception("TypeError",u"Common factory runtime has been released");self->context_->erase(objectOf(receiver),key);};
        auto ownKeys=[self,weak](const Value& receiver){auto lease=weak.lock();if(!lease||!lease->load())throw Exception("TypeError",u"Common factory runtime has been released");return self->context_->ownKeys(receiver);};
        auto readPrototype=[self,weak,objectOf](const Value& receiver){auto lease=weak.lock();if(!lease||!lease->load())throw Exception("TypeError",u"Common factory runtime has been released");return self->context_->prototype(objectOf(receiver));};
        auto writePrototype=[self,weak,objectOf](const Value& receiver,std::optional<ObjectRef> prototype){auto lease=weak.lock();if(!lease||!lease->load())throw Exception("TypeError",u"Common factory runtime has been released");self->context_->setPrototype(objectOf(receiver),prototype);};
        auto isCallable=[self](const Value& value){
            auto lease=self->requireExecution();(void)lease;
            const auto* object=std::get_if<ObjectRef>(&value.data);
            return object&&self->context_->kind(*object)==ObjectKind::function;
        };
        auto resolve=[self](const Value& value){auto lease=self->requireExecution();(void)lease;return self->creatorFromValue(value);};
        const std::weak_ptr<FactoryOwner> weakFactory=factory;
        auto expose=[self,weakFactory](const tree::FactoryCreatorRef<CommonWrapper>& creator){
            auto lease=self->requireExecution();(void)lease;
            auto owner=weakFactory.lock();
            if(!owner)throw Exception("TypeError",u"Common factory owner has been released");
            return Value(self->creatorFunction(creator,owner));
        };
        auto readApply=[self](const Value& selected){auto lease=self->requireExecution();(void)lease;return self->context_->get(selected,Key(u"apply"));};
        auto apply=[self](const Value& selected,const Value& applyFunction,tree::FactoryArgs args)->CommonWrapper*{
            auto lease=self->requireExecution();(void)lease;
            const auto* function=std::get_if<ObjectRef>(&selected.data);
            if(!function)throw Exception("TypeError",u"Common factory creator is not a function");
            const auto sourceArgs=self->context_->array(static_cast<std::uint32_t>(args.size()));
            for(std::size_t i=0;i<args.size();++i){
                const auto index=std::to_string(i);
                self->context_->set(Value(sourceArgs),Key(std::u16string(index.begin(),index.end())),self->sourceValue(args[i]));
            }
            const std::array<Value,2> applyArgs{Value(Undefined{}),Value(sourceArgs)};
            const auto result=self->context_->callMember(applyFunction,selected,applyArgs,u"node[kind].apply");
            const auto* wrapper=std::get_if<ObjectRef>(&result.data);
            if(!wrapper)throw Exception("TypeError",u"Common factory creator returned no wrapper object");
            return &self->wrapperFromIdentity_(*wrapper);
        };
        auto callDirect=[self](const Value& receiver,const Value& selected,
                               tree::FactoryArgs args)->CommonWrapper*{
            auto lease=self->requireExecution();(void)lease;
            const auto* function=std::get_if<ObjectRef>(&selected.data);
            if(!function)throw Exception("TypeError",u"this.node[kind] is not a function");
            std::vector<Value> values;values.reserve(args.size());
            for(const auto& argument:args)values.push_back(self->sourceValue(argument));
            const Value result=self->context_->call(*function,receiver,values);
            const auto* wrapper=std::get_if<ObjectRef>(&result.data);
            if(!wrapper)throw Exception("TypeError",u"Common factory creator returned no wrapper object");
            return &self->wrapperFromIdentity_(*wrapper);
        };
        auto replacement=std::make_shared<tree::ContextFactoryCreatorAccess<CommonWrapper>>(
            std::move(object),std::weak_ptr<void>(factory),executionLease_,
            std::move(read),std::move(write),std::move(remove),std::move(ownKeys),
            std::move(readPrototype),std::move(writePrototype),std::move(isCallable),std::move(resolve),
            std::move(expose),std::move(readApply),std::move(apply),std::move(callDirect));
        factory->creatorAccess(std::move(replacement));
    }

public:
    CommonFactoryLiveFields(Context& context,MapContextBindings& maps,
        ContextPropertyBindings& properties,std::weak_ptr<std::atomic_bool> executionLease,
        ClassFromValue classFromValue,ClassToValue classToValue,
        NodeFromIdentity nodeFromIdentity,NodeToIdentity nodeToIdentity,
        WrapperFromIdentity wrapperFromIdentity,WrapperToIdentity wrapperToIdentity)
        :context_(&context),maps_(&maps),properties_(&properties),
         executionLease_(std::move(executionLease)),classFromValue_(std::move(classFromValue)),
         classToValue_(std::move(classToValue)),nodeFromIdentity_(std::move(nodeFromIdentity)),
         nodeToIdentity_(std::move(nodeToIdentity)),wrapperFromIdentity_(std::move(wrapperFromIdentity)),
         wrapperToIdentity_(std::move(wrapperToIdentity)){}

    void bind(const std::shared_ptr<FactoryOwner>& factory,ObjectRef identity,
              std::shared_ptr<void> owner) {
        auto lease=requireExecution();
        const auto self=shared_from_this();
        if(!factory)throw Exception("TypeError",u"Cannot bind a null CommonWrapperFactory");

        auto defaultKind=std::make_shared<NativeDataField>(owner,
            [factory]{return factory->defaultKind;},
            [self,factory](Value value){
                auto lease=self->requireExecution();(void)lease;
                factory->defaultKindValue(std::move(value));
            },
            [self,factory]{auto lease=self->requireExecution();(void)lease;
                factory->defaultKindValue(Value(Undefined{}));});
        factory->defaultKindKey([self](const Value& value){
            auto lease=self->requireExecution();(void)lease;
            const Value primitive=self->context_->toPrimitive(value,PrimitiveHint::stringHint);
            if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
            if(const auto* text=std::get_if<std::u16string>(&primitive.data))return Key(*text);
            return Key(property_host::string(self->properties_->from(primitive)));
        });
        context_->bindData(identity,Key(u"defaultKind"),std::move(defaultKind));

        auto mapField=std::make_shared<NativeDataField>(owner,
            [self,factory](){
                auto lease=self->requireExecution();(void)lease;
                if(const auto raw=factory->nodeMapSourceValue())return *raw;
                auto current=tree::projectFactoryClassMap<CommonWrapper>(factory->nodeMapAccess(),self->classToValue_);
                const Value mapValue(self->maps_->bind(current));
                const auto* object=std::get_if<ObjectRef>(&mapValue.data);
                if(!object)throw Exception("TypeError",u"Map binding did not produce an object");
                auto storage=self->maps_->resolve(*object);
                auto access=tree::typedFactoryClassMap<CommonWrapper>(storage,self->classFromValue_,self->classToValue_);
                access->sourceValue(mapValue);
                factory->nodeMapAccess(std::move(access));
                return mapValue;
            },
            [self,factory](Value value){
                auto lease=self->requireExecution();(void)lease;
                factory->nodeMapAccess(self->sourceMapAccess(value));
            },
            [self,factory]{auto lease=self->requireExecution();(void)lease;
                factory->nodeMapAccess(self->sourceMapAccess(Value(Undefined{})));});
        context_->bindData(identity,Key(u"nodeMap"),std::move(mapField));

        currentNodeObject(factory);
        auto nodeField=std::make_shared<NativeDataField>(owner,
            [self,factory](){
                auto lease=self->requireExecution();(void)lease;
                if(const auto source=factory->creatorAccess()->sourceValue())return *source;
                if(const auto source=factory->creatorSourceObject())return Value(*source);
                return Value(self->currentNodeObject(factory));
            },
            [self,factory](Value value){
                auto lease=self->requireExecution();(void)lease;
                const Value retained=value;
                self->installContextCreatorOwner(factory,std::move(value));
                if(const auto* object=std::get_if<ObjectRef>(&retained.data)){
                    const auto found=std::find_if(self->nodeObjects_.begin(),self->nodeObjects_.end(),
                        [&](const auto& entry){return entry.first==factory.get();});
                    if(found==self->nodeObjects_.end())self->nodeObjects_.emplace_back(factory.get(),*object);
                    else found->second=*object;
                }
            },
            [self,factory]{auto lease=self->requireExecution();(void)lease;
                self->installContextCreatorOwner(factory,Value(Undefined{}));});
        context_->bindData(identity,Key(u"node"),std::move(nodeField));

        // Native layout calls (CommonWrapperFactory::wrap/createWrapper) and
        // the Source Context method share these live field readers and one
        // AbstractFactory selection/fallback/apply sequence. Weak captures
        // avoid retaining the Context binding state through the factory.
        using CreateHooks=tree::AbstractFactory<CommonWrapper>::SourceCreateHooks;
        CreateHooks hooks;
        const auto weakSelf=weak_from_this();
        factory->sourceNodeMapReader([weakSelf,identity](){
            auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            const Value current=self->context_->get(Value(identity),Key(u"nodeMap"));
            return self->sourceMapAccess(current);
        });
        const std::weak_ptr<FactoryOwner> weakFactoryForReader=factory;
        factory->sourceCreatorAccessReader([weakSelf,identity,weakFactoryForReader](){
            auto self=weakSelf.lock();
            if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            auto factory=weakFactoryForReader.lock();
            if(!factory)throw Exception("TypeError",u"Common factory owner has been released");
            const Value current=self->context_->get(Value(identity),Key(u"node"));
            auto access=factory->creatorAccess();
            if(const auto source=access->sourceValue();source&&*source==current)return access;
            self->installContextCreatorOwner(factory,current);
            return factory->creatorAccess();
        });
        hooks.readNode=[weakSelf,identity]{
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            return self->context_->get(Value(identity),Key(u"node"));
        };
        hooks.readDefaultKind=[weakSelf,identity]{
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            return self->context_->get(Value(identity),Key(u"defaultKind"));
        };
        hooks.toKey=[weakSelf](const Value& value){
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            const Value primitive=self->context_->toPrimitive(value,PrimitiveHint::stringHint);
            if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
            return Key(property_host::string(self->properties_->from(primitive)));
        };
        hooks.lookup=[weakSelf](const Value& node,const Key& key){
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            return self->context_->get(node,key);
        };
        hooks.readApply=[weakSelf](const Value& selected){
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            return self->context_->get(selected,Key(u"apply"));
        };
        hooks.argumentValue=[weakSelf](const std::any& value){
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            return self->sourceValue(value);
        };
        hooks.invoke=[weakSelf](const Value& selected,const Value& apply,std::span<const Value> arguments){
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            const auto forwarded=self->context_->array(static_cast<std::uint32_t>(arguments.size()));
            for(std::size_t index=0;index<arguments.size();++index){
                const auto ascii=std::to_string(index);
                self->context_->set(Value(forwarded),Key(std::u16string(ascii.begin(),ascii.end())),arguments[index]);
            }
            const std::array<Value,2> applyArguments{Value(Undefined{}),Value(forwarded)};
            return self->context_->callMember(apply,selected,applyArguments,u"node[kind].apply");
        };
        hooks.resolveProduct=[weakSelf](const Value& result){
            auto self=weakSelf.lock();if(!self)throw Exception("TypeError",u"Common factory fields have been released");
            auto lease=self->requireExecution();(void)lease;
            const auto* object=std::get_if<ObjectRef>(&result.data);
            if(!object)throw Exception("TypeError",u"Common factory creator did not return a wrapper object");
            return &self->wrapperFromIdentity_(*object);
        };
        factory->sourceCreateHooks(std::move(hooks));
        (void)lease;
    }

    Value wrappers(const std::shared_ptr<FactoryOwner>& factory){
        auto lease=requireExecution();(void)lease;
        if(const auto source=factory->creatorAccess()->sourceValue())return *source;
        return Value(currentNodeObject(factory));
    }
};
}
