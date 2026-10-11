# P23b child-value return and consumer audit

Read-only audit for the root-frozen ownership/storage decisions in
`docs/math-port/p23-child-storage-design.md` and
`docs/math-port/p23a-child-array-contract.md`. The upstream is the pinned
MathJax 4.1.3 source tree. This report inventories all 31 native child-array
consumers from `evidence/p23-sparse-identity-closure-audit.md`, then examines
the public and internal node-return boundaries where a `Product*` does not
preserve the source value. It does not authorize migration or assert that the
current native graph has sparse-child parity.

The pinned source revision is `fb987178c2d279a99b0db5ea02e751691703955e`.
Relevant source SHA-256 values are listed at the end. Native line numbers below
refer to the checked-out files examined for this report.

## Return-value findings that affect the interface

The source expressions below return a JavaScript value, not necessarily a
non-null node pointer. A raw pointer can encode Node versus a single null-ish
case only; it cannot preserve the source Undefined/Null tag. A `ChildValue`
slot read must remain tagged until the caller's original truthiness test,
identity comparison, or dereference point.

| API / source member | Exact source behavior and possible value | Current native boundary and mismatch |
|---|---|---|
| `AbstractNode.appendChild(child)` — `Tree/Node.ts:263–267` | `childNodes.push(child)` happens first, then `child.parent = this`, then the original `child` is returned. A normal successful return is that same Node. If `child` is Undefined or Null, the slot write has already happened when the parent access throws; the error text/state differ by tag. | `tree/node.ixx:78–84` accepts `Product*`, pushes it, then tests null and assigns parent. It preserves write-before-error for `nullptr` but cannot distinguish Undefined from Null at the argument, stored slot, error, or possible return. `Node` declares `Product* appendChild` in `node_types.ixx:24`. |
| `AbstractNode.replaceChild(newChild,oldChild)` — `Node.ts:272–283` | `childIndex(oldChild)` uses `indexOf` strict identity. If found, it writes `newChild` to the slot before `newChild.parent` access; only after that succeeds does it inspect/clear `oldChild.parent`. It returns `newChild` whether or not a match existed. Thus return can be Undefined, Null, or Node; a null-ish replacement can leave the new slot written before throwing. | `node.ixx:86–96` returns `Product*`, and pointer comparison/index search cannot distinguish a hole from Null. The signature in `node_types.ixx:25` cannot express an Undefined replacement or return. |
| `AbstractNode.removeChild(child)` — `Node.ts:288–295` | Uses `indexOf`; if found, `splice(i,1)` occurs before `child.parent = null`. A present Null can therefore be removed before the parent dereference throws. It always returns the original argument, even when not found: Undefined, Null, or Node. Holes are skipped by `indexOf`; explicit Null can match. | `node.ixx:98–106` returns a raw pointer. `childIndex` at 108–113 searches dense slots, so the future implementation must preserve ChildValue equality/presence before removal and retain the original tagged argument for the return. |
| Empty-node `appendChild` / `replaceChild` — `Node.ts:378–387` | These overrides do not mutate. `appendChild` returns its input unchanged; `replaceChild` returns `oldChild`, not `newChild`. Both can return Undefined, Null, or Node under runtime calls. | `node.ixx:192–195` returns `Product*`; the override's special return identity is expressible for Node/null but not Undefined. It must remain a distinct virtual override. |
| `AbstractMmlNode.appendChild` — `MmlNode.ts:607–633` | With negative arity, delegates to child zero, ignores the delegate's result, then returns the original argument. Infinite-arity inferred-child flattening appends each child via `super.appendChild` and returns the original inferred row. Other paths return `super.appendChild`'s result. Initial `child.isInferred` dereference precedes normalization; for malformed nullish child arguments, it throws before any append. | `mml_node.cpp:252–278` returns `MmlNode*`; the delegated/direct distinctions are there, but the raw argument and result lose Undefined versus Null. Flattening at 262–268 uses dense iteration, so holes/inherited values and exact append-failure state must be re-expressed. |
| `AbstractMmlNode.replaceChild` — `MmlNode.ts:639–645` | Negative arity delegates through child zero but returns `newChild`, not the inner result. Other cases return the superclass result, which also returns `newChild` regardless of whether found. | `mml_node.cpp:280–288` returns a pointer and routes through `child_at`; both arguments need tagged values if the contract includes source-reachable nullish calls. |
| `AbstractMmlNode.core/coreMO` — `MmlNode.ts:650–659` | Both return `this`, always a Node. The `AbstractMmlEmptyNode` and `MmlMath`/base empty branch likewise return `this` (`MmlNode.ts:1355–1364`). | `mml_node.cpp:290–291`, `mml_empty.cpp:37–38` return `MmlNode*`; these particular overrides are Node-only and do not need to synthesize Null/Undefined. They are nevertheless virtual downstream targets. |
| `AbstractMmlLayoutNode.core()` and `AbstractMmlBaseNode.core()` — `MmlNode.ts:1139–1141,1185–1187` | Directly return `childNodes[0]`. Empty length/out-of-range or a hole returns Undefined; an own Null returns Null; a node slot returns that exact Node. No dereference or fallback occurs in this method. | `mml_branches.cpp:13–16,115–117,137–139` uses `child_zero`, which returns nullptr for empty and otherwise the front pointer. It collapses Undefined/Null and does not call prototype Get. These `core()` overrides are declared as `MmlNode*` in `mml_node.ixx:179,188`. |
| Layout/Base `coreMO()` — `MmlNode.ts:1146–1148,1192–1194` | Reads child zero, then calls that value's `coreMO()`. A missing/hole or Null slot throws at the member access, with a different nullish error; a Node delegates and returns that method's exact result. It is not equivalent to `core()` returning the child slot. | `mml_branches.cpp:119–143` calls `required_child_zero().coreMO()`, but the helper distinguishes empty from explicit null only in dense storage and cannot see holes/prototype values. The return type is raw `MmlNode*`. |
| `MmlMrow.core()` / `coreMO()` — `MmlNodes/mrow.ts:95–110` | `core()` first evaluates virtual `isEmbellished` and `_core == null`; either false path returns `this`. Otherwise it returns `childNodes[_core]` directly: Undefined, Null, or the selected Node are all possible. `coreMO()` has the same predicate/re-read, then dereferences `.coreMO()` on the selected value; a nullish child throws, otherwise the nested return is passed through. | `mrow.cpp:50–67` has the correct predicate/re-read placement but its indexed pointer read returns nullptr for out-of-range and Null alike; `coreMO` emits one pointer-null error. `mrow.ixx:10` exposes both methods as `MmlNode*`. Root must freeze whether the return boundary is a tagged `ChildValue` and where it is unwrapped. |
| `MmlMaction.selected` — `MmlNodes/maction.ts:61–65` | Computes an index from current child length and `selection`; returns `childNodes[i] || factory.create('mrow')`. A normal selected child is Node. Hole, Undefined, Null, or an index that does not name an own/inherited truthy child triggers fallback creation, so these falsey slots are deliberately collapsed here. The length read precedes selection numeric coercion; the array is re-read for indexed Get afterward. | `maction_selection.cpp:14–42` reproduces read/coercion/rebind timing but pointer indexing collapses hole/Undefined/Null before the source `||` projection. `maction.ixx:12` returns `MmlNode*`. The selection consumer may project falsey tags to fallback, but must not make this the global child-access rule. |
| `MmlMaction.core/coreMO` — `maction.ts:84–93` | Calls selected, then delegates to `selected.core()` or `selected.coreMO()`. The selected result normally is a Node because of the `||` fallback; the delegated method's return/error behavior remains relevant. | `maction_selection.cpp:58–70` uses a raw selected pointer and raw pointer return; downstream identity/tag semantics of the delegated core operation are lost. |
| `MmlMrow.firstNonSpace/lastNonSpace` — `mrow.ts:125–149` | Each returns the first/last truthy child whose `isSpacelike` is false; otherwise explicitly returns Null. Holes/Undefined/Null are skipped by the truthiness condition. A present Node may be returned by identity. `lastNonSpace` captures initial length but re-reads `this.childNodes` on every indexed iteration. | `mrow.cpp:81–104` returns pointer/null and records lastNonSpace's re-read. The normal source result is Node or explicit Null, not Undefined, but sparse Get/prototype semantics and receiver rebind still matter. |
| `MmlMo.coreParent()` — `MmlNodes/mo.ts:235–249` | Starts with `this`, walks parent links while embellished/core identity and non-math checks pass, and returns `embellished || this`; its normal result is always a Node, even if there is no qualifying parent. | `mo_classes.cpp:15–26` returns a pointer and preserves fallback identity. Its condition calls `parent.coreMO()`, whose tagged return/error boundary is separate. |
| `MmlNode.Parent` and `mError()` — `MmlNode.ts:575–580,987–1011` | `Parent` returns the first logical parent or explicit Null; `mError` returns Null when already inside `merror`, otherwise the newly created error node. These source paths have the narrower Node-or-Null result (unlike direct child reads). | `mml_node.cpp` parent/error methods and `mml_empty.cpp:47` use `MmlNode*`; that representation can express this normal result set if null is kept as the explicit Null projection. It must not be reused to encode arbitrary child Get results. |
| `NodeUtil.getChildAt(node,position)` / `getCoreMO(node)` — `input/tex/NodeUtil.ts:191–193,273–275` | `getChildAt` directly returns indexed Get: Undefined for a hole/out-of-range absent property, Null for explicit null, or Node. `getCoreMO` directly returns `node.coreMO()` and therefore preserves delegated result or its thrown error. `setChild` at 202–208 does indexed assignment first and only sets parent when child is truthy; `copyChildren` at 216–221 reads every index then calls setChild, so a source hole becomes a present Undefined target slot. | There is currently no `src/math/input/tex/NodeUtil` native implementation. Its future direct return and write adapter must use tagged ChildValue, not be routed through appendChild or a generic required-node helper. This is a concrete source/native closure dependency, not supplied by P23a ChildArray alone. |

