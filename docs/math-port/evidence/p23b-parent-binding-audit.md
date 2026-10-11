# P23b — parent binding and child-array mutation order audit

Read-only review against pinned MathJax 4.1.3 (`fb987178c2d279a99b0db5ea02e751691703955e`), the current P23 sparse-child audit, the existing Source 111 Node/NodeUtil observations, and current `src/math`. This report records source-visible behavior and the native boundary; it does not approve a migration or choose a new API. No build, test, or experiment was run.

## `parent` is a data slot, not a Source accessor

In pinned `core/Tree/Node.ts` (SHA-256 `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0`), `AbstractNode.parent` is declared as a public field initialized to `null` at line 166. The methods assign it directly: `appendChild` at 263–266; `replaceChild` at 272–282; `removeChild` at 288–295. `core/MmlTree/MmlNode.ts` (SHA-256 `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`) redeclares `parent: MmlNode` as a field at 420–423; the MML constructor writes the inferred row's parent directly at 449–454. `AbstractMmlEmptyNode` likewise declares the parent field at 1233–1245. A pinned-tree search found no `get parent()` or `set parent()` declaration, nor `get/set childNodes()`; the lower-case `parent` and `childNodes` names are ordinary mutable own slots in this profile.

There is a distinct capitalized `Parent` getter. `AbstractMmlNode.Parent` reads `this.parent`, then repeatedly calls `parent.notParent` and `parent.Parent` (MmlNode.ts:575–580). The empty-node implementation returns its direct parent (1313–1315). `notParent` has real subclass getter overrides on mathchoice, mrow, mstyle, TeXAtom, and semantics; they define logical ancestry, not a parent setter. These getters do not themselves mutate `childNodes`. The source overrides are: mathchoice `notParent` (mathchoice.ts:64), mrow `isInferred`/`notParent` (mrow.ts:207,214), mstyle `notParent` (mstyle.ts:59), TeXAtom `notParent` (TeXAtom.ts:68), and semantics `notParent` (semantics.ts:63). They are virtual reads and their exact dispatch/recursion matter, but should not be mistaken for a property accessor callback on `parent`.

Therefore a plain parent write in the pinned production classes does not invoke user code or change an array. The fixed Source oracle nevertheless installs an own accessor on selected observer nodes to expose the exact read/write point and test whether that callback can mutate a captured child array. `tools/math-port/sparse-child-oracle.cjs` (SHA-256 `fe8bf79a2d4040f52c65432e0ab795348708f4f94aa04a4fda5d8ae56ab8f176`) has the `setChild-captured-target-before-parent-setter` case at line 41: its `Object.defineProperty(nodes.b,'parent',{get,set})` setter rebinds `nodes.old.childNodes` after `NodeUtil.setChild` has captured and written the old array. `tools/math-port/sparse-mml-oracle.cjs` (SHA-256 `5abbafb661a571520081bf91c19aad208e49484c0f3316a8910037445d64a387`) similarly has ten `*-parent-setter-*` observer cases (lines 55–58), covering append/delete/shrink/source-rebind/target-rebind during the observed parent write. These are real fixed-oracle observations of Source operation order, not evidence that the pinned MathJax production classes declare `parent` as an accessor. They are also not a claim that arbitrary JavaScript descriptors or proxies are part of the typed native contract. Do not attribute callback reentrancy to every production `child.parent = ...` operation; preserve the source write point so the bounded observer binding can observe it.

## Direct parent references in the native 31-file child-consumer set

The set below is the 31-file `childNodes`/`FactoryChildren` inventory in `p23-sparse-identity-closure-audit.md`. “Direct” means a source-level read/write of the `parent` field or logical `Parent()` call in the native file, not a parent change performed by a called child-management method.

