// Live Context fields for the actual core/MmlTree/MmlNode owners.
// MathJax 4.1.3 source is Apache-2.0; see LICENSE-MathJax.txt.
export module folia.math.core.mml_tree.mml_node_context_fields;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.core.mml_tree.attributes_bindings;
export import folia.math.util.native_identities;
export import folia.math.core.tree.node_context_bindings;
import std;

export namespace folia::math::mml {
class MmlNodeContextFields {
    struct State;
    std::shared_ptr<State> state_;
public:
    using BindChildren=std::function<options::ObjectRef(Children)>;
    using ChildrenFromValue=std::function<Children(options::Value)>;
    using FactoryIdentity=std::function<options::ObjectRef(const std::shared_ptr<MmlFactory>&)>;
    using ResolveFactory=std::function<std::shared_ptr<MmlFactory>(options::ObjectRef)>;
    MmlNodeContextFields(options::Context&,ContextPropertyBindings&,
        AttributesContextBindings&,NativeIdentityBindings<MmlNode*>&,
        tree::TreeNodeContextBindings<MmlNode>&,
        std::weak_ptr<std::atomic_bool> executionLifetime,
        BindChildren,ChildrenFromValue,FactoryIdentity,ResolveFactory);
    MmlNodeContextFields(const MmlNodeContextFields&)=delete;
    MmlNodeContextFields& operator=(const MmlNodeContextFields&)=delete;
    void bind(options::ObjectRef identity,MmlNode&,
              const std::shared_ptr<MmlFactory>& owner);
};
}
