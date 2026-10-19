// Context method projection for the actual core/MmlTree/MmlFactory owner.
// MathJax 4.1.3 source is Apache-2.0; see LICENSE-MathJax.txt.
export module folia.math.core.mml_tree.mml_factory_context_methods;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.core.tree.factory_context_bindings;
export import folia.math.util.context_property_bindings;
export import folia.math.util.native_identities;
import std;

export namespace folia::math::mml {
class MmlFactoryContextMethods {
    struct State;
    std::shared_ptr<State> state_;
public:
    using ResolveFactory=std::function<std::shared_ptr<MmlFactory>(options::ObjectRef)>;
    using FindNodeIdentity=std::function<std::optional<options::ObjectRef>(MmlNode&)>;
    MmlFactoryContextMethods(options::Context&,ContextPropertyBindings&,
        NativeIdentityBindings<MmlNode*>&,std::shared_ptr<tree::FactoryContextBaseModule>,
        std::weak_ptr<std::atomic_bool>,
        ResolveFactory,FindNodeIdentity);
    MmlFactoryContextMethods(const MmlFactoryContextMethods&)=delete;
    MmlFactoryContextMethods& operator=(const MmlFactoryContextMethods&)=delete;
    // Installs the inherited Source create method on the shared factory
    // prototype and connects this already-canonical factory identity to it.
    void bind(options::ObjectRef identity,const std::shared_ptr<MmlFactory>& owner);
    options::ObjectRef prototype()const;
    options::ObjectRef constructor()const;
    options::ObjectRef basePrototype()const;
};
}
