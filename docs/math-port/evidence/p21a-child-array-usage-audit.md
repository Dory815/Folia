# P21a child-array usage audit

Read-only inventory of the current native child-array surface and the pinned
MathJax 4.1.3 TeX source call domain. This report records observed operations
and data shapes for the root-owned storage/API freeze; it does not prescribe a
container design.

## Scan boundary and counts

The scan covered `src/math/core/tree`, `src/math/core/mml_tree`,
`src/math/input/tex`, and `tests/math`, plus all TypeScript under the pinned
`.reference/MathJax-src/ts/input/tex` tree selected by the enabled profile in
`docs/math-port/scope.json` (SHA-256
`53e1bef466ae717954c692123b133d889921aaf4b89ceaff21e37730a0c22ee3`). The
profile is `tools/mathjax-bundle/entry.cjs` (SHA-256
`a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9`). The
source closure is the reviewed 246-file TeX closure for MathJax commit
`fb987178c2d279a99b0db5ea02e751691703955e`.

| Scan | Count | Meaning |
|---|---:|---|
| Native core/MML files with child handle/type references | 31 files | All `*.ixx` and `*.cpp` matches for `childNodes`, `Children`, or `FactoryChildren` in the two native trees. |
| Native input/tex files with those references | 0 files | Current parser modules do not directly manipulate the child handle. |
| `tests/math/*.cpp` files with child handle/type references | 19 files | 498 matching source lines; these include generated oracle fixtures and scenario setup. |
| Pinned TeX `.ts` lines mentioning `childNodes` | 145 | Direct field reads/writes and array operations across the scanned TeX source. |
| Pinned TeX `.ts` lines calling `NodeUtil.getChildAt`, `setChild`, `copyChildren`, or `getChildren` | 88 | Calls include `getChildren` used as a live-array return; the count includes nested calls and other named helper calls in the same line. |

These are textual scan counts, not runtime reachability or test-case counts.

## Native representation and exact-type boundaries

`src/math/core/tree/factory_types.ixx:35` defines
`FactoryChildren<Product>` as exactly `std::shared_ptr<std::vector<Product*>>`.
`src/math/core/tree/node_types.ixx:13,23` stores that handle as `childNodes`
and declares `setChildren` with the same alias. `src/math/core/mml_tree/mml_node.ixx:14`
aliases `Children` to `tree::FactoryChildren<MmlNode>`; it is not a second
container type.

Construction passes the exact alias through `std::any`:

| Site | Operation |
|---|---|
| `src/math/core/tree/node_factory.ixx:17–33` | Missing constructor arg 1 is normalized to `std::any(FactoryChildren<Product>(make_shared<vector<Product*>>()))`; the overload taking the alias places it in the second `std::any`. |
| `src/math/core/tree/node.ixx:33–41` | Initializes a fresh vector, then `std::any_cast<FactoryChildren<Product>>(&args[1])`; wrong present types throw, while a present exact alias containing null is rejected before iteration. |
| `src/math/core/mml_tree/mml_node.cpp:133–147` | Initializes exact `Children`, accepts `std::any_cast<Children>(&args[1])`, distinguishes a `nullptr_t` carrier, and rejects other present shapes before inferred-row construction/`setChildren`. |
| `tests/math/reference_factory.cpp:99,102` | Oracle assertions `any_cast` the exact `FactoryChildren<Probe>` type and compare handle identity, including the handle passed through constructor args. |

`std::any_cast` is exact-type, not a span/vector-convertible API. Current
initialization contracts also distinguish the null shared handle from a
non-null empty vector and from an explicit `std::nullptr_t` argument. The
native wrapper shape `src/math/core/tree/wrapper_types.ixx:13` is
`optional<FactoryChildren<Product>>`, so it additionally distinguishes an
absent wrapper child-list property from a present null handle.

## Native operation inventory

The current native representation is mutable and aliasing: copying a
`FactoryChildren` value copies only the shared pointer; replacing the
`childNodes` member changes which array later reads see, while already-captured
handles still refer to the old vector. Existing code relies on both behaviors.