The following are child-slot reads consumed by Boolean/predicate expressions,
not node-return APIs, but they also cannot be normalized at the ChildArray
boundary. `MmlMstyle.notParent` (`mstyle.ts:59–61`) returns the raw first child
when it is falsey (Undefined or Null), otherwise returns the one-child boolean;
the current `bool MmlMstyle::notParent` cannot preserve that JavaScript return
union. `MmlMrow.core()` only returns `this` on the false predicate path; on the
true path the source direct child value must remain tagged. `MmlMo.isAccent`
(`mo.ts:286–310`) indexes mover/munder/munderover slots and calls `coreMO()`;
missing versus null child fails at that access. `MmlMaction`'s selected-child
fallback is the notable opposite: its `||` intentionally maps every falsey
slot to a newly created mrow.

## All 31 native consumers and corresponding Source paths

This list preserves the closure audit's 31 native source-file inventory. Rows
marked “adapter” are native helper/storage files whose behavior is driven by
the cited Source callers; they are still in the migration closure.

| # | Native consumer and locations | Fixed Source counterpart / child behavior |
|---:|---|---|
| 1 | `src/math/core/tree/factory_types.ixx:35` | `core/Tree/Node.ts:48,176`: one `childNodes` array property; Root's alias change must make this the sole typed sparse handle. |
| 2 | `src/math/core/tree/node_factory.ixx:18–37` | `Node.ts:176,186–196`: constructor argument normalization; exact child-handle type is part of the Source construction boundary. |
| 3 | `src/math/core/tree/node.ixx:33–192` | `Node.ts:253–351,373–402`: set/append/replace/remove/index/copy/walk/join and Empty overrides. Captured array identity, live for-of length, holes, and nullish write/error order differ by method. |
| 4 | `src/math/core/tree/node_types.ixx:13–26` | `Node.ts:41–48,90–112`: public node/child interface. Current pointer signatures cannot preserve tagged child input/output on append/replace/remove. |
| 5 | `src/math/core/tree/wrapper.ixx:32–48` | `core/Tree/Wrapper.ts:152–168`: wrapper walk checks `if (child)` and skips falsey values; for-of captures the wrapper child array and observes live iteration length. |
| 6 | `src/math/core/tree/wrapper_types.ixx:7–18` | `Wrapper.ts:55,120`: optional wrapper child-list handle is separate from a child slot; absent list versus present-null handle must not be confused with a hole/value. |
| 7 | `src/math/core/mml_tree/arithmetic_host.ixx:12–23` | `MmlNode.ts` inherited child traversal and MML direct-first-child methods; current `arithmeticChild` rejects empty/null through raw pointers. Each caller must choose required dereference versus falsey projection at its own Source site. |
| 8 | `src/math/core/mml_tree/mml_branches.cpp:13–166` | `MmlNode.ts:1109–1218`: AbstractMmlLayoutNode/BaseNode direct core child, coreMO dereference, setTeXclass, and captured/live child traversals. `child_zero` loses hole/Undefined/Null. |
| 9 | `src/math/core/mml_tree/mml_node.cpp:42–553` | `MmlNode.ts:418–658,681,790–855,906–927,973–1011`: base constructor, MML copy, set/append/replace, parent-position scan, arity trim/grow, inheritance/verification, and error replacement. These include both direct slot return/delegation and partial mutation. |
| 10 | `src/math/core/mml_tree/mml_node.ixx:14–214` | `MmlNode.ts:170,418,650–659,1139–1148,1185–1194,1355–1364`: MML child alias and virtual core/child API. Current `MmlNode*` virtual returns cannot encode direct Undefined versus Null. |
| 11 | `src/math/core/mml_tree/mo_host.ixx:11` | `MmlNodes/mo.ts:267–273,286–304,393–400`: direct child lookup for core text, accent determination and last-child BIN normalization. `moChild` currently returns nullptr for both out-of-range and a null slot; source may instead read inherited numeric data or throw on dereference. |
| 12 | `src/math/core/mml_tree/multiscript_host.ixx:10–22` | `MmlNodes/mmultiscripts.ts:71–99,115–135`: indexed slots, `splice`, parity insertion, and required child dereferences. `multiscriptSlot` currently collapses absent/null. |
| 13 | `src/math/core/mml_tree/operator_class.cpp:53–63` | `MmlNodes/mo.ts:381–402`: parent ascent and compare `parent.core()` / final `childNodes[length-1]` by identity. The final array read can be Undefined/Null/Node; comparison does not dereference it. |
| 14 | `src/math/core/mml_tree/table_host.ixx:7–49` | `MmlNodes/mtr.ts:82–125`, `mtable.ts:140–199`: indexed row/cell access and replacement chains. `tableSlot`/`tableNode` presently collapse/proactively reject nullish values; Source determines the exact dereference point. |
| 15 | `src/math/core/mml_tree/mml_nodes/maction_behavior.cpp:29–95` | `MmlNodes/maction.ts:114–160`: tooltip slot checks and child inheritance. Optional chaining skips both Undefined and Null, but must read current slot at each source access. |
| 16 | `src/math/core/mml_tree/mml_nodes/maction_selection.cpp:14–70` | `maction.ts:61–93`: `selected` truthy fallback, selected `core` and `coreMO`. `selected` intentionally consumes falsey ChildValue; core methods must retain their distinct return semantics. |
| 17 | `src/math/core/mml_tree/mml_nodes/mathchoice.cpp:34–50` | `MmlNodes/mathchoice.ts:80–83`: direct selected slot `||` a factory-created mrow, then parent replacement, then inherited-attribute call on the captured child. If replacement callbacks rebind arrays/parent, the captured selected child is still used. |
| 18 | `src/math/core/mml_tree/mml_nodes/menclose.cpp:18–24` | `MmlNodes/menclose.ts:76–79` plus child access at setTeXclass: first child direct dereference then another read after the child callback. Missing/null must error after the correct earlier side effects. |
| 19 | `src/math/core/mml_tree/mml_nodes/mfenced.cpp:14–162`, `.ixx:13` | `MmlNodes/mfenced.ts:80–123,129–160`: optional truthy first/child reads, captured count, re-reads, `slice(1)`, separator construction and child list processing. Sparse slice and `if(child)` are materially different operations. |
| 20 | `src/math/core/mml_tree/mml_nodes/mfrac.cpp:13–26,29–65` | `MmlNodes/mfrac.ts:83–88,129–140`: for-of setTeXclass and indexed numerator/denominator inheritance. The for-of path dereferences yielded child without a truthy guard; direct indices likewise fail at the member call. |
| 21 | `src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp:19–110` | `MmlNodes/mmultiscripts.ts:71–135`: several separately captured loops, current index reads, splice insertion, and append followed by a fresh length/index read. Do not use one generic dense iteration policy. |
| 22 | `src/math/core/mml_tree/mml_nodes/mo_classes.cpp:15–99` | `MmlNodes/mo.ts:235–310`: coreParent's Node fallback and accent child selection. The latter direct-indexes then invokes coreMO; falsey tests and throws belong to each branch. |
| 23 | `src/math/core/mml_tree/mml_nodes/mo_scripts.cpp:79–94` | `MmlNodes/mo.ts:547–562`: selected base/under/over slots and short-circuit coreMO comparisons. The nested conditions intentionally may read under for the over test. |
| 24 | `src/math/core/mml_tree/mml_nodes/mroot.cpp:12–36` | `MmlNodes/mroot.ts:82–105`: direct base/index child calls. Base child is processed before root-index child; a nullish first slot aborts before slot 1. |
| 25 | `src/math/core/mml_tree/mml_nodes/mrow.cpp:19–155` | `MmlNodes/mrow.ts:58–182`: for-of predicates/classes, direct core/coreMO, first/last nonspace, and first-child update after traversal. `lastNonSpace` re-reads array; `core` returns direct slot; `coreMO` dereferences it. |
| 26 | `src/math/core/mml_tree/mml_nodes/mstyle.cpp:16–71` | `MmlNodes/mstyle.ts:59–61,82–117`: raw falsey first child in `notParent` and nested child-length reads; direct inheritance call. The native bool return cannot represent raw Undefined/Null short-circuit results. |
| 27 | `src/math/core/mml_tree/mml_nodes/msubsup.cpp:18–73` | `MmlNodes/msubsup.ts:84–107`: captures one `nodes` array handle, accesses base/sub/sup in order, returns early on falsey sup, and computes script levels before each call. A hole at sup is read as Undefined; the same captured array remains selected across callbacks. |
| 28 | `src/math/core/mml_tree/mml_nodes/mtable.cpp:41–147` | `MmlNodes/mtable.ts:140–211`: row normalization, inherited row slots, live index loop with removal/replacement and nested append chains. Captured row handle, current receiver rebinding, and returned replacement identity all matter. |
| 29 | `src/math/core/mml_tree/mml_nodes/mtr.cpp:16–99` | `MmlNodes/mtr.ts:82–139`: captured for-of normalization and later child traversal; `replaceChild(...).appendChild(...)` consumes the replacement return before appending. |
| 30 | `src/math/core/mml_tree/mml_nodes/munderover.cpp:48–101,103–147` | `MmlNodes/munderover.ts:102–177`: direct base/under/over lookups, script getters, truthy optional over branch, coreMO and accents. `if (!node) return` semantics differ from required child access. |
The table has 30 behavioral rows because row 19 groups `mfenced.cpp` and
`mfenced.ixx`; those two entries make the exact closure count 31 native files.
Pinned TeX `NodeUtil.ts:67–89,191–221,273–275` is an additional required
downstream Source caller but does not yet have a native file in this list. The
complete exact native file list is:

