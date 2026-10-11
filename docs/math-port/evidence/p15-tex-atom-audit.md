# P15 prerequisite audit: `TeXAtom`

This is a source audit only. It does not implement or accept the native class.

## Pinned source identity and member ledger

Pinned source: `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/TeXAtom.ts`  
SHA-256: `BAD114224392B94852456A4636B85AD90A5A2C96B63E6C7911D155DB299F7028`  
MathJax source version: 4.1.3; license header at lines 1–16 is Apache-2.0.

| Source member / phase | Lines | Behavior and native implication |
|---|---:|---|
| Imports | 24–27 | Uses `MmlFactory`, `PropertyList`, `AbstractMmlBaseNode`, `MmlNode`, `TEXCLASS`, and `MmlMo`. The last import is required for the late prototype method replacement. |
| `static defaults` | 38–40 | Creates a fresh shallow object by spreading `AbstractMmlBaseNode.defaults`. It adds no own defaults. It is not the same object identity as the base defaults object. A native binding should expose this class's own defaults binding with equivalent copied entries, rather than aliasing the base object. |
| Field `texclass` | 45 | Field initialization sets `TEXCLASS.ORD`. It must happen after the base constructor phase and before the constructor body reads `texClass`. |
| `kind` getter | 50–52 | Returns the exact string `TeXAtom`. |
| `arity` getter | 59–61 | Returns `-1`, selecting the base constructor's inferred-row handling for arbitrary child count. |
| `notParent` getter | 68–70 | Returns `true`; `Parent` traversal skips this wrapper as a MathML container. |
| Constructor | 75–82 | Calls `super(factory, attributes, children)` first; then calls `setProperty('texClass', this.texClass)`. This makes the ORD class explicit in the property graph for serialization. Preserve the post-super write and its order; setting only the protected field does not reproduce it. |
| `setTeXclass(prev)` | 87–90 | Unconditionally invokes `this.childNodes[0].setTeXclass(null)` and discards that return value, then returns `this.adjustTeXclass(prev)`. There is no empty-child guard: malformed/zero-child state fails when child zero is accessed. It does not call `AbstractMmlBaseNode.setTeXclass`. |
| Initial `adjustTeXclass(prev)` | 97–99 | The class body initially defines an identity method returning `prev`. This is subsequently replaced at module evaluation (below); do not treat the stub as final behavior. |
| Prototype method assignment | 101–104 | Replaces `TeXAtom.prototype.adjustTeXclass` with the exact function object `MmlMo.prototype.adjustTeXclass`. This is shared method identity/delegation, not merely similar logic. The native design needs to share/delegate to the accepted `MmlMo` implementation and must not leave the identity stub active. |

## Inherited behavior and source order

`TeXAtom` inherits the rest of `AbstractMmlBaseNode` (`MmlNode.ts:1169–1222`). It therefore inherits `isEmbellished` from child zero (`1178–1180`), `core()` as child zero (`1185–1187`), and `coreMO()` as child zero's `coreMO()` (`1192–1194`). Its own `setTeXclass` is an override, so the base implementation's class-reset, prev-class setup, embellishment/`mi` branching, script-child walk, and base-kind class transfer do not run. In particular its child is classified in isolation with `null` before MmlMo's `adjustTeXclass` applies TeX operator spacing/class rules using the original `prev`.

The base node's class and parent behavior is defined by `MmlNode.ts` (`texClass` accessors at 497–506; `notParent` at 586–588; `Parent` traversal at 590 onward; `core`/`coreMO` at 650–659). `TeXAtom` overrides `notParent` to true and inherits the ordinary mutable texClass accessors. The distinction matters: source `setTeXclass` calls the wrapped child with null, while callers can still see the atom's own ORD class, and parent traversal skips the atom.