| Native child consumer | Direct parent access in current C++ | Source-level implication |
|---|---|---|
| `core/tree/factory_types.ixx` | none | Parent identity is held by node instances, not the factory child-handle type. |
| `core/tree/node_factory.ixx` | none | Constructor argument normalization does not access parent. |
| `core/tree/node.ixx` | writes in append/replace/remove | Direct assignments occur after/between child-array mutations; see operation order below. |
| `core/tree/node_types.ixx` | `Product* parent = nullptr` field | Typed nullable identity slot; no getter/setter. |
| `core/tree/wrapper.ixx` | none | Wrapper traversal reads child handles and dispatches child walking. |
| `core/tree/wrapper_types.ixx` | none | No parent binding. |
| `core/mml_tree/arithmetic_host.ixx` | none | Child access helper only. |
| `core/mml_tree/mml_branches.cpp` | none | Branches dispatch through child/node operations. |
| `core/mml_tree/mml_node.cpp` | direct reads/writes and virtual calls | Parent/Parent traversal, child-position scan, and `mError` replacement. |
| `core/mml_tree/mml_node.ixx` | declares virtual `Parent()` methods, no lowercase parent slot | Logical parent access is a distinct virtual method. |
| `core/mml_tree/mo_host.ixx` | none | No direct parent binding. |
| `core/mml_tree/multiscript_host.ixx` | none | Typed node view only. |
| `core/mml_tree/operator_class.cpp` | direct pointer traversal | Reads parent chain and child/core relationship. |
| `core/mml_tree/table_host.ixx` | none | Helper does not own parent operations. |
| `core/mml_tree/mml_nodes/maction_behavior.cpp` | none | Parent mutations occur only if delegated generic node operation is called. |
| `core/mml_tree/mml_nodes/maction_selection.cpp` | none | No direct parent field. |
| `core/mml_tree/mml_nodes/mathchoice.cpp` | parent read, then virtual `replaceChild` | Replacement can mutate parent child list. |
| `core/mml_tree/mml_nodes/menclose.cpp` | none | No direct parent field. |
| `core/mml_tree/mml_nodes/mfenced.cpp` | direct parent write for synthesized fence node | Data-slot assignment; no callback in pinned Source. |
| `core/mml_tree/mml_nodes/mfenced.ixx` | none | Declaration only. |
| `core/mml_tree/mml_nodes/mfrac.cpp` | none | No direct parent field. |
| `core/mml_tree/mml_nodes/mmultiscripts.cpp` | direct parent write after splice | Slot write follows a child-array insertion. |
| `core/mml_tree/mml_nodes/mo_classes.cpp` | parent pointer read | Reads core node's parent identity. |
| `core/mml_tree/mml_nodes/mo_scripts.cpp` | virtual `Parent()` calls | Uses logical-parent getter; no direct lower-case field write. |
| `core/mml_tree/mml_nodes/mroot.cpp` | none | No direct parent field. |
| `core/mml_tree/mml_nodes/mrow.cpp` | none | Parent behavior is through generic methods/getters. |
| `core/mml_tree/mml_nodes/mstyle.cpp` | none | No direct parent field. |
| `core/mml_tree/mml_nodes/msubsup.cpp` | none | No direct parent field. |
| `core/mml_tree/mml_nodes/mtable.cpp` | reads `child.parent`, calls its virtual `removeChild`, then writes parent | Critical reparent/mutation sequence below. |
| `core/mml_tree/mml_nodes/mtr.cpp` | reads `this->parent` when arity is one | Reads parent's `attributes.side`; nullish parent throws on member read. |
| `core/mml_tree/mml_nodes/munderover.cpp` | none | No direct parent field. |



The direct native matches, checked with the current source, are: `tree/node.ixx:82,91–93,103`; `tree/node_types.ixx:12`; `mml_tree/mml_node.cpp:145,235–239,296–306,530–553`; `operator_class.cpp:50–58`; `mml_nodes/mathchoice.cpp:47`; `mml_nodes/mfenced.cpp:166`; `mml_nodes/mmultiscripts.cpp:39`; `mml_nodes/mo_classes.cpp:23,58`; `mml_nodes/mo_scripts.cpp:36,75`; `mml_nodes/mtable.cpp:118–120`; and `mml_nodes/mtr.cpp:32`. These native matches are not all setter calls: most are direct pointer reads, and the source field assignments themselves are non-callback writes.

## Exact source mutation/capture order

The generic `Node.ts` implementation is the reference for `AbstractNode`; P23's native shared handle is currently `shared_ptr<vector<Product*>>` (`node_types.ixx:8–14`, SHA-256 `5f278b03d99bc40026b46fb38698d1ab4ca0dfc9fc65fa143ad7a1e9879e165a`). Its ordinary field operations and dynamically dispatched mutators have separate effects:

