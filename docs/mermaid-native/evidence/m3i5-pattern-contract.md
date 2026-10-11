# M3i5 Root frozen pattern stage packet

This packet restores highp ordinals Gather 11, Reflect 47, Repeat 48, Bilinear 49
and Bicubic 50, using original tiny-skia 0.12.0 arithmetic and the existing pinned
wide providers. It is not the pattern producer, a complete pipeline executor,
blitter acceptance, complete scene rendering, or permission to integrate Folia.
Rendering-function acceptance stays at 34/70.

## Fixed architecture and responsibilities

`sampling/pattern/types.rs` is Root-owned. SourcePixmap has private immutable
borrowed RGBA bytes and validated nonzero contiguous dimensions. Its from_bytes
matches PixmapRef storage checks and permits excess bytes; it performs no color
conversion or copying. Exposed data/width/height accessors are read-only. The
source borrow is separate from the mutable destination ResourceContext. Safe Rust
ownership prevents aliasing a live immutable source and mutable destination.

TileContext retains independent raw f32 scale/inv_scale. SamplerContext reuses
scene::paint::SpreadMode and raw inverse dimensions. PatternContext owns these
small projections plus SourcePixmap. The stage borrows this context immutably;
no stage re-derives its cached inverse fields or normalizes nonfinite coordinates.
Transform stays in the previously accepted shared common-stage GradientProgram
projection for this isolated packet. A future unified producer context is a
separate explicit architectural packet, not another authoritative scene.

`pattern/mod.rs` exports these types and pattern_high_stage. The original lowp
pointer table has none of these stages; pattern_low_stage returns SamplingError
for every ordinal without changing registers. Root owns module wiring, these
APIs, manifests, observers, immutable fixtures, scripts and acceptance reports.

Sol may change only `sampling/pattern/highp.rs` and write
`m3i5-pattern-implementation.md`. Its fixed signature is
`pattern_high_stage(PipelineStage, &mut HighRegisters, &PatternContext<'_>) -> Result<(), SamplingError>`.
All eight source/destination registers retain their original ordering. Supported
stages update registers through the original highp body; all others return the
same unsupported error without mutation. A private temporary vector-register
adapter and no-op next_stage are allowed, as in blending; the caller owns actual
continuation. No second image store, pipeline, layout, scene, SVG intermediary,
platform API, scalar numeric approximation or fallback is allowed.

Map Source gather_ix, ulp_sub, exclusive tiling, 2x2/4x4 sampling, load_8888 and
mad explicitly. Keep original operation/tap/channel/lane order. Adapt pixel access
to the borrowed contiguous RGBA accessor only; do not modify the pinned wide
provider. Every lane is evaluated for tail invocations; stores separately limit
destination writes. Bicubic does not secretly add clamps: the caller emits 2/3.
Source licensing must accompany the migrated body. Do not run builds/tests,
change fixtures, commit, delegate, or modify any other file.

## Frozen original evidence and actual red

Root's append-only original Source observer sets actual TileCtx/SamplerCtx and
PixmapRef, then executes original compiler, start and stage pointers. It checks
all original destination captures remain unchanged before projecting registers
and cursor. `resource-stage-oracle.cjs --pattern --check` reproduces 4,032 highp
cases / 23,680 captures, gold SHA
`7e5d28402733d9eda3d1ee36f54330ea8a8bb51bd20ab4093efa113f6ffb859e`.
Cases cover four image dimensions, premultiplied byte samples, Pad/Reflect/Repeat,
separately varied cached inverse fields, force/non-force, full/tail/multirow runs,
NaN/infinite/signed-zero coordinates, single stages and common-stage compositions.
The Seed/Transform/Gather composition uses original identity Transform; this is
not evidence for every affine producer route.

Actual native modules compile with the frozen signature. All 4,032 cases reach
actual unsupported stage stubs and fail; unsupported high/low cases separately
pass 459 assertions. Log: build/m3i5-pattern-red.log. Root also reproduced the
unchanged earlier resource 2,959, terminal 418 and blend 4,686 fixtures after
extending the original observer. Source algorithms are not generated from native.

Root will compare every register bit/cursor, source borrow and unchanged bytes,
require effective compiling semantic mutants, validate the storage-constructor
boundary against actual PixmapRef, and run relevant regressions. No compile
failure, runtime indexing panic or mere source-text match counts as acceptance.
Pattern::push_stages quality demotion/transform/opacity/color-space preparation,
three blitter paths, full execution, SVG/D2D consumers and Folia remain separate
mandatory gates.
