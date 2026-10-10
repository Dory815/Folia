# P8b token, layout, and base node implementation evidence

This packet ports the executable members of `AbstractMmlTokenNode`,
`AbstractMmlLayoutNode`, and `AbstractMmlBaseNode` from the pinned MathJax 4.1.3
source at commit `fb987178c2d279a99b0db5ea02e751691703955e`. Root owns frozen
declarations, bindings, reference expectations, builds, and acceptance. This
report is evidence for the implementation, not acceptance.

## Method ledger

TS line references are in `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts`;
the pinned emitted implementation is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNode.js`.

| Class / source member | Native member | Branches and observable behavior preserved |
|---|---|---|
| `AbstractMmlTokenNode.isToken` (TS 1031-1034; CJS 682-688) | `isToken()` | Always returns true. |
| `AbstractMmlTokenNode.isEmpty` (TS 1039-1048; CJS 689-710) | `isEmpty()` | Iterates the current child array; returns false for every non-TextNode (including null), or a TextNode with nonempty UTF-16 text; true for an empty list or only empty TextNodes. Uses `dynamic_cast<TextNode*>` so registered subclasses retain class identity. A null child-array handle raises TypeError. Uses the shared tree-call guard for virtual `TextNode.getText()` re-entry. |
| `AbstractMmlTokenNode.getText` (TS 1055-1067; CJS 712-733) | `getText()` | Concatenates TextNode text in iteration order. Other nodes are skipped unless the frozen `TextContentBinding` is present; an absent binding is skipped and a present empty/non-callable binding raises TypeError. A bound callback is invoked at the exact traversal point and its UTF-16 result is appended. A null child raises TypeError at the source-equivalent `in` operation. Uses the shared tree-call guard for virtual text/callback re-entry. |
| `AbstractMmlTokenNode.setChildInheritedAttributes` (TS 1072-1085; CJS 735-751) | `setChildInheritedAttributes(...)` | Uses a live iterator over one captured child-array handle; only actual `AbstractMmlNode` instances receive the same attributes handle and display/level/prime values. TextNode, XMLNode, other empty nodes, and null are skipped. A null child-array handle raises TypeError. |
| `AbstractMmlTokenNode.walkTree` (TS 1090-1097; CJS 753-771) | `walkTree(...)` | Calls the visitor on self first, then walks only actual `AbstractMmlNode` children, skipping TextNode/XMLNode/other empty nodes and nulls. Captures the array after the self callback, then observes its live size while retaining that array identity if callbacks replace the node's `childNodes`. Returns the original `std::any` data handle. Uses the shared `TreeCallGuard` on every recursive virtual traversal entry. |
| `AbstractMmlLayoutNode.isSpacelike` (TS 1118-1120; CJS 782-787) | `isSpacelike()` | Delegates to child zero. Missing child or null child produces TypeError; nested layout dispatch is bounded by the shared tree-call guard. |
| `AbstractMmlLayoutNode.isEmbellished` (TS 1125-1127; CJS 789-794) | `isEmbellished()` | Delegates to child zero, retaining virtual dispatch and the same missing/null TypeError distinction. |
| `AbstractMmlLayoutNode.arity` (TS 1132-1134; CJS 796-801) | `arity()` | Always returns -1. |
| `AbstractMmlLayoutNode.core` (TS 1139-1141; CJS 803-804) | `core()` | Returns child zero directly; an empty array maps JavaScript `undefined` to native null, while a null child-array handle raises TypeError. |
| `AbstractMmlLayoutNode.coreMO` (TS 1146-1148; CJS 806-807) | `coreMO()` | Delegates to child zero's virtual `coreMO`; missing/null child raises TypeError and recursive chains use `TreeCallGuard`. |
| `AbstractMmlLayoutNode.setTeXclass` (TS 1153-1157; CJS 809-812) | `setTeXclass(previous)` | Calls child zero's `setTeXclass(previous)`, then rereads current child zero for `updateTeXclass`, and returns the child's result. The reread preserves mutations made during the child call and does not dispatch through a potentially overridden `core()`. Missing/null child raises TypeError. This override and the public base entry both use the shared operation guard. |
| `AbstractMmlBaseNode.isEmbellished` (TS 1178-1180; CJS 823-828) | `isEmbellished()` | Delegates to child zero virtually; missing/null child raises TypeError and nested dispatch uses the shared tree-call guard. |
| `AbstractMmlBaseNode.core` (TS 1185-1187; CJS 830-831) | `core()` | Returns child zero directly, with empty-array-to-null and null-array TypeError behavior. |
| `AbstractMmlBaseNode.coreMO` (TS 1192-1194; CJS 833-834) | `coreMO()` | Delegates to child zero virtually; missing/null child raises TypeError and recursive chains use `TreeCallGuard`. |
| `AbstractMmlBaseNode.setTeXclass` (TS 1199-1224; CJS 836-867) | `setTeXclass(previous)` | Calls `getPrevClass`, sets ORD, reads child zero, then processes an existing base. An embellished node or `mi` delegates with the incoming previous node and updates from a fresh virtual `core()` lookup; otherwise the base is processed with null and a TeXAtom copies its optional class. It then snapshots `slice(1)` after base handling and sends null to each non-null script child. Returns the truthy base result or this node. State mutation and errors occur in source order; this override and the public base entry both use the shared operation guard. |

The `defaults()` methods are deliberately absent here: root defines Token's
copied default record and Layout/Base's aliases in `mml_bindings.cpp`, as the
contract requires. No new declarations or default identities were introduced.

## Native adaptations and boundaries

RTTI `dynamic_cast` models the source's `instanceof` checks for TextNode and
AbstractMmlNode. The only supported non-TextNode `textContent` property is the
root-frozen optional `TextContentBinding`: disengaged models an absent property;
an engaged empty function models a present non-callable property; an engaged
callable is invoked. This intentionally does not evaluate arbitrary JS or infer
properties from `kind()`.

The accepted child storage is a shared array handle. Iteration retains the
handle acquired at the corresponding JavaScript for-of start and checks its
current size, so appends are visible while reassignment of `childNodes` does not
replace the active iterator's array. The base-node scripts use a vector snapshot
because source `slice(1)` copies after base handling. Empty child arrays return
null from `core()` (the native representation of source `undefined`); methods
that immediately read a member from that missing child throw the host TypeError.
The implementation uses the frozen `TreeCallGuard<MmlNode>` for recursive
walkTree and delegated layout/base property chains; inherited-attribute and
TeX-class entry guards remain in the public AbstractMmlNode implementation.

The packet does not implement concrete MathML nodes, parser behavior, generic
tree algorithms, arbitrary JavaScript property bags/callbacks, or rendering.
The frozen Node-generated `reference_mml_node.cpp` fixture and root's build,
mutation, and full-suite checks have not been run by this agent.

## Source fingerprints

| Source | SHA-256 |
|---|---|
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNode.js` | `86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45` |
