# M3i2 — Root freeze for resource-independent executable sampling

Root owns architecture, interfaces, numeric provider, Source observers, corpus,
tests, mutation cases, manifests, builds, review and acceptance. Sol may write
only `src/scene/prepare/sampling/highp.rs`, `lowp.rs` under the vendored Mermaid
crate and `docs/mermaid-native/evidence/m3i2-sampling-implementation.md`.
No API/provider changes, tests, fixtures, builds, commits or further delegation.

This is executable sampling, preserving the original highp/lowp split, register
phases and vector numeric provider. The original Source responsibilities remain
separate: stop/gradient preparation in prepare's existing modules; whole-pipeline
control in pipeline_control; stage execution in sampling/highp and sampling/lowp;
private vector arithmetic in sampling/wide. Backend/platform and diagram layout
do not belong here. One mutable existing GradientProgram supplies transform,
two-stop/general contexts and conical p/mask. No second stop preparation,
precision selection, lookup cache, or backend-specific copy of these equations.

The public register order is r,g,b,a,dr,dg,db,da: highp has eight raw f32 lanes
per register; lowp has sixteen u16 lanes. Lowp coordinates are the original
split/join bit representation, not sixteen separately converted floats. The
two entry points take PipelineStage, registers, the same mutable GradientProgram
and an integer dx/dy origin. Execute exactly one selected stage, preserving every
other register/context value. No continuation cursor belongs in this one-stage
API. Source next_stage is omitted solely because the caller owns sequencing.

Highp owns Source stage ordinals 0,1,2,3,4,6,46 and 51 through 78 (35 bodies).
Lowp owns 0,1,4,6,46,51,52,53,54,55,57 (11 bodies). Corresponding private helpers
gradient_lookup, two, mad, cond_to_mask, split, join, round_f32_to_u16 and div255
retain Source arithmetic, branches and ordering. Source fields factor/bias/color
r,g,b,a bind to existing GradientColor.components[0..3]; the conical mask/p fields
bind directly to GradientProgram.radial_mask/radial_p; two-stop fields bind to
two_stop. Do not convert operations to generic scalar min/max/floor/round/powf,
use fused multiply-add, introduce decimal tolerances or discard NaN/signed zero.
All other stage ordinals return SamplingError with exact requested precision and
stage, before changing registers/context. This is an explicit incomplete packet
boundary, not a usable final-paint pipeline or a fallback renderer.

Root preserved all nine original wide files and the original BSD license. Only
the original default compile profile predicates std=true, simd=true and
no-std-float=false are bound as cfg constants. Every operation body and every
target_feature branch remains unchanged. Exact byte/provenance verification is
`node tools/mermaid-native/sampling-numeric-vendor.cjs --check`; pins are in
evidence/m3i2-numeric-provider-pins.json. Native Cargo adds only already-locked
bytemuck 1.25.0 and cfg-if 1.0.4; other resolved versions stay unchanged.

Root's Source oracle compiles a byte-guarded copy of original tiny-skia 0.12.0
with its original default std/simd/png profile. It calls original builder push,
set_force_hq and compile; reads the actual high/low function arrays; interleaves
setup/read-only capture pointers into a separate Vec after successful compile;
and runs the original start function with original stage/return pointers intact.
Original files are unchanged before their append-only observer suffixes. TLS is
scoped to the synchronous call, rejects nesting and clears during unwinding.
It never links native Mermaid or recomputes Source stage equations. Actual local
Gradient stop indices are not observable between stages and are not fabricated:
threshold/duplicate-stop selection is checked through resulting raw RGBA lanes.

`sampling-oracle.cjs --check` reproduces 6,678 cases, 5,733 highp and 945 lowp,
with 15,750 after-stage captures. Every owned stage is covered with force on/off,
all 21 rotated boundary/nonfinite/signed-zero/subnormal inputs, three stop-context
families and 18 ordered multi-stage paths. Capture compares every raw register
lane, conical mask and full/tail invocation coordinates. General stops include
duplicate thresholds and distinctive factor/bias channels. Final native tests
also assert immutable context retention and explicit unsupported-stage errors.
Fixture SHA256 is
`0a62bfb1aae676569f098e75c905732b5c26250d39083116f56efcee5f3e3ce8`.

The first Source run failed because the observer supplied coordinate-packed u16
values to lowp premultiply, whose Source input phase is byte color channels.
Root corrected that setup before fixture creation/delegation: raw premultiply
gets [0,255] channels; generated multi-stage paths reaching premultiply have
paint-range alpha while their RGB contexts remain unnormalized. That failed
observer run is not a passed comparison. Raw stage probes and synthetic ordered
paths do not claim valid Paint/blitter execution. Destination load/blend/store,
gamma suffix construction, resource lifetime, completed scenes and GPU/backend
parity remain separate required acceptance work. No pixels are claimed here.

Execution profile: rustc 1.96.1, x86_64-pc-windows-msvc, debug, no RUSTFLAGS or
CARGO_ENCODED_RUSTFLAGS; original target cfg includes SSE/SSE2/SSE3 and no
AVX/AVX2/SSE4.1. Acceptance on other targets/profiles is not inferred.

Root requires a real compiled red test for both precision stubs before Sol is
authorized. The named highp, lowp and unsupported-stage tests all compiled and
failed at their stubs before delegation; raw final-red log SHA256 is
`039af36e6369e122b8dc376c8307636fd7ed8590ac895dfb27641e7cb8cb0549`.
Acceptance then requires unchanged original Source observations,
all native assertions, semantic mutants that compile and are detected through
assertions rather than crashes, the explicit native matrix including actual
Direct2D WARP tests, and independent Source/diff review. Rendering-function
acceptance remains 34/70; full SVG/Direct2D gates and Folia wiring remain open.

Root freezes mutation families before implementation review: omission of each
of all 35 highp and all 11 lowp stages; accepting an unsupported stage in either
precision; premature primitive conversion or wrong coordinate split/join;
threshold comparison, stop index, factor/bias component or arithmetic changes;
lost mask writes/application; lost seed destination reset where Source performs
it; rounding/normalization/alpha-clamp changes; and wrong gamma branches or
precision-specific arithmetic. Effective semantic mutants must compile and
fail an assertion. A failed build or overflow/access crash is rejected as
mutation evidence. Algebraically equivalent mutants are not counted as detected.