| Source operation | Source order and array effect | Parent effect/callback status |
|---|---|---|
| `setChildren(children)` (Node.ts:253–258) | Rebinds `this.childNodes=[]` first; then consumes `children` with array `for..of`, invoking `this.appendChild(child)` for each item. The input iterator captures its source array, while its length is live. | `appendChild` is a virtual method call per item and can rebind/grow the receiver child list. Parent write is downstream of that call. |
| `appendChild(child)` (263–267) | Reads current `this.childNodes`, pushes the child, then returns it. | Assigns `child.parent=this` as a plain own-data write after push; this assignment has no pinned accessor callback. |
| `replaceChild(new, old)` (272–283) | Calls virtual `this.childIndex(old)` first. If found, it then reads current `this.childNodes` and writes slot `i`. | Writes `new.parent=this`; next reads `old.parent`; if identical to `this`, writes `old.parent=null`. Parent assignments are direct; the intervening `childIndex` call can mutate/rebind the array before the slot is selected. |
| `removeChild(child)` (288–295) | Calls virtual `this.childIndex(child)`; if found, reads current `this.childNodes` and `splice`s that array. | Clears `child.parent` after splice. `childIndex` can mutate/rebind before the current array is spliced. |
| `copy()` (308–317) | Calls `factory.create`, copies properties, then iterates `this.childNodes` via `for..of`; each truthy child receives virtual `copy()`, and the destination receives virtual `appendChild()`. | Construction/copy/append callbacks can change child handles. The iteration is over the array captured when evaluating the loop expression; in-place array edits and its live length remain observable, while rebinding the field does not redirect the existing iterator. |

MML overrides add a concrete redirection path. `MmlNode.ts` (SHA-256 `cc0594…b8656`): its constructor creates an inferred row, writes `this.childNodes=[row]`, sets `row.parent=this`, then calls virtual `this.setChildren(children)` (lines 443–459). `setChildren` forwards to `this.childNodes[0].setChildren(children)` when `arity<0`, otherwise qualified generic `super.setChildren` (595–600). `appendChild` similarly forwards to inferred child 0; for an inferred child it either `forEach`s its children into the generic append path (the `forEach` captures the array and initial length) or creates a replacement `mrow`, calls its virtual `setChildren(original.childNodes)`, copies attributes/properties, and finally appends (607–633). `replaceChild` on negative arity forwards to child 0; otherwise it calls the generic implementation (639–645). These are actual methods which may change child-list identity or length; the adjacent `parent` assignments are not the callback.

MML `copy` (MmlNode.ts:467–491) calls factory creation first, captures `this.childNodes` into local `children`, optionally switches that local to the inferred row's child handle, then iterates it. Each non-null child's virtual `copy()` is followed by virtual `copied.appendChild`; a null slot pushes directly to the destination's current array. This preserves distinct source/destination handles and means the destination's append override can reshape its list while the source iteration continues on the captured source list.

`NodeUtil.ts` (SHA-256 `bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2`) is different from node virtual methods. `setChild` at lines 191–208 captures `const children=node.childNodes`, writes `children[position]=child`, then if `child` is truthy assigns `child.parent=node`. There is no node `setChild` virtual dispatch and the parent assignment is a plain field write. `copyChildren` at 211–221 captures `oldNode.childNodes` once and runs an index loop against its live `.length`; each iteration calls `this.setChild(newNode,i,children[i])`. The destination array is re-read by `setChild` on each iteration. A parent accessor callback is not part of this fixed path.

Two direct parent-call sites deserve special attention:

- `MmlNode.mError` (MmlNode.ts:987–1011) reads the current parent, calls virtual `parent.isKind('merror')`, then creates nodes. It later reads `this.parent` and dispatches virtual `replaceChild(error,this)`; in one branch it subsequently calls `error.appendChild(this)`. `isKind` is a read/dispatch point, but the actual child-list mutation comes from the `replaceChild`/`appendChild` methods. Native mirror: `mml_node.cpp:529–555`.
- `Mtable.verifyChildren` (mtable.ts SHA-256 `8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda`, lines 167–198) loops by index over `this.childNodes.length`; it calls child `isKind`, and then parent `removeChild` or `replaceChild`. It appends the selected child/wrapper to `mtr`; when `fixMtables` is false, it reads the child's then-current `parent`, invokes that parent's virtual `removeChild`, then writes `child.parent=this` directly. The initial table loop uses the table's current child array on each index read, with length re-read by the loop condition. `replaceChild`/`removeChild` can alter that array and `i--` compensates for removal. The parent pointer is read after the earlier append/reparent call, not cached before it. Native mirror: `mml_nodes/mtable.cpp:115–125`.

`Mtr.setChildInheritedAttributes` (mtr.ts SHA-256 `ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f`, lines 81–100) performs virtual `replaceChild(...).appendChild(...)` repairs first. Only after this loop does its `arity===1` branch read `this.parent.attributes.get('side')` (line 90). This is a fresh direct parent read after child replacement callbacks; it does not call a parent mutator. The current native `mtr.cpp:31–35` snapshots `this->parent` once in the branch and then reads the parent's attributes. A centralized parent binding must preserve this timing and nullish read failure.

