# P23h1 — predicate implementation ledger

Implemented only the four bodies frozen by `docs/math-port/evidence/p23h1-predicate-contract.md`: `AbstractMmlNode::isEmpty`, `AbstractMmlTokenNode::isEmpty`, `MmlMrow::isSpacelike`, and `MmlMrow::isEmbellished`. No declaration, helper, fixture, test, or other method was changed. No build or test was run; Root owns independent validation and acceptance.

## Method mapping

| Native method | Pinned MathJax source | Implementation correspondence |
|---|---|---|
| `src/math/core/mml_tree/mml_node.cpp` — `AbstractMmlNode::isEmpty` | `MmlNode.ts:547–552`; pinned `MmlNode.js:243–255` | Adds `TreeCallGuard<MmlNode>`, captures the `childNodes` handle once, and throws the frozen `TypeError: Object is not iterable.` when it is null. The index loop re-reads the captured array's live size and uses `ChildArray::get`, preserving prototype lookup and holes as `Missing`. `nodeOrNull()` applies the source truthiness test for the only falsey child alternatives (Undefined/Null); truthy nodes receive virtual `isEmpty()`, and false short-circuits. |
| `src/math/core/mml_tree/mml_branches.cpp` — `AbstractMmlTokenNode::isEmpty` | `MmlNode.ts:1040–1048`; pinned `MmlNode.js:689–700` | Uses the same captured/live array traversal and null-array TypeError. Every value must dynamically cast to `TextNode`; Undefined, Null, holes and non-TextNode children return false. Actual `TextNode` subclasses dispatch virtual `getText()`, and nonempty UTF-16 text returns false. It does not call child `isEmpty()` or the token node's own `getText()`. |
| `src/math/core/mml_tree/mml_nodes/mrow.cpp` — `MmlMrow::isSpacelike` | `mrow.ts:57–66`; pinned `mrow.js:81–99` | Captures one array handle, traverses live length/`Get`, and requires each yielded `NodeValue` as a node before virtual `isSpacelike()`. There is deliberately no falsey skip: a hole/Undefined or Null throws at that access; false short-circuits. An empty iterable returns true. |
| `src/math/core/mml_tree/mml_nodes/mrow.cpp` — `MmlMrow::isEmbellished` | `mrow.ts:72–89`; pinned `mrow.js:104–136` | Preserves local `embellished` state and the live loop index. Undefined/Null are skipped but still consume an index. For nodes it calls `isEmbellished()` first; a second true returns false immediately, while the first writes `core_ = i`. Only a false embellished result triggers `isSpacelike()`, whose false result short-circuits. It returns the local flag after the loop and never resets `core_` on entry. |

All four bodies use the existing `TreeCallGuard<MmlNode>` so recursive virtual calls share the accepted tree-call budget. Each loop captures the original shared array handle once, calls `get(i)` for each current index, and checks current `size()` on each iteration. This gives sparse-array prototype lookup and mutations to the captured array while avoiding redirection if a child callback rebinds the node's `childNodes`. No new helper or alternate storage was introduced.

Root review correction: the mapping table above describes the initial authorized
delivery. Root had frozen an incorrect generic Null iterator message. The actual
original comparison exposed four message differences; Root corrected all four
bodies to `object null is not iterable (cannot read property Symbol(Symbol.iterator))`.
Original gold was preserved. The current implementation follows that actual
original result, not the incorrect initial contract wording.

## Evidence and scope limits

The behavior mapping follows the pinned MathJax 4.1.3 TS/CJS fingerprints in P23h and the Root-frozen P23h1 contract. The 75 Source299 predicate rows and 111 supplemental actual-registry probes are frozen by Root; they were not run or edited here. Their positive `isEmbellished`/core-cache cases are supplemental probes, not properties inferred from the 75 negative/falsey Source299 rows.

This delivery does not implement or claim acceptance for `core`, `coreMO`, `notParent`, token text concatenation, or other predicates. It does not claim that the full module graph, prior packets, or Folia integration builds successfully. Root must independently compile and run the frozen assertions and mutations before accepting the packet.
