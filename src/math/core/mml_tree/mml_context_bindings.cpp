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
    nodeModule_->initializeTreeClasses(*treeNodes_,lease_,[weak](ObjectRef receiver,Key field,ObjectRef object,
        tree::TreeOwnedObjectHandoff& handoff){
        if(const auto self=weak.lock())self->observeTreeOwnedObject(receiver,std::move(field),object,handoff);
    });
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
        [weakOwner]{
            const auto self=weakOwner.lock();
            if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->selectedSourceBackend();
        },
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
        },
        [weakOwner](ObjectRef receiver,SourceReceiverProvenance provenance,bool hasNoProduct){
            const auto self=weakOwner.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            return self->beginReceiverInitialization(receiver,provenance,hasNoProduct);
        },
        [weakOwner](std::uint64_t id){if(const auto self=weakOwner.lock())self->endReceiverInitialization(id);});
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
    if(sourceBackend_){sourceBackend_->clearIdentityBinder(this);sourceBackend_->clearProductPropertiesBirthObserver(this);sourceBackend_->clearDefaultAttributesBirthObserver(this);sourceBackend_->clearSourceClassInfoResolver(this);}
    if(treeNodes_)nodeModule_->detachContextBindings(*treeNodes_);
    nodeMethods_.reset();factoryFields_.reset();factoryMethods_.reset();nodeFields_.reset();
    sourceAllocator_.reset();sourceBackend_.reset();treeNodes_.reset();
    factoryBaseModule_.reset();initialized_=false;
}
MmlContextBindings::~MmlContextBindings(){
    lease_->store(false,std::memory_order_release);
    for(auto& weak:boundFactories_)if(auto factory=weak.lock()){
        factory->clearIdentityBinder(this);
        factory->clearProductPropertiesBirthObserver(this);
        factory->clearDefaultAttributesBirthObserver(this);
        if(factoryFields_)factoryFields_->detach(factory);
    }
    if(sourceBackend_){sourceBackend_->clearIdentityBinder(this);sourceBackend_->clearProductPropertiesBirthObserver(this);sourceBackend_->clearDefaultAttributesBirthObserver(this);sourceBackend_->clearSourceClassInfoResolver(this);}
    if(treeNodes_)nodeModule_->detachContextBindings(*treeNodes_);
    arrays_.clear();factories_.clear();nodes_.clear();
}
MmlContextBindings::SourceAllocationDomain::SourceAllocationDomain(
        std::weak_ptr<MmlContextBindings> owner,std::shared_ptr<MmlFactory> factory,
        ObjectRef factoryIdentity)
    :owner_(std::move(owner)),factory_(std::move(factory)),factoryIdentity_(factoryIdentity){}
MmlFactory& MmlContextBindings::SourceAllocationDomain::factory()const{
    if(!factory_||retired_)throw Exception("TypeError",u"MML Source allocation domain has been released");
    return *factory_;
}
MmlContextBindings::ActiveSourceAllocation::ActiveSourceAllocation(
        std::weak_ptr<MmlContextBindings> owner,std::uint64_t activationId)noexcept
    :owner_(std::move(owner)),activationId_(activationId){}
MmlContextBindings::ActiveSourceAllocation::ActiveSourceAllocation(
        ActiveSourceAllocation&& other)noexcept
    :owner_(std::move(other.owner_)),activationId_(std::exchange(other.activationId_,0)){}
