# P25 Parent and childPosition implementation

Implemented only `AbstractMmlNode::Parent() const` and
`AbstractMmlNode::childPosition()` in `src/math/core/mml_tree/mml_node.cpp`,
following pinned `MmlNode.ts` (SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`).

`Parent()` captures the virtual `getParent()` result once. While that tagged
value contains a node, it reads the node's raw virtual `notParent()` and applies
JS truthiness. A truthy value advances through the node's virtual logical
`Parent()`; otherwise the current `NodeValue` is returned unchanged. Null and
Undefined therefore remain distinct in the result.

`childPosition()` follows the separate raw-parent algorithm. It captures
`getParent()` once, and while each node's raw `notParent()` is truthy it stores
that exact tagged node before reading its raw virtual `getParent()`. It does
not invoke logical `Parent()` or `childIndex()`. With no saved node, the search
target is this node. For a final node parent it captures that parent's current
child-array handle, reports the source TypeError for a null array, then reads
every indexed value while rechecking live length; holes yield Undefined,
prototype values are visible, and the first identity match returns its index.
No match or no final parent returns an empty `optional<size_t>`. Root's initial
frozen contract had the null-array error text wrong; the original Source
observation is `object null is not iterable (cannot read property
Symbol(Symbol.iterator))`, and the implementation uses that exact message.

Both methods keep the existing `TreeCallGuard<MmlNode>` resource boundary. No
other bodies, declarations, helpers, or storage were changed. No build or tests
were run; Root owns Source/native acceptance.
