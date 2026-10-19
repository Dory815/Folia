// Candidate Context own-field projection for the actual MmlFactory owner.
export module folia.math.core.mml_tree.mml_factory_context_fields;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.util.context_property_bindings;
export import folia.math.util.factory_context_field_views;
export import folia.math.util.factory_context_source_map_access;
export import folia.math.util.map_bindings;
export import folia.math.util.native_identities;
export import folia.math.core.mml_tree.mml_source_allocator;
export import folia.math.core.tree.node_context_classes;
import std;

export namespace folia::math::mml {
class MmlFactoryContextFields {
    struct State;
    std::shared_ptr<State> state_;
public:
    using ResolveFactory=std::function<std::shared_ptr<MmlFactory>(options::ObjectRef)>;
    using FindNodeIdentity=std::function<std::optional<options::ObjectRef>(MmlNode&)>;
    using ArgumentToValue=std::function<options::Value(const std::any&)>;
    MmlFactoryContextFields(options::Context&,options::MapContextBindings&,
        ContextPropertyBindings&,NativeIdentityBindings<MmlNode*>&,
        NativeIdentityBindings<Children>&,
        tree::TreeNodeContextBindings<MmlNode>&,
        tree::TreeNodeContextClasses<MmlNode>&,
        NativeIdentityBindings<MmlFactory*>&,std::weak_ptr<std::atomic_bool>,
        ResolveFactory,FindNodeIdentity,ArgumentToValue,
        std::shared_ptr<MmlSourceAllocator>);
    MmlFactoryContextFields(const MmlFactoryContextFields&)=delete;
    MmlFactoryContextFields& operator=(const MmlFactoryContextFields&)=delete;
    void bind(options::ObjectRef,const std::shared_ptr<MmlFactory>&,
              options::ObjectRef factoryPrototype,
              options::ObjectRef nodePrototype,
              options::ObjectRef tokenPrototype,
              options::ObjectRef abstractMmlNodePrototype);
    void bindNodePrototype(options::ObjectRef,MmlNode&);
    options::ObjectRef nodeObject(const std::shared_ptr<MmlFactory>&);
    std::shared_ptr<MmlClassInfo> sourceClassInfo(const tree::FactoryClassRef<MmlNode>&);
    void detach(const std::shared_ptr<MmlFactory>&);
    bool isAbstractMmlNode(options::Value);
    std::function<bool(options::Value)> abstractMmlNodePredicate()const;
    std::function<bool(options::Value)> textNodePredicate()const;
};
}
