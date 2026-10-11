// Root compile-time interface invariants. This supplies no Source gold.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.operator_class;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.core.mml_tree.mml_nodes.maction;
namespace mm=folia::math::mml;
using V=mm::NodeValue;
template<class T>constexpr bool taggedFamily=
 std::is_same_v<decltype(std::declval<T&>().core()),V> &&
 std::is_same_v<decltype(std::declval<T&>().coreMO()),V> &&
 std::is_same_v<decltype(std::declval<T&>().setTeXclass(V{})),V> &&
 std::is_same_v<decltype(std::declval<const T&>().Parent()),V> &&
 std::is_same_v<decltype(std::declval<const T&>().notParent()),folia::math::Property>;
static_assert(taggedFamily<mm::MmlNode> && taggedFamily<mm::AbstractMmlNode> &&
 taggedFamily<mm::AbstractMmlEmptyNode> && taggedFamily<mm::AbstractMmlLayoutNode> &&
 taggedFamily<mm::AbstractMmlBaseNode> && taggedFamily<mm::MmlMrow> &&
 taggedFamily<mm::MmlInferredMrow> && taggedFamily<mm::MmlMstyle> &&
 taggedFamily<mm::MmlMspace> && taggedFamily<mm::MmlMo> &&
 taggedFamily<mm::MmlTeXAtom> && taggedFamily<mm::MmlMaction>);
static_assert(std::is_same_v<decltype(std::declval<mm::MmlNode&>().appendChild(V{})),V>);
static_assert(std::is_same_v<decltype(std::declval<mm::MmlNode&>().replaceChild(V{},V{})),V>);
static_assert(std::is_same_v<decltype(std::declval<mm::MmlNode&>().removeChild(V{})),V>);
static_assert(std::is_same_v<decltype(mm::adjustOperatorTeXclass(std::declval<mm::MmlNode&>(),V{})),V>);
// Narrow Source Node|Null projection stays explicit for these filtered results.
static_assert(std::is_same_v<decltype(std::declval<mm::MmlMrow&>().firstNonSpace()),mm::MmlNode*>);
static_assert(std::is_same_v<decltype(std::declval<mm::MmlMo&>().coreParent()),mm::MmlNode*>);
