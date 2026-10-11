# M3f root freeze — shared radial stage preparation

Root freezes types, function, Source corpus and tests before implementation.
This packet extends the accepted M3e geometry with a backend-independent radial
stage plan. SVG and Direct2D must consume the same prepared mathematical
responsibility. This alone does not finish stop preparation, sampling, pixels,
complete scene compilation, any complete backend or Folia integration.

## Responsibility and ownership

Root owns radial_stage_types.rs, prepare/mod.rs, source adapter/oracle/lock,
radial-stages.json, radial_stages_parity.rs, mutation checks and acceptance.
Sol owns only scene/prepare/radial_stages.rs and
docs/mermaid-native/evidence/m3f-radial-stages-implementation.md. No other edits,
builds, tests, dependency changes, commits, delegation or integration by Sol.

The frozen function is prepare_radial_stages(kind: RadialKind)->RadialStagePlan.
Keep the existing RadialKind as the only geometry state. The result contains
f32 p0/p1, ordered before_color and after_color sequences of RadialStage, and
geometry_lowp_compatible. The enum's stable 1..13 codes are defined by root.
Only ApplyVectorMask belongs after color, after optional premultiplication.
The compatibility flag refers to radial stages only; final precision selection
also checks every other stage and force-HQ. No backend may infer whole-pipeline
precision from this flag alone.

Source: pinned tiny-skia 0.12.0 radial_gradient.rs push_stages (234–313) and
FocalData is_focal_on_circle/is_well_behaved/is_natively_focal (66–80).
Full file hashes and lowp table correspondence are in the read-only M3f audit.
Port Source branch order, operations and inclusive nearly-zero predicate exactly.
Use tiny_skia_path::Scalar for the existing Source nearly-zero operation.
Do not approximate by a Direct2D radial brush, merge predicates, pre-reject
nonfinite values, reorder multiplication/division, combine f32 expressions,
or replace masks by clamping. Radial zero-radius first branch, radius max/
difference scale/bias, Strip squared scaled radius, Focal reciprocal and offset,
all formula selection and post conditions remain independent Source decisions.
Only a single XYToRadius pre-stage without post stages is lowp-compatible in
this projection, verified against the actual Source lowp function table.

## Independent acceptance frozen before dispatch

Root's ignored pristine Source copy appends two observers only. The adapter
constructs a Source GradientType from raw typed input and uses an identity
two-stop Gradient wrapper, then executes unchanged actual RadialGradient::
push_stages. It reads actual context p0/p1 and filters the actual emitted stage
sequence. Eligibility is read from actual Source lowp function pointers.
No native function is linked into the oracle. All Rust source bodies are
byte-guarded; the only permitted copy differences are appended observer shims.

The **3,266** actual-Source cases include all accepted M3e conical kinds and
additional raw-kind inputs covering f32 signed zero/subnormal, negative,
infinite/NaN, equal/concentric radii, swapped/non-swapped Focal, inclusive
on-circle/native-focal thresholds and neighboring f32 values, and strict
focal-sign boundaries. Tags: Radial 1,249 / Strip 95 / Focal 1,922; 505 cases
are lowp-compatible. Fixture SHA256:
71cc0ab0cbab59eaee266a1261c8b5e2bee1aa109a6152cd3b5a0bb70e831e23.

Every parameter compares bit-exactly except NaN by category; both stage lists
and compatibility compare exactly. Root verifies oracle --check, the new
native target, effective semantic mutants, and the prior explicit Mermaid
matrix with the fixed Source-comparison font. Compile failure or panic cannot
count as mutant detection or successful equivalence. Root records failures.
Full stop/interpolation/precision/actual WARP pixel work remains separate.
