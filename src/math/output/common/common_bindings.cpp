// Canonical Context/native bindings for MathJax output/common.
module folia.math.output.common;
import :bindings;
import :factory_context_fields;
import folia.math.core.output_jax;
import folia.math.core.tree.factory_context_bindings;
import folia.math.core.math_document;
import folia.math.core.math_render_actions;
import folia.math.util.native_identities;
import folia.math.util.option_types;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
using namespace options;

namespace {
class BoundStyleJsonSheet final : public style_json::StyleJsonSheet {
public:
    using style_json::StyleJsonSheet::StyleJsonSheet;
    PropertyList& sourceStyles(){return styles_;}
    const PropertyList& sourceStyles()const{return styles_;}
    void replaceSourceStyles(PropertyList styles){styles_=std::move(styles);}
};

std::u16string valueText(const Property& value){return property_host::string(value);}

struct WrapperContextLifetime {
    std::shared_ptr<LayoutJob> job;
    std::shared_ptr<NativeObject> output;
};
}

struct CommonFactoryContextBindings::State : std::enable_shared_from_this<State> {
    struct ConstructorEntry {
        std::u16string name;
        ObjectRef object;
    CommonFactoryContextBindings::FactoryAllocator allocate;
    };
    Context& context;
    NativeIdentityBindings<CommonWrapperFactory*> factories;
    NativeIdentityBindings<CommonWrapper*> wrappers;
    std::vector<std::shared_ptr<ConstructorEntry>> constructors;
    std::vector<std::pair<ObjectRef,CommonWrapperClassRef>> wrapperClasses;
    ContextPropertyBindings* properties=nullptr;
    MmlNodeResolver resolveMmlNode;
    MmlNodeIdentityResolver resolveMmlNodeIdentity;
    MapContextBindings* maps=nullptr;
    std::shared_ptr<CommonFactoryLiveFields> liveFields;
    std::shared_ptr<tree::FactoryContextBindings<CommonWrapper,mml::MmlNode>> genericFactoryBindings;
    std::shared_ptr<tree::FactoryContextBaseModule> sharedFactoryBase;
    util::RuntimeModules* runtimeModules=nullptr;
    std::weak_ptr<std::atomic_bool> executionLifetime;
    std::optional<ObjectRef> abstractFactoryConstructor;
    std::optional<ObjectRef> abstractWrapperFactoryConstructor;
    std::optional<ObjectRef> abstractFactoryPrototype;
    std::optional<ObjectRef> abstractWrapperFactoryPrototype;

    explicit State(Context& cx):context(cx),factories(cx),wrappers(cx){}

    void installFactoryMethods(){
        if(abstractFactoryPrototype)return;
        if(!genericFactoryBindings)
            throw Exception("TypeError",u"Generic Factory Context bindings must be initialized before Common constructors");
        abstractFactoryConstructor=genericFactoryBindings->abstractFactoryConstructor();
        abstractWrapperFactoryConstructor=genericFactoryBindings->abstractWrapperFactoryConstructor();
        abstractFactoryPrototype=genericFactoryBindings->abstractFactoryPrototype();
        abstractWrapperFactoryPrototype=genericFactoryBindings->abstractWrapperFactoryPrototype();
    }
    std::optional<ObjectRef> constructor(std::u16string_view name)const {
        const auto found=std::find_if(constructors.begin(),constructors.end(),[&](const auto& entry){return entry->name==name;});
        if(found==constructors.end())return {};
        return (*found)->object;
    }
    std::shared_ptr<CommonWrapperFactory> resolve(ObjectRef object)const {
        auto* identity=factories.resolve(object);
        const auto* payload=std::any_cast<NativeBindingPayload<CommonWrapperFactory*>>(&context.payload(object));
        if(!payload||!payload->lifetime)
            throw Exception("Unimplemented",u"CommonWrapperFactory Context identity has no retained actual owner");
        auto owner=std::static_pointer_cast<CommonWrapperFactory>(payload->lifetime);
        if(owner.get()!=identity)
            throw Exception("Unimplemented",u"CommonWrapperFactory Context identity does not match its owner");
        return owner;
    }
    CommonWrapperClassRef wrapperClass(ObjectRef actualConstructor)const {
        context.kind(actualConstructor);
        const auto found=std::find_if(wrapperClasses.begin(),wrapperClasses.end(),
            [&](const auto& entry){return entry.first==actualConstructor;});
        if(found==wrapperClasses.end())
            throw Exception("TypeError",u"Wrapper constructor has no canonical native class descriptor");
        const auto* classData=std::any_cast<CommonWrapperClassData>(&found->second->staticData);
        if(!classData||!classData->sourceConstructor||*classData->sourceConstructor!=actualConstructor)
            throw Exception("TypeError",u"Wrapper class descriptor lost its Source constructor identity");
        return found->second;
    }
    CommonOutputJax& resolveJax(const Value& value)const {
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object){
            if(std::holds_alternative<Null>(value.data))
                throw Exception("TypeError",u"Cannot read properties of null (reading 'font')");
            if(std::holds_alternative<Undefined>(value.data))
                throw Exception("TypeError",u"Cannot read properties of undefined (reading 'font')");
            throw Exception("TypeError",u"CommonWrapperFactory.jax is not a CommonOutputJax object");
        }
        context.kind(*object);
        const auto* stored=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(*object));
        if(!stored||!*stored)
            throw Exception("TypeError",u"CommonWrapperFactory.jax is not a CommonOutputJax object");
        auto output=std::dynamic_pointer_cast<CommonOutputJax>(*stored);
        if(!output||output->instanceIdentity()!=*object)
            throw Exception("TypeError",u"CommonWrapperFactory.jax is not a CommonOutputJax object");
        return *output;
    }
    ObjectRef bind(const std::shared_ptr<CommonWrapperFactory>& factory,ObjectRef constructorRef){
        if(!factory)throw Exception("TypeError",u"Cannot bind a null CommonWrapperFactory");
        // Native constructor callbacks and Source wrapper.factory reads both
        // reach this binder.  The latter must reuse the factory's first
        // canonical instance: rebuilding its jax field would collide with the
        // field installed by bindData and could also lose its subclass
        // constructor/prototype identity.
        if(const auto existing=factories.find(factory.get())){
            if(resolve(*existing)!=factory)
                throw Exception("Unimplemented",u"CommonWrapperFactory identity resolves to a different actual owner");
            return *existing;
        }
        auto owner=std::static_pointer_cast<void>(factory);
        const auto object=factories.instance(factory.get(),constructorRef,owner);
        if(!genericFactoryBindings)
            throw Exception("TypeError",u"Generic Factory Context bindings are not initialized");
        genericFactoryBindings->bindBase(
            std::static_pointer_cast<tree::AbstractFactory<CommonWrapper>>(factory),object);
        auto field=std::make_shared<NativeDataField>(owner,
            [factory]{return factory->jaxValue;},
            [factory](Value value){factory->jaxValue=std::move(value);});
        context.bindData(object,Key(u"jax"),std::move(field));
        std::weak_ptr<State> weak=shared_from_this();
        factory->bindJaxResolver([weak](const Value& value)->CommonOutputJax&{
            auto state=weak.lock();
            if(!state)throw Exception("Unimplemented",u"Common factory Context binding has been released");
            return state->resolveJax(value);
        });
        return object;
    }
    ObjectRef bindWrapper(CommonWrapper& wrapper,CommonWrapperClassRef descriptor,
                          std::shared_ptr<LayoutJob> job){
        if(!job)throw Exception("TypeError",u"Cannot bind a wrapper without its active layout job");
        if(!descriptor)throw Exception("TypeError",u"Cannot bind a wrapper without its selected class");
        const auto* data=std::any_cast<CommonWrapperClassData>(&descriptor->staticData);
        if(!data||!data->sourceConstructor)
            throw Exception("TypeError",u"Wrapper class has no Source constructor identity");
        auto& output=wrapper.factory().jax();
        const auto* outputOwner=std::any_cast<std::shared_ptr<NativeObject>>(
            &context.payload(output.instanceIdentity()));
        if(!outputOwner||!*outputOwner)
            throw Exception("TypeError",u"Common output Jax has no retained native owner");
        auto lifetime=std::make_shared<WrapperContextLifetime>();
        lifetime->job=std::move(job);
        lifetime->output=*outputOwner;
        return wrappers.instance(&wrapper,*data->sourceConstructor,
                                 std::static_pointer_cast<void>(lifetime));
    }
    ObjectRef objectForWrapper(const CommonWrapper& wrapper)const{
        const auto object=wrappers.find(const_cast<CommonWrapper*>(&wrapper));
        if(!object)throw Exception("TypeError",u"Common wrapper has no canonical Context identity");
        return *object;
    }
};

