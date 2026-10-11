# P23 sparse-slot and node-identity closure audit

Read-only inventory for root's storage/API decision. This report does not choose or freeze a representation, authorize implementation, or widen the source behavior being ported. Baseline evidence read: `p21a-root-prerequisite-review.md` and `p21a-child-array-usage-audit.md`. Pinned source is MathJax `fb987178c2d279a99b0db5ea02e751691703955e`; the cited P21a child-array audit covers the fixed 246-file profile closure and lists all 31 current native source files referencing `childNodes`/`FactoryChildren`.

## Four source-visible slot states

The source values in `childNodes` are node objects, `null`, or absent numeric properties. They need distinct native states:

| State | Pinned JS Array behavior | Current `FactoryChildren<Product>` vector | Current `PropertyArray` graph |
|---|---|---|---|
| Hole (no own numeric property) | `hasOwn`/`in` can distinguish it; indexed Get ordinarily yields `undefined`, subject to prototype lookup. | Not representable: every index below `vector::size()` is a stored `Product*`. | Representable by `size()` covering the index and `has(i)==false`; `get(i)` returns `Missing` if no inherited numeric slot resolves. |
| Present `undefined` | Own property exists, Get yields `undefined`. `copyChildren` reads it and indexed assignment creates a present target property. | Not representable separately from a null pointer, and native dereference paths have no undefined marker. | Representable as `has(i)==true` and value `Missing{}`; `get` alone cannot distinguish from a hole, so presence and value must both be observed. |
| Present `null` | Own property exists, Get yields `null`; source truthiness is false. | A stored `nullptr` is representable, but has no indication whether source value was `null`, a hole, or undefined. | Representable as `has(i)==true` and `PropertyNull{}`; distinct from Missing. |
| Present node identity | Own property contains the exact mutable Node object; equality/parent links and callbacks observe object identity. | Representable as `Product*` identity in the slot. | Not currently representable: `Property::Value` has no `NativeObjectRef` alternative and the existing `PropertyStorage` pointer is a *property-container* identity, not a node. |

The concrete hole is source reachable. `BaseMethods.ts:248-249` creates `msubsup` with `[base]`; `msubsup.ts:64-74` places `sup` at slot 2; `BaseItems.ts:267` later writes that index with `NodeUtil.setChild`. Superscript-first parsing therefore leaves slot 1 absent. Explicit null is also reachable at `BaseMethods.ts:824-826, 1398-1400`, `MathtoolsItems.ts:65`, and `MathtoolsMethods.ts:919`. `NodeUtil.copyChildren` (`NodeUtil.ts:216-221`) reads every source index to the live length and assigns it to the target, converting a hole to a present `undefined` slot. P21a records these source observations and the actual-profile callsite inventory.

`PropertyArray` already models holes versus present Missing/null using `arrayPresent` and `arrayValues` (`src/math/core/tree/property.ixx:44-51, 232-260`). `set(i, value)` grows the indexed length and marks the slot present; `resize` creates absent slots; `erase` clears presence; `get` resolves through `PropertyList` so inherited numeric values may be observed (`property.ixx:79-110, 156-171, 237-260`; `property_array.cpp:5-22`). `spread()` densifies by reading and pushing each position (`property.ixx:257`); `slice()` checks `HasProperty` and leaves holes absent (`property_array.cpp:5-22`). Those facts establish slot-state capability, not compatibility with node-valued slots.

## Current native handle, call, return, and write inventory

The actual child handle is exactly `FactoryChildren<Product> = shared_ptr<vector<Product*>>` (`factory_types.ixx:35`). `Node<Product>::childNodes` is a public, non-null-by-default field (`node_types.ixx:8-14`); it has no getter/setter wrapper. `setChildren` is a separate virtual method (`node_types.ixx:23`). `MmlNode` uses the exact alias through `using Children = tree::FactoryChildren<MmlNode>` (`mml_node.ixx:11-15`). A handle copy aliases the vector. Direct assignment to the public field rebinds the owner; existing captures continue to alias the old vector. This distinction is observable at wrapper and node callback/re-read sites.