| Operation | Native sites and observed behavior |
|---|---|
| Construct empty / assign handle | `node_types.ixx:13`; `node.ixx:33–41,70–75`; `mml_node.cpp:133–147,242–249`; `mfenced.cpp:160–162`. Empty arrays are non-null vectors. `setChildren` in generic Node replaces the receiver's vector, then appends children individually; MML may route through inferred rows before generic handling. |
| Indexed read / bounds handling | `node.ixx:73–74,127–129,148–150,162–167`; `mml_node.cpp:42–45,191–196,224–225,245–283,303–306,443–444,522–523`; MML node methods and `*_host.ixx` helpers use `size()` checks followed by `operator[]`. Most native helper indices are `size_t`; source-style absent reads are represented as nullptr or an exception according to the caller, with no separate hole marker. |
| Indexed write | `node.ixx:89`; `multiscript_host.ixx:22–27`; MML methods such as `mml_node.cpp:266` and `maction_selection.cpp:37–39`. Writes use existing vector slots or insertion; ordinary indexed assignment does not grow the vector or create holes. |
| Append | `node.ixx:79–80`; `mml_node.cpp:143,198`; `mfenced.cpp:160`; `mml_node.cpp:416` via a push loop. `push_back` is used; no `emplace_back` match was found. |
| Insert / erase | `multiscript_host.ixx:22–27` inserts with `vector::insert(begin()+min(index,size), value)`. `node.ixx:101` removes with `vector::erase`. No native child-array `resize`, `emplace`, `.data()`, or span construction was found. |
| Iterators and ranges | `node.ixx:110–112` uses `std::find(begin,end)`; `mml_node.cpp:412–414` constructs a replacement vector from an iterator range; `mml_branches.cpp:165–166` and `mfenced.cpp:123–124` copy ranges with `assign`; `wrapper.ixx`, Node/MML traversals, and most node methods use indexed loops with live `size()` reads. |
| Alias capture / replacement | `wrapper.ixx:38`, `mml_node.cpp:190,222,262`, and specialized nodes capture shared handles before iteration/callbacks. `node.ixx:71` replaces the vector in `setChildren`; `mml_node.cpp:412` replaces it during arity trimming; tests deliberately mutate captured arrays and separately rebind the owner's member. |
| Null slots | `node.ixx` permits push/replace of a null pointer only until code dereferences it; MML methods commonly interpret null as absent or throw. `mml_node.cpp:198` explicitly appends nullptr while copying a missing child. Existing dense vectors can store nullptr but do not say whether it came from an explicit null or a sparse hole. |

All 31 native files that matched are listed below with the number of matching
lines and their first/last match line; the preceding table groups their
operations by behavior.

