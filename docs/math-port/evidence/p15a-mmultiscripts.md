# P15a `mmultiscripts` implementation evidence

This packet ports four executable methods from MathJax 4.1.3. The pinned source is `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mmultiscripts.ts`, SHA-256 `bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7`; the production-resolved CJS source is `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNodes/mmultiscripts.js`, SHA-256 `db40a3543cffbd706c0070ae0877285ce22b07db3a7e530e56f42eaa7b2d922e`. The implementation is [mmultiscripts.cpp](../../../src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp). Source and port carry MathJax's Apache-2.0 attribution; see `src/math/LICENSE-MathJax.txt`.

## Complete member ledger

| Source member | Source lines | Native mapping |
| --- | ---: | --- |
| `MmlMmultiscripts.defaults` | 37–39 | Root binding provides the own shallow-copy defaults object. |
| `MmlMmultiscripts.kind` | 44–46 | Root binding returns `"mmultiscripts"`. |
| `MmlMmultiscripts.arity` | 53–55 | Root binding returns minimum-one arity. |
| `MmlMmultiscripts.setChildInheritedAttributes` | 65–105 | `MmlMmultiscripts::setChildInheritedAttributes` below. |
| `MmlMmultiscripts.verifyChildren` | 112–142 | `MmlMmultiscripts::verifyChildren` below. |
| `MmlMprescripts.defaults` | 154–156 | Root binding provides the own shallow-copy defaults object. |
| `MmlMprescripts.kind` | 161–163 | Root binding returns `"mprescripts"`. |
| `MmlMprescripts.arity` | 168–170 | Root binding returns 0. |
| `MmlMprescripts.verifyTree` | 177–186 | `MmlMprescripts::verifyTree` below. |
| `MmlNone.defaults` | 198–200 | Root binding provides the own shallow-copy defaults object. |
| `MmlNone.kind` | 205–207 | Root binding returns `"none"`. |
| `MmlNone.arity` | 212–214 | Root binding returns 0. |
| `MmlNone.verifyTree` | 221–230 | `MmlNone::verifyTree` below. |

The four method definitions enter the shared `TreeCallGuard<MmlNode>`. Static defaults, accessors, class construction and factory bindings are root-owned and are not duplicated here.

## Method-to-source behavior mapping

`MmlMmultiscripts::setChildInheritedAttributes` maps to source lines 65–105. It calls the current child at index 0 first with the original attributes/display/level/prime values. Each subsequent loop condition captures the current `childNodes` handle to read its length; each indexed read captures the property again. A child callback that rebinds `this.childNodes` therefore changes the next source access. In-place insertion mutates the actual shared handle selected after the zero-argument factory call.

Only the first `mprescripts` marker changes the local marker flag. On an even marker index the method creates `none`, then resolves the current array and inserts before that index, then writes `none.parent=this`, then increments local `i` in addition to the loop increment. If creation returns null, insertion occurs before the parent dereference fails, matching the permitted partial mutation. The marker branch does not increment the script counter or inherit attributes to the inserted placeholder. Non-marker children receive the same `AttributeList` handle, `false`, `host::add(level, 1)`, and the raw truthy `prime` value or the boolean parity result. The level addition is done after the child receiver and primestyle are established, and the source uses JS primitive addition rather than numeric-only increment.

After scanning, parity is read from the then-current child array. When repair is required, factory creation precedes virtual `appendChild`. The current child array used as the indexed receiver is captured after append, then a fresh current array is read for length; the former is indexed at the latter's final position. The final slot is validated and its method receiver is bound before evaluating `level + 1`; the call uses the original `prime`, not the alternating script value. An empty array or null final slot raises the host `TypeError` at receiver access, before level addition and after any preceding append effects.

`MmlMmultiscripts::verifyChildren` maps to source lines 112–142. It captures `options.fixMmultiscripts` once as a raw `Property`; every condition uses JS truthiness of that captured value. Both loop length checks and indexed reads resolve the current child property independently. Repeated-marker diagnostics explicitly sequence `child.kind()` before this node's `kind()`, then pass the original options handle and `short=true`; scanning continues only if `mError` returns. The first marker's even-index error and the final parity error are guarded by the captured option and omit the short flag (native default false). A throwing callback stops immediately. After custom checks, the method makes the qualified `MmlMsubsup::verifyChildren` call, retaining the source's superclass order and child verification.

`MmlMprescripts::verifyTree` maps to lines 177–186, and `MmlNone::verifyTree` to lines 221–230. Each first calls qualified `AbstractMmlNode::verifyTree(options)`. Only after it returns does each method read the current direct `parent` field and invoke virtual `parent->isKind("mmultiscripts")` if non-null. It does not use logical `Parent()`, so an inferred-row direct parent is rejected even if its ancestor is a multiscript. For invalid parents, the method then reads `kind()` and calls `mError` with the exact message, the same nullable options state, and `short=true`. `multiscriptErrorOptions` preserves absent options: base verification can return on null, the parent check still runs, and `mError` can create/annotate its error node before its null options read throws. The existing parent-`merror` early return remains in the base implementation.

## Host adaptations and remaining boundary

The frozen `multiscript_host.ixx` operations provide checked typed access to current child arrays and slots, insertion without premature parent assignment, exact zero-argument factory creation, raw `host::add`/truthiness behavior, and nullable error options. The accepted child representation is a dense vector of node pointers; it does not expose arbitrary JavaScript proxies, getters on array indices, or sparse-hole identity. Per the root-frozen contract, null/out-of-range child reads fail at the operation that requires the node; no unchecked native pointer dereference is used.

No geometry, script placement, registry wiring, rendering, or repair beyond the source's inherited-attribute normalization is part of this packet. The actual-CJS reference fixture was frozen by root at 1,286 assertions across 222 cases, 480 raw inheritance rows, and 1,161 source-value records. I did not alter or execute it, build, or change shared interfaces. Completion of these method definitions is not packet acceptance; root owns independent source review, mutation/build checks, and acceptance.
