# Root frozen LinearGradient/RadialGradient producer wrappers

Prerequisite: m3i6c-gradient-builder-acceptance.md. Generic Gradient and Pattern
already write the sole common PipelineBuilder. This packet binds actual original
LinearGradient and RadialGradient push_stages to that same builder. Constructors,
SolidColor fallback/variant selection and public Shader dispatch are separate
remaining duties; do not infer their acceptance from controlled original state.

## Exact responsibilities and authorization

Sol may implement only the existing stub bodies in:
- linear_program.rs: push_linear_gradient_stages.
- radial_program.rs: push_radial_gradient_stages.

Write m3i6d-gradient-shader-implementation.md. Root froze declarations/module
exports, real native observer and gold. Do not change declarations, any other
body/helper/provider, tests/golds/tools/Source, Cargo/build wiring; no builds,
tests, commits or delegation. Root owns every acceptance gate.

Linear mirrors original linear_gradient.rs:94–96: call the accepted generic
push_gradient_stages with no radial callback plan and the caller's builder;
return its result. It must not initialize or replace context, stage storage,
prefix or force flag.

Radial mirrors radial_gradient.rs:234–311. Use the already accepted
prepare_radial_stages(kind) provider for original p0/p1 and exact pre/post plan.
Reset the whole native radial context group before calling the generic producer:
radial_mask is eight zero u32 lanes and radial_p is [plan.p0,plan.p1], retaining
raw f32 signed-zero/nonfinite bits. Both writes finish before generic SeedShader,
including a full-prefix capacity panic or inverse failure. The existing plan
has only local callback metadata, not a persistent context or alternate builder.
Pass that plan and the original builder directly to generic Gradient; return its
actual result. Preserve all inactive context groups, prefix and force flag.
No copied context, reconstructed default builder, SVG transit or fallback path.

## Actual original observations and real red

Root's append-only observer constructs controlled private original wrapper state
from actual Gradient::new/apply_opacity and executes actual LinearGradient or
RadialGradient push_stages. Original constructors/dispatch are not substituted
for these calls and are not claimed accepted. Every original Rust file is byte
checked against original plus frozen observer appends. Pinned linear source SHA:
`d18c92f0762cc5f266241da3c868a769e25fea0c224df4aa8e64cf9a22156704`.

The immutable **53,862** complete-state records include 13,988 original gradient
inputs for Linear, 3,266 original radial-kind stimuli, all 21 original radial
stage orders and 20–32 prefix capacity boundaries with sentinel context. Outcomes
are 32,458 true, 108 false, 21,216 panic and 80 rejected color inputs. The full
stage order, all persistent groups/force, warnings and partial states are observed.
Fixture SHA:
`add70dc18ff8dd0bc67e266e45cd6dbc5a7d92ab72e68d657d50d55b8d9853a9`.

The real crate compiles and the actual native observer fails assertions with
**53,654 mismatches / 53,782 executed stubs**: 15,572 Linear and 38,210 Radial.
Eighty rejected inputs never execute a wrapper; 128 coincident Linear full-prefix
panic records are not individually red. There is no constructor barrier here:
the payload counts independently prove both actual entry points execute.
Red log: build/m3i6d-gradient-shader-red-per-entry.log, SHA
`1dffa7b80ed9b878c3fbd4f724bbadbf3d1947050781485378e6a710d65bd504`.

Require zero mismatches/stubs, effective compiling mutants for each wrapper and
radial reset/partial-write order, unchanged generic/Pattern/geometry/stage gold,
and Root's complete native/WARP matrix. Existing whole-rendering tally remains
34/70; full shared executor, three blitter paths, Shader variant/constructor
dispatch, shared scene paint/text, final SVG/Direct2D and Folia remain open.