CommonRuntimeBindings::CommonRuntimeBindings(util::RuntimeModules& modules,
    FontDataContextBindings& fonts,CommonFactoryContextBindings& factories,
    folia::math::ContextPropertyBindings& properties)
    :modules(modules),fonts(fonts),factories(factories),properties(properties),
     bboxes(modules.context(),properties){
    auto* context=&modules.context();
    if(context!=&fonts.context()||context!=&factories.context()||
       context!=&properties.context())
        throw Exception("TypeError",u"Common output bindings must share one actual Context");
}
Context& CommonRuntimeBindings::context()const{return modules.context();}

CommonFactoryContextBindings::CommonFactoryContextBindings(Context& context)
    :state_(std::make_shared<State>(context)){}
CommonFactoryContextBindings::~CommonFactoryContextBindings()=default;
Context& CommonFactoryContextBindings::context()const{return state_->context;}
std::shared_ptr<void> CommonFactoryContextBindings::lifetimeToken()const{
    return std::static_pointer_cast<void>(state_);
}
CommonFactoryContextBindings::WrapperClassResolver
CommonFactoryContextBindings::wrapperClassResolver()const{
    const std::weak_ptr<State> weak=state_;
    return [weak](ObjectRef constructor){
        const auto state=weak.lock();
        if(!state)throw Exception("Unimplemented",u"Common wrapper class bindings have been released");
        return state->wrapperClass(constructor);
    };
}
CommonFactoryContextBindings::FactoryResolver
CommonFactoryContextBindings::factoryResolver()const{
    const std::weak_ptr<State> weak=state_;
    return [weak](ObjectRef object){
        const auto state=weak.lock();
        if(!state)throw Exception("Unimplemented",u"Common factory bindings have been released");
        return state->resolve(object);
    };
}

