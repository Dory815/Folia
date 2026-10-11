# Root frozen M3i6a ownership consolidation

Read m3i6-shared-context-architecture.md. This packet moves existing persistent
fields into one PipelineContext; it introduces no new sampling, shader producer,
numeric algorithm, blitter, backend or Folia behavior. Root has relocated the
value type declarations into pipeline_context_types.rs, with type-identity
re-exports from former locations. Agents may not change that Root-owned file.

## Exact final APIs and file duties

GradientProgram contains only returned, stages and `context: PipelineContext`.
The former transform/two_stop/gradient/radial_p/radial_mask fields disappear.
gradient_program.rs writes program.context directly, retaining every original
operation, warning, partial-state rule and stage order. Do not copy that context
into another retained owner or add Deref/old-field execution compatibility.

sample_high_stage and sample_low_stage retain stage/register/origin arguments but
replace `&mut GradientProgram` with `&mut PipelineContext`. Numeric bodies access
those same moved fields directly. No accepted formula/provider/control changes.

PatternContext has two lifetimes, `<'source,'context>`, and only
`source: SourcePixmap<'source>, pipeline: &'context PipelineContext`.
The pattern entry points use `&PatternContext<'_, '_>`; their temporary adapter
borrows this view and reads pipeline.limit_x/limit_y/sampler. No copied context,
cached inverse recomputation, source image copy or independent mutable fields.

ResourceContext has three lifetimes, `<'pipeline,'dst,'mask>`, and fields
`pipeline: &'pipeline mut PipelineContext`, destination, mask and aa_mask.
Uniform color and current coverage are removed from the view and read through
pipeline by highp/lowp resource entries. Destination/mask/AA keep their existing
run duties. Common and resource stages in one invocation consume this same owner.

Sol may edit only these ten existing production files under scene/prepare:
gradient_program_types.rs, gradient_program.rs, sampling/highp.rs,
sampling/lowp.rs, sampling/pattern/types.rs, sampling/pattern/highp.rs,
sampling/pattern/mod.rs, sampling/resources/types.rs,
sampling/resources/highp.rs and sampling/resources/lowp.rs (ten files).
Write m3i6a-context-implementation.md with field/call mapping and limits. Do not
change any test/fixture/gold, manifest/module wiring, provider, default value type,
other source file, build, commit or delegate. Root owns every observer binding,
new shared-write witness, independent mutation and acceptance.

## Baseline and verification

Root's append-only Source default observer executes actual Context::default and
reads every persistent group; fixture SHA
`d02ab0b72988be7722e914495990e20e2d80b9f17e2dda8a2c4ca77e1f03fe28`.
The native default test compares all 15 groups. Root's pre-consolidation comparison
also covers actual gradient program, sampling, pattern and resource/terminal
corpora. There is no new algorithm stub in this mechanical ownership packet;
it is verified as unchanged original behavior through the new APIs, not called
an algorithm red port. Root will update only observer transports to the frozen
new signatures, preserving every expected original value and assertion.

Root must pass all relevant original bit/sequence/context/resource comparisons,
exercise mutable writes through the sole owner and subsequent borrowed views,
require effective mutations that expose detached-state misuse, and run the full
explicit native matrix with actual WARP. Source generators must reproduce all
unchanged golds. Field/call-site review must find no retained old owner or fallback.
Function acceptance stays 34/70; Pattern producer, full executor, three blitter
paths and final SVG/Direct2D/Folia remain separate gates.