| File | Match lines | Line span |
|---|---:|---:|
| `src/math/core/tree/factory_types.ixx` | 1 | 35–35 |
| `src/math/core/tree/node_factory.ixx` | 2 | 24–30 |
| `src/math/core/tree/node.ixx` | 17 | 33–192 |
| `src/math/core/tree/node_types.ixx` | 2 | 13–23 |
| `src/math/core/tree/wrapper.ixx` | 3 | 36–41 |
| `src/math/core/tree/wrapper_types.ixx` | 1 | 13–13 |
| `src/math/core/mml_tree/arithmetic_host.ixx` | 1 | 12–12 |
| `src/math/core/mml_tree/mml_branches.cpp` | 14 | 14–163 |
| `src/math/core/mml_tree/mml_node.cpp` | 30 | 42–520 |
| `src/math/core/mml_tree/mml_node.ixx` | 2 | 14–128 |
| `src/math/core/mml_tree/mo_host.ixx` | 1 | 11–11 |
| `src/math/core/mml_tree/multiscript_host.ixx` | 4 | 10–22 |
| `src/math/core/mml_tree/operator_class.cpp` | 5 | 53–63 |
| `src/math/core/mml_tree/table_host.ixx` | 4 | 11–20 |
| `src/math/core/mml_tree/mml_nodes/maction_behavior.cpp` | 6 | 29–90 |
| `src/math/core/mml_tree/mml_nodes/maction_selection.cpp` | 4 | 18–33 |
| `src/math/core/mml_tree/mml_nodes/mathchoice.cpp` | 1 | 34–34 |
| `src/math/core/mml_tree/mml_nodes/menclose.cpp` | 2 | 18–20 |
| `src/math/core/mml_tree/mml_nodes/mfenced.cpp` | 12 | 14–162 |
| `src/math/core/mml_tree/mml_nodes/mfenced.ixx` | 1 | 13–13 |
| `src/math/core/mml_tree/mml_nodes/mfrac.cpp` | 2 | 19–20 |
| `src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp` | 11 | 19–103 |
| `src/math/core/mml_tree/mml_nodes/mo_classes.cpp` | 2 | 41–42 |
| `src/math/core/mml_tree/mml_nodes/mo_scripts.cpp` | 1 | 79–79 |
| `src/math/core/mml_tree/mml_nodes/mroot.cpp` | 1 | 19–19 |
| `src/math/core/mml_tree/mml_nodes/mrow.cpp` | 19 | 19–142 |
| `src/math/core/mml_tree/mml_nodes/mstyle.cpp` | 3 | 16–71 |
| `src/math/core/mml_tree/mml_nodes/msubsup.cpp` | 3 | 23–26 |
| `src/math/core/mml_tree/mml_nodes/mtable.cpp` | 11 | 41–147 |
| `src/math/core/mml_tree/mml_nodes/mtr.cpp` | 5 | 16–99 |
| `src/math/core/mml_tree/mml_nodes/munderover.cpp` | 4 | 23–117 |

## Pinned TeX source array operations and indices

`NodeUtil.ts` is pinned at SHA-256
`bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2`.
`getChildren` returns the exact mutable child array at lines 67–69;
`getChildAt` is a direct `childNodes[position]` read at 191–193;
`setChild` captures the array and performs `children[position] = child` at
202–208, then assigns `child.parent` only for a truthy child; `copyChildren`
captures the source array and iterates its live length at 216–221. Those are
native JavaScript Array operations, not an append/replace abstraction.

Within the 246-file profile closure, the 88 helper-call lines use fixed child
indices `0`, `1`, `2`, MML-defined `sub`/`sup`/`under`/`over` slots, or integer
loop/state positions such as `position`, `pos`, and `i`. Examples include
`BaseItems.ts:195–210,252,267`, `BaseMethods.ts:220–238,295–313,352–355,824–826`,
`AmsMethods.ts:434–470,574–586`, `MathtoolsConfiguration.ts:76–85`, and
`ParseUtil.ts:641`. The inspected callsites contain no explicit negative,
fractional, or NaN index argument. The type is JavaScript `number`, however,
and NodeUtil itself does not validate or normalize an index before bracket
access/assignment; this report distinguishes the typed source surface from
the observed registered call arguments.

For reproducibility, every direct `getChildAt`/`setChild` callsite is grouped
here (the nested call in `MathtoolsMethods.ts:402` accounts for two
`getChildAt` invocations):

| Helper | Complete caller line set in the scanned profile |
|---|---|
| `getChildAt` | `base/BaseItems.ts:195,198`; `base/BaseMethods.ts:220,223,238,295,298,313,352,355`; `ams/AmsMethods.ts:58,434,446,448,458,459,470`; `mathtools/MathtoolsMethods.ts:402` (nested twice). |
| `setChild` | `amscd/AmsCdMethods.ts:138,145`; `base/BaseItems.ts:210,252,267,849`; `base/BaseMethods.ts:824–826,1398–1400`; `ams/AmsMethods.ts:574,586`; `mathtools/MathtoolsConfiguration.ts:80`; `mathtools/MathtoolsItems.ts:67`; `ParseUtil.ts:641`. |

Concrete source-visible slot states are:

* Out-of-range reads return `undefined`; `getChildAt` does not bounds-check.
  Callers often test truthiness or use `||`, while some index directly into
  nested child arrays.
