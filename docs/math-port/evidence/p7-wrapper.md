# P7 generic wrapper implementation evidence

This packet ports the generic `AbstractWrapper` from pinned MathJax 4.1.3
`ts/core/Tree/Wrapper.ts` (SHA256
`8729a810aabd6eb3ad22021464f2f27d86116fae9edc0f818527f82e517bf7d9`). The
implementation is `src/math/core/tree/wrapper.ixx`; the root-owned
`wrapper_types.ixx`, `node_types.ixx`, `tree-contracts.md`, and generated
`tests/math/reference_tree.cpp` were read only. This report records the port
for root review; it is not an acceptance claim.

| Upstream member | Native mapping | Behavior and evidence |
| --- | --- | --- |
| `AbstractWrapper.node`, declared `Wrapper.ts:115` | inherited public `Wrapper<Product,Source>::node` | A mutable, borrowed `Source*`, initialized by the base storage constructor. `kind()` observes its current value on every access. A null handle raises TypeError when kind is requested. |
| `AbstractWrapper.childNodes`, declared `Wrapper.ts:120` | inherited `std::optional<FactoryChildren<Product>>` | The source declaration has no initializer, so the pinned CJS instance initially has no `childNodes` property. Native `nullopt` represents absence; an engaged optional containing a null shared handle represents a present null value and throws when traversal tries to iterate it. |
| `AbstractWrapper.factory`, declared `Wrapper.ts:125` | inherited protected `WrapperServices<Product,Source>&` | A borrowed service reference used by `wrap`; ownership remains with the wrapper factory/session. |
| `kind` getter, `Wrapper.ts:130–132` | `AbstractWrapper::kind() const` | Forwards to the wrapped node's virtual kind each time. A null node maps the source property-access failure to `FactoryError("TypeError", ...)`. |
| constructor, `Wrapper.ts:141–147` | `AbstractWrapper(WrapperServices<Product,Source>&, Source*)` | Calls the base storage constructor only, storing factory and node. It does not initialize child wrappers or call derived virtual behavior during C++ base construction. |
| `wrap(node)`, `Wrapper.ts:152–154` | virtual `AbstractWrapper::wrap(Source*,FactoryArgs)` | Discards the additional native argument span and calls `factory.wrap(source, FactoryArgs{})`; this preserves the source base method's dropped-extra-arguments behavior. The concrete service may provide later override behavior. |
| `walkTree(func,data)`, `Wrapper.ts:159–169` | virtual `AbstractWrapper::walkTree(TreeVisitor<Product> const&, std::any const&)` | Enters `TreeCallGuard<Product>`, visits `this` first, and only then checks child presence. It captures the current shared array after the callback, iterates its live length, skips null children, dispatches each child's virtual walk, and returns the same data value/handle. Replacing the wrapper's optional child array in a callback does not replace the captured array; mutations to that captured vector remain visible during iteration. Present-null children throw TypeError after the self visit. |

The native class uses `Base=Wrapper<Product,Source>` by default and derives
from `Base`, so later wrapper families can supply a root-contract base while
retaining this behavior. `Wrapper<Product,Source>` in the frozen interface
owns the single storage location for `factory`, `node` and optional
`childNodes`; this module adds no parallel fields, wrapping cache, parent links,
CommonWrapper behavior or rendering state.

The resource guard is the root-owned per-product `TreeCallGuard<Product>` with
a 512-call limit. It fails before invoking the callback at the over-limit
frame and unwinds through RAII. This is the accepted native resource bound for
recursive wrapper traversal, not a claim about arbitrary VM recursion. The
visitor is a native synchronous callback; arbitrary JavaScript callbacks and
prototype-based property presence are outside this bridge.

The root-generated reference fixture was not modified. Its wrapper checks cover
initially absent child state, mutable node/kind forwarding, base `wrap` dropping
extras, same-data identity, self-first traversal, live-array growth, replacement
of the current child-array property, null-child skipping, present-null failure,
and the resource boundary. No build or test was run for this packet.