`factory_types.ixx`, `node_factory.ixx`, `node.ixx`, `node_types.ixx`,
`wrapper.ixx`, `wrapper_types.ixx`, `arithmetic_host.ixx`, `mml_branches.cpp`,
`mml_node.cpp`, `mml_node.ixx`, `mo_host.ixx`, `multiscript_host.ixx`,
`operator_class.cpp`, `table_host.ixx`, `maction_behavior.cpp`,
`maction_selection.cpp`, `mathchoice.cpp`, `menclose.cpp`, `mfenced.cpp`,
`mfenced.ixx`, `mfrac.cpp`, `mmultiscripts.cpp`, `mo_classes.cpp`,
`mo_scripts.cpp`, `mroot.cpp`, `mrow.cpp`, `mstyle.cpp`, `msubsup.cpp`,
`mtable.cpp`, `mtr.cpp`, and `munderover.cpp`.

## Downstream consumer and override obligations

The source's node-return override family is `AbstractNode` append/replace/remove;
`AbstractEmptyNode` append/replace; `AbstractMmlNode` append/replace/core/coreMO;
`AbstractMmlLayoutNode` and `AbstractMmlBaseNode` core/coreMO; empty MML
core/coreMO; `MmlMrow` core/coreMO/firstNonSpace/lastNonSpace;
`MmlMaction` selected/core/coreMO; and `MmlMo.coreParent`. These are separate
virtual paths, not one “get child” operation. The direct-return overrides
(`core`, `selected`, slot helpers) can expose tagged values; the delegated
overrides can pass through another method's tagged value; self/fallback methods
are Node-only. Empty-node methods deliberately return different arguments.

