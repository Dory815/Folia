# M3i5 Root borrowed pattern stage acceptance

Root accepts the five highp image sampling stages and their direct helpers,
using the frozen borrowed SourcePixmap/TileContext/SamplerContext boundary and
unchanged original wide providers. This does not accept a pattern producer,
complete pipeline/blitter, complete SVG/Direct2D rendering or Folia integration.
Rendering-function acceptance stays at 34/70 until the containing paths pass.

## Original observation and independent verification

The original 4,032 cases / 23,680 captures were frozen before delegation. Actual
native callable stubs compiled and rejected all 4,032 cases. The Sol delivery
then passed the full original raw-bit comparison. Root's independent mutation
inspection found that only square/wide image dimensions were present: a mutant
using min(height,width) for the y clamp survived. Root did not accept that run.

Root added an independent original corpus of 3,024 cases / 17,760 captures for
1x7, 2x3 and 5x17 images. Existing gold bytes are unchanged. All **7,056 pattern
cases / 41,440 captures** now compare every lane/channel bit and continuation
cursor, with source borrow identity and unchanged source bytes checked separately.
The height-clamp mutant is now detected. Nonfinite/signed-zero inputs, separately
varied cached inverse fields, spread modes, tail/full runs and compositions are
covered. Composed Seed/Transform uses identity Transform only.

Root additionally invokes actual original public PixmapRef::from_bytes and its
accessors for **1,694 storage cases** (155 accepted), including zero/extreme
dimensions, insufficient and excess bytes and original immutable borrow identity.
The native counterpart matches all cases. Constructor acceptance does not mean
that every accepted excess-byte shape is safe to sample: original gather's typed
view also requires its full byte length to be divisible by four. The native
bytemuck view preserves that behavior; malformed sampling panic parity has not
been independently executed and remains outside this acceptance.

The final focused suite passes **2,778,524 assertions / three tests**:
2,776,370 pattern, 1,695 PixmapRef and 459 unsupported-stage assertions.
`pattern-stage-mutations.cjs` detects **30 compiling semantic mutants** by
assertion failures. It rejects crashes, overflow, bounds failures and compile
failure as evidence, guards original working source/tests/gold bytes, and records
the final results in build/mermaid-pattern-stage-mutations/results.json.
Mutants cover all five stage omissions, unsupported precision entries, register
and lane ordering, byte scaling, x/y clamps, cached inverse use, tiling formulas,
filter half-offsets and first taps, accumulation order/weights/polynomial factors,
per-tap spread and constructor validation/storage identity.

Original generators reproduce these independent fixture SHA256s:

- Original pattern: `7e5d28402733d9eda3d1ee36f54330ea8a8bb51bd20ab4093efa113f6ffb859e`
- Supplemental tall: `4b9bfeb78c10e75cc671df507003d82c05766c7d2f81a97fedf024b3e15df7ec`
- PixmapRef storage: `48cbe6927020af99c5c273771904bb2c6ca8baaa6f097115940b65d0257dca37`

The earlier 2,959 resource, 418 terminal and 4,686 blend golds also reproduce
unchanged after the observer extension. Original functions stay untouched in
append-only source copies; the native never supplies expected values.

## Consumer regressions and architectural boundary

Root's explicit locked/offline native matrix passes **19 targets / 59 tests**,
including **11 actual Direct2D WARP tests**, with pinned Arial. Matrix log
build/m3i5-final-native-matrix.log SHA256
`fbe75f80c237eb5a7acb305ad7405899c8a5122d8232729bed65df0d67446766`.
These existing WARP contracts do not imply whole Mermaid pixel parity or GPU
execution of this CPU sampling packet.

Original Source function mappings and byte-view adaptations are documented in
m3i5-pattern-implementation.md. Registers are temporary vectors copied through
the frozen stage entry point; the context retains one immutable source borrow.
All eight lanes execute even on a tail. Bicubic clamps stay caller stages 2/3.
There is no image copy, SVG intermediary or platform dependency in preparation.

Pattern::push_stages inverse/quality/opacity/color-space construction and its
failure/partial-state semantics, one complete shared shader context and executor,
three blitter paths, skips/memset/A8, shared scene paint/text parity and final
SVG/Direct2D/Folia acceptance remain mandatory work. Release/profile behavior and
every malformed raw sampling domain are not certified by this debug corpus.
