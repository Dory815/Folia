# OutputJax source member ledger and implementation map

Source inventory and current implementation map for the pinned MathJax 4.1.3
AbstractOutputJax. Root still owns final acceptance; this is not an acceptance
report. Gate 26 compiled and linked the implementation. Its strict 14-row
comparison matched 13 rows; the remaining `base-defaults` observer was reading
the FunctionList wrapper rather than its Source-observed `items` array. The
observer is corrected. Post-Gate-26 constructor metadata corrections await
compilation.

## Pinned source

- ts/core/OutputJax.ts, SHA-256 e4932ba7a5d8f782022fa0ab3c8a8bc05a3072704cd8802213fdadb2722789a3.
- Installed CJS cjs/core/OutputJax.js, SHA-256 9f751917529fdf076b65746a9c910526599944193a9bffcc78c765f7140f8047.
- tools/math-port/output-jax-original-oracle.cjs records 14 observations from that CJS build. It does not read native output.

## Complete class member surface

OutputJax<N,T,D> (lines 39–120) declares name, options, preFilters, postFilters,
and adaptor; setAdaptor(adaptor), initialize(), reset(...args), abstract
typeset(math, document?) and escaped(math, document?), getMetrics(document),
styleSheet(document), and pageElements(document). These declarations define
the host-facing contract; only the abstract methods have no base implementation.

AbstractOutputJax<N,T,D> (lines 126–260) owns:

| Member | Source behavior |
| --- | --- |
| static NAME | "generic"; subclasses may replace it. |
| static OPTIONS | Class object with preFilters: [], postFilters: []. |
| instance options, preFilters, postFilters | Assigned by constructor in that order. |
| instance adaptor | Initialized to null before constructor body. |
| constructor(options = {}) | Captures this.constructor once as CLASS; calls defaultOptions({}, CLASS.OPTIONS), then userOptions(result, options); assigns options; constructs pre-filter list from this.options.preFilters, then post-filter list. |
| getter name | Reads current this.constructor.NAME on every access. |
| setAdaptor(adaptor) | Directly stores exact argument identity; returns undefined. |
| initialize() | No-op; returns undefined. |
| reset(..._args) | No-op; accepts arbitrary arguments; returns undefined. |
| abstract typeset(math, document?) | No base body. |
| abstract escaped(math, document?) | No base body. |
| getMetrics(_document) | No-op; returns undefined. |
| styleSheet(_document) | Returns null. |
| pageElements(_document) | Returns null. |
| protected executeFilters(filters, math, document, data) | Allocates one ordinary record {math, document, data}, passes that same object to filters.execute, then returns its current data property. Ignores executor return. |

The concrete CJS prototype inventory is part of oracle row 14: enumerable,
writable methods are setAdaptor, initialize, reset, getMetrics, styleSheet,
pageElements, and executeFilters. constructor and the name getter are
non-enumerable. The getter is configurable and has no setter. typeset and
escaped are absent from the abstract class prototype, rather than installed as
throwing stubs.

## Original oracle row mapping

| Oracle row | Source members/behavior constrained |
| --- | --- |
| base-defaults | Constructor defaults; name; null adaptor; empty pre/post filter lists. |
| derived-options-and-live-name | Captured derived class OPTIONS, shallow defaults/user merge, distinct destination and nested override, live name after NAME mutation. |
| set-adaptor-preserves-identity | Exact adaptor assignment, undefined return, later mutation and null assignment. |
| base-lifecycle-and-page-return-values | initialize, reset, getMetrics, styleSheet, pageElements; absence of base typeset/escaped. |
| filter-same-record-and-live-data | One filter record; exact math/document identities; mutation and replacement of data; final record data returned. |
| filter-order-and-false-stop | Delegation to canonical FunctionList.execute; priority/equal-priority order and strict false early stop. |
| configured-filter-definition-and-priority | Constructor option definitions feed the two real FunctionList instances and ordered execution. |
| filter-undefined-replacement | executeFilters returns replacement undefined without normalization. |
| filter-thrown-error-propagates | No catch or wrap around filter exceptions. |
| unknown-option-source-warning | Actual defaultOptions/userOptions path, including warning and omission of unknown key. |
| null-options | Null options accepted by the source helper; defaults retained. |
| null-filter-definition | Null preFilters accepted by FunctionList and represented as empty. |
| instances-do-not-share-filter-lists | Each instance allocates independent pre/post lists. |
| prototype-method-inventory | Actual CJS own prototype names and descriptors, including absent abstract methods. |

## Boundaries

DOMAdaptor, MathDocument, and concrete MathItem behavior are imported types;
their behavior is not supplied by this class. The pinned entry uses
liteAdaptor({fontSize: 16}); LiteAdaptor/NodeMixin measurement, DOM identity,
and serialization belong to the separate adaptor packet. Common output,
wrapper measurement/placement, and SVG serialization remain separate packets.
These OutputJax observations alone cannot establish those behaviors.

## Native implementation map

`src/math/core/output_jax.ixx/.cpp` owns the Source class surface and base
algorithms. `initializeOutputJax` calls the repository's `defaultOptions`,
then `userOptions`, and constructs the two existing `FunctionList` instances
in Source order. `name` reads the live instance constructor and its current
`NAME`; `setAdaptor`, base lifecycle methods, null output methods, and
`executeFilters` correspond directly to the source members above. The filter
record is one shared `OutputJaxFilterData` for the whole execution, with
math/document/data retained on that record and returned data read after
execution. The Context prototype installs the nine CJS-observed own members
in source order and preserves the getter/data descriptor distinction. The
Context `reset` method is the source no-op and returns undefined; its adapter
does not currently dispatch to a native subclass override. The typed native
`AbstractOutputJax::reset` remains virtual for native consumers.
Because the pinned CJS build emits `AbstractOutputJax` as an ordinary
`function AbstractOutputJax(options)`, the Context constructor reports
`length === 1` and retains an ordinary writable, non-enumerable,
non-configurable `prototype` property.

The native observer is `tools/math-port/output-jax-native-candidate.cpp` and
the focused checks are `tools/math-port/output-jax-reference-tests.cpp`.
Both establish the derived constructor's static prototype link and its
instance prototype link to AbstractOutputJax. The focused checks additionally
exercise direct typed reset rest-argument forwarding. They are not registered
or built yet; no native comparison or test pass is claimed here.

The Context binding now exposes `options`, `preFilters`, `postFilters`, and
`adaptor` as native data fields backed by the same OutputJax object, and the
filter record's `math`, `document`, and `data` fields are live bindings to the
single record used for execution. The Context methods and native field
bindings retain converters over the same `FunctionListContextBindings` state,
so their Property/Context record conversion uses one identity cache. The
typed native path preserves a `NativeObjectRef` adaptor as that exact shared
owner and reports Context conversion as unsupported until the canonical
DOMAdaptor identity registry exists; it does not manufacture a surrogate
Context object. Typed `MathDocumentReference` values map Missing, Null, and
the existing `MathDocumentService*` alternative individually into the filter
record's Context-capable variant. The
14-row candidate passes all 14 frozen rows in the fresh G27 comparison, and
the focused OutputJax suite passes 98 assertions in 21 tests. This is a
bounded host checkpoint, not full OutputJax/MathJax acceptance. The separate
typed native path still cannot turn arbitrary `MathItem*` or
`MathDocumentService*` values into Context identities; those cases fail
explicitly until the real owner bindings exist. A canonical DOMAdaptor/
MathDocument output host packet is also still absent, so these OutputJax
observations do not establish the later DOM/Common/SVG path.
