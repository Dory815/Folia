export module folia.math.core.mml_tree.mo_types;
export import folia.math.core.mml_tree.node_properties;
import std;

export namespace folia::math::mml {
// Source distinguishes an undefined _texClass before class initialization,
// null (which asks texClass() to resolve the operator definition), and a
// concrete class value. The actual concrete class lives only in node state.
enum class MoTexClassMode : std::uint8_t { unset, cleared, explicit_value };
} // namespace folia::math::mml