| Native entry or consumer | Current behavior and source correspondence |
|---|---|
| Factory argument path | `node_factory.ixx:18-37` normalizes missing argument 0 to `PropertyList{}` and missing argument 1 to an exact `std::any(FactoryChildren<Product>)` containing a fresh empty vector; the typed overload also passes an exact alias in `std::any`. Exact type is required downstream. |
| Generic node initialization | `node.ixx:22-42` reads properties, accepts only exact `FactoryChildren<Product>` in `args[1]`, rejects a present wrong shape and null shared handle, and calls `setChildren` only when non-empty. The source generic constructor accepts `N[]` (`core/Tree/Node.ts:186-196`). |
| Generic set/copy/append | `node.ixx:70-84`: `setChildren` first rebinds to a fresh empty vector then copies by indexed read/append; `appendChild` pushes the exact pointer and then sets its parent. This corresponds to source `AbstractNode.setChildren`/`appendChild` (`Node.ts`); source child slots can contain holes/null, which the dense loop cannot preserve. |
| Replace/remove/index | `node.ixx:86-113`: `replaceChild` finds pointer identity via `childIndex`, writes an existing slot, then sets parent links; `removeChild` erases and clears parent; `childIndex` uses `std::find` and returns optional index. No indexed set grows the vector. |
| Generic copy/traversal/string | `node.ixx:115-171`: deep copy skips null pointers; `walkTree` skips null; `toString` captures the shared handle and count, emits empty text for null, and uses `TreeJoinGuard` on the vector address. These consumers currently cannot visit or stringify a hole separately from null/present undefined. |
| Empty-node overrides | `node.ixx:182-205`: empty nodes ignore `setChildren`, do not append/replace, and return no child index; traversal is self-only. This is a real virtual dispatch consumer of the same handle API. |
| Wrapper child view | `wrapper_types.ixx:7-18` stores `optional<FactoryChildren<Product>>`; `nullopt` means absent child-list property, engaged null means present null handle. `wrapper.ixx:32-48` returns early for absent, captures the handle, iterates by live vector size, and skips null pointers. Source `Wrapper.ts` traverses the current child array. |
| MML initialization | `mml_node.cpp:122-161`: ignores generic initialization args, accepts exact MML `Children` in `args[1]` or explicit `nullptr_t`, creates inferred row storage when arity is negative, then dispatches `setChildren`. MML attributes are initialized after children. This is the current constructor boundary where the `std::any` child handle is accepted. |
| MML copy | `mml_node.cpp:171-203`: captures child handle, unwraps a single inferred row to its captured child handle, reads indices, deep-copies non-null children, and appends nullptr for absent/null observations. Vector copy currently loses hole/present-undefined distinction and retains no external object wrapper. |
| MML overrides and flatten | `mml_node.cpp:242-288`: negative-arity nodes forward `setChildren`, append, replace through inferred child slot; inferred rows flatten into append for infinite-arity receivers; finite receivers wrap inferred rows in a new `mrow`. Handles are captured and forwarded directly. |
| MML parent/arity/iteration | `mml_node.cpp:303-306, 406-419, 439-447, 475-485, 519-526`: parent position scans sibling pointer identity; arity repair truncates by rebinding a new vector or grows by `appendChild`; inherited-attribute and verification loops capture then index the current vector and reject null child pointers. |
| Other MML branch and typed helpers | `mml_branches.cpp:14-166`, `arithmetic_host.ixx:12`, `mo_host.ixx:11`, `multiscript_host.ixx:11-28`, `table_host.ixx:12-21`, `operator_class.cpp:53-63`: front/core child access, iterator/index scans, captured handles, and parent/child pointer comparisons. `moChild` returns nullptr for out-of-range, collapsing hole/null/undefined. |

