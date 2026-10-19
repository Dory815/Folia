// Candidate-only shared argument conversion for the pinned MmlNode and
// AbstractNodeFactory constructor call paths.
export module folia.math.core.mml_tree.mml_factory_context_values;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.util.context_property_bindings;
export import folia.math.util.native_identities;
export import folia.math.core.tree.node_context_bindings;
import std;

export namespace folia::math::mml {
std::vector<std::any> mmlFactoryContextArguments(
    ContextPropertyBindings&,
    NativeIdentityBindings<Children>&,
    tree::TreeNodeContextBindings<MmlNode>&,
    std::span<const options::Value> arguments);
}
