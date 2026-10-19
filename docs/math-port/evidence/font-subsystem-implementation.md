# MathJax 4.1.3 font subsystem implementation ledger

The implementation follows the pinned source commit
`fb987178c2d279a99b0db5ea02e751691703955e`. Source method bodies and defaults
are mapped below; this ledger records implementation scope and known host
boundaries, not Root acceptance.

| Pinned source | SHA-256 | Native owner |
| --- | --- | --- |
| `ts/output/common/Direction.ts` | `cfa6ccfacb73e6991d373a3677a097356de2092405c742f29811337fd9e511c2` | `src/math/output/common/direction.ixx` |
| `ts/output/common/FontData.ts` | `7d8eeca536fa1b7c14517590b9c5a372ad10b34667d196d09b030f88cda780b2` | `src/math/output/common/font_data.ixx/.cpp` |
| `ts/output/svg/FontData.ts` | `403f59e6cb71fa8baf291402e085daaa25d1d11e8fc48640e370498da6a83da9` | `src/math/output/svg/font_data.ixx/.cpp` |
| NewCM + bbm/bboldx/dsfont CJS font inputs | see frozen lock | `src/math/fonts/newcm_font_data.*`, `svg_extensions.*` |

## Source method mapping

`Direction.ts` exports the exact `NONE`, `V`, and `H` strings; the native
`DIRECTION` record and its aliases hold the same values.

`FontData.ts` static `charOptions`, `defineDynamicFiles`, `dynamicSetup`,
`adjustDelimiters`, and `addExtension` map to `FontDataClass::charOptions`,
`defineDynamicFiles`, `dynamicSetup`, `adjustDelimiters`, and `addExtension` in
`font_data.cpp`. Its default variants, CSS font tuples, SMP tables, remap
tables, parameters, and empty delimiter/character maps are initialized by the
`FontDataClass` constructor. Glyph data, CSS tuples, variant definitions, and
SMP remap arrays use `PropertyArray` as their sole native storage, preserving
variable length, holes, explicit undefined/null values, and shared storage
identity. The static Source `dynamicExtensions` map is
represented by the class-owned ordered `dynamicExtensions` collection.

The instance constructor, `CLASS`, `setOptions`, `addExtension`, and `styles`
accessors map to `FontData` construction and accessors. `createVariant`,
`createVariants`, `remapSmpChars`, and `smpChar` retain the variant prototype
chain, linked-map propagation, and Unicode mathematical-alphanumeric remaps.
`defineChars`, `defineCssFonts`, `defineDelimiters`, `defineRemap`,
`defineDynamicCharacters`, and `flattenRanges` map to the corresponding native
methods. The character and delimiter maps retain inherited lookup; inherited
glyph entries resolve to the same canonical `PropertyArray` tuple. Variant
records also carry SVG cache metadata on the canonical shared record.

The two `addExtension` methods intentionally pass different values to
`defineDynamicFiles`: the class method passes `data.name`, while the instance
method passes its `prefix` argument, matching the pinned source call sites.

`dynamicFileName`, `loadDynamicFile`, `loadDynamicFiles`,
`loadDynamicFilesSync`, and `loadDynamicFileSync` map to the native path
resolver and existing `async::Promise`/retry queue. `addDynamicFontCss` remains
the common virtual extension point. `getDelimiter`, `getSizeVariant`,
`getStretchVariant`, `getStretchVariants`, `getChar`, `getVariant`,
`getCssFont`, `getFamily`, and `getRemappedChar` map to the corresponding
common methods. Missing values use optional/tagged native results; pending
dynamic values retain the dynamic-file/null state rather than fabricating a
glyph.

`svg/FontData.ts` class defaults and constructor map to `SvgFontDataClass` and
`SvgFontData`. Static `charOptions` and `addExtension` retain the common
implementation and merge SVG cache IDs into the class metadata. Free function
`AddPaths` mutates and returns its original font map, storing path/content in
the same character options record consumed by common layout.

The supplemental tuple-presence oracle is
`tools/math-port/font-tuple-presence-oracle.cjs`; its current Source output is
`build/math-port/font-tuple-presence-original.json`. The native candidate
`tools/math-port/font-tuple-presence-native-candidate.cpp` observes tuple
lengths, option-slot values and identity, `AddPaths` mutation/return identity,
and repeated `getChar` tuple identity. It has not yet been compiled or compared
against that output. `charOptions` rejects missing, dynamic, and non-array
records with the Source `Error` name and message; length-three tuples gain a
fresh fourth-slot object, and existing fourth slots (including undefined and
null) pass through unchanged. `getChar` models dynamic placeholders with the
native `DynamicFileRef` alternative; arbitrary truthy non-array Property
objects do not yet carry a canonical dynamic-file identity and remain a known
representation boundary.

The generated NewCM class/instance initialize all 33 static variants, 2,315
glyphs, 222 delimiters, cache IDs, and all 40 dynamic shard recipes. The
generator writes one `newcm_dynamic_setup_<shard>.cpp` per pinned shard and a
small `newcm_dynamic_setup.inc` aggregator, retaining upstream shard order.
Recipes live in native-only bundle metadata; they do not pre-populate the
Source-visible `DynamicFile.setup` field. A successful provider load activates
the recipe only if the provider did not already install that field. Provider
failure leaves it absent, preserving the observed missing-setup failure and
retry state. Generated extension builders provide the pinned bbm
(5 variants/290 glyphs), bboldx
(3/391), and dsfont (2/60) tables. Generated `.inc` files are build products
under `build/math-port/font-generated`; they are not tracked source files.