* Explicit null is deliberately written by enabled base macros, for example
  `BaseMethods.ts:824–826` and `1398–1400` (`setChild` at slots 1), and explicit
  nulls are appended by `MathtoolsMethods.ts:919` and `MathtoolsItems.ts:65`.
  `setChild` still grows the array and assigns no parent for that falsey value.
* A sparse hole is concretely reachable: `BaseMethods.ts:248–249` creates an
  `msubsup` with only `[base]`, whose source `sup` index is 2
  (`core/MmlTree/MmlNodes/msubsup.ts:64–74`); the `SubsupItem` later writes
  that slot with `NodeUtil.setChild(top, position, ...)` at
  `base/BaseItems.ts:267`. For a superscript-first input, ordinary JS indexed
  assignment therefore leaves slot 1 as a hole. Reading it returns
  `undefined`; that is distinguishable from the explicit null written at
  `BaseMethods.ts:825,1399` or nulls appended in the cited mathtools paths.
  The source `copyChildren` loop reads holes as undefined and then assigns that
  value to each target numeric slot, so the target has a present undefined
  element rather than the original hole.
* No fractional or negative `NodeUtil.getChildAt`/`setChild` callsite was found
  in the closure. Some callers perform arithmetic on `indexOf`, such as
  `BussproofsUtil.ts:165,177`, and direct negative-property reads can occur
  when the searched child is absent; these are direct JS array expressions,
  not NodeUtil indices (the negative key is an ordinary non-index property,
  and yields undefined absent a matching named property). `childIndex` in the
  native core returns `optional`, while source `indexOf` returns `-1`, so that
  distinct source use is visible. The native tests currently exercise null
  slots, not sparse holes or negative/fractional JS array keys.

Direct child-array APIs add mutation forms beyond the NodeUtil helpers:

| Pinned caller | Source lines | Observed array operation |
|---|---:|---|
| `ams/AmsMethods.ts` | 62 | `splice(0,1)` removal. |
| `base/BaseConfiguration.ts` | 127 | `splice(childIndex(mml),1)` removal; source `childIndex` may be `-1`. |
| `bussproofs/BussproofsUtil.ts` | 165,177,382 | `indexOf`-relative neighbor reads; `unshift(mspace)`. |
| `empheq/EmpheqUtil.ts` | 112,177,179,182–183 | `slice`, reverse of a copy, `unshift`, and indexed slot swaps. |
| `mathtools/MathtoolsConfiguration.ts` | 76–85 | Retains `getChildren` handle, tests indexed values, writes missing script positions, then `splice(1,2)`. |
| `mathtools/MathtoolsItems.ts` | 65 | `unshift(null)` explicit-null slot. |
| `mathtools/MathtoolsMethods.ts` | 919 | `NodeUtil.getChildren(mml).push(null,null)`. |
| `base/BaseItems.ts` | 418–419, 1334–1341, 1550–1558 | Indexed reads; passes an existing child array to node creation and replaces the source member with a fresh empty array. |

In the profile scan, additional direct operations include spread/concat and
iteration over `getChildren` in `AmsItems.ts:101–234`, `FilterUtil.ts:99–233`,
`ParseUtil.ts:347–403`, `MathtoolsMethods.ts:218`, and `EmpheqUtil.ts:56–91`.
All preserve or consume the ordinary JavaScript array behavior of the
`childNodes` handle.

## Test-side use

The 19 matching `tests/math` files use the existing vector API directly.
`reference_tree.cpp` and `reference_mml_node.cpp` exercise handle capture,
push-back during traversal, rebinding the owner's child handle, vector
replacement, null child pointers, and null handle failure. Node/MML oracle
fixtures build children with `make_shared<vector<MmlNode*>>(initializer_list)`
and snapshot them by iterating the dense vector. `reference_factory.cpp`
checks exact `std::any` child-handle type and pointer identity. Specialized
fixtures (`reference_maction.cpp`, `reference_multiscript.cpp`,
`reference_table.cpp`, `reference_mfenced.cpp`, and others) exercise live
mutation/rebinding and null slots through methods. The current native test
representation has no hole bitmap or JavaScript property-key index, so these
tests cannot encode a sparse hole separately from nullptr.

