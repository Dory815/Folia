# P19b1 argument expansion implementation evidence

This implementation corresponds to the frozen contract in
`docs/math-port/argument-expansion-contract.md`. It implements only the native
Property-domain operation; it does not accept TokenMap or parser behavior.

The upstream witness is MathJax 4.1.3 `ts/input/tex/TokenMap.ts` (SHA-256
`0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e`) and its
pinned CJS lowering `tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/TokenMap.js`
(SHA-256 `9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7`).
The relevant lowering is `__read` and `__spreadArray` at CJS lines 17–43;
MacroMap's ordinary and CommandMap dispatch use the spread at lines 218 and
237. The helper's first operation reads `Symbol.iterator`; absent an iterator,
`__spreadArray` uses `Array.prototype.slice.call`.

| Native operation | Upstream behavior represented | Adaptation and preserved result |
| --- | --- | --- |
| Nullish guard | `__read` property access on `null`/`undefined` throws before a spread result is made | `expand` checks `Missing` and `PropertyNull` first and reports a TypeError through the generic Property host. |
| String branch | Pristine String iterator yields one string per Unicode code point | `property_host::text` reconstructs the internal UTF-16 sequence; `utf16::codePoints` pairs valid adjacent surrogates and retains lone units; `debugText` emits lossless internal WTF-8 for each yielded string. |
| Array branch | Array iteration performs indexed `Get`; holes yield `undefined`, and inherited indexed properties are visible | The frozen PropertyArray is traversed by index with `PropertyArray::get`, so own and inherited slots are read and absent slots become `Missing`. Nested Property handles are copied without cloning their storage. |
| Generic no-iterator branch | `__spreadArray` falls back to `slice.call(from, 0)` | `property_host::get(input, "length")` followed by `property_host::number` provides ToLength's accepted coercions; then ascending indexed Gets populate a new dense vector. |
| ToLength | Converts to Number, truncates toward zero, maps NaN/negative to zero, and clamps at 2^53−1 | The implementation applies those operations before converting to an integer. Positive infinity clamps to the maximum and therefore fails the subsequent ArrayCreate bound. |
| ArrayCreate bound | Slice creates an Array of the ToLength result and rejects lengths above 2^32−1 | `expand` raises RangeError before reserving result storage or reading indices. It imposes no smaller semantic length cap. |

Each successful call owns a fresh result vector. Elements that are Property
objects remain handles to the same shared graph, so nested identity is retained
and later mutation of the input does not resize or rewrite the returned slot
vector. The C++ vector is dense; JavaScript holes are represented by `Missing`
values in the output argument list, matching iterator/spread output.

The host boundary is intentionally limited to the frozen `Property` domain:
scalar values, native arrays, ordered records/prototypes, and registered
intrinsic metadata. It has no arbitrary symbols, descriptors, accessors,
Proxies, boxed strings, custom iterators, or evaluated conversion callbacks.
Those JavaScript objects are not simulated. A property value stored at an
array-like numeric index can itself retain a nested Property identity; no
Macro `args` value is normalized or changed here. Macro construction and
TokenMap invocation remain separate responsibilities.

No build or test was run by this packet owner; root owns independent oracle,
focused tests, build validation, and acceptance.
