# Root original Pattern producer observations

Root's append-only observer compiles and executes the actual tiny-skia 0.12.0
Pattern::new, Shader::transform/apply_opacity, Shader::push_stages dispatch,
Pattern::push_stages and RasterPipelineBuilder::new/push/push_transform bodies.
No native Mermaid crate or native-derived expected value participates. Every
original Rust source file in the copied crate is byte checked against its original
plus only the frozen observer append. Pins are recorded in the immutable fixture.

The observer snapshots stored pattern dimensions/spread/quality/opacity/transform,
source-pixel borrow identity, actual private stage order, all fifteen persistent
Context field groups and force_hq_pipeline, actual warnings, and the return or
capacity panic outcome. Input sentinels populate persistent fields directly to
expose stale overwrites and preservation of inactive groups; they are stimuli,
not formulas for expected outputs. Prefixes call the actual builder push method.
Rust catch_unwind permits observing completed stage/context writes after the
original ArrayVec capacity panic; panic payload text is outside this fixture.

The frozen corpus has **13,284 cases**: 11,664 basic combinations, 972 sequential
transform/opacity combinations and 648 capacity-prefix combinations. Outcomes:
**11,792 true, 972 false, 520 panic**. It covers three pixmap dimensions, all three
qualities/spreads/four color spaces, twelve transforms (identity, fractional and
integer translations, scaling/shear, singular, nonfinite, subnormal and signed
zero), nine opacity values (finite bounds, signed zero and nonfinite), multiple
ordered transforms and opacity applications, sentinel/default context and force
inputs, and original stage prefixes 27–32. Inverse failure leaves the original
prefix/context untouched; overflow snapshots preserve the actual preceding writes.

Fixture SHA:
`e7f086362d8d8544d0b20ac9face8b2fbc630b70fc3e7a869e0188419e2744c5`.
Reproduce with `node tools/mermaid-native/pattern-producer-oracle.cjs --check`.
The observer transport lives in pattern-producer-reference and its Cargo lock
is committed; generated crate/build outputs remain ignored.

This is original evidence only. There is no new native producer declaration,
callable red or algorithm acceptance yet, and no Sol implementation is authorized
by this document. Root must next freeze one common builder owning the Source
32-stage list plus the sole PipelineContext, borrowed PreparedPattern state and
the exact file/API duties; add and prove actual native entry-point red; then
delegate algorithms. The existing accepted compile control must consume that
builder's context. Gradient preparation must ultimately write that same builder,
with no retained second context owner or old-signature execution fallback.

Rendering function acceptance stays 34/70. Complete executor, all three blitter
paths, shared scene paint/text, full SVG/Direct2D comparison and Folia integration
remain mandatory. No full backend or Folia success is claimed here.
