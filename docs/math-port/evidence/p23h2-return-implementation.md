# P23h2 — raw core and `notParent` implementation ledger

Implemented only the seven bodies frozen in `docs/math-port/evidence/p23h2-return-contract.md`: `child_zero`, `required_child_zero`, layout/base `coreMO`, `MmlMrow::core`, `MmlMrow::coreMO`, and `MmlMstyle::notParent`. No declarations, other functions, helpers, storage, fixtures, or golds were changed. No build or test was run; Root owns independent validation and acceptance.

## Method mapping

| Native method | Pinned MathJax source | Implementation correspondence |
|---|---|---|
| `mml_branches.cpp` — `child_zero` | `MmlNode.ts:1139–1142` and `1185–1188` | Performs the single current `Get(0)` and returns the resulting tagged `NodeValue`; holes/out-of-range remain Undefined and explicit Null remains Null. A null child array errors as a read of index `0`, before any conversion or member lookup. |
| `mml_branches.cpp` — `required_child_zero` | Existing layout/base predicates `MmlNode.ts:1118–1136`, `1178–1182` | Performs the same current indexed Get, then uses `requireNode(member)` to preserve the source member-access error for Undefined/Null. Existing predicate wrappers were not changed. |
| `mml_branches.cpp` — `AbstractMmlLayoutNode::coreMO` | `MmlNode.ts:1146–1147` | Guards recursive dispatch, gets current child zero through `child_zero`, calls that node's virtual `coreMO`, and returns its exact `NodeValue`, including an Undefined or Null result. |
| `mml_branches.cpp` — `AbstractMmlBaseNode::coreMO` | `MmlNode.ts:1192–1193` | Same direct child-zero access and virtual call semantics as layout `coreMO`, with its own recursive call guard. |
| `mml_nodes/mrow.cpp` — `MmlMrow::core` | `mrow.ts:95–102` | Calls virtual `isEmbellished()` first. False or missing `core_` returns this without reading the array. Otherwise it reads the current optional index after the getter, reads that slot from the current array, and returns the tagged value unchanged. An absent array reports its actual index in the null-array read error; present out-of-range returns Undefined. Index zero is engaged, not confused with absent. |
| `mml_nodes/mrow.cpp` — `MmlMrow::coreMO` | `mrow.ts:105–110` | Uses the same getter/cache/current-array order, then requires the selected slot's `coreMO` member and calls it virtually. This preserves the source error for a selected Undefined/Null and returns the virtual method's raw tagged result. |
| `mml_nodes/mstyle.cpp` — `MmlMstyle::notParent` | `mstyle.ts:59–61` | Reads current slot zero; falsey Undefined/Null is returned as raw `Property`. For a truthy node, it performs a second current Get(0), requires that result's `childNodes`, checks that array's length is exactly one, and returns a boolean Property. Null top-level arrays fail on index `0`; null inner arrays fail on `length`. The parent array's own length is never substituted. |

## Source-order and data-boundary notes

The operations use the one existing sparse `ChildArray`/`NodeValue` graph. `ChildArray::get(0)` is the source indexed Get and includes prototype values. `NodeValue` keeps hole/Undefined separate from explicit Null; `Property` does the same for `notParent`. No null pointer is used as a substitute for both states.

For row core access, `isEmbellished()` executes before either the optional cache check or the child-array read. The subsequent array and index are not captured before virtual dispatch, so a getter can update the cache or rebind the array. An engaged index zero is honored. A null array after the getter produces a TypeError naming the current numeric index, including `999`; a non-null array with an absent slot returns Undefined from `core()` or produces the corresponding `coreMO` member-access TypeError. No result is normalized to the row or to Null.

For `MmlMstyle.notParent`, the first Get is used only for the JavaScript truthiness branch. A falsey first result is returned unchanged. A truthy first result triggers a second read of the current array's slot zero before the child's `childNodes.length` access. This retains the two source reads instead of caching the first child. The native child-array field is a typed handle; arbitrary JavaScript accessor/proxy behavior is outside this packet's representation, as the frozen contract states.

This delivery does not implement or claim acceptance for other predicates, TeX classes, layout, rendering, or full MML. The original Return32 plus 155 supplemental actual-CJS cases and mutation checks remain Root's acceptance gate; their outcome is not claimed here.
