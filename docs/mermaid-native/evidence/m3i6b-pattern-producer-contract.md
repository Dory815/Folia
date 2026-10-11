# Root frozen borrowed Pattern producer and common builder

Prerequisites: m3i6a-context-acceptance.md and m3i6b-pattern-producer-source.md.
Root has declared the exact native APIs, one common 32-entry ArrayVec stage list,
one persistent PipelineContext owner, borrowed PreparedPattern fields, and native
Source observer. The direct arrayvec dependency uses the already locked 0.7.8;
all unrelated dependency versions remain unchanged. No backends or Folia are
changed by this packet.

## Exact file duties and authorized implementation

pipeline_builder.rs owns the private stage list, force flag and public context.
Root's new/default/set_force/getters are mechanical ownership bindings.
compile(self) consumes that owner through the already accepted compile_pipeline;
it must not clone context or create a retained producer-specific store.
Sol may implement only its existing push and push_transform stub bodies.
push performs the original ArrayVec push, so capacity overflow panics before any
later action. push_transform retains the finite/nonidentity guard; inside the
guard, push Transform happens before writing context.transform. A capacity panic
must leave transform unchanged. Do not silently truncate, return false, pre-grow
the list, weaken the guard or add a second stage list.

pattern_types.rs owns only FilterQuality and PreparedPattern's borrowed source,
stored quality/spread/NormalizedF32 opacity/Transform. It has no PipelineContext,
scene ownership, pixel clone, SVG or platform API. This is the selected original
Shader::Pattern variant's state; other Shader variants are separate duties.
pattern_program.rs owns its four existing stub methods: new, transform,
apply_opacity and push_stages. Sol may implement only these four bodies. Parameter
names may change; types, declarations, field duties and all other files may not.
Write m3i6b-pattern-producer-implementation.md. Do not edit tests, fixtures/golds,
observers/generators, Cargo/module wiring, other algorithms or original Source.
No agent builds/tests, commits or delegation; Root owns every acceptance gate.

## Source operation order

new retains the borrowed source and stores original constructor arguments, using
the original NormalizedF32::new_clamped semantics (nonfinite differs from later
opacity applications). transform post-concatenates in original order.
apply_opacity multiplies stored opacity by the original Scalar::bound(0,1)
result, then NormalizedF32::new(...).unwrap; use the existing tiny-skia-path Scalar
provider rather than a rewritten clamp or numeric special case.

push_stages first inverts the stored transform. Failure emits the original warning
and returns false before modifying the incoming builder. Then push SeedShader,
call builder.push_transform, select local quality, preserve the original identity
or any translation downgrade plus the subsequent shadowed integer-translation
branch. Do not change the pattern's stored quality.

Nearest writes limit_x then limit_y before optional Repeat/Reflect and Gather;
Pad has no tile stage. Bilinear/Bicubic write sampler before their sampling stage;
Bicubic then pushes Clamp0 and ClampA. Saved opacity != ONE writes current_coverage
before pushing Scale1Float. Color-space expansion is last (Linear none; Gamma2,
SimpleSRGB, FullSRGBGamma select the original expand stage). Return true only after
all successful pushes. Preserve every inactive context field, incoming stage
prefix, source borrow, partial field write and overflow location. The builder's
force flag is untouched. No enum-only mock success or fallback is allowed.

## Actual red and acceptance

The frozen original corpus is 13284 cases, SHA
`e7f086362d8d8544d0b20ac9face8b2fbc630b70fc3e7a869e0188419e2744c5`.
Native actual entry points compile against the full real Mermaid crate with the
original lock plus arrayvec root dependency. All **13284 complete-record
comparisons mismatch**, and **13284 actual constructor stub panics execute**.
The constructor barrier prevents later stubs from executing in this initial red;
this is not a claim of separate red calls for all six methods. The observer
records an absent native constructed state, never fills a fake expected model.
Red log: build/m3i6b-pattern-producer-red.log. Expected capacity panics are
observations of Source; crashes and unrelated native failures do not count as
passing cases.

After delivery require all original params/stages/context/borrow/warning/outcome
records, independent compiling semantic mutants for each method and source-order
capacity effects, and a move/compile ownership witness. Root must reproduce
unchanged original gold and pass the expanded explicit native matrix with WARP.
Whole rendering tally remains 34/70; complete shared executor/three blitter paths,
gradient writing into this common builder, final SVG/D2D and Folia remain open.
