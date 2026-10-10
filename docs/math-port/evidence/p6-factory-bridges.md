# P6 factory bridge implementation evidence

This packet maps MathJax 4.1.3's NodeFactory and WrapperFactory source to the
two native factory bridge modules. The source files have the hashes pinned in
the root-owned `factory-contracts.md`; this evidence does not change those
contracts or claim root acceptance.

| Upstream source member | Native mapping | Preserved behavior / adaptation |
| --- | --- | --- |
| `NodeFactory<N,C>` interface, `NodeFactory.ts:34–47`, `create` declaration `39–46` | `AbstractNodeFactory<Product>::create(std::string_view, PropertyList, FactoryChildren<Product>)` and raw `create(std::string_view, FactoryArgs)` | Compile-time interface becomes the typed C++ overload plus the root service override. Product constructor owns property copying and child adoption; this bridge forwards the handles without cloning. |
| `AbstractNodeFactory<N,C>` class, `NodeFactory.ts:54–68`; inherited constructor | `AbstractNodeFactory<Product>` derives from native `AbstractFactory<Product>` and inherits its constructors | The generic factory owns registration/default-kind behavior. This class adds no node map, fields, metadata or fallback path of its own. |
| `AbstractNodeFactory.create(kind, properties={}, children=[])`, `NodeFactory.ts:61–67` | Typed `create` in `src/math/core/tree/node_factory.ixx` | Each omitted property argument gets a fresh `PropertyList`; an omitted or null typed children handle gets a fresh child vector. Existing `PropertyList` and non-null child handles preserve shared identity. It dispatches with `createDirect`, so an absent requested kind raises TypeError even if a generic default kind exists. |
| Raw inherited service `create(kind, ...args)` as overridden by the NodeFactory method above | Raw `create(std::string_view, FactoryArgs) override` | Normalizes only the first two positions. Missing or empty `std::any` values receive fresh defaults; a populated `std::any`, including one holding `std::nullptr_t`, passes through unchanged. Extra arguments are discarded. The temporary two-element array is valid for the synchronous constructor call only. |
| `WrapperFactory<N,C,WW,WC>` interface, `WrapperFactory.ts:37–51`, `wrap` declaration `50` | `AbstractWrapperFactory<Product,Node>::wrap(Node*, FactoryArgs)` | The interface's variadic extras are represented by a span of native `std::any` values. The returned product is the native generic factory product type. |
| `AbstractWrapperFactory<N,C,WW,WC>` class, `WrapperFactory.ts:62–77`; inherited constructor | `AbstractWrapperFactory<Product,Node>` derives from native `AbstractFactory<Product>` and inherits its constructors | Registration, default-kind fallback, class identity and product ownership remain the generic factory's responsibility. No wrapper registry or automatic wrapper construction is introduced. |
| `AbstractWrapperFactory.wrap(node,...args)`, `WrapperFactory.ts:74–76` | `wrap` in `src/math/core/tree/wrapper_factory.ixx` | Rejects null with TypeError before dispatch, reads `node->kind()` exactly once, prepends the exact same `Node*`, forwards every extra argument in order, and calls generic `create`, preserving its fallback behavior. |

The two upstream source hashes, verified against the pinned checkout, are:

| Source | SHA256 |
| --- | --- |
| `.reference/MathJax-src/ts/core/Tree/NodeFactory.ts` | `b02c5ac2cabb61755785d35def6b57f8246322c626f548df600ca81af27b4599` |
| `.reference/MathJax-src/ts/core/Tree/WrapperFactory.ts` | `8c59ed6a5e646e0736730070ba87161d7060f1f4a083e92afcc8012345537cea` |

The native bridge uses template product types, `FactoryArgs` spans, `PropertyList`
shared handles, `FactoryChildren<Product>` shared handles and `std::any` to
carry constructor arguments. This is the frozen raw-TeX host adaptation; it
does not attempt to model JavaScript's arbitrary values or its full variadic
runtime. Node property-copying, child-parent relationships, wrapper tree state,
and constructor initialization remain in later packets and are not implemented
here.

`tests/math/reference_factory.cpp` and `tools/math-port/factory-oracle.cjs`
were read only. Root owns all builds and acceptance. No build or test was run
for this packet.
