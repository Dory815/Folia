# P19b1 native argument expansion — root-frozen packet

Owner: root. This is a native host prerequisite for the pinned CJS lowering of
`TokenMap.ts` (`0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e`)
and its actual `TokenMap.js`
(`9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7`).
It does not accept TokenMap, MapHandler, or a complete TeX parser.

## Responsibility and API

`folia.math.util.argument_expansion`, namespace `folia::math::arguments`, exports
`std::vector<Property> expand(const Property&)`. It depends only on the accepted
generic Property/numeric/UTF-16 host and standard library. It returns dense,
owned argument slots while retaining the identity of nested Property objects.
No TeX, MML, JavaScript evaluator, Windows, or rendering dependency belongs here.

This API binds the existing native Property domain: raw scalars, native arrays,
ordered data records/prototypes, and the accepted intrinsic metadata identities.
Strings use the pristine String iterator, combining valid surrogate pairs and
retaining lone surrogates. Native arrays have data slots and ordinary prototype
lookup; absent slots become Missing arguments. Other values follow the CJS
no-iterator `Array.prototype.slice.call` path: ToLength of the generic length,
then ascending indexed values. Null and Missing fail with TypeError before any
argument result. ToLength clamps to [0, 2^53-1], truncates toward zero, and maps
NaN/negative values to zero; slice ArrayCreate rejects lengths above 2^32-1 with
RangeError before allocating/reading indices. Do not invent a smaller length cap.

Property has no arbitrary symbol keys, descriptors, Proxies, user-evaluated
conversion callbacks, boxed strings with iterator overrides, or custom iterator
objects. This packet does not claim to implement those JavaScript objects.
Future native extension protocols require a separately frozen typed interface;
neither a JSON approximation nor an evaluator is authorized here. Source getters,
iterator-close, partial reads for CharacterMap destructuring, and callback
invocation ordering belong to later packets. Macro retains its raw args: do not
eagerly expand them in its constructor or alter the accepted Macro interface.

## Ownership and acceptance

The Luna implementation agent owns only `src/math/util/argument_expansion.cpp`
and `docs/math-port/evidence/p19b1-argument-expansion-implementation.md`.
Root owns the interface, build registration, actual-CJS oracle, tests, mutation
runner, expected values, and final acceptance. No agent edits another file,
commits, changes scope counts, or connects Folia.

Root compares the complete result or Source error category by invoking the
unchanged pinned MacroMap.parse with raw Macro.args values. Cases cover dense
and sparse arrays, own/inherited undefined slots, prototype aliasing, records,
scalar/intrinsic values, length coercion and limits, supplementary characters,
combining characters, NUL, and lone surrogates. Additional native ownership
witnesses check nested identity, fresh result storage and independence after
input mutation. Compiling mutants must be detected by unchanged assertions;
compilation failures and crashes do not count. Run the focused tests and full
core build/test. No TS source-file acceptance count increases for this host.