Downstream source consumers that depend on the exact value include:

* `Node.ts` copy skips falsey children, `walkTree` iterates captured/live
  child-array slots and recurses only on truthy values, and `toString` uses
  `Array.join`, whose nullish slots contribute empty text.
* `MmlNode.ts` copy's `for-of` reads a hole as Undefined, then its `else`
  branch pushes explicit Null; generic Node copy instead skips falsey children.
  `setChildren` replaces the child list before iteration; append/replace/remove
  parent writes follow the ordering above.
* MML inherited/verification traversal (`MmlNode.ts:846–855,973–977`) calls
  each yielded child's method without a nullish guard. `AbstractMmlTokenNode`
  traversal (`1072–1083,1090–1097`) first applies the `instanceof` test, so
  nullish/non-MML slots are skipped. These cannot share one “skip null” loop.
* `maction`, `mrow`, `mstyle`, `mfenced`, `mo`, `mfrac`, `mroot`, `msubsup`,
  `munderover`, `mmultiscripts`, `mtr`, and `mtable` use mixtures of direct
  index Get, `||`, optional chaining, for-of, forEach, slice, splice, and
  callback-time property re-reads. Their native callsites are listed above.
* TeX `NodeUtil.getChildAt` returns the direct value; `setChild` assigns before
  the truthiness-guarded parent write; `copyChildren` reads every index and
  writes it, densifying holes as present Undefined. `getCoreMO` directly
  returns the virtual result. No existing native `NodeUtil` module currently
  represents these operations.