MmlContextBindings::ActiveSourceAllocation::~ActiveSourceAllocation(){
    if(!activationId_)return;
    if(auto owner=owner_.lock())owner->deactivateSourceAllocation(activationId_);
}
std::shared_ptr<MmlFactory> MmlContextBindings::selectedSourceBackend()const{
    return activeSourceBackends_.empty()?sourceBackend_:activeSourceBackends_.back().second;
}
void MmlContextBindings::deactivateSourceAllocation(std::uint64_t id)noexcept{
    const auto frame=std::find_if(activeSourceBackends_.begin(),activeSourceBackends_.end(),
        [id](const auto& entry){return entry.first==id;});
    if(frame!=activeSourceBackends_.end())activeSourceBackends_.erase(frame);
}
MmlContextBindings::SourceAllocationDomain MmlContextBindings::makePrivateSourceAllocationDomain(){
    if(!initialized_)throw Exception("TypeError",u"MML Context bindings are not initialized");
    auto factory=std::make_shared<MmlFactory>();
    return SourceAllocationDomain{weak_from_this(),std::move(factory),{}};
}
void MmlContextBindings::bindSourceAllocationDomain(SourceAllocationDomain& domain){
    if(domain.owner_.lock().get()!=this||!domain.factory_||domain.retired_)
        throw Exception("TypeError",u"MML Source allocation domain belongs to another Context");
    if(domain.factoryIdentity_.context){
        const auto current=factories_.find(domain.factory_.get());
        if(!current||*current!=domain.factoryIdentity_)
            throw Exception("TypeError",u"Private MML Source factory identity no longer matches its Context owner");
    }
    try{
        const auto identity=bindFactory(domain.factory_);
        if(domain.factoryIdentity_.context&&identity!=domain.factoryIdentity_)
            throw Exception("TypeError",u"Private MML Source factory identity changed during binding retry");
        domain.factoryIdentity_=identity;
    }
    catch(...){
        // bindFactory is completion-aware. Preserve any identity it installed
        // so a later call can finish binding before the domain is activated,
        // or failure cleanup can retire this same exact domain.
        if(const auto current=factories_.find(domain.factory_.get()))
            domain.factoryIdentity_=*current;
        throw;
    }
}
void MmlContextBindings::retireSourceAllocationDomain(SourceAllocationDomain& domain){
    const auto owner=domain.owner_.lock();
    if(owner.get()!=this||!domain.factory_)
        throw Exception("TypeError",u"MML Source allocation domain belongs to another Context");
    if(domain.retired_)return;
    if(std::any_of(activeSourceBackends_.begin(),activeSourceBackends_.end(),
        [&](const auto& entry){return entry.second.get()==domain.factory_.get();}))
        throw Exception("TypeError",u"Cannot retire an active MML Source allocation domain");
    if(domain.factoryIdentity_.context){
        const auto current=factories_.find(domain.factory_.get());
        if(!current||*current!=domain.factoryIdentity_||!factories_.retire(domain.factory_.get()))
            throw Exception("TypeError",u"Private MML Source factory identity no longer matches its Context owner");
        domain.factoryIdentity_={};
    }
    // Factory.node and its per-creator functions are separate Context records
    // whose callbacks retain this factory. Retire them individually so a
    // failure leaves the domain handle available for an idempotent retry.
    if(factoryFields_)factoryFields_->retireFactoryOwner(domain.factory_);
    domain.factory_->clearIdentityBinder(this);
    domain.factory_->clearProductPropertiesBirthObserver(this);
    domain.factory_->clearDefaultAttributesBirthObserver(this);
    if(factoryFields_)factoryFields_->detach(domain.factory_);
    std::erase_if(boundFactories_,[&](const auto& weak){
        const auto factory=weak.lock();
        return !factory||factory==domain.factory_;
    });
    domain.retired_=true;
}
MmlContextBindings::ActiveSourceAllocation MmlContextBindings::activateSourceAllocation(
        SourceAllocationDomain& domain){
    if(domain.owner_.lock().get()!=this||!domain.factory_||domain.retired_||!domain.factoryIdentity_.context)
        throw Exception("TypeError",u"MML Source allocation domain belongs to another Context");
    if(nextSourceActivationId_==0)
        throw Exception("RangeError",u"MML Source allocation activation space is exhausted");
    const auto id=nextSourceActivationId_;
    activeSourceBackends_.emplace_back(id,domain.factory_);
    ++nextSourceActivationId_;
    return ActiveSourceAllocation{weak_from_this(),id};
}
Context& MmlContextBindings::context()const{return context_;}
std::array<std::size_t,3> MmlContextBindings::nativeOwnerCountsForDiagnostics()const{
    const auto projections=nodeModule_->properties().projectionCountsForDiagnostics();
    return {nodeModule_->attributes().canonicalHandleCountForDiagnostics(),
            projections[0],projections[1]};
}
RequestIdentityCapture MmlContextBindings::beginPrivateRendererProjectionCapture(
        std::shared_ptr<MmlFactory> nativeProductFactory){
    if(privateRendererProjectionCapture_)
        throw Exception("TypeError",u"A previous private MML projection capture is still pending retirement");
    if(!nativeProductFactory)
        throw Exception("TypeError",u"Private MML projection capture requires its actual request product factory");
    const auto factoryIdentity=factories_.find(nativeProductFactory.get());
    if(!factoryIdentity)
        throw Exception("TypeError",u"Private MML projection capture factory is not the canonical Context owner");
    const auto canonicalFactory=resolveFactory(*factoryIdentity);
    if(canonicalFactory.get()!=nativeProductFactory.get()||
       canonicalFactory.owner_before(nativeProductFactory)||nativeProductFactory.owner_before(canonicalFactory))
        throw Exception("TypeError",u"Private MML projection capture factory is not the canonical Context owner");
    const std::weak_ptr<MmlContextBindings> weak=weak_from_this();
    nativeProductFactory->setDefaultAttributesBirthObserver(this,
        [weak](const Property& storage,MmlFactory& factory){
            const auto self=weak.lock();
            if(!self)throw Exception("TypeError",u"MML projection capture owner has expired");
            self->observeNativeDefaultAttributes(storage,factory);
        });
    RequestIdentityCapture token;
    try{token=nodeModule_->attributes().beginRequestCapture();}
    catch(...){nativeProductFactory->clearDefaultAttributesBirthObserver(this);throw;}
    // The single inline optional is non-allocating for an empty capture. Once
    // Attributes returns a token, cleanup state is therefore already retained.
    try{
        privateRendererProjectionCapture_.emplace(
            PrivateRendererProjectionCapture{token,std::move(nativeProductFactory),{},{},{},{},false});
    }catch(...){
        nodeModule_->attributes().retireCapturedRequestBindings(token);
        // The local canonical owner still exists because emplace did not
        // complete; remove only our hook and preserve any foreign hook.
        canonicalFactory->clearDefaultAttributesBirthObserver(this);
        throw;
    }
    activePrivateRendererProjectionCapture_=token;
    return token;
}
void MmlContextBindings::retirePrivateRendererProjectionCapture(RequestIdentityCapture token){
    const auto suppliedOwner=token.owner.lock();
    const auto storedOwner=privateRendererProjectionCapture_
        ?privateRendererProjectionCapture_->token.owner.lock():std::shared_ptr<const void>{};
    if(token.id==0||!suppliedOwner||!storedOwner||
       suppliedOwner.owner_before(storedOwner)||storedOwner.owner_before(suppliedOwner)||
       privateRendererProjectionCapture_->token.id!=token.id)
        throw Exception("TypeError",u"Private MML projection capture token is not active in this registry");
    if(activePrivateRendererProjectionCapture_&&activePrivateRendererProjectionCapture_->id==token.id)
        activePrivateRendererProjectionCapture_.reset();
    auto& capture=*privateRendererProjectionCapture_;
    capture.nativeProductFactory->clearDefaultAttributesBirthObserver(this);
    if(!capture.attributesRetired){
        nodeModule_->attributes().retireCapturedRequestBindings(token);
        capture.attributesRetired=true;
    }
    if(capture.pendingNativeProperties){
        nodeModule_->properties().retireNativeProjectionForStorage(
            capture.pendingNativeProperties->storage);
        capture.pendingNativeProperties.reset();
    }
    auto& nativeProperties=capture.nativeProperties;
    while(!nativeProperties.empty()){
        nodeModule_->properties().retireNativeProjectionForStorage(nativeProperties.back().storage);
        nativeProperties.pop_back();
    }
    if(capture.pendingBirth){
        const auto& pending=*capture.pendingBirth;
        nodeModule_->properties().retireCapturedSourceObject(pending.object,pending.storage);
        capture.pendingBirth.reset();
    }
    auto& objects=capture.sourceObjects;
    while(!objects.empty()){
        const auto& pending=objects.back();
        nodeModule_->properties().retireCapturedSourceObject(pending.object,pending.storage);
        objects.pop_back();
    }
    privateRendererProjectionCapture_.reset();
}
std::uint64_t MmlContextBindings::beginReceiverInitialization(
        ObjectRef receiver,SourceReceiverProvenance provenance,bool hasNoProduct){
    if(provenance!=SourceReceiverProvenance::constructed||!hasNoProduct||
       !activePrivateRendererProjectionCapture_||activeSourceBackends_.empty())return 0;
    if(!privateRendererProjectionCapture_||
       privateRendererProjectionCapture_->token.id!=activePrivateRendererProjectionCapture_->id)
        throw Exception("TypeError",u"Private MML projection capture record is missing");
    const auto factory=activeSourceBackends_.back().second;
    if(!factory||factory==sourceBackend_)return 0;
    if(nextReceiverScopeId_==0)throw Exception("RangeError",u"Private MML receiver scope space is exhausted");
    const auto id=nextReceiverScopeId_++;
    receiverInitializationScopes_.push_back(ReceiverInitializationScope{
        id,receiver,factory,privateRendererProjectionCapture_->token.id});
    return id;
}
void MmlContextBindings::endReceiverInitialization(std::uint64_t id)noexcept{
    if(!id)return;
    const auto found=std::find_if(receiverInitializationScopes_.rbegin(),receiverInitializationScopes_.rend(),
        [&](const auto& scope){return scope.id==id;});
    if(found!=receiverInitializationScopes_.rend())
        receiverInitializationScopes_.erase(std::next(found).base());
}
void MmlContextBindings::observeTreeOwnedObject(ObjectRef receiver,Key field,ObjectRef object,
        tree::TreeOwnedObjectHandoff& handoff){
    if(receiverInitializationScopes_.empty()||field!=Key(u"properties"))return;
    const auto& scope=receiverInitializationScopes_.back();
    if(scope.receiver!=receiver)return;
    if(!activePrivateRendererProjectionCapture_||activePrivateRendererProjectionCapture_->id!=scope.captureId||
       activeSourceBackends_.empty()||activeSourceBackends_.back().second!=scope.expectedOwner)
        throw Exception("TypeError",u"Private MML receiver initialization changed its capture or actual factory owner");
    if(!privateRendererProjectionCapture_||
       privateRendererProjectionCapture_->token.id!=scope.captureId)
        throw Exception("TypeError",u"Source receiver refers to an inactive MML projection capture");
    // A custom accessor may retain this fresh value outside the request.
    // Inspect raw descriptors only; never call a getter/setter to decide ownership.
    auto prototype=context_.prototype(receiver);
    while(prototype){
        if(const auto descriptor=context_.descriptor(Value(*prototype),field);descriptor&&descriptor->accessor)return;
        prototype=context_.prototype(*prototype);
    }
    if(const auto descriptor=context_.descriptor(Value(receiver),field);descriptor&&descriptor->accessor)return;
    auto& capture=*privateRendererProjectionCapture_;
    for(const auto& current:capture.sourceObjects)
        if(current.receiver==receiver&&current.object==object)return;
    if(capture.pendingBirth)
        throw Exception("TypeError",u"Another Source object birth is already pending in this non-reentrant capture");
    // Install an inline exact token before conversion or vector growth can fail.
    // The generic Tree layer will leave the Context record to this token after
    // claim(), including when `from()` fails before it can return storage.
    capture.pendingBirth.emplace(CapturedSourceObject{receiver,object,{},scope.expectedOwner});
    handoff.claim();
    auto& properties=nodeModule_->properties();
    const auto source=properties.from(Value(object));
    const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&source.value);
    if(!storage||!*storage)
        throw Exception("TypeError",u"Fresh Source properties object did not produce a retained source storage");
    capture.pendingBirth->storage=*storage;
    if(!(*storage)->binding||(*storage)->binding->sourceObjectIdentity()!=std::optional<ObjectRef>{object})
        throw Exception("TypeError",u"Fresh Source properties storage lost its exact Context object identity");
    // Copies are non-throwing; vector allocation is the only failing step, and
    // on failure the inline pending token still owns the exact object/storage.
    capture.sourceObjects.push_back(*capture.pendingBirth);
    capture.pendingBirth.reset();
}
void MmlContextBindings::observeNativeProductProperties(
        MmlNode& node,const Property& value,MmlFactory& factory){
    if(!activePrivateRendererProjectionCapture_)return;
    if(!privateRendererProjectionCapture_||
       privateRendererProjectionCapture_->token.id!=activePrivateRendererProjectionCapture_->id)
        throw Exception("TypeError",u"Private MML native product refers to a missing projection capture");
    auto& capture=*privateRendererProjectionCapture_;
    if(capture.nativeProductFactory.get()!=&factory)return;
    if(capture.pendingNativeProperties)
        throw Exception("TypeError",u"Another native MML properties birth is pending in this non-reentrant capture");
    const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
    if(!storage||!*storage||(*storage)->binding)
        throw Exception("TypeError",u"Native MML properties birth must be an unbound fresh PropertyStorage");
    const auto identity=nodes_.find(&node);
    if(!identity)
        throw Exception("TypeError",u"Native MML properties birth is not owned by the request product factory");
    const auto actualOwner=resolveProductOwner(*identity,node);
    if(actualOwner.get()!=capture.nativeProductFactory.get()||
       actualOwner.owner_before(capture.nativeProductFactory)||
       capture.nativeProductFactory.owner_before(actualOwner))
        throw Exception("TypeError",u"Native MML properties birth is not owned by the request product factory");
    // A Source accessor may retain the object passed by the native field
    // writer. Do not retire storage exposed through an accessor-backed field.
    auto prototype=context_.prototype(*identity);
    while(prototype){
        if(const auto descriptor=context_.descriptor(Value(*prototype),Key(u"properties"));
           descriptor&&descriptor->accessor)return;
        prototype=context_.prototype(*prototype);
    }
    // Keep the exact storage before any vector growth. If projection or later
    // node initialization throws, request teardown still has the storage token.
    capture.pendingNativeProperties.emplace(
        PrivateRendererProjectionCapture::NativePropertiesBirth{*storage});
    capture.nativeProperties.push_back(*capture.pendingNativeProperties);
    capture.pendingNativeProperties.reset();
}
void MmlContextBindings::observeNativeDefaultAttributes(const Property& value,MmlFactory& factory){
    if(!activePrivateRendererProjectionCapture_)return;
    if(!privateRendererProjectionCapture_||
       privateRendererProjectionCapture_->token.id!=activePrivateRendererProjectionCapture_->id)
        throw Exception("TypeError",u"Private MML default attributes refer to a missing projection capture");
    auto& capture=*privateRendererProjectionCapture_;
    if(capture.nativeProductFactory.get()!=&factory)return;
    const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
    if(!storage||!*storage||(*storage)->binding)
        throw Exception("TypeError",u"MML default attributes birth must be a fresh native PropertyStorage");
    if(capture.pendingNativeProperties)
        throw Exception("TypeError",u"Another native MML property birth is pending in this non-reentrant capture");
    capture.pendingNativeProperties.emplace(
        PrivateRendererProjectionCapture::NativePropertiesBirth{*storage});
    capture.nativeProperties.push_back(*capture.pendingNativeProperties);
    capture.pendingNativeProperties.reset();
}
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
    if(privateRendererProjectionCapture_){
        for(const auto& source:privateRendererProjectionCapture_->sourceObjects)
            if(source.receiver==identity&&source.expectedOwner!=owner)
                throw Exception("TypeError",u"MML Source properties birth belongs to another actual product factory");
    }
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
            if(!target){
                // Keep the current canonical-record-first order: node
                // field binders rely on the actual class prototype.
                (void)self->nodes_.bind(&node,object,std::static_pointer_cast<void>(owner));
                self->factoryFields_->bindNodePrototype(object,node);
            }
            (void)self->bindNode(object,node,owner);
        });
        factory->setProductPropertiesBirthObserver(this,[weak](MmlNode& node,const Property& storage,MmlFactory& owner){
            const auto self=weak.lock();if(!self)throw Exception("TypeError",u"MML Context binding owner has expired");
            self->observeNativeProductProperties(node,storage,owner);
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