Other real callsites include `mathchoice` selecting a branch then calling `parent.replaceChild` (mathchoice.ts:74–85); `FilterUtil.ts` (SHA-256 `dc821e2483fcd0820159a13d984b2bf299952e8c9f40418d72ff4c183d93053a`, lines 103–117, 204–250, 336–339) and `BaseConfiguration.ts` (SHA-256 `578d2c6b03f2f3e902476a0b73a5323b12262215fcbdc831a1135c1d9c1737ad`, lines 121–145) do analogous `parent.replaceChild` and direct `childNodes.splice` work; `mo` and MML code read direct/logical parents to compute ancestry/position. Those callsites reinforce method dispatch and array capture order, not an accessor on the lowercase field.

## Child consumers and possible mutation sources

Across the 31 native consumers, parent-boundary effects fall into three classes:

1. Direct pointer reads/writes only: current parent identity is copied/read or assigned; no callback is implied. Examples are `operator_class.cpp`, `mo_classes.cpp`, `mfenced.cpp`, and the synthesized `none.parent` write in `mmultiscripts.cpp`.
2. Virtual method calls through a parent reference: `mError`, mathchoice, and Mtable call `replaceChild`/`removeChild`; generic `Node` methods call `childIndex`/`appendChild`; MML overrides may route operations into inferred row child 0 or flatten/replace an inferred row. These calls can mutate child-list contents/length or rebind a relevant node's handle.
3. Captured-versus-current child array behavior: a local shared handle/array iterator remains tied to its captured object after field rebind; a fresh `this.childNodes` read sees the replacement object. Methods mixing callbacks and later reads must preserve which side is captured. The Source 111 observations and P23 sparse audit cover holes, live length, and identity effects. Production parent data-field writes add no callback, while the fixed observer accessors deliberately add one at the exact parent read/write boundary.

Getters with relevant virtual dispatch are `Parent`, `notParent`, `isInferred`, `isToken`, `kind`, and `isKind`. In the pinned core implementations these compute/read state; they do not write child arrays. Overrides can affect *which* nodes are traversed or whether a branch is taken. The production field is ordinary data, while the fixed Source oracles deliberately add an own `parent` accessor to selected observer instances to distinguish evaluation/capture order. That controlled instrumentation is an accepted source observation; it does not establish that arbitrary external descriptors, proxies, or descriptor mutation are generally supported.

## Minimum binding boundary evidenced here

The source evidence requires the native semantics to keep these operations distinct:

- A nullable, identity-preserving parent read/write. Production `parent` is a data field, but the fixed observer fixtures require one bounded, overridable native access seam at the read/write site so they can observe receiver/order effects; that seam does not imply support for arbitrary JavaScript descriptors.
- Dynamic dispatch for the actual node methods invoked through that field, especially `childIndex`, `appendChild`, `setChildren`, `replaceChild`, and `removeChild`, with the exact call site/order retained.
- Child-array observation that distinguishes captured handle/iterator from a fresh field read, plus live index/length, hole/present-null/present-undefined and direct rebind/splice operations. The existing `shared_ptr<vector<Product*>>` retains C++ node pointer identity but cannot represent sparse slots; P23's separate closure describes that gap.

This is a minimum description of required binding behavior, not a chosen class/API. A proposed single `ChildValue<Product>` parent backing can represent node identity and the explicit `null` parent value without a second raw-pointer track, provided node construction initializes it to `Null` (the current `ChildValue{}` default is `Missing`, which is not the source default). If root freezes virtual `getParent()`/`setParent(ChildValue<Product>)` as the one access boundary, the base implementation can read/write that sole storage and bounded observer subclasses can override those methods to model the oracle's accessor callback; production code must not silently route some accesses through a raw field and others through the virtual binding. This is a possible fit for the evidence, not a decision about the final API. A null or undefined *receiver* on a JavaScript assignment such as `child.parent = value` throws `TypeError` (“Cannot set properties of null/undefined (setting 'parent')”); assigning `null` as the value to a valid child's parent field is legal and is required by `removeChild`. Thus a read-side `requireNode()` error helper must not be reused to reject a valid `setParent(Null)`; the source-order-sensitive caller must first validate/dereference the receiver. A controlled observer override is not a general JS descriptor/proxy engine. Root must still decide the typed write-receiver check and how the same identity/lifetime and sparse child-array graph are represented across generic Tree and MML.

## Evidence locations

