# Root frozen common Gradient producer

Prerequisite: m3i6b-pattern-producer-acceptance.md. Root reviewed the old
GradientProgram preparation and removed its separate Vec<GradientStage> and
PipelineContext owner. The only production entry point for this packet is
`push_gradient_stages(&PreparedGradient, ColorSpace, Option<&RadialStagePlan>,
&mut PipelineBuilder) -> bool` in gradient_program.rs. There is no retained old
signature, compatibility adapter, second stage list or second persistent context.

## File duties and authorization

Sol may implement only the existing frozen push_gradient_stages stub body in
gradient_program.rs and write m3i6c-gradient-builder-implementation.md. Existing
GradientColor/GradientContext helpers, stop preparation, color expansion and
pipeline builder are already accepted services. Do not change those bodies,
declarations, tests/golds/Source observers, Cargo/module wiring, other files,
builds/tests, commits or delegate. Root owns every acceptance gate.

The supplied radial plan represents only original generic pre/post callbacks.
Its p0/p1 and compatibility metadata are not consumed by generic Gradient.
RadialGradient's own context initialization (including whole mask reset) and
LinearGradient/RadialGradient dispatch are a later, separate packet requiring
actual original wrapper observations. Do not initialize or reset radial context
here. The previous program test now uses one actual PipelineBuilder and sets
radial parameters only as the original observer's caller stimulus; no retained
native execution fallback exists.

## Original algorithm and partial effects

Port tiny-skia 0.12.0 src/shaders/gradient.rs:106–259. Push SeedShader first,
then invert gradient.transform. Failure warns with the original text and returns
false while preserving prefix, Seed and all context. On success post-concatenate
points_to_unit in original order, then call builder.push_transform. Radial pre
stages follow, each mapped explicitly to the corresponding PipelineStage, in
order. Spread follows: ReflectX1/RepeatX1, or PadX1 only for uniform stops.

For two stops, expand original colors and write two_stop factor/bias before
pushing EvenlySpaced2StopGradient. For general stops, construct one local
GradientContext using the original endpoint exclusion, zero-length interval,
factor/bias arithmetic, len-before-padding and t-value rules. Push Gradient
before assigning that local context into builder.context.gradient. Therefore
capacity panic preserves different partial writes in these two branches.
Inactive context groups must survive. Conditional Premultiply precedes radial
post stages; return true only after all pushes succeed. Do not alter incoming
force flag, prefix, capacity or warnings.

## Actual Source and red evidence

Root's append-only copied original crate calls actual Gradient::new,
apply_opacity, Gradient::push_stages and RasterPipelineBuilder methods. Every
original Rust file is byte checked against original plus frozen observer append.
Original bodies are untouched; native output never supplies expected values.
The corpus has **20,644** cases: 13,988 existing input records and 6,656 cases
with 20–32 prefix stages, sentinel/default context and force flags. Original
outcomes are 16,670 true, 84 false, 3,810 panic and 80 rejected color inputs.
It observes stages, all persistent context groups, force flag, warnings and
partial effects. Generic callbacks do not initialize radial parameters.
Fixture SHA:
`a21784340fb84f0c8504832a038fd1e255078017bfe356fcd09bca243d6bd749`.

The actual native declaration compiles in the real crate and executes **20,564
producer stub calls**. There are **20,052 complete-record mismatches**. Eighty
rejected inputs never call it; 512 stub panic snapshots coincide with original
capacity panics. These are explicitly not 20,644 individually red records.
Red log: build/m3i6c-gradient-builder-red.log, SHA
`4664f48dfe2ecf006290ffe4b62662d356d0cb914d65a3593e53501564248136`.

Require zero mismatches/stubs, effective compiling mutations for algorithm and
partial-write order, unchanged old gradient/stop/radial gold, and Root's complete
native/WARP matrix. Prior 34/70 rendering tally is historical while this producer
is red; do not count this replacement accepted before that gate. Full executor,
three blitter paths, wrapper dispatch, shared scene and final SVG/D2D/Folia remain
open. No Folia integration is authorized before full acceptance.