## Required root freezes before the full consumer migration

1. Freeze the only `FactoryChildren` alias and every exact `std::any` cast at
   factory/node/wrapper boundaries. The ownership model remains
   `FactoryServices` owning Products through `unique_ptr`; ChildArray stores
   only borrowed Product identity.
2. Freeze which node-return virtual methods and child-slot helpers carry
   `ChildValue<Product>` at the boundary. At minimum, direct Get results used
   by `core()`/`selected()`/`NodeUtil.getChildAt`, and append/replace/remove
   arguments and return values, need Undefined/Null/Node identity. Do not return
   a raw pointer and attempt to reconstruct the lost tag later.
3. Freeze callsite-specific projections: selected's `||` fallback, first/last
   nonspace falsey skipping and explicit Null return, coreMO's immediate
   dereference/error, generic Node falsey loops, and MML copy's explicit Null
   push. A global `requireNode`, `nodeOrNull`, or “null means absent” helper
   would change different source sites in opposite ways.
4. Freeze parent-link side effects against ChildValue arguments. Append writes
   array then accesses `.parent`; replace writes the slot, accesses new
   `.parent`, then old `.parent`; remove splices first then accesses
   `.parent`. Source error cases can leave partial slot and parent changes.
5. Freeze iteration forms per source method: for-of captures array identity but
   checks live length; forEach captures length and HasProperty; direct loops
   sometimes capture array, sometimes reread `this.childNodes`; slice/splice
   preserve holes and inherited numeric values. A common C++ range loop is not
   sufficient.
