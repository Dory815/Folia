// Context prototype projection for the existing Source Tree/MML node methods.
export module folia.math.core.mml_tree.mml_node_context_methods;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.util.native_identities;
export import folia.math.util.context_property_bindings;
export import folia.math.core.tree.node_context_classes;
export import folia.math.core.tree.node_context_bindings;
export import folia.math.core.mml_tree.mml_node_algorithms;
export import folia.math.core.mml_tree.attributes_bindings;
import std;

export namespace folia::math::mml {
struct MmlNodeContextPrototypes {
    options::ObjectRef node{},mmlNode{},token{};
};
class MmlNodeContextMethods {
    struct State;
    std::shared_ptr<State> state_;
public:
    MmlNodeContextMethods(options::Context&,NativeIdentityBindings<MmlNode*>&,
                          ContextPropertyBindings&,
                          AttributesContextBindings&,
                          tree::TreeNodeContextBindings<MmlNode>&,
                          tree::TreeNodeContextClasses<MmlNode>&,
                          std::function<options::Value(tree::FactoryChildren<MmlNode>)>,
                          std::function<bool(options::Value)> isAbstractMmlNode,
                          std::weak_ptr<std::atomic_bool> executionLifetime,
                          std::function<bool(options::Value)> isTextNode,
                          std::optional<MmlNodeContextPrototypes> sharedPrototypes={});
    MmlNodeContextMethods(const MmlNodeContextMethods&)=delete;
    MmlNodeContextMethods& operator=(const MmlNodeContextMethods&)=delete;
    void bind(options::ObjectRef identity,MmlNode&,bool preservePrototype=false);
    options::ObjectRef nodePrototype()const;
    options::ObjectRef tokenPrototype()const;
    options::ObjectRef mmlNodePrototype()const;
};
}
