# Root frozen complete-pipeline executor packet

This packet implements the existing compiled program's invocation boundary,
not a second stage builder or renderer. Source is pinned tiny-skia 0.12.0:
pipeline/mod.rs `RasterPipeline::run` at 496-542, highp.rs `start` at 146-200,
lowp.rs `start` at 165-217. The existing compiler, unique builder/context,
numerical stages and borrowed-resource services are established architecture.

## Ownership and file duties

`pipeline_executor_types.rs` is Root-frozen. An invocation borrows one immutable
SourcePixmap and mask, one exclusive destination and its AA value. It contains
no context. `CompiledPipeline<PipelineContext>` remains the unique persistent
context owner. ResourceContext and PatternContext are temporary views into that
same owner; never clone the context or source/destination pixels.

`pipeline_executor.rs` dispatches only on the already compiled precision.
`pipeline_executor/highp.rs` ports original highp start/control with 8 lanes.
`pipeline_executor/lowp.rs` ports original lowp start/control with 16 lanes.
Keep these roles and the existing frozen public signatures. Helpers may be
private within these three files. No numerical stage body belongs here; use
accepted sampling, resources, blending and pattern stages as applicable.
Do not recompute compiler eligibility, regenerate full/tail lists or create a
parallel stage/context cache. Each compiled operation controls full versus tail
resource access. Return stops the current tile. MaskU8 Stop skips the remaining
stages of that tile and continues the next tile/row.

Initialize register banks exactly once per original start invocation, before
the row loop, not once per tile. Reset cursor and function index per tile. Full
programs run for complete widths; tail programs run for the final remainder of
each row. Preserve registers across tiles and rows, including partial effects
before a panic. Dispatch each stage family explicitly rather than swallowing an
unsupported-service result. Original highp LoadDestinationU8/StoreU8/LoadMaskU8
are empty bodies without continuation, so they stop the current tile. This is
different from resource_high_stage's isolated Unsupported contract; do not
change that service or turn these original whole-program cases into errors.
The original empty compiled program panics when run with a nonempty rectangle;
do not invent a successful empty-program fallback.

Inputs in this first witness have Source-valid positive rectangles and buffers.
No new bounds/normalization/defaulting policy is authorized. Public rectangle
validation and invalid-resource API coverage belong in a later explicit packet.

## Frozen original witness and actual red

Root's append-only observer runs original builder.compile followed by unchanged
`RasterPipeline::run`. Original function lists are only read for pointer IDs;
no capture stage, seeded register bank, substituted start loop or native output
contributes to gold. All original .rs files are checked byte-for-byte against
their source plus the exact observer append. The oracle pins original pipeline
mod/highp/lowp/pixmap files, observes all destination bytes and the complete
persistent context before/after, and catches original panics with their partial
effects. It does not directly observe per-stage cursor/register traces.

The fixture contains 4,608 cases in 45 complete program compositions. It covers
force/high/low precision, full and tail widths, nonzero origin, multiple rows,
all 27 blend operations, coverage, AA, A8, mask-zero Stop and register reuse.
198 original runs panic: 108 empty compiled programs and 90 original debug
u16 arithmetic overflows. Those panics and their partial destination state
remain expected; they must not be filtered out or normalized into successes.
Fixture SHA-256:
`91234a25c7659d4c35df43a1d8387d136d4f811ab34279d6d3bc5b045ecbdabd`.
This witness does not yet cover whole-executor gradient/radial or borrowed-image
sampling compositions, three-pipeline blitter construction, constructors and
Shader dispatch, final SVG/Direct2D equality or Folia.

Root independently compiled the actual typed graph and executed the three
frozen executor stubs against this fixture. All 4,608 records mismatch and all
4,608 calls explicitly return Unimplemented; the test fails assertions rather
than failing compilation or crashing. No coincident noop is accepted as green.
Red log `build/m3i7a-executor-red.log` SHA-256:
`d20da3e3dc258337799cac506b18bd61cf51e979cf14a2f48eb2e3e57b2a3807`.

## Exact agent permissions and Root acceptance

Sol may change only the implementation bodies/private imports/helpers in the
three executor files above, and write m3i7a-executor-implementation.md. No public
types/signatures, other production files, tests, expected fixtures, reference
observer/tools, pins/locks, compiler, stage providers, build wiring, original
dependencies or Folia changes. No build/test/mutation run, commit or delegation.
If an established provider cannot preserve an original whole-run case, report
the responsible boundary; do not patch it unilaterally or change the witness.

Root owns original oracle --check reproduction, all independent native tests,
supplementary original witnesses, mutation faults, full native matrix and the
acceptance report. Require zero mismatches, stub calls and error returns for
the frozen witness. Then extend actual run coverage to gradient/radial/pattern
families and verify ownership/partial effects before claiming full executor
acceptance. Only after executor, blitter, Shader dispatch and final backends
pass their gates may Mermaid be integrated into Folia.