The complete 31-file child-handle use set in `src/math/core/tree` and `src/math/core/mml_tree` (line spans are the actual first/last match lines; detailed per-operation callsites are in P21a's native inventory table):

| File | Match lines |
|---|---:|
| `src/math/core/tree/factory_types.ixx` | 35 |
| `src/math/core/tree/node_factory.ixx` | 24-30 |
| `src/math/core/tree/node.ixx` | 33-192 |
| `src/math/core/tree/node_types.ixx` | 13-23 |
| `src/math/core/tree/wrapper.ixx` | 36-41 |
| `src/math/core/tree/wrapper_types.ixx` | 13 |
| `src/math/core/mml_tree/arithmetic_host.ixx` | 12 |
| `src/math/core/mml_tree/mml_branches.cpp` | 14-163 |
| `src/math/core/mml_tree/mml_node.cpp` | 42-523 |
| `src/math/core/mml_tree/mml_node.ixx` | 14-128 |
| `src/math/core/mml_tree/mo_host.ixx` | 11 |
| `src/math/core/mml_tree/multiscript_host.ixx` | 10-22 |
| `src/math/core/mml_tree/operator_class.cpp` | 53-63 |
| `src/math/core/mml_tree/table_host.ixx` | 11-20 |
| `src/math/core/mml_tree/mml_nodes/maction_behavior.cpp` | 29-90 |
| `src/math/core/mml_tree/mml_nodes/maction_selection.cpp` | 18-33 |
| `src/math/core/mml_tree/mml_nodes/mathchoice.cpp` | 34 |
| `src/math/core/mml_tree/mml_nodes/menclose.cpp` | 18-20 |
| `src/math/core/mml_tree/mml_nodes/mfenced.cpp` | 14-162 |
| `src/math/core/mml_tree/mml_nodes/mfenced.ixx` | 13 |
| `src/math/core/mml_tree/mml_nodes/mfrac.cpp` | 19-20 |
| `src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp` | 19-103 |
| `src/math/core/mml_tree/mml_nodes/mo_classes.cpp` | 41-42 |
| `src/math/core/mml_tree/mml_nodes/mo_scripts.cpp` | 79 |
| `src/math/core/mml_tree/mml_nodes/mroot.cpp` | 19 |
| `src/math/core/mml_tree/mml_nodes/mrow.cpp` | 19-142 |
| `src/math/core/mml_tree/mml_nodes/mstyle.cpp` | 16-71 |
| `src/math/core/mml_tree/mml_nodes/msubsup.cpp` | 23-26 |
| `src/math/core/mml_tree/mml_nodes/mtable.cpp` | 41-147 |
| `src/math/core/mml_tree/mml_nodes/mtr.cpp` | 16-99 |
| `src/math/core/mml_tree/mml_nodes/munderover.cpp` | 23-117 |

No native method returns the child-array handle through a dedicated return API: consumers access the public field and copy the shared handle. Returns from methods are node pointers, indexes/optionals, traversal data, or strings. Writes include direct member rebinds, vector `push_back`, indexed assignment, `erase`, iterator-range reconstruction, and `setChildren`'s copy loop. In particular, MML rebind sites include inferred-row initialization (`mml_node.cpp:140-147`) and arity truncation (`412-414`); shared-handle captures include copy (`190-193`), append flatten (`262-266`), and the many callback-sensitive MML nodes in the table above.

Existing test consumers are also still dense-vector based. `tests/math/reference_factory.cpp:99,102` verifies exact `std::any_cast<FactoryChildren<Probe>>` and shared-handle identity. `tests/math/reference_tree.cpp:49-69,116-148,158-183,200-228` exercises handle capture/rebind, append, null pointers, and wrapper optional/null-handle cases; it does not encode holes. P21a lists the full 19-file matching test set: `reference_arithmetic_order.cpp`, `reference_arithmetic.cpp`, `reference_atom_semantics.cpp`, `reference_factory.cpp`, `reference_html_node.cpp`, `reference_leaf.cpp`, `reference_maction.cpp`, `reference_mathchoice.cpp`, `reference_mfenced.cpp`, `reference_mml_node.cpp`, `reference_mml_registry.cpp`, `reference_mo_order.cpp`, `reference_mo_values.cpp`, `reference_mo.cpp`, `reference_multiscript.cpp`, `reference_script.cpp`, `reference_simple_mml.cpp`, `reference_table.cpp`, `reference_tree.cpp`.

## If a PropertyArray slot must carry Node identity

There is a reusable *type* called `NativeObjectRef` in `src/math/util/object_types.ixx:7-10` and it is already used as the `FunctionReceiver` alternative in `callable_types.ixx:6-8`. It is **not** part of the canonical Property value today. `Property::Value` at `src/math/core/tree/property.ixx:23-40` contains Missing, PropertyNull, scalars, builtin metadata, `shared_ptr<PropertyStorage>`, and FunctionRef only. `tree::Node<Product>` derives from `FactoryNode` (`src/math/core/tree/node_types.ixx:8`) while `NativeObject` is declared separately in `object_types.ixx:9`; no Node-to-NativeObjectRef bridge is present in these declarations. Product lifetime is currently factory-owned `unique_ptr` (`factory_types.ixx:50-52`, `factory.ixx:100-104,126`; class construction/adoption `mml_tree/class_binding.ixx:14-17`), whereas NativeObjectRef is a shared pointer. How a Property-held reference represents the already-owned node, and how it preserves identity/lifetime, is therefore unbound in the current declarations. The property array's backing slots are `vector<Property>` plus presence bits (`property.ixx:44-51`). Thus adding a Node reference value would make property arrays capable of preserving slot identity at the canonical slot layer, but every actual child consumer listed above would have to handle conversion/read/write and maintain live-array alias/rebind semantics; the current `std::any` constructor accepts only `FactoryChildren`.

The concrete consumers that would need review if Node identity is stored in those slots are: generic `Node` storage/API and all `AbstractNode` algorithms (`node_types.ixx:8-31`, `node.ixx:22-205`); factory argument normalization and exact casts (`node_factory.ixx:18-37`, `node.ixx:33-42`); the optional wrapper handle and walker (`wrapper_types.ixx:7-18`, `wrapper.ixx:32-48`); MML's `Children` alias, initializer, copy, set/append/replace, arity repair, traversals and verification (`mml_node.ixx:11-16`, `mml_node.cpp:122-161,171-203,242-306,406-447,475-526`); and every specialized helper/node file in the 31-file table (including `mml_branches.cpp`, `mo_host.ixx`, `multiscript_host.ixx`, `table_host.ixx`, plus `maction`, `mfenced`, `mfrac`, `mmultiscripts`, `mrow`, `mtable`, `mtr`, `munderover`, and the remaining nodes). `tests/math` consumers from P21a currently construct and inspect dense vectors and use exact `std::any_cast<FactoryChildren<...>>`; those tests cannot encode a hole or a Property-held node identity without coordinated fixture/oracle updates.

The current coercion/property host does not supply Node semantics for such a value:

* `property_host::truthy` / `nullish` (`src/math/util/property_host.ixx:12-19`) handle explicit scalar nullish values and otherwise treat object-like values as truthy. They have no special case for a null `NativeObjectRef` carrier.
* `PropertyList::object/record` (`property.ixx:114-122`) and `PropertyArray::array` (`239-241`) recognize only the shared `PropertyStorage` alternative. `NativeObjectRef` is not an object record or an array under current helpers.
* `property_host::primitive` (`property_host.ixx:67-90`) invokes native Object/Array metadata only for `PropertyList::record`; a separate Node ref would fall through as its own variant value. `string` (`92-119`) ends in “Unbound native string conversion”; `number` (`216-223`) routes through primitive/string conversion and is likewise unbound. No Node-to-primitive conversion exists.
* `property_host::get` (`121-174`) supports PropertyStorage, arrays, strings, numbers, and `BuiltinProperty`. A NativeObjectRef would match none: numeric keys fall through to Missing; other string keys fall back to Object.prototype metadata via `PropertyList{}.get(key)`. That is not Node property/member access. There is no binding from the reference to `childNodes`, `parent`, `kind`, Node methods, or MML attributes.
* `ownKeys` (`property_host.ixx:195-204`) enumerates only PropertyStorage and string units; a Node ref would produce no own keys. `objectAssign` (`207-211`) consequently copies no Node properties. `set` (`212-215`) allows only record receivers and errors on a NativeObjectRef. These are current gaps if arrays/copying expose Node values through generic Property operations.
* `PropertyList` prototype resolution, `getOwn`, `hasOwn`, `has`, `keys`, and `assign` (`property.ixx:79-110,131-155,156-228`) operate on PropertyStorage/prototype records; there is no node-object property resolution. Default `Property` equality delegates to variant equality (`property.ixx:23-42`), so a shared pointer alternative could compare identity, but null-ref versus JS null and ownership/lifetime are not specified by the current API.
* Existing `mo_host::moHead` (`src/math/core/mml_tree/mo_host.ixx:21-27`) accepts arrays and primitive strings and returns generic `Property` elements. Existing tuple access (`mml_bindings.cpp:17-32`) expects PropertyArray or generic `host::get`; neither path downcasts or validates Node identity. They are real generic array consumers, but not existing child-array adapters.

These are inventory findings only. They identify precisely where the current Property conversion/lookup boundary does not bind a Node object reference; they do not prescribe how those gaps should be addressed.

## Core paths and source/evidence locations

| File | Relevant lines | SHA-256 |
|---|---:|---|
| `src/math/core/tree/property.ixx` | 23-51, 79-110, 114-228, 232-262 | `1c2dc6779ba822b1e07aff1345390e0cd62d0222458b4784483d2cdd86e12a3a` |
| `src/math/core/tree/factory_types.ixx` | 35 | `25542be82bc00641c5f5880362aaae8f8c022b635df0914011bff33f10ba4033` |
| `src/math/core/tree/node_types.ixx` | 8-31 | `5f278b03d99bc40026b46fb38698d1ab4ca0dfc9fc65fa143ad7a1e9879e165a` |
| `src/math/core/tree/node_factory.ixx` | 18-37 | `c39d713686b348b1f73d1aab5f2230c9dd218de463b3d67218d8f322c4539e92` |
| `src/math/core/tree/node.ixx` | 22-205 | `0be5adbf63239fb4bcbfd459f9267d59c10a3f713aae2c1e202cf64888ec7fd1` |
| `src/math/core/tree/wrapper_types.ixx` | 7-18 | `6c0def2fc0709745c4d8025757c62ba7f1dc75f7440524343b03de637a153ed4` |
| `src/math/core/tree/wrapper.ixx` | 20-49 | `f49bb0710da6c1ddd288a05156f359c5bb62672f7f4a3ff889968535fb5d291d` |
| `src/math/core/mml_tree/mml_node.ixx` | 11-16, 225-255 | `36f47c27f389ac337c8f08dd890d066ba362f8ed9e51ea9ad53cad999ceeba3e` |
| `src/math/core/mml_tree/mml_node.cpp` | 122-203, 242-306, 406-447, 475-526 | `5af784d8fbf6057f9e9a14a9ea3b8afa83349a2b34b6668edd0d80b69b91d43f` |
| `src/math/util/property_host.ixx` | 12-19, 66-184 | `df79652431896f37073fbdef4bcd668a09893c8547c6369f63fb21e492c144ea` |
| `src/math/util/object_types.ixx` | 7-10 | `9d3a0bf6d07182d7bdb6f08021200f5f4b01fe526f538bff337392b516adaa07` |
| `src/math/util/callable_types.ixx` | 6-8 | `c87635e28607bba6949d2213a44b68490a3e986d49faaf913b46c9fb323d962d` |
| `src/math/core/tree/factory.ixx` | 100-104, 126 | `587c552f55387e0a8b6b5a4cd35c3ac6fdb71197d4783f7aa01a2b0ff595286e` |
| `src/math/core/mml_tree/class_binding.ixx` | 14-17 | `243ce11de86efb8d84314c776a1f13f68033c92b7cd8f22567c22d61f1a0a4e8` |
| `src/math/core/tree/property_array.cpp` | 5-22 | `93f64576c8aa1f9ab0df4f02146f2e40d4eb15a44d6bde8306508e1c93718986` |
| `.reference/MathJax-src/ts/core/Tree/Node.ts` | 41-48, 159-197 | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts` | 129-170 | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `.reference/MathJax-src/ts/input/tex/NodeUtil.ts` | 67-69, 191-221 | `bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2` |
| `docs/math-port/evidence/p21a-root-prerequisite-review.md` | full review | P21a evidence |
| `docs/math-port/evidence/p21a-child-array-usage-audit.md` | native/source use inventories | P21a evidence |

No source or test execution was performed for this audit.
