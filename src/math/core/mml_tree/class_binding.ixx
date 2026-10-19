// Root-owned native class binding for the source constructor object.
export module folia.math.core.mml_tree.class_binding;
export import folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node_initialization;
import std;
export namespace folia::math::mml {
template<class T>tree::FactoryClassRef<MmlNode> classBinding(
    std::u16string sourceConstructorName={},std::u16string sourceKind={},
    std::u16string sourcePrototypeParent={},
    std::uint8_t sourceConstructorArity=0,
    std::optional<double> sourceArity=std::nullopt,
    std::optional<bool> sourceIsInferred=std::nullopt,
    bool sourceOwnDefaults=std::is_base_of_v<AbstractMmlNode,T>,
    bool sourceOwnAccents=false){
    auto result=std::make_shared<tree::FactoryClass<MmlNode>>();
    result->nativeType=typeid(T);
    auto info=std::make_shared<MmlClassInfo>();
    info->sourceConstructorName=std::move(sourceConstructorName);
    info->sourceKind=std::move(sourceKind);
    info->sourcePrototypeParent=std::move(sourcePrototypeParent);
    info->sourceConstructorArity=sourceConstructorArity;
    info->sourceArity=sourceArity;
    info->sourceIsInferred=sourceIsInferred;
    info->sourceOwnDefaults=sourceOwnDefaults;
    info->sourceOwnAccents=sourceOwnAccents;
    if constexpr(std::is_base_of_v<AbstractMmlNode,T>)info->defaultsBinding=[]()->std::optional<PropertyList>{return T::defaults();};
    if constexpr(requires{T::accents();})info->accentsBinding=[] {return T::accents();};
    if constexpr(requires{T::template staticBinding<T>();})info->staticServices=T::template staticBinding<T>();
    result->staticData=info;
    const std::weak_ptr<tree::FactoryClass<MmlNode>> capturedClass(result);
    result->construct=[info,capturedClass](Factory& factory,tree::FactoryArgs args){
        const auto actualClass=capturedClass.lock();
        if(!actualClass)throw Error("TypeError",u"MML constructor class is no longer live");
        const auto* mmlFactory=dynamic_cast<MmlFactory*>(&factory);
        const bool deferred=mmlFactory&&mmlFactory->hasAdoptIdentityTarget();
        auto owned=std::make_unique<T>(factory);auto* node=owned.get();
        node->bindNativeConstructorClass(actualClass);
        if constexpr(std::is_base_of_v<AbstractMmlNode,T>)
            node->bindConstructorClass(mmlFactory?mmlFactory->sourceClassInfo(actualClass):info);
        factory.adopt(std::move(owned));
        if(deferred)return node;
        if constexpr(std::is_base_of_v<AbstractMmlNode,T>){
            node->initializeNode({});
            node->initializeMml(args,{},&factory);
            node->initializeClassFields();
            if constexpr(requires(T& value){value.initializeSourceFields();})node->initializeSourceFields();
        }else{
            node->initializeNode(args);
            if constexpr(requires(T& value){value.initializeSourceFields();})node->initializeSourceFields();
        }
        node->completeSourceInitialization();
        return node;
    };
    result->initialize=[info,capturedClass=std::weak_ptr<tree::FactoryClass<MmlNode>>(result)](
            tree::FactoryServices<MmlNode>& factory,MmlNode& base,tree::FactoryArgs args,
            tree::FactorySetChildren<MmlNode> setChildren){
        auto* node=dynamic_cast<T*>(&base);
        if(!node)throw Error("TypeError",u"MML constructor receiver has an incompatible native class");
        if constexpr(std::is_base_of_v<AbstractMmlNode,T>){
            const auto current=capturedClass.lock();
            const auto* mmlFactory=dynamic_cast<MmlFactory*>(&factory);
            node->bindConstructorClass(current&&mmlFactory
                ?mmlFactory->sourceClassInfo(current):info);
            node->initializeMml(args,std::move(setChildren),&factory);
            if(current&&node->nativeConstructorClass()==current)node->initializeClassFields();
            if constexpr(requires(T& value){value.initializeSourceFields();})node->initializeSourceFields();
        }else{
            node->initializeNode(args);
            if constexpr(requires(T& value){value.initializeSourceFields();})node->initializeSourceFields();
        }
        node->completeSourceInitialization();
    };
    // Context constructors have already executed AbstractNode's Source base
    // constructor on the exact receiver. After the descriptor allocator binds
    // its one real product to that receiver, initialize through the same live
    // operation view used by native initializeMml; only class fields follow.
    result->initializeSource=[info,capturedClass=std::weak_ptr<tree::FactoryClass<MmlNode>>(result)](
            tree::FactoryServices<MmlNode>& allocator,MmlNode& base,
            options::Value rawFactory,options::Value rawAttributes,options::Value rawChildren){
        auto* node=dynamic_cast<T*>(&base);
        if(!node)throw Error("TypeError",u"MML constructor receiver has an incompatible native class");
        if constexpr(std::is_base_of_v<AbstractMmlNode,T>){
            const auto& operation=node->sourceOperationView();
            if(!operation)throw Error("TypeError",u"MML constructor receiver has no live Source operation view");
            const auto current=capturedClass.lock();
            // Constructor metadata belongs to the allocation service. The
            // receiver's Source factory field can be an accessor or foreign
            // value, and is read only by initializeMmlNode in Source order.
            auto* mmlFactory=dynamic_cast<MmlFactory*>(&allocator);
            node->bindConstructorClass(current&&mmlFactory
                ?mmlFactory->sourceClassInfo(current):info);
            auto view=operation();
            initializeMmlNode(view,std::move(rawFactory),std::move(rawAttributes),std::move(rawChildren));
            if(current&&node->nativeConstructorClass()==current)node->initializeClassFields();
            if constexpr(requires(T& value){value.initializeSourceFields();})node->initializeSourceFields();
        }else{
            // Empty Source nodes (notably TextNode) have no MML initializer;
            // their class-declared Source fields still run after the base.
            if constexpr(requires(T& value){value.initializeSourceFields();})node->initializeSourceFields();
        }
        node->completeSourceInitialization();
    };
    result->isInstance=[](const MmlNode* node){return dynamic_cast<const T*>(node)!=nullptr;};
    return result;
}
}
