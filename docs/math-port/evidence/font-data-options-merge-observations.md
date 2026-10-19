# FontData `mergeOptions` receiver and array observations

Pinned source: MathJax 4.1.3 CJS `output/common/FontData.js` and
`util/Options.js` (source lock `fb987178c2d279a99b0db5ea02e751691703955e`).
The reproducible Source probe is
[`font-array-patch-original-oracle.cjs`](../../../tools/math-port/font-array-patch-original-oracle.cjs);
it records 13 observations in the ignored `build/math-port` directory.

`mergeOptions(obj, dst, src)` first branches on Source truthiness. A falsy
source returns `obj[dst]` directly; it does not call `defaultOptions` and does
not assign. For a truthy source, it builds a one-key definition record and
passes that record to `defaultOptions`, which delegates to `Options.insert`.
The destination object, source patch, nested arrays, and element objects keep
their normal JavaScript identities during those reads and writes.

The array cases observed from the pinned implementation are distinct:

- An empty ordinary object patch leaves the destination array identity intact.
- An empty `"[+]"` patch creates a new array while retaining existing element
  identities.
- An empty `"[-]"` patch also creates a new, dense filtered array.
- A combined remove/append patch filters first, then spreads appended values.
- Removal uses strict object identity through `indexOf`; spreading a sparse
  append array produces dense `undefined` entries.
- A plain array source takes `Options.insert`'s replacement branch. It creates
  a fresh array and recursively inserts own enumerable values; holes remain
  absent and nested object/array values are copied by the actual Source helper.

The native helper in `font_data_options.{ixx,cpp}` uses the real
`options::defaultOptions` entry point and the existing
`ContextPropertyBindings` projection. Its falsy branch performs the direct
receiver read. `FontDataClass::addExtension` and `FontData::addExtension` now
route their `defaultVariants`, `defaultSizeVariants`, `defaultStretchVariants`,
`sizeVariants`, and `stretchVariants` merges through that helper on their
canonical constructor or instance receiver. The Source fields are bound through
the same helper to the owners' canonical `PropertyArray` storage; there is no
separate typed patch loop. The native witness in
[`font-data-options-native-tests.cpp`](../../../tools/math-port/font-data-options-native-tests.cpp)
exercises the receiver identity, sparse append, and object-identity removal
paths against that helper.

The latest production wiring and its constructor-backed observer have not yet
been built or compared. The explicit remaining Source boundary is assignment
of a non-array value to one of these statically typed array fields: the current
native field adapter reports a TypeError where JavaScript would retain the
assigned value and fail only at a later array operation. The pinned extension
profile uses arrays and `[+]`/`[-]` patches; no acceptance claim is made for
that arbitrary-value assignment path.
