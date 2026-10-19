module folia.math.core.mml_tree.mml_context_bindings;
import folia.math.core.mml_tree.mml_factory_context_values;
import std;

namespace folia::math::mml {
using namespace options;

namespace {
class MmlChildrenContextView final:public NativeIndexedView {
    NativeIdentityBindings<MmlNode*>& nodes_;
    Children children_;
    std::weak_ptr<std::atomic_bool> lease_;
    bool lengthWritable_=true;
public:
    MmlChildrenContextView(NativeIdentityBindings<MmlNode*>& nodes,Children children,
                           std::weak_ptr<std::atomic_bool> lease)
        :nodes_(nodes),children_(std::move(children)),lease_(std::move(lease)){
        if(!children_)throw Exception("TypeError",u"MML child array is null");
    }
    void active()const{
        const auto live=lease_.lock();
        if(!live||!live->load(std::memory_order_acquire))
            throw Exception("TypeError",u"MML Context child-array view has expired");
    }
    std::uint32_t length()const override{
        active();
        if(children_->size()>std::numeric_limits<std::uint32_t>::max())
            throw Exception("RangeError",u"MML child array length exceeds the Context array limit");
        return static_cast<std::uint32_t>(children_->size());
    }
    Descriptor lengthDescriptor()const override{
        active();
        return Descriptor::data(Value(static_cast<double>(length())),lengthWritable_,false,false);
    }
    std::optional<Descriptor> indexDescriptor(std::uint32_t index)const override{
        active();
        if(index>=length()||!children_->hasOwn(index))return {};
        const auto child=children_->get(index);
        if(child.kind()==tree::ChildKind::Undefined)return Descriptor::data(Value(Undefined{}));
        if(child.kind()==tree::ChildKind::Null)return Descriptor::data(Value(Null{}));
        const auto identity=nodes_.find(child.nodeOrNull());
        if(!identity)throw Exception("TypeError",u"MML child has no canonical Context identity");
        return Descriptor::data(Value(*identity));
    }
    std::vector<std::uint32_t> ownIndexKeys()const override{
        active();
        std::vector<std::uint32_t> result;result.reserve(children_->size());
        for(std::uint32_t index=0;index<length();++index)if(children_->hasOwn(index))result.push_back(index);
        return result;
    }
    void defineIndex(std::uint32_t index,Descriptor descriptor)override{
        active();
        if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
            throw Exception("Unimplemented",u"MML child arrays do not store per-index descriptor flags");
        if(std::holds_alternative<Undefined>(descriptor.value.data)){
            children_->set(index,tree::ChildValue<MmlNode>(Missing{}));return;
        }
        if(std::holds_alternative<Null>(descriptor.value.data)){
            children_->set(index,tree::ChildValue<MmlNode>(PropertyNull{}));return;
        }
        const auto* object=std::get_if<ObjectRef>(&descriptor.value.data);
        if(!object)throw Exception("TypeError",u"MML child array values must be canonical MML nodes, null or undefined");
        children_->set(index,tree::ChildValue<MmlNode>(nodes_.resolve(*object)));
    }
    void defineLength(Descriptor descriptor)override{
        active();
        if(descriptor.accessor||descriptor.enumerable||descriptor.configurable)
            throw Exception("TypeError",u"Invalid array length descriptor");
        if(!lengthWritable_&&descriptor.writable)throw Exception("TypeError",u"Cannot redefine non-writable property");
        const auto* number=std::get_if<double>(&descriptor.value.data);
        if(!number||!std::isfinite(*number)||*number<0||*number>4294967295.0||std::trunc(*number)!=*number)
            throw Exception("RangeError",u"Invalid array length");
        children_->resize(static_cast<std::size_t>(*number));lengthWritable_=descriptor.writable;
    }
    void deleteIndex(std::uint32_t index)override{active();children_->erase(index);}
};
}

MmlContextBindings::MmlContextBindings(Context& context,
        std::shared_ptr<MmlNodeContextModule> nodeModule,
        std::shared_ptr<MapContextBindings> maps)
    :context_(context),nodeModule_(std::move(nodeModule)),maps_(std::move(maps)),
     lease_(std::make_shared<std::atomic_bool>(true)),nodes_(context),
     factories_(context),arrays_(context){
    if(!nodeModule_||!maps_||
       &nodeModule_->context()!=&context_||&maps_->context()!=&context_)
        throw Exception("TypeError",u"MML Context bindings require one Context module and Map owner");
}
std::shared_ptr<MmlContextBindings> MmlContextBindings::create(Context& context,
        util::RuntimeModules& runtimeModules,
        std::shared_ptr<MmlNodeContextModule> nodeModule,
        std::shared_ptr<MapContextBindings> maps){
    auto result=std::shared_ptr<MmlContextBindings>(new MmlContextBindings(
        context,std::move(nodeModule),std::move(maps)));
    try{result->initialize(runtimeModules);}
    catch(...){result->rollbackInitialization();throw;}
    return result;
}
void MmlContextBindings::initialize(util::RuntimeModules& runtimeModules){
    if(initialized_)throw Exception("TypeError",u"MML Context bindings initialized twice");
    const auto weak=weak_from_this();
    treeNodes_=std::make_unique<tree::TreeNodeContextBindings<MmlNode>>(
        context_,nodes_,nodeModule_->properties(),lease_);
    nodeModule_->initializeTreeClasses(*treeNodes_,lease_);
    // Resolve this separately cached module only after Context attachment so
    // a failed cache lookup exercises the same rollback path as later setup.
    factoryBaseModule_=runtimeModules.getOrCreate<tree::FactoryContextBaseModule>(
        u"core/Tree/Factory",[&runtimeModules]{return std::make_shared<tree::FactoryContextBaseModule>(runtimeModules);});
    sourceBackend_=std::make_shared<MmlFactory>();
    // The concrete callbacks are assembled here after shared ownership exists.
    // None capture ParserRuntime; all edges back to this owner are weak.
    const std::weak_ptr<MmlContextBindings> weakOwner=weak;
    sourceBackend_->setIdentityBinder(this,[weakOwner](MmlNode& node,std::shared_ptr<MmlFactory> owner){
        const auto self=weakOwner.lock();
        if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
        const auto target=owner->takeAdoptIdentityTarget();
        const auto identity=target?*target:self->context_.object();
        (void)self->bindNode(identity,node,owner);
    });
    sourceAllocator_=std::make_shared<MmlSourceAllocator>(context_,sourceBackend_,lease_,
        [weakOwner](ObjectRef object){
            const auto self=weakOwner.lock();
            if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->nodes_.resolve(object);
        },
        [weakOwner](ObjectRef object,MmlNode& node){
            const auto self=weakOwner.lock();
            if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->resolveProductOwner(object,node);
        },
        [weakOwner](ObjectRef object,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            (void)self->bindNode(object,node,owner);
        },
        [weakOwner](ObjectRef receiver,Value factory){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            const std::array<Value,1> arguments{std::move(factory)};
            (void)self->context_.call(self->nodeModule_->treeClasses().abstractNodeConstructor(),
                                      Value(receiver),arguments);
        });
    nodeFields_=std::make_unique<MmlNodeContextFields>(context_,nodeModule_->properties(),
        nodeModule_->attributes(),nodes_,*treeNodes_,lease_,
        [weakOwner](Children children){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->bindChildren(std::move(children));
        },
        [weakOwner](Value value)->Children{
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            if(const auto* object=std::get_if<ObjectRef>(&value.data)){
                try{return self->arrays_.resolve(*object);}catch(const NativeIdentityResolutionFailure&){}
            }
            return self->treeNodes_->arrayView(std::move(value));
        },
        [weakOwner](const std::shared_ptr<MmlFactory>& owner){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            const auto identity=self->factories_.find(owner.get());
            if(!identity)throw Exception("Unimplemented",u"MML node factory has no canonical Context identity");
            return *identity;
        },
        [weakOwner](ObjectRef object){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->resolveFactory(object);
        });
    factoryMethods_=std::make_unique<MmlFactoryContextMethods>(context_,nodeModule_->properties(),nodes_,
        factoryBaseModule_,lease_,
        [weakOwner](ObjectRef object){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->resolveFactory(object);
        },
        [weakOwner](MmlNode& node){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->findNode(node);
        });
    factoryFields_=std::make_unique<MmlFactoryContextFields>(context_,*maps_,nodeModule_->properties(),
        nodes_,arrays_,*treeNodes_,nodeModule_->treeClasses(),factories_,lease_,
        [weakOwner](ObjectRef object){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->resolveFactory(object);
        },
        [weakOwner](MmlNode& node){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->findNode(node);
        },
        [weakOwner](const std::any& argument)->Value{
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            if(const auto* value=std::any_cast<Value>(&argument))return *value;
            if(const auto* property=std::any_cast<Property>(&argument))return self->nodeModule_->properties().to(*property);
            if(const auto* record=std::any_cast<PropertyList>(&argument))return self->nodeModule_->properties().to(Property(*record));
            if(const auto* children=std::any_cast<tree::FactoryChildren<MmlNode>>(&argument))return Value(self->bindChildren(*children));
            throw Exception("Unimplemented",u"MmlFactory argument has no Context identity conversion");
        },sourceAllocator_);
    const auto prototypes=nodeModule_->initializeSourceMethods(nodes_,*treeNodes_,lease_,
        factoryFields_->abstractMmlNodePredicate(),factoryFields_->textNodePredicate());
    nodeMethods_=std::make_unique<MmlNodeContextMethods>(context_,nodes_,nodeModule_->properties(),
        nodeModule_->attributes(),*treeNodes_,nodeModule_->treeClasses(),
        [weakOwner](Children children)->Value{
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            if(!children)return Null{};return Value(self->bindChildren(std::move(children)));
        },factoryFields_->abstractMmlNodePredicate(),lease_,factoryFields_->textNodePredicate(),prototypes);
    sourceBackend_->setSourceClassInfoResolver(this,[weakOwner](const tree::FactoryClassRef<MmlNode>& cls){
        const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
        return self->factoryFields_->sourceClassInfo(cls);
    });
    // Adapter initialization follows the dependency order in package 02:
    // Context + module/Map -> canonical registries -> weak callbacks -> Source
    // prototypes and factory class module. ParserRuntime only stores this owner.
    initialized_=true;
}
void MmlContextBindings::rollbackInitialization()noexcept{
    lease_->store(false,std::memory_order_release);
    if(sourceBackend_){sourceBackend_->clearIdentityBinder(this);sourceBackend_->clearSourceClassInfoResolver(this);}
    if(treeNodes_)nodeModule_->detachContextBindings(*treeNodes_);
    nodeMethods_.reset();factoryFields_.reset();factoryMethods_.reset();nodeFields_.reset();
    sourceAllocator_.reset();sourceBackend_.reset();treeNodes_.reset();
    factoryBaseModule_.reset();initialized_=false;
}
MmlContextBindings::~MmlContextBindings(){
    lease_->store(false,std::memory_order_release);
    for(auto& weak:boundFactories_)if(auto factory=weak.lock()){
        factory->clearIdentityBinder(this);
        if(factoryFields_)factoryFields_->detach(factory);
    }
    if(sourceBackend_){sourceBackend_->clearIdentityBinder(this);sourceBackend_->clearSourceClassInfoResolver(this);}
    if(treeNodes_)nodeModule_->detachContextBindings(*treeNodes_);
    arrays_.clear();factories_.clear();nodes_.clear();
}
Context& MmlContextBindings::context()const{return context_;}
NativeIdentityBindings<MmlNode*>& MmlContextBindings::nodes(){return nodes_;}
NativeIdentityBindings<MmlFactory*>& MmlContextBindings::factories(){return factories_;}
NativeIdentityBindings<Children>& MmlContextBindings::arrays(){return arrays_;}
tree::TreeNodeContextBindings<MmlNode>& MmlContextBindings::treeNodes(){return *treeNodes_;}
MmlNodeContextFields& MmlContextBindings::nodeFields(){return *nodeFields_;}
MmlFactoryContextMethods& MmlContextBindings::factoryMethods(){return *factoryMethods_;}
MmlFactoryContextFields& MmlContextBindings::factoryFields(){return *factoryFields_;}
MmlNodeContextMethods& MmlContextBindings::nodeMethods(){return *nodeMethods_;}
MmlSourceAllocator& MmlContextBindings::sourceAllocator(){return *sourceAllocator_;}
std::shared_ptr<MmlFactory> MmlContextBindings::resolveProductOwner(ObjectRef object,MmlNode& node)const{
    if(nodes_.resolve(object)!=&node)
        throw Exception("TypeError",u"MML receiver does not resolve to its canonical native product");
    const auto* binding=std::any_cast<NativeBindingPayload<MmlNode*>>(
        &std::as_const(context_).payload(object));
    if(!binding||binding->identity!=&node||!binding->lifetime)
        throw Exception("TypeError",u"MML receiver has no retained native allocation owner");
    // bindNode stores the actual MmlFactory owner in this canonical payload.
    return std::static_pointer_cast<MmlFactory>(binding->lifetime);
}
MmlNode& MmlContextBindings::bindNode(ObjectRef identity,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
    if(!owner)throw Exception("TypeError",u"MML node requires its actual native factory owner");
    if(const auto existing=nodes_.find(&node);existing&&*existing!=identity)
        throw Exception("TypeError",u"MML node already has a different canonical Context identity");
    (void)nodes_.bind(&node,identity,std::static_pointer_cast<void>(owner));
    std::shared_ptr<NodeBindingState> progress;
    {
        auto* binding=std::any_cast<NativeBindingPayload<MmlNode*>>(&context_.payload(identity));
        if(!binding||binding->identity!=&node||!binding->lifetime)
            throw Exception("TypeError",u"MML node Context record has no canonical native binding");
        const std::shared_ptr<MmlFactory> retainedOwner(binding->lifetime,owner.get());
        if(binding->lifetime.get()!=static_cast<void*>(owner.get())||
           retainedOwner.owner_before(owner)||owner.owner_before(retainedOwner))
            throw Exception("TypeError",u"MML node binding retry changed its native owner");
        auto* state=std::any_cast<std::shared_ptr<NodeBindingState>>(&binding->metadata);
        if(!state){
            if(binding->metadata.has_value())
                throw Exception("TypeError",u"MML node Context record metadata has an incompatible owner");
            binding->metadata=std::make_shared<NodeBindingState>();
            state=std::any_cast<std::shared_ptr<NodeBindingState>>(&binding->metadata);
        }
        if(!state||!*state)throw Exception("TypeError",u"MML node Context record has invalid binding progress");
        progress=*state;
    }
    if(!progress->fields){
        nodeFields_->bind(identity,node,owner);
        auto* current=std::any_cast<NativeBindingPayload<MmlNode*>>(&context_.payload(identity));
        const auto* currentProgress=current?std::any_cast<std::shared_ptr<NodeBindingState>>(&current->metadata):nullptr;
        const auto canonical=nodes_.find(&node);
        if(!current||current->identity!=&node||!canonical||*canonical!=identity||!currentProgress||*currentProgress!=progress)
            throw Exception("TypeError",u"MML node identity changed during field binding");
        progress->fields=true;
    }
    if(!progress->methods){
        nodeMethods_->bind(identity,node,true);
        auto* current=std::any_cast<NativeBindingPayload<MmlNode*>>(&context_.payload(identity));
        const auto* currentProgress=current?std::any_cast<std::shared_ptr<NodeBindingState>>(&current->metadata):nullptr;
        const auto canonical=nodes_.find(&node);
        if(!current||current->identity!=&node||!canonical||*canonical!=identity||!currentProgress||*currentProgress!=progress)
            throw Exception("TypeError",u"MML node identity changed during method binding");
        progress->methods=true;
    }
    return node;
}
MmlNode& MmlContextBindings::resolveNode(ObjectRef object)const{
    auto& node=*nodes_.resolve(object);
    if(!nodeBindingComplete(object))throw Exception("TypeError",u"MML node Context binding is incomplete");
    return node;
}
bool MmlContextBindings::nodeBindingComplete(ObjectRef object)const{
    const auto* binding=std::any_cast<NativeBindingPayload<MmlNode*>>(&std::as_const(context_).payload(object));
    if(!binding||!binding->identity)return false;
    const auto canonical=nodes_.find(binding->identity);
    if(!canonical||*canonical!=object)return false;
    const auto* state=std::any_cast<std::shared_ptr<NodeBindingState>>(&binding->metadata);
    return state&&*state&&(*state)->fields&&(*state)->methods;
}
std::optional<ObjectRef> MmlContextBindings::findNode(MmlNode& node)const{return nodes_.find(&node);}
ObjectRef MmlContextBindings::bindChildren(Children children){
    if(!children)throw Exception("TypeError",u"MML children identity is null");
    if(const auto known=arrays_.find(children))return *known;
    auto view=std::make_shared<MmlChildrenContextView>(nodes_,children,lease_);
    return arrays_.bind(std::move(children),context_.nativeArray(std::move(view)));
}
Children MmlContextBindings::resolveChildren(ObjectRef object)const{return arrays_.resolve(object);}
ObjectRef MmlContextBindings::bindFactory(std::shared_ptr<MmlFactory> factory){
    if(!factory)throw Exception("TypeError",u"MmlFactory owner is null");
    auto canonical=factories_.find(factory.get());
    if(!canonical){
        (void)factories_.bind(factory.get(),context_.object(),std::static_pointer_cast<void>(factory));
        canonical=factories_.find(factory.get());
    }
    if(!canonical)throw Exception("TypeError",u"MmlFactory Context identity was not retained");
    const auto identity=*canonical;
    std::shared_ptr<FactoryBindingState> progress;
    {
        auto* binding=std::any_cast<NativeBindingPayload<MmlFactory*>>(&context_.payload(identity));
        if(!binding||binding->identity!=factory.get()||!binding->lifetime)
            throw Exception("TypeError",u"MmlFactory Context record has no canonical native owner");
        const std::shared_ptr<MmlFactory> retainedOwner(binding->lifetime,binding->identity);
        if(binding->lifetime.get()!=static_cast<void*>(factory.get())||
           retainedOwner.owner_before(factory)||factory.owner_before(retainedOwner))
            throw Exception("TypeError",u"MmlFactory Context identity belongs to another native owner");
        auto* progressSlot=std::any_cast<std::shared_ptr<FactoryBindingState>>(&binding->metadata);
        if(!progressSlot){
            if(binding->metadata.has_value())
                throw Exception("TypeError",u"MmlFactory Context record metadata has an incompatible owner");
            binding->metadata=std::make_shared<FactoryBindingState>();
            progressSlot=std::any_cast<std::shared_ptr<FactoryBindingState>>(&binding->metadata);
        }
        if(!progressSlot||!*progressSlot)
            throw Exception("TypeError",u"MmlFactory Context record has invalid binding progress");
        progress=*progressSlot;
    }
    // Record cleanup ownership before either adapter can install a resolver or
    // other callback into this externally owned factory and then throw.
    auto known=std::find_if(boundFactories_.begin(),boundFactories_.end(),[&](const auto& weak){return weak.lock()==factory;});
    if(known==boundFactories_.end())boundFactories_.push_back(factory);
    const auto requireCurrentProgress=[&](const char16_t* stage){
        const auto currentIdentity=factories_.find(factory.get());
        if(!currentIdentity||*currentIdentity!=identity)
            throw Exception("TypeError",u"MmlFactory identity changed during binding");
        auto* current=std::any_cast<NativeBindingPayload<MmlFactory*>>(&context_.payload(identity));
        const auto* currentProgress=current?std::any_cast<std::shared_ptr<FactoryBindingState>>(&current->metadata):nullptr;
        if(!current||current->identity!=factory.get()||!currentProgress||*currentProgress!=progress)
            throw Exception("TypeError",std::u16string(u"MmlFactory binding progress changed during ")+stage);
    };
    if(!progress->methods){
        factoryMethods_->bind(identity,factory);
        requireCurrentProgress(u"method binding");
        progress->methods=true;
    }
    if(!progress->fields){
        factoryFields_->bind(identity,factory,factoryMethods_->prototype(),nodeMethods_->nodePrototype(),
            nodeMethods_->tokenPrototype(),nodeMethods_->mmlNodePrototype());
        requireCurrentProgress(u"field binding");
        progress->fields=true;
    }
    if(!progress->binder){
        const std::weak_ptr<MmlContextBindings> weak=weak_from_this();
        factory->setIdentityBinder(this,[weak](MmlNode& node,std::shared_ptr<MmlFactory> owner){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            const auto target=owner->takeAdoptIdentityTarget();
            const auto object=target?*target:self->context_.object();
            (void)self->bindNode(object,node,owner);
            if(!target)self->factoryFields_->bindNodePrototype(object,node);
        });
        requireCurrentProgress(u"identity binder installation");
        progress->binder=true;
    }
    return identity;
}
std::shared_ptr<MmlFactory> MmlContextBindings::resolveFactory(ObjectRef object)const{
    const auto& payload=std::as_const(context_).payload(object);
    const auto* binding=std::any_cast<NativeBindingPayload<MmlFactory*>>(&payload);
    if(!binding||!binding->identity||!binding->lifetime)
        throw NativeIdentityResolutionFailure(typeid(MmlFactory*).name(),payload.has_value()?payload.type().name():"<empty>");
    if(factories_.resolve(object)!=binding->identity)
        throw Exception("TypeError",u"Context object has no canonical MmlFactory owner");
    return std::shared_ptr<MmlFactory>(binding->lifetime,binding->identity);
}
}