`AbstractMmlBaseNode.defaults` directly aliases `AbstractMmlNode.defaults` in source (`MmlNode.ts:1172–1173`); `TeXAtom.defaults`, in contrast, is a new spread object. The copied properties are shallow references/values, and mutation of the base defaults object after class evaluation does not mutate the already-created TeXAtom object. The native constructor binding captures a `MmlClassInfo` defaults callback per class (`src/math/core/mml_tree/class_binding.ixx:6–13`), so a real TeXAtom binding should return a class-owned defaults object rather than reusing the base binding identity.

The upstream constructor body executes after `super`, but the current generic C++ class binding only sequences `bindConstructorClass`, `initializeMml`, and `initializeClassFields` (`class_binding.ixx:14–22`). `initializeClassFields` is the existing post-MML initialization hook (`mml_node.ixx:99–103`), but the TeXAtom-specific `setProperty('texClass', texClass)` is a constructor-body side effect, not just field initialization. A future implementation needs an explicit, source-ordered class initialization action or a class-specific post-super hook. It should not move this write into generic `initializeMml`, because that would alter the construction semantics of every MML class.

## Existing test-only descriptors and fixtures

These are test scaffolds and do not model the real TeXAtom class:

* `tests/math/reference_mo.cpp:26` defines `absentTeXAtom()` whose `isInstance` callback always returns false, and `classes()` installs it under the `TeXAtom` key. `tools/math-port/mo-oracle.cjs:21` generates the same fake descriptor. It keeps type checks from failing on a missing factory key but guarantees no object is recognized as a TeXAtom.
* `tools/math-port/mo-order-oracle.cjs:51` defines `Atom extends AbstractMmlNode` whose only TeXAtom behavior is `kind === 'TeXAtom'`. Its generated native type is the corresponding minimal `Atom` at `tests/math/reference_mo_order.cpp:35`; the fixture replaces the registration and uses one for core-text traversal (`mo-order-oracle.cjs:106`, generated witness in `reference_mo_order.cpp:528`). It does not implement inferred-row construction, ORD property state, `notParent`, or MmlMo's `adjustTeXclass`.
* `tools/math-port/mml-node-oracle.cjs:24,30` and generated `tests/math/reference_mml_node.cpp:24,31` also use an `Atom` leaf with only the `kind` string. This is for tree kind checks, not TeXAtom semantics.

The native source currently has one special check in `src/math/core/mml_tree/mml_branches.cpp:158` (`base->isKind("TeXAtom")` followed by reading `base->texClass()`). That kind-based behavior can observe a test dummy as well as a real class; it does not supply TeXAtom construction or classing. `MmlNode.ts:1210–1212` has the analogous base handling in `AbstractMmlBaseNode.setTeXclass`, which the TeXAtom override itself bypasses.

## Native replacement requirements for root freeze

When the actual class is authorized and frozen, replace the test-only registration in the relevant fixture with the real `MmlTeXAtom` class binding for semantic tests. Do not retain `absentTeXAtom`, the leaf `Atom`, or a branch that falls back to those descriptors when the real class is unavailable; that would keep a second behavior path. Keep narrowly scoped dummy/kind witnesses only in fixtures whose stated target is specifically `isKind` or core traversal mechanics, if the frozen test contract requires that isolation. Tests for the class itself should assert: fresh defaults identity and copied contents; source child construction into the inferred-row shape; `kind`, `arity`, and skipped-parent behavior; post-super explicit `texClass` property; empty/malformed child failure at the first-child access; child classification with null followed by the shared `MmlMo.adjustTeXclass` behavior and its exact return value.

The native declarations already expose the needed base virtuals and post-initialization hook (`src/math/core/mml_tree/mml_node.ixx:87–136`) and `classBinding<T>()` captures defaults and constructs/adopts before initialization (`class_binding.ixx:6–25`). This audit identifies no specific missing TeXAtom-facing virtual in those declarations. The constructor-body property write is the phase seam to freeze explicitly. No native implementation, test change, build, or acceptance was performed here.