| Source/native file | Relevant lines | SHA-256 |
|---|---:|---|
| `.reference/MathJax-src/ts/core/Tree/Node.ts` | 41–48, 159–197, 253–317, 366–404 | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts` | 410–459, 467–491, 575–580, 595–645, 987–1011, 1233–1245, 1299–1315 | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `.reference/MathJax-src/ts/input/tex/NodeUtil.ts` | 67–69, 191–221 | `bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mtable.ts` | 106–198 | `8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mmultiscripts.ts` | 65–95, 112–160 | `bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mathchoice.ts` | 64, 74–85 | `e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mrow.ts` | 207, 214 | `af58f0c1aa39a71d6c3dc5addf358a8cefeecb3f76cb483ec940f3a7ea72b4b9` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mstyle.ts` | 59 | `7442e2885d2f4605d42f5b4472b3183f9cbadbd8ed5c0aeddf03b8f23c1d8616` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/TeXAtom.ts` | 68 | `bad114224392b94852456a4636b85ad90a5a2c96b63e6c7911d155db299f7028` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/semantics.ts` | 63 | `e609dfc3d7510a996a87804e3929cd4f53a5aa060003e351f45f1fc5dcfb6e96` |
| `.reference/MathJax-src/ts/input/tex/FilterUtil.ts` | 103–117, 204–250, 336–339 | `dc821e2483fcd0820159a13d984b2bf299952e8c9f40418d72ff4c183d93053a` |
| `.reference/MathJax-src/ts/input/tex/base/BaseConfiguration.ts` | 121–145 | `578d2c6b03f2f3e902476a0b73a5323b12262215fcbdc831a1135c1d9c1737ad` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mfenced.ts` | 174–190 | `64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397` |
| `src/math/core/tree/node_types.ixx` | 8–23 | `5f278b03d99bc40026b46fb38698d1ab4ca0dfc9fc65fa143ad7a1e9879e165a` |
| `src/math/core/tree/node.ixx` | 70–113 | `0be5adbf63239fb4bcbfd459f9267d59c10a3f713aae2c1e202cf64888ec7fd1` |
| `src/math/core/mml_tree/mml_node.ixx` | 11–16, 62–64, 127–128, 204–205 | `36f47c27f389ac337c8f08dd890d066ba362f8ed9e51ea9ad53cad999ceeba3e` |
| `src/math/core/mml_tree/mml_node.cpp` | 122–160, 171–203, 235–287, 294–309, 529–555 | `5af784d8fbf6057f9e9a14a9ea3b8afa83349a2b34b6668edd0d80b69b91d43f` |
| `src/math/core/mml_tree/mml_nodes/mtable.cpp` | 100–150 | `f524cba94457c11b6e546502a6091dcc33e895691c8ddfeaf3921baa73355686` |
| `src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp` | 20–50 | `14cbc09660c2a8372bb766f3a3cc5fab2ea9695938a6da1ce57c7794143beac9` |
| `src/math/core/mml_tree/operator_class.cpp` | 45–64 | `90d327e59054be048bd12cd4971d2749ab5ae9714808d90dd0d7224e9c8152ef` |
| `src/math/core/mml_tree/mml_nodes/mathchoice.cpp` | 35–60 | `9647a930a0650726308d5f50ea384f8a76b20c1c229fb465dc82f8be84cd1765` |
| `src/math/core/mml_tree/mml_nodes/mfenced.cpp` | 150–175 | `4db6ae771a37685fcb17514577516ddfb990b825aa811bf820f18a7eba8dbca8` |
| `src/math/core/mml_tree/mml_nodes/mo_classes.cpp` | 15–64 | `7eba824c7e1a71baa5c6610f146b78c0d5bb2e2a21fa2b2c3f0300c16fd1e9b3` |
| `src/math/core/mml_tree/mml_nodes/mo_scripts.cpp` | 25–80 | `c57c6788077b900bd59e8923698256d319bdf74a3644d92524f9019212d84fa1` |
| `docs/math-port/evidence/p21a-child-array-usage-audit.md` | complete 31-file and Source-callsite inventories | P21a prerequisite evidence |
| `docs/math-port/evidence/p23-sparse-identity-closure-audit.md` | complete sparse-slot/identity analysis | P23 prerequisite evidence |
| `tools/math-port/sparse-child-oracle.cjs` | 41 | `fe8bf79a2d4040f52c65432e0ab795348708f4f94aa04a4fda5d8ae56ab8f176`; pins generic Node and NodeUtil CJS inputs at script lines 6–7 |
| `tools/math-port/sparse-mml-oracle.cjs` | 55–58 | `5abbafb661a571520081bf91c19aad208e49484c0f3316a8910037445d64a387`; pins Node/MML/factory/NodeUtil CJS inputs at script lines 6–17 |

No implementation, test, oracle, manifest, build, or commit was changed or run. This report is not a claim that the sparse Node migration or Source gate has passed.