The full matching test-file set is:

```text
reference_arithmetic_order.cpp  reference_arithmetic.cpp
reference_atom_semantics.cpp    reference_factory.cpp
reference_html_node.cpp         reference_leaf.cpp
reference_maction.cpp           reference_mathchoice.cpp
reference_mfenced.cpp            reference_mml_node.cpp
reference_mml_registry.cpp      reference_mo_order.cpp
reference_mo_values.cpp         reference_mo.cpp
reference_multiscript.cpp      reference_script.cpp
reference_simple_mml.cpp        reference_table.cpp
reference_tree.cpp
```

## Key source hashes

| File | SHA-256 |
|---|---|
| `src/math/core/tree/factory_types.ixx` | `25542be82bc00641c5f5880362aaae8f8c022b635df0914011bff33f10ba4033` |
| `src/math/core/tree/node_types.ixx` | `5f278b03d99bc40026b46fb38698d1ab4ca0dfc9fc65fa143ad7a1e9879e165a` |
| `src/math/core/tree/node.ixx` | `0be5adbf63239fb4bcbfd459f9267d59c10a3f713aae2c1e202cf64888ec7fd1` |
| `src/math/core/tree/node_factory.ixx` | `c39d713686b348b1f73d1aab5f2230c9dd218de463b3d67218d8f322c4539e92` |
| `src/math/core/tree/wrapper.ixx` | `f49bb0710da6c1ddd288a05156f359c5bb62672f7f4a3ff889968535fb5d291d` |
| `src/math/core/mml_tree/mml_node.ixx` | `36f47c27f389ac337c8f08dd890d066ba362f8ed9e51ea9ad53cad999ceeba3e` |
| `src/math/core/mml_tree/mml_node.cpp` | `5af784d8fbf6057f9e9a14a9ea3b8afa83349a2b34b6668edd0d80b69b91d43f` |
| `src/math/core/mml_tree/multiscript_host.ixx` | `481e775cce3c176d62cb17edb90954013adff6f618cbea98a6b16a2b884e828e` |
| `src/math/core/mml_tree/mml_nodes/mrow.cpp` | `4cade65c6a7a3f8eea955f530906e667589a7212e0669db146e90f41393beb70` |
| `src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp` | `14cbc09660c2a8372bb766f3a3cc5fab2ea9695938a6da1ce57c7794143beac9` |
| `tests/math/reference_tree.cpp` | `7ee97781f2348a071cf836edc0c6969f47290adc9d660203bce7cb0191d4bd85` |
| `tests/math/reference_mml_node.cpp` | `a34004baa7ead53796eee175c2e4015aa025af70562487e5273d1c1dac8eac25` |
| `.reference/MathJax-src/ts/input/tex/NodeUtil.ts` | `bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/msubsup.ts` | `37fd01fa0c44ceab5b4ed30ba3d95ab43ffb8aed55db27c7290d6d786131f6e7` |
| `.reference/MathJax-src/ts/input/tex/base/BaseItems.ts` | `0c0d8448f3577be6642c95a412682310362819d2d383ab83b817d2888e4e5f0c` |
| `.reference/MathJax-src/ts/input/tex/base/BaseMethods.ts` | `7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda` |
| `.reference/MathJax-src/ts/input/tex/mathtools/MathtoolsMethods.ts` | `ad6562e23ba65cafdb480afc33387982c7f7b14a2886ae7844dfbe64ec44e918` |
| `.reference/MathJax-src/ts/input/tex/ams/AmsMethods.ts` | `e07f24fe8df2dd12cfdaf850efee73c3d36824783001b22e1624f3db877c2ec1` |
| `docs/math-port/scope.json` | `53e1bef466ae717954c692123b133d889921aaf4b89ceaff21e37730a0c22ee3` |
| `tools/mathjax-bundle/entry.cjs` | `a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9` |
