# P25 Layout/Base TeX-class implementation

This packet replaces only `AbstractMmlLayoutNode::setTeXclass` and
`AbstractMmlBaseNode::setTeXclass` in `src/math/core/mml_tree/mml_branches.cpp`.
It does not claim packet acceptance; Root owns source replay, regressions,
mutations, and build validation.

The pinned source is `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts`,
SHA-256 `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`.
`AbstractMmlLayoutNode.setTeXclass` at line 1153 first requires slot zero,
dispatches its virtual `setTeXclass(previous)`, stores that tagged return, then
reads the current child array and slot zero again for virtual
`updateTeXclass`. It returns the first call's original tagged result. The native
body uses `required_child_zero` for the initial member access and `child_zero`
after the callback; it does not retain the original base node for the update.
The existing `TreeCallGuard<MmlNode>` remains active across both calls.

`AbstractMmlBaseNode.setTeXclass` at line 1199 preserves this order:

1. Virtual `getPrevClass(previous)`, then virtual `setTexClass(ORD)`.
2. Capture tagged child slot zero and initialize the result to `Null`.
3. For a node base, evaluate virtual `this.isEmbellished` before the base's
   virtual `isKind("mi")`. The true branch captures the base's virtual
   `setTeXclass(previous)` result, then calls virtual `this.core()` and passes
   that returned tag to virtual `updateTeXclass`.
4. The false branch calls the captured base's virtual `setTeXclass(Null)` and
   ignores its result. It then checks `isKind("TeXAtom")`; on a match, it reads
   the base's virtual `texClass()` and writes that value through this node's
   virtual `setTexClass`.
5. After those callbacks, reread the current child array and take its
   `slice(1)` snapshot. Walk that distinct snapshot by index, reading each slot
   including holes and prototype values, and dispatch `setTeXclass(Null)` only
   for node-valued entries. Return the captured result only when it is a node;
   otherwise return this node.

The implementation uses `ChildValue` tags throughout. `child_zero` preserves
the null-array failure at the original access; the post-callback null-array
check preserves the later `slice` access failure. `ChildArray::slice(1)` keeps
the source array snapshot, holes, and inherited indexed values. The subsequent
indexed reads observe that snapshot's `for...of` values; they do not switch to a
dense vector or follow a rebound original array. Source setters and virtual
calls remain in their source positions, so an exception leaves earlier class
and callback effects intact.

Root's first full-graph compile rejected the initial `const auto* const`
declaration for the captured base: it made the pointed-to node const and blocked
the source-required virtual setter calls. The declaration now keeps the pointer
binding const while leaving the node mutable (`auto* const`), preserving those
virtual writes. Root will rerun the compile and acceptance checks.

No build or test was run, as required by the packet. The frozen oracle and all
acceptance checks remain for Root to run independently.