## Source evidence and verification performed

`tools/math-port/font-data-oracle.cjs` reads the pinned CJS font classes and
the source dynamic loader. It records entry hashes/counts, selected glyph,
delimiter, remap, CSS, and cache observations, then loads the actual NewCM
Latin dynamic shard through the original synchronous loader and records its
resolved U+00C0 glyph. It then loads all 40 shards synchronously and records
every own glyph table and delimiter table on all 33 resolved variants.
Latest oracle output is `build/math-port/font-original.json`.

`tools/math-port/font-data-generate.cjs --check-lock --output-dir
<absolute-path>` verified the generated tables against
`font-data-source-lock.json`: 94 pinned inputs comprise four package entry
files, 46 reachable transitive CJS files, four font package manifests, and 40
dynamic shards. The latest generated counts are NewCM 33/2,315/222, bbm
5/290/0, bboldx 3/391/0, dsfont 2/60/0. `tests/math/font_data_source.cpp`
contains native static-table, extension, glyph, metrics, remap, and delimiter
observations. The original oracle additionally records instance options,
parameter values, ordered size/stretch variants, CSS records/remaps, variant
cache IDs, style getter/setter identity, and both distinct and shared glyph
record cases. It captures each static extension merge's before/after variant,
glyph/delimiter hash, cache IDs, parameter map and dynamic extension names. The
NewCM-specific `OPTIONS.dynamicPrefix` override is preserved after the SVG
class defaults are copied, matching the pinned package path
`@mathjax/mathjax-newcm-font/js/svg/dynamic`. The oracle's static class-state
record keeps `variantNames` separate from the `variants` definition array, so
the comparator checks both. It compares every captured pre-merge state; the
initial state has no `before` field, while all three actual extension records
do.
`tools/math-port/font-data-native-candidate.cpp` emits the native tables and
synchronous dynamic resolution; Root owns its standalone target.
Gate 19 produced a fresh native dump at `build/math-port/font-native-19-b.json`.
The strict full-table comparator found seven mismatches, all in the class
extension snapshots' CSS tuples. Source had constructed an instance of the
NewCM class before taking those snapshots; the constructor mutates the shared
default CSS tuples from `unknown` to `unknownFamily`. The native observer now
performs that same real instance construction before recording merge state.
That observer correction has not yet been rebuilt or re-compared. The
full-table candidate uses a successful synchronous provider to activate
bundled NewCM recipes; it does not exercise external-provider failure behavior.
Root owns the native build and acceptance.

`tools/math-port/font-dynamic-lifecycle-oracle.cjs` separately observes Source
setup registration and sync/async load success, failure and repeat calls. Its
current original output is `build/math-port/font-dynamic-lifecycle-original.json`.
In particular, CJS creates each dynamic record without `setup`; requiring the
shard installs `setup` during the provider call. On sync provider failure the
catch marks the record failed and the same call then throws because setup is
still missing; async failure rejects from that missing setup, and a later
async retry rejects the recorded failed-file error. These results remain
Source observations, not native acceptance. `tests/math/font_data_source.cpp`
also covers pending async setup, failed and missing providers, repeated sync
calls, and provider-installed setup callbacks that throw. A synchronous setup
throw occurs after `dynamic.promise` is stored, so the next sync call is a
no-op; an asynchronous setup throw rejects the call's chained promise while
leaving the loaded promise fulfilled, so a later call invokes setup again.
The fresh gate 19 lifecycle dump at
`build/math-port/font-lifecycle-native-19-b.json` matched all nine frozen
sync/async source traces under the strict lifecycle comparator.

Native async `FontData::loadDynamicFile` requires a genuine shared owner for
the `FontData` instance: promise callbacks retain that same `shared_ptr` so
the receiver remains alive until the queue drains. Async callers and test
candidates must therefore construct the font with `std::make_shared` and keep
that reference for the full operation. Sync-only entrypoints do not use this
ownership path. This is an explicit lifetime requirement of the native async
API, not a copied font object or a non-owning pointer wrapper.

## Boundaries still requiring Root review

The generated native dynamic setup covers every pinned NewCM shard, but the
native provider still must be installed by the owning render/runtime path to
match source load/retry behavior end to end. Generic JavaScript coercion of an
arbitrary object-valued `dynamicPrefix` also requires the shared options host;
the implementation does not stringify unknown objects as a substitute.
Consequently the source ledger and fixture samples do not establish complete
font subsystem or SVG/Folia acceptance.

Gate 19 also ran the supplemental FontData/Direction method dump; its strict
comparison had zero mismatches (`font-methods-native-19-b.json`). The tuple
presence candidate exposed two observer bugs: it serialized the three-slot
tuple before `charOptions` appended slot 3, and its error-result branch emitted
invalid JSON. Both candidate-source issues are corrected, but gate 19's tuple
executable predates those edits and its output must not be used as a result.

Candidate-output audit correction: `font-dynamic-lifecycle-native-candidate.cpp`
now installs a real `CharDataArray` with the provider's fixed metrics and path
options, then serializes the record returned by `getChar`; it does not report a
literal glyph result or reduce success to an existence check. Missing glyphs
and absent metric slots use the oracle's explicit `undefined` marker, while a
present null glyph remains JSON `null`. The full-table candidate also serializes
optional metrics with the `undefined` marker. Its class/instance snapshots,
tables, dynamic records, and lifecycle fields are read from native state. The
static class extension method has a void native return, corresponding to the
Source method's `undefined`; its output field records that return contract.