6. Freeze the `NodeUtil` child-value adapter and all 31 consumer callsites
   together. It must retain direct indexed value tags and separate append,
   replace, remove, and copy semantics; no parallel dense compatibility path.

## Source hashes inspected

| Pinned Source file | SHA-256 |
|---|---|
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `ts/core/Tree/Wrapper.ts` | `8729a810aabd6eb3ad22021464f2f27d86116fae9edc0f818527f82e517bf7d9` |
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `ts/core/MmlTree/MmlNodes/maction.ts` | `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895` |
| `ts/core/MmlTree/MmlNodes/mrow.ts` | `af58f0c1aa39a71d6c3dc5addf358a8cefeecb3f76cb483ec940f3a7ea72b4b9` |
| `ts/core/MmlTree/MmlNodes/mathchoice.ts` | `e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76` |
| `ts/core/MmlTree/MmlNodes/mstyle.ts` | `7442e2885d2f4605d42f5b4472b3183f9cbadbd8ed5c0aeddf03b8f23c1d8616` |
| `ts/core/MmlTree/MmlNodes/mo.ts` | `2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605` |
| `ts/core/MmlTree/MmlNodes/mfenced.ts` | `64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397` |
| `ts/core/MmlTree/MmlNodes/mroot.ts` | `b4b8f9deddb22efe7580067d8845d7cb225046e244eb93da8b0e931eff76e99a` |
| `ts/core/MmlTree/MmlNodes/msubsup.ts` | `37fd01fa0c44ceab5b4ed30ba3d95ab43ffb8aed55db27c7290d6d786131f6e7` |
| `ts/core/MmlTree/MmlNodes/munderover.ts` | `b9bf04f77922c50044c4d99b3093882cab9b35ec36203f20e1e999997f9adf82` |
| `ts/core/MmlTree/MmlNodes/mmultiscripts.ts` | `bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7` |
| `ts/core/MmlTree/MmlNodes/mfrac.ts` | `4ded5fc9922d462d02818523d676c30299d08cfe7794067bd4e20950c8fccfc7` |
| `ts/core/MmlTree/MmlNodes/mtr.ts` | `ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f` |
| `ts/core/MmlTree/MmlNodes/mtable.ts` | `8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda` |
| `ts/input/tex/NodeUtil.ts` | `bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2` |
| `ts/core/MmlTree/MmlNodes/menclose.ts` | `ffa4ab507744cb4f190312ca8dcd692474c333bee6e19c86709554aa9dad4333` |

No source test, build, or experiment was run for this audit. It is a boundary
review only; Root owns the ensuing API freeze, native implementation, oracle,
and acceptance.