ObjectRef CommonFactoryContextBindings::registerConstructor(std::u16string name,
    std::optional<ObjectRef> parentConstructor,FactoryAllocator allocator){
    auto state=state_;
    if(const auto existing=state->constructor(name))return *existing;
    state->installFactoryMethods();
    if(!allocator)allocator=[](Context&,ObjectRef,std::span<const Value>){return std::make_shared<CommonWrapperFactory>();};
    auto entry=std::make_shared<State::ConstructorEntry>();
    entry->name=name;
    entry->allocate=std::move(allocator);
    auto native=state->context.nativeConstructor(name,
        [weak=std::weak_ptr<State>(state),entry](Context& cx,std::span<const Value> args)->ObjectRef{
            auto owner=weak.lock();
            if(!owner)throw Exception("Unimplemented",u"Common factory module has been released");
            if(entry->object==ObjectRef{})throw Exception("Unimplemented",u"Common factory constructor is not registered");
            auto factory=entry->allocate(cx,entry->object,args);
            if(!factory)throw Exception("TypeError",u"Common factory constructor returned no factory");
            return owner->bind(factory,entry->object);
        });
    entry->object=native;
    state->context.define(native,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    state->context.define(native,Key(u"name"),Descriptor::data(Value(name),false,false,true));
    const auto prototype=std::get<ObjectRef>(state->context.get(Value(native),Key(u"prototype")).data);
    if(!parentConstructor&&name==u"CommonWrapperFactory")
        parentConstructor=state->abstractWrapperFactoryConstructor;
    if(parentConstructor){
        state->context.setPrototype(native,parentConstructor);
        const auto parentPrototype=state->context.get(Value(*parentConstructor),Key(u"prototype"));
        const auto* parent=std::get_if<ObjectRef>(&parentPrototype.data);
        if(!parent)throw Exception("TypeError",u"CommonWrapperFactory parent has no prototype object");
        state->context.setPrototype(prototype,*parent);
    }
    // The pinned CJS getter is installed with Object.defineProperty and is
    // therefore non-enumerable. It returns the creator object itself.
    auto getter=state->context.function(u"get Wrappers",
        [weak=std::weak_ptr<State>(state)](Context& cx,Value receiver,std::span<const Value>)->Value{
            const auto owner=weak.lock();
            if(!owner)throw Exception("Unimplemented",u"Common factory module has been released");
            const auto* object=std::get_if<ObjectRef>(&receiver.data);
            if(!object)throw Exception("TypeError",u"CommonWrapperFactory receiver is not an object");
            const auto* payload=std::any_cast<NativeBindingPayload<CommonWrapperFactory*>>(&cx.payload(*object));
            if(!payload||!payload->identity)throw Exception("TypeError",u"CommonWrapperFactory receiver is not bound");
            if(!owner->liveFields)
                throw Exception("Unimplemented",u"Common factory live fields are not bound");
            const auto factory=owner->resolve(*object);
            return owner->liveFields->wrappers(factory);
        });
    state->context.define(prototype,Key(u"Wrappers"),Descriptor::access(getter,std::nullopt,false,true));
    state->constructors.emplace_back(std::move(entry));
    return native;
}
void CommonFactoryContextBindings::bindFactoryMethods(util::RuntimeModules& modules,ContextPropertyBindings& properties,
    MapContextBindings& maps,MmlNodeResolver resolveMmlNode,
    MmlNodeIdentityResolver resolveMmlNodeIdentity,
    std::weak_ptr<std::atomic_bool> executionLifetime){
    if(&modules.context()!=&state_->context||&properties.context()!=&state_->context||&maps.context()!=&state_->context||
       !resolveMmlNode||!resolveMmlNodeIdentity)
        throw Exception("TypeError",u"Common factory methods require same-Context canonical bindings");
    if(state_->genericFactoryBindings){
        const auto sameExecutionOwner=!state_->executionLifetime.owner_before(executionLifetime)&&
            !executionLifetime.owner_before(state_->executionLifetime);
        if(state_->properties!=&properties||state_->maps!=&maps||state_->runtimeModules!=&modules||!sameExecutionOwner)
            throw Exception("TypeError",u"Common factory methods are already bound to another runtime owner");
        return;
    }
    if(state_->properties&&state_->properties!=&properties)
        throw Exception("TypeError",u"Common factory methods are already bound to another property owner");
    state_->properties=&properties;
    state_->maps=&maps;
    state_->runtimeModules=&modules;
    state_->sharedFactoryBase=modules.getOrCreate<tree::FactoryContextBaseModule>(u"core/Tree/Factory",[&]{
        return std::make_shared<tree::FactoryContextBaseModule>(modules);
    });
    state_->resolveMmlNode=std::move(resolveMmlNode);
    state_->resolveMmlNodeIdentity=std::move(resolveMmlNodeIdentity);
    state_->executionLifetime=std::move(executionLifetime);
    const std::weak_ptr<State> weakSelf=state_;
    auto classFrom=[weakSelf](const Value& value)->tree::FactoryClassMapValue<CommonWrapper>{
        const auto self=weakSelf.lock();
        if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
        if(std::holds_alternative<Undefined>(value.data)||std::holds_alternative<Null>(value.data))return tree::FactoryClassMapValue<CommonWrapper>::source(value);
        const auto* constructor=std::get_if<ObjectRef>(&value.data);
        if(!constructor)throw Exception("TypeError",u"nodeMap value is not a wrapper constructor");
        return tree::FactoryClassMapValue<CommonWrapper>::native(self->wrapperClass(*constructor));
    };
    auto classTo=[weakSelf](const tree::FactoryClassMapValue<CommonWrapper>& mapped)->Value{
        const auto self=weakSelf.lock();
        if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
        if(const auto* raw=mapped.sourceValue())return *raw;
        const auto* descriptor=mapped.nativeClass();
        if(!descriptor||!*descriptor)throw Exception("TypeError",u"nodeMap class descriptor is empty");
        const auto* data=std::any_cast<CommonWrapperClassData>(&(*descriptor)->staticData);
        if(!data||!data->sourceConstructor)
            throw Exception("TypeError",u"nodeMap class descriptor has no Source constructor");
        return Value(*data->sourceConstructor);
    };
    state_->liveFields=std::make_shared<CommonFactoryLiveFields>(state_->context,maps,properties,
        state_->executionLifetime,std::move(classFrom),std::move(classTo),
        [weakSelf](ObjectRef identity)->mml::MmlNode&{
            const auto self=weakSelf.lock();
            if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
            return self->resolveMmlNode(identity);
        },
        [weakSelf](mml::MmlNode& node)->ObjectRef{
            const auto self=weakSelf.lock();
            if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
            if(!self->resolveMmlNodeIdentity)
                throw Exception("TypeError",u"MML node identity resolver is unavailable");
            return self->resolveMmlNodeIdentity(node);
        },
        [weakSelf](ObjectRef identity)->CommonWrapper&{
            const auto self=weakSelf.lock();
            if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
            return *self->wrappers.resolve(identity);
        },
        [weakSelf](CommonWrapper& wrapper)->ObjectRef{
            const auto self=weakSelf.lock();
            if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
            return self->objectForWrapper(wrapper);
        });

    using Bindings=tree::FactoryContextBindings<CommonWrapper,mml::MmlNode>;
    Bindings::Hooks hooks;
    hooks.executionLease=state_->executionLifetime;
    hooks.resolveBase=[weakSelf](ObjectRef receiver)->std::optional<Bindings::BaseOwner>{
        const auto self=weakSelf.lock();
        if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
        const auto& payload=std::as_const(self->context).payload(receiver);
        if(!payload.has_value())return {};
        if(const auto* common=std::any_cast<NativeBindingPayload<CommonWrapperFactory*>>(&payload)){
            if(!common->identity||!common->lifetime)
                throw Exception("TypeError",u"Common factory receiver has an invalid native owner");
            auto owner=self->resolve(receiver);
            if(owner.get()!=common->identity)
                throw Exception("TypeError",u"Common factory receiver owner identity mismatch");
            return std::static_pointer_cast<tree::AbstractFactory<CommonWrapper>>(owner);
        }
        if(const auto* base=std::any_cast<NativeBindingPayload<tree::AbstractFactory<CommonWrapper>*>>(&payload)){
            if(!base->identity||!base->lifetime)
                throw Exception("TypeError",u"AbstractFactory receiver has an invalid native owner");
            auto owner=std::static_pointer_cast<tree::AbstractFactory<CommonWrapper>>(base->lifetime);
            if(owner.get()!=base->identity)
                throw Exception("TypeError",u"AbstractFactory receiver owner identity mismatch");
            return owner;
        }
        throw Exception("TypeError",u"Factory receiver already has an incompatible native owner");
    };
    hooks.bindBase=[weakSelf](const Bindings::BaseOwner& owner,ObjectRef receiver){
        const auto self=weakSelf.lock();
        if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
        const auto& payload=std::as_const(self->context).payload(receiver);
        if(payload.has_value()){
            if(const auto* common=std::any_cast<NativeBindingPayload<CommonWrapperFactory*>>(&payload)){
                const auto resolved=self->resolve(receiver);
                if(!common->identity||resolved.get()!=common->identity||
                   static_cast<tree::AbstractFactory<CommonWrapper>*>(resolved.get())!=owner.get())
                    throw Exception("TypeError",u"Cannot replace an existing Common factory owner");
                return;
            }
            if(const auto* base=std::any_cast<NativeBindingPayload<tree::AbstractFactory<CommonWrapper>*>>(&payload)){
                if(base->identity!=owner.get())
                    throw Exception("TypeError",u"Cannot replace an existing AbstractFactory owner");
                return;
            }
            throw Exception("TypeError",u"Factory receiver already has an incompatible native owner");
        }
        self->context.setPayload(receiver,NativeBindingPayload<tree::AbstractFactory<CommonWrapper>*>{
            owner.get(),std::static_pointer_cast<void>(owner)});
    };
    hooks.allocateBase=[](ObjectRef){
        return std::make_shared<tree::AbstractFactory<CommonWrapper>>();
    };
    hooks.allocateWrapper=[](ObjectRef){
        return std::static_pointer_cast<tree::AbstractFactory<CommonWrapper>>(
            std::make_shared<tree::AbstractWrapperFactory<CommonWrapper,mml::MmlNode>>());
    };
    hooks.initializeFields=[weakSelf](const Bindings::BaseOwner& owner,ObjectRef receiver){
        const auto self=weakSelf.lock();
        if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
        if(!self->liveFields)throw Exception("Unimplemented",u"Common factory field owner is not initialized");
        self->liveFields->bind(owner,receiver,std::static_pointer_cast<void>(owner));
    };
    hooks.instanceOf=[weakSelf](Context& context,const Value& instance,const Value& constructor){
        const auto self=weakSelf.lock();
        if(!self)throw Exception("Unimplemented",u"Common factory bindings have been released");
        return ordinaryInstanceOf(context,instance,constructor);
    };
    state_->genericFactoryBindings=std::make_shared<Bindings>(
        state_->context,state_->sharedFactoryBase,std::move(hooks));
    state_->installFactoryMethods();
}
ObjectRef CommonFactoryContextBindings::constructor()const{
    const auto value=state_->constructor(u"CommonWrapperFactory");
    if(!value)throw Exception("Unimplemented",u"CommonWrapperFactory constructor is not registered");
    return *value;
}
ObjectRef CommonFactoryContextBindings::bind(const std::shared_ptr<CommonWrapperFactory>& factory){
    return state_->bind(factory,constructor());
}
ObjectRef CommonFactoryContextBindings::bind(const std::shared_ptr<CommonWrapperFactory>& factory,
    ObjectRef actualConstructor){
    state_->context.kind(actualConstructor);
    return state_->bind(factory,actualConstructor);
}
std::shared_ptr<CommonWrapperFactory> CommonFactoryContextBindings::resolve(ObjectRef object)const{return state_->resolve(object);}
std::shared_ptr<CommonWrapperFactory> CommonFactoryContextBindings::construct(ObjectRef constructorRef,
    std::span<const Value> args){
    const auto object=state_->context.construct(constructorRef,args);
    return state_->resolve(object);
}
void CommonFactoryContextBindings::bindWrapperFactory(CommonWrapperFactory& factory){
    if(&factory.jax().optionsContext()!=&state_->context)
        throw Exception("TypeError",u"Common wrapper factory belongs to a different Context");
    const std::weak_ptr<State> weak=state_;
    factory.bindWrapperInstances(
        [weak](CommonWrapper& wrapper,CommonWrapperClassRef descriptor,
               std::shared_ptr<LayoutJob> job)->ObjectRef{
            auto state=weak.lock();
            if(!state)throw Exception("Unimplemented",u"Common wrapper Context bindings have been released");
            return state->bindWrapper(wrapper,std::move(descriptor),std::move(job));
        },
        [weak](const CommonWrapper& wrapper)->ObjectRef{
            auto state=weak.lock();
            if(!state)throw Exception("Unimplemented",u"Common wrapper Context bindings have been released");
            return state->objectForWrapper(wrapper);
        });
}
CommonWrapper& CommonFactoryContextBindings::resolveWrapper(ObjectRef object)const{
    return *state_->wrappers.resolve(object);
}
CommonFactoryContextBindings::OwnerCounts CommonFactoryContextBindings::ownerCountsForDiagnostics()const{
    std::unordered_set<const LayoutJob*> jobs;
    state_->wrappers.visitCanonicalHandles([&](CommonWrapper*,ObjectRef object){
        const auto& value=std::as_const(state_->context).payload(object);
        const auto* binding=std::any_cast<NativeBindingPayload<CommonWrapper*>>(&value);
        if(!binding||!binding->lifetime)return;
        const auto lease=std::static_pointer_cast<WrapperContextLifetime>(binding->lifetime);
        if(lease&&lease->job)jobs.insert(lease->job.get());
    });
    return {state_->wrappers.canonicalHandleCount(),jobs.size()};
}
std::size_t CommonFactoryContextBindings::retireRendererLayoutJob(const std::shared_ptr<LayoutJob>& job){
    if(!job)return 0;
    std::vector<CommonWrapper*> retired;
    state_->wrappers.visitCanonicalHandles([&](CommonWrapper* wrapper,ObjectRef object){
        const auto& payload=std::as_const(state_->context).payload(object);
        const auto* binding=std::any_cast<NativeBindingPayload<CommonWrapper*>>(&payload);
        if(!binding||!binding->lifetime)return;
        const auto lifetime=std::static_pointer_cast<WrapperContextLifetime>(binding->lifetime);
        if(lifetime&&lifetime->job==job)retired.push_back(wrapper);
    });
    std::size_t count=0;
    for(auto* wrapper:retired)if(state_->wrappers.erase(wrapper))++count;
    return count;
}
CommonOutputJax& CommonFactoryContextBindings::resolveJax(const Value& value)const{return state_->resolveJax(value);}
void CommonFactoryContextBindings::bindWrapperClass(CommonWrapperClassRef descriptor,
    ObjectRef actualConstructor){
    if(!descriptor)throw Exception("TypeError",u"Cannot bind an empty wrapper class descriptor");
    state_->context.kind(actualConstructor);
    auto* classData=std::any_cast<CommonWrapperClassData>(&descriptor->staticData);
    if(!classData)
        throw Exception("TypeError",u"Wrapper class descriptor has no canonical CommonWrapperClassData");
    if(classData->sourceConstructor&&*classData->sourceConstructor!=actualConstructor)
        throw Exception("TypeError",u"Wrapper descriptor is already bound to a different Source constructor");
    const auto found=std::find_if(state_->wrapperClasses.begin(),state_->wrapperClasses.end(),
        [&](const auto& entry){return entry.first==actualConstructor;});
    if(found!=state_->wrapperClasses.end()){
        if(found->second!=descriptor)
            throw Exception("TypeError",u"Wrapper constructor is already bound to another native class descriptor");
        return;
    }
    classData->sourceConstructor=actualConstructor;
    state_->wrapperClasses.emplace_back(actualConstructor,std::move(descriptor));
}
CommonWrapperClassRef CommonFactoryContextBindings::resolveWrapperClass(ObjectRef actualConstructor)const{
    return state_->wrapperClass(actualConstructor);
}

struct LinebreakVisitorContextBindings::State
    : std::enable_shared_from_this<LinebreakVisitorContextBindings::State> {
    Context& context;
    CommonFactoryContextBindings& factories;
    NativeIdentityBindings<Linebreaks*> instances;
    std::optional<ObjectRef> baseConstructor;
    std::optional<ObjectRef> visitorConstructor;

    State(Context& cx,CommonFactoryContextBindings& bindings)
        :context(cx),factories(bindings),instances(cx){
        if(&cx!=&bindings.context())
            throw Exception("TypeError",u"LinebreakVisitor and Common factory bindings must share one Context");
    }
    std::shared_ptr<Linebreaks> resolve(ObjectRef object)const{
        auto* identity=instances.resolve(object);
        const auto* payload=std::any_cast<NativeBindingPayload<Linebreaks*>>(&context.payload(object));
        if(!payload||!payload->lifetime)
            throw Exception("Unimplemented",u"Linebreaks Context identity has no retained actual owner");
        auto owner=std::static_pointer_cast<Linebreaks>(payload->lifetime);
        if(owner.get()!=identity)
            throw Exception("Unimplemented",u"Linebreaks Context identity does not match its owner");
        return owner;
    }
    ObjectRef bind(const std::shared_ptr<Linebreaks>& visitor,ObjectRef constructor){
        if(!visitor)throw Exception("TypeError",u"Cannot bind a null Linebreaks instance");
        return instances.instance(visitor.get(),constructor,std::static_pointer_cast<void>(visitor));
    }
};

LinebreakVisitorContextBindings::LinebreakVisitorContextBindings(
    Context& context,CommonFactoryContextBindings& factories)
    :state_(std::make_shared<State>(context,factories)){}
LinebreakVisitorContextBindings::~LinebreakVisitorContextBindings()=default;
Context& LinebreakVisitorContextBindings::context()const{return state_->context;}
ObjectRef LinebreakVisitorContextBindings::registerConstructor(){
    auto state=state_;
    if(state->visitorConstructor)return *state->visitorConstructor;
    auto baseNative=state->context.nativeConstructor(u"Linebreaks",
        [weak=std::weak_ptr<State>(state)](Context&,std::span<const Value> args)->ObjectRef{
            auto owner=weak.lock();
            if(!owner)throw Exception("Unimplemented",u"Linebreaks module has been released");
            if(!owner->baseConstructor)throw Exception("Unimplemented",u"Linebreaks constructor is not registered");
            if(args.empty())throw Exception("TypeError",u"Linebreaks constructor requires a wrapper factory");
            const auto* factoryObject=std::get_if<ObjectRef>(&args.front().data);
            if(!factoryObject)throw Exception("TypeError",u"Linebreaks factory is not an object");
            auto factory=owner->factories.resolve(*factoryObject);
            auto visitor=std::make_shared<Linebreaks>(*factory);
            return owner->bind(visitor,*owner->baseConstructor);
        });
    state->baseConstructor=baseNative;
    state->context.define(baseNative,Key(u"length"),Descriptor::data(Value(1),false,false,true));
    state->context.define(baseNative,Key(u"name"),Descriptor::data(Value(u"Linebreaks"),false,false,true));
    const auto basePrototype=std::get<ObjectRef>(state->context.get(Value(baseNative),Key(u"prototype")).data);
    const auto breakToWidth=state->context.function(u"breakToWidth",
        [](Context&,Value,std::span<const Value>)->Value{
            // The Source base implementation is intentionally a no-op.
            return Undefined{};
        });
    state->context.define(breakToWidth,Key(u"name"),Descriptor::data(Value(u"breakToWidth"),false,false,true));
    state->context.define(breakToWidth,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    state->context.define(basePrototype,Key(u"breakToWidth"),Descriptor::data(Value(breakToWidth),true,false,true));

    auto derivedNative=state->context.nativeConstructor(u"LinebreakVisitor",
        [weak=std::weak_ptr<State>(state)](Context&,std::span<const Value> args)->ObjectRef{
            auto owner=weak.lock();
            if(!owner)throw Exception("Unimplemented",u"LinebreakVisitor module has been released");
            if(!owner->visitorConstructor)throw Exception("Unimplemented",u"LinebreakVisitor constructor is not registered");
            if(args.empty())throw Exception("TypeError",u"LinebreakVisitor constructor requires a wrapper factory");
            const auto* factoryObject=std::get_if<ObjectRef>(&args.front().data);
            if(!factoryObject)throw Exception("TypeError",u"LinebreakVisitor factory is not an object");
            auto factory=owner->factories.resolve(*factoryObject);
            auto visitor=std::make_shared<LinebreakVisitor>(*factory);
            return owner->bind(visitor,*owner->visitorConstructor);
        });
    state->visitorConstructor=derivedNative;
    state->context.define(derivedNative,Key(u"length"),Descriptor::data(Value(1),false,false,true));
    state->context.define(derivedNative,Key(u"name"),Descriptor::data(Value(u"LinebreakVisitor"),false,false,true));
    const auto derivedPrototype=std::get<ObjectRef>(state->context.get(Value(derivedNative),Key(u"prototype")).data);
    state->context.setPrototype(derivedNative,baseNative);
    state->context.setPrototype(derivedPrototype,basePrototype);
    return derivedNative;
}
ObjectRef LinebreakVisitorContextBindings::constructor()const{
    if(!state_->visitorConstructor)
        throw Exception("TypeError",u"LinebreakVisitor constructor is not registered");
    return *state_->visitorConstructor;
}
ObjectRef LinebreakVisitorContextBindings::bind(const std::shared_ptr<Linebreaks>& visitor,
    ObjectRef constructor){
    state_->context.kind(constructor);
    return state_->bind(visitor,constructor);
}
std::shared_ptr<Linebreaks> LinebreakVisitorContextBindings::resolve(ObjectRef object)const{
    return state_->resolve(object);
}
std::optional<ObjectRef> LinebreakVisitorContextBindings::objectFor(const Linebreaks* visitor)const{
    if(!visitor)return {};
    return state_->instances.find(const_cast<Linebreaks*>(visitor));
}
std::pair<std::shared_ptr<Linebreaks>,ObjectRef> LinebreakVisitorContextBindings::construct(
    ObjectRef constructor,std::span<const Value> args){
    const auto object=state_->context.construct(constructor,args);
    return {state_->resolve(object),object};
}

struct StyleJsonContextBindings::State : std::enable_shared_from_this<State> {
    Context& context;
    ContextPropertyBindings& properties;
    NativeIdentityBindings<style_json::StyleJsonSheet*> instances;
    std::optional<ObjectRef> ctor;
    explicit State(Context& cx,ContextPropertyBindings& bindings)
        :context(cx),properties(bindings),instances(cx){}

    std::shared_ptr<style_json::StyleJsonSheet> resolve(ObjectRef object)const{
        auto* identity=instances.resolve(object);
        const auto* payload=std::any_cast<NativeBindingPayload<style_json::StyleJsonSheet*>>(&context.payload(object));
        if(!payload||!payload->lifetime)
            throw Exception("Unimplemented",u"StyleJsonSheet Context identity has no retained actual owner");
        auto owner=std::static_pointer_cast<style_json::StyleJsonSheet>(payload->lifetime);
        if(owner.get()!=identity)throw Exception("Unimplemented",u"StyleJsonSheet Context identity does not match its owner");
        return owner;
    }
    ObjectRef bind(const std::shared_ptr<style_json::StyleJsonSheet>& value){
        if(!value)throw Exception("TypeError",u"Cannot bind a null StyleJsonSheet");
        const auto* bound=dynamic_cast<const BoundStyleJsonSheet*>(value.get());
        if(!bound)throw Exception("Unimplemented",u"StyleJsonSheet owner does not expose its actual styles field");
        if(!ctor)throw Exception("Unimplemented",u"StyleJsonSheet class is not registered");
        auto lifetime=std::static_pointer_cast<void>(value);
        const auto object=instances.instance(value.get(),*ctor,lifetime);
        auto selfState=shared_from_this();
        auto styles=std::make_shared<NativeDataField>(lifetime,
            [selfState,value](){
                auto actual=std::dynamic_pointer_cast<BoundStyleJsonSheet>(value);
                return selfState->properties.to(Property(actual->sourceStyles()));
            },
            [selfState,value](Value source){
                Property converted=selfState->properties.from(std::move(source));
                auto actual=std::dynamic_pointer_cast<BoundStyleJsonSheet>(value);
                auto record=PropertyList::record(converted);
                if(!record)throw Exception("Unimplemented",u"Replacing StyleJsonSheet.styles with a non-record is outside the native typed owner profile");
                actual->replaceSourceStyles(*record);
            });
        context.bindData(object,Key(u"styles"),std::move(styles));
        return object;
    }
    std::shared_ptr<style_json::StyleJsonSheet> self(Value receiver)const{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object)throw Exception("TypeError",u"StyleJsonSheet method called on an incompatible receiver");
        return resolve(*object);
    }
    Value arg(std::span<const Value> arguments,std::size_t index)const{
        return index<arguments.size()?arguments[index]:Value(Undefined{});
    }
    ObjectRef install(){
        if(ctor)return *ctor;
        auto selfState=shared_from_this();
        const auto constructor=context.nativeConstructor(u"StyleJsonSheet",
            [selfState](Context&,std::span<const Value> args)->ObjectRef{
                Value initial=args.empty()?Value(Null{}):args.front();
                auto owner=std::make_shared<BoundStyleJsonSheet>(selfState->properties.from(std::move(initial)));
                return selfState->bind(owner);
            });
        ctor=constructor;
        context.define(constructor,Key(u"length"),Descriptor::data(Value(1),false,false,true));
        context.define(constructor,Key(u"name"),Descriptor::data(Value(u"StyleJsonSheet"),false,false,true));
        const auto prototype=std::get<ObjectRef>(context.get(Value(constructor),Key(u"prototype")).data);
        auto method=[&](std::u16string name,std::uint32_t arity,NativeCall body){
            const auto function=context.function(name,std::move(body));
            context.define(function,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
            context.define(prototype,Key(name),Descriptor::data(Value(function),true,true,true));
        };
        std::weak_ptr<State> weak=selfState;
        auto cssGetter=context.function(u"get cssText",[weak](Context&,Value receiver,std::span<const Value>)->Value{
            auto state=weak.lock();if(!state)throw Exception("Unimplemented",u"StyleJsonSheet module has been released");
            return state->self(std::move(receiver))->cssText();
        });
        context.define(prototype,Key(u"cssText"),Descriptor::access(cssGetter,std::nullopt,false,true));
        method(u"addStyles",1,[weak](Context&,Value receiver,std::span<const Value> args)->Value{
            auto state=weak.lock();if(!state)throw Exception("Unimplemented",u"StyleJsonSheet module has been released");
            state->self(std::move(receiver))->addStyles(state->properties.from(state->arg(args,0)));return Undefined{};
        });
        method(u"removeStyles",0,[weak](Context&,Value receiver,std::span<const Value> args)->Value{
            auto state=weak.lock();if(!state)throw Exception("Unimplemented",u"StyleJsonSheet module has been released");
            auto sheet=state->self(std::move(receiver));
            for(const auto& arg:args){const std::array<std::u16string,1> selector{valueText(state->properties.from(arg))};sheet->removeStyles(selector);}
            return Undefined{};
        });
        method(u"clear",0,[weak](Context&,Value receiver,std::span<const Value>)->Value{
            auto state=weak.lock();if(!state)throw Exception("Unimplemented",u"StyleJsonSheet module has been released");
            state->self(std::move(receiver))->clear();return Undefined{};
        });
        method(u"getStyleString",0,[weak](Context&,Value receiver,std::span<const Value>)->Value{
            auto state=weak.lock();if(!state)throw Exception("Unimplemented",u"StyleJsonSheet module has been released");
            return state->self(std::move(receiver))->getStyleString();
        });
        method(u"getStyleRules",2,[weak](Context&,Value receiver,std::span<const Value> args)->Value{
            auto state=weak.lock();if(!state)throw Exception("Unimplemented",u"StyleJsonSheet module has been released");
            const auto styles=args.empty()?Property(Missing{}):state->properties.from(args[0]);
            const auto spacesValue=state->properties.from(state->arg(args,1));
            const auto spaces=spacesValue.missing()?std::u16string{}:valueText(spacesValue);
            const auto rules=state->self(std::move(receiver))->getStyleRules(styles,spaces);
            const auto array=state->context.array(static_cast<std::uint32_t>(rules.size()));
            for(std::size_t i=0;i<rules.size();++i){
                const auto ascii=std::to_string(i);
                state->context.set(Value(array),Key(std::u16string(ascii.begin(),ascii.end())),Value(rules[i]));
            }
            return array;
        });
        method(u"getStyleDefString",2,[weak](Context&,Value receiver,std::span<const Value> args)->Value{
            auto state=weak.lock();if(!state)throw Exception("Unimplemented",u"StyleJsonSheet module has been released");
            const auto styles=state->properties.from(state->arg(args,0));
            const auto spaces=valueText(state->properties.from(state->arg(args,1)));
            return state->self(std::move(receiver))->getStyleDefString(styles,spaces);
        });
        return constructor;
    }
};

StyleJsonContextBindings::StyleJsonContextBindings(Context& context,ContextPropertyBindings& properties)
    :state_(std::make_shared<State>(context,properties)){}
StyleJsonContextBindings::~StyleJsonContextBindings()=default;
ObjectRef StyleJsonContextBindings::constructor(){return state_->install();}
ObjectRef StyleJsonContextBindings::bind(const std::shared_ptr<style_json::StyleJsonSheet>& sheet){state_->install();return state_->bind(sheet);}
std::shared_ptr<style_json::StyleJsonSheet> StyleJsonContextBindings::resolve(ObjectRef object)const{return state_->resolve(object);}
std::shared_ptr<style_json::StyleJsonSheet> StyleJsonContextBindings::construct(std::span<const Value> args){
    const auto object=state_->context.construct(state_->install(),args);return state_->resolve(object);
}

CommonJaxClassBindings::CommonJaxClassBindings(OptionFunctionBindings& functions,
    FunctionListContextBindings& filters,CommonRuntimeBindings& runtime)
    :context_(&functions.context()),functions_(&functions),filters_(&filters),runtime_(&runtime){
    if(context_!=&runtime.context())
        throw Exception("TypeError",u"Common Jax bindings must share the runtime Context");
}

ObjectRef CommonJaxClassBindings::bindClassDefaults(ObjectRef actualClass){
    auto& context=*context_;
    context.kind(actualClass);
    if(context.hasOwn(Value(actualClass),Key(u"OPTIONS")))return actualClass;
    // AbstractOutputJax.OPTIONS is inherited by the actual Common constructor;
    // make the Source object-spread result as one new ordinary record.
    const auto parent=context.prototype(actualClass);
    const ObjectRef options=context.object();
    if(parent){
        const Value base=context.get(Value(*parent),Key(u"OPTIONS"));
        if(const auto* source=std::get_if<ObjectRef>(&base.data))
            for(const auto& key:context.enumerableStringKeys(base))
                context.set(Value(options),Key(key),context.get(base,Key(key)));
        else if(!std::holds_alternative<Undefined>(base.data)&&!std::holds_alternative<Null>(base.data))
            throw Exception("TypeError",u"AbstractOutputJax.OPTIONS is not an object");
    }
    auto set=[&](std::u16string_view key,Value value){context.set(Value(options),Key(std::u16string(key)),std::move(value));};
    set(u"scale",1.0);set(u"minScale",.5);set(u"mtextInheritFont",false);set(u"merrorInheritFont",false);
    set(u"mtextFont",u"");set(u"merrorFont",u"serif");set(u"mathmlSpacing",false);
    set(u"skipAttributes",Value(context.object()));set(u"exFactor",.5);set(u"displayAlign",u"center");
    set(u"displayIndent",u"0");set(u"displayOverflow",u"overflow");
    const auto breaks=context.object();context.set(Value(breaks),Key(u"inline"),true);
    context.set(Value(breaks),Key(u"width"),u"100%");context.set(Value(breaks),Key(u"lineleading"),.2);
    context.set(Value(breaks),Key(u"LinebreakVisitor"),Null{});set(u"linebreaks",breaks);
    set(u"font",u"");set(u"fontExtensions",context.array());set(u"htmlHDW",u"auto");
    set(u"wrapperFactory",Null{});set(u"fontData",Null{});set(u"fontPath",u"@mathjax/%%FONT%%-font");
    set(u"styleJson",Null{});
    context.define(actualClass,Key(u"OPTIONS"),Descriptor::data(Value(options),true,true,true));
    context.set(Value(actualClass),Key(u"NAME"),u"Common");
    const ObjectRef commonStyles=context.object();
    auto rules=[&](std::u16string_view selector,std::initializer_list<std::pair<std::u16string_view,Value>> entries){
        const auto style=context.object();for(const auto& [name,value]:entries)context.set(Value(style),Key(std::u16string(name)),value);
        context.set(Value(commonStyles),Key(std::u16string(selector)),style);
    };
    rules(u"mjx-container[overflow=\"scroll\"][display]",{{u"overflow",u"auto clip"},{u"min-width",u"initial !important"}});
    rules(u"mjx-container[overflow=\"truncate\"][display]",{{u"overflow",u"hidden clip"},{u"min-width",u"initial !important"}});
    rules(u"mjx-container[display]",{{u"display",u"block"},{u"text-align",u"center"},{u"justify-content",u"center"},{u"margin",u".7em 0"},{u"padding",u".3em 2px"}});
    rules(u"mjx-container[display][width=\"full\"]",{{u"display",u"flex"}});
    rules(u"mjx-container[justify=\"left\"]",{{u"text-align",u"left"},{u"justify-content",u"left"}});
    rules(u"mjx-container[justify=\"right\"]",{{u"text-align",u"right"},{u"justify-content",u"right"}});
    context.set(Value(actualClass),Key(u"commonStyles"),Value(commonStyles));
    // MathJax ts/output/common.ts: addExtension(font, prefix = ''). The
    // delegation is deliberately live: a replaced font or method is the
    // receiver used by this invocation, including when the method is borrowed.
    const auto prototype=std::get<ObjectRef>(context.get(
        Value(actualClass),Key(u"prototype")).data);
    const auto html=context.function(u"",
        [](Context& cx,Value receiver,std::span<const Value> args)->Value{
            const Value adaptor=cx.get(receiver,Key(u"adaptor"));
            const Value node=cx.get(adaptor,Key(u"node"));
            const auto* function=std::get_if<ObjectRef>(&node.data);
            if(!function||cx.kind(*function)!=ObjectKind::function)
                throw Exception("TypeError",u"this.adaptor.node is not a function");
            const Value type=args.empty()?Value(Undefined{}):args[0];
            const Value definition=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
                ?Value(cx.object()):args[1];
            const Value children=args.size()<3||std::holds_alternative<Undefined>(args[2].data)
                ?Value(cx.array()):args[2];
            const Value nameSpace=args.size()<4?Value(Undefined{}):args[3];
            const std::array<Value,4> forwarded{type,definition,children,nameSpace};
            return cx.call(*function,adaptor,forwarded);
        });
    context.define(html,Key(u"length"),Descriptor::data(Value(4),false,false,true));
    context.define(prototype,Key(u"html"),Descriptor::data(Value(html),true,true,true));
    const auto addExtension=context.function(u"",
        [](Context& cx,Value receiver,std::span<const Value> arguments)->Value{
            const auto data=arguments.empty()?Value(Undefined{}):arguments[0];
            const auto prefix=arguments.size()<2||
                std::holds_alternative<Undefined>(arguments[1].data)
                ?Value(u""):arguments[1];
            const auto font=cx.get(receiver,Key(u"font"));
            const auto method=cx.get(font,Key(u"addExtension"));
            const std::array<Value,2> forwarded{data,prefix};
            const auto* function=std::get_if<ObjectRef>(&method.data);
            if(!function||cx.kind(*function)!=ObjectKind::function)
                throw Exception("TypeError",u"this.font.addExtension is not a function");
            return cx.call(*function,font,forwarded);
        });
    // The pinned compiled Source assigns an anonymous two-argument function
    // to the prototype and applies its default prefix inside that function.
    context.define(addExtension,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    context.define(prototype,Key(u"addExtension"),
        Descriptor::data(Value(addExtension),true,true,true));
    const std::weak_ptr<void> runtimeLifetime=runtime_->lifetimeToken();
    const auto getBBox=context.function(u"",
        [runtime=runtime_,runtimeLifetime](Context& cx,Value receiver,
                           std::span<const Value> args)->Value{
            const auto lifetime=runtimeLifetime.lock();
            if(!lifetime)
                throw Exception("Unimplemented",u"CommonOutputJax source methods have been released");
            auto& output=runtime->factories.resolveJax(receiver);
            if(args.empty()||!std::holds_alternative<ObjectRef>(args[0].data))
                throw Exception("TypeError",u"MathItem argument is not a MathItem object");
            const auto itemIdentity=std::get<ObjectRef>(args[0].data);
            const auto* itemPayload=std::any_cast<std::shared_ptr<NativeObject>>(
                &cx.payload(itemIdentity));
            auto itemOwner=itemPayload?std::dynamic_pointer_cast<core::MathItem>(*itemPayload):nullptr;
            const auto* itemSource=itemOwner?
                dynamic_cast<const core::MathItemSourceIdentity*>(itemOwner.get()):nullptr;
            if(!itemOwner||!itemSource||&itemSource->sourceContext()!=&cx||
               itemSource->sourceIdentity()!=itemIdentity)
                throw Exception("TypeError",u"MathItem argument has no canonical native owner");

            if(args.size()<2||!std::holds_alternative<ObjectRef>(args[1].data))
                throw Exception("TypeError",u"MathDocument argument is not a MathDocument object");
            const auto documentIdentity=std::get<ObjectRef>(args[1].data);
            const auto* documentPayload=std::any_cast<std::shared_ptr<NativeObject>>(
                &cx.payload(documentIdentity));
            auto documentOwner=documentPayload?
                std::dynamic_pointer_cast<NativeObject>(*documentPayload):nullptr;
            auto* documentService=documentOwner?
                dynamic_cast<core::MathDocumentService*>(documentOwner.get()):nullptr;
            if(!documentService||&documentService->sourceContext()!=&cx||
               documentService->sourceIdentity()!=documentIdentity)
                throw Exception("TypeError",u"MathDocument argument has no canonical native owner");
            auto bbox=output.getBBox(*itemOwner,documentService);
            if(!bbox)throw Exception("TypeError",u"CommonOutputJax.getBBox returned no BBox owner");
            return runtime->bboxes.bind(bbox);
        });
    context.define(getBBox,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    context.define(prototype,Key(u"getBBox"),
        Descriptor::data(Value(getBBox),true,true,true));
    return actualClass;
}
}
