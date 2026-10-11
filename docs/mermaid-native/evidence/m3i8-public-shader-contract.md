# Root public Shader implementation packet

This packet implements tiny-skia 0.12.0 public factories and five-variant
Shader dispatch. It does not accept the blitter's three pipelines, a complete
retained scene, SVG/Direct2D parity or Folia integration. Existing scene paint
descriptions are f64/id-based; the prepared shader is the one f32 value with
normalized stops and borrowed Pattern pixels.

Root froze actual original factory and dispatch observations before assignment.
shader-public.json has 7,905 complete records, SHA-256
84587d00ad5a893a106f3a75831ce1ed4103b721435274c07e060dfcc450ec96.
Constructors: 3,159 None, 4,746 Some; selected variants: Solid 2,217,
Linear 504, Radial 576, Sweep 504, Pattern 945. Dispatch: 4,047 true, 439 false,
260 original panics, 3,159 not called. Original warnings are part of each record.
Source observer appends read-only accessors at their original field-ownership
modules; all original Rust files remain byte-identical prefixes. Pattern private
field access and alloc::vec::Vec qualification were corrected before gold was
created. Both initial creation and --check execute the original factories.

Root owns declarations and transport. PreparedShader<'source> has exactly
SolidColor(StopColor), LinearGradient(PreparedGradient),
RadialGradient{base,kind}, SweepGradient{base,t0,t1}, Pattern(PreparedPattern).
No owned image copy, retained pipeline context, parallel backend shader, f64
substitution, source-text switch or SVG intermediate. Factories return
Option<PreparedShader<'static>>; Pattern keeps its original source lifetime.
PremultipliedStopColor contains normalized components, constructed through
StopColor::premultiply and read by PipelineBuilder. The accepted SIMD provider
sampling::wide is narrowly visible to this parent module; use its original
f32x4, not a replacement scalar average or different provider.

Sol owns only these bodies and local original-source helpers:

| File under src/scene/prepare | Responsibility |
|---|---|
| linear_program.rs | new_linear_gradient and its original f32 matrix/average helpers; preserve existing producer |
| radial_program.rs | new_radial_gradient, including original prechecks/variant selection and thin-ring stop rewrite; reuse accepted radial geometry; preserve existing producer |
| sweep_program.rs | new_sweep_gradient and push_sweep_gradient_stages |
| shader_dispatch.rs | is_opaque, transform, apply_opacity, push_stages on PreparedShader |
| gradient_stops.rs | only the new StopColor::premultiply body |
| pipeline_builder.rs | only the new push_uniform_color body |

No other bodies/declarations, types, module registration, test/oracle/gold,
manifest, pins, build wiring, scene/backends, Folia, builds, tests, commits or
delegation. Sol writes m3i8-public-shader-implementation.md only as a report.
Keep original copyright/license attribution with transplanted algorithms.

Preserve original check and operation order, f32 intermediate rounding,
NormalizedF32 behavior, None/Solid/gradient choice and partial effects before
panic. Linear empty/single, finite length, 1/32768 degeneracy, Pad last color vs
Reflect/Repeat SIMD weighted average precede inversion and original unit matrix.
Radial negative radii precede stop count; singleton, concentric/Strip/Focal,
swapped geometry and Pad thin-ring hard stops must match the full original
constructor, not just its already accepted geometry classification. Sweep checks
angles before stops, then inverse, degenerate recursive hard-stop rewrite,
whole-circle Pad, t0/t1. Stage producer writes conical p0/p1 before generic
gradient production; XYToUnitAngle and conditional scale-bias occur in its
pre-color callback. Root restored Source's generic before/after callback seam
in gradient_program.rs; no duplicated gradient program or radial-shaped Sweep
adapter. That boundary preserves its existing linear/radial wrapper API.

Shader transforms use original post_concat per variant; opacity uses the
original normalized operations (including original panics). Radial/Pattern
is_opaque are always false. Solid dispatch expands color space, premultiplies,
then calls the same builder. Uniform builder computes four original
(component*255+0.5) as u16 values, pushes its stage, then assigns context;
capacity failure must preserve the old context. No normalized saturation or
premature context mutation replacing that order.

Root compiled the real production package and ran shader_public_parity:
7,521 mismatches and actual stub calls in the full original fixture, plus six
direct dependency calls hitting actual stubs. Compilation succeeded and both
tests failed assertions. Some dependent calls are blocked by earlier stubs
(e.g. premultiply before uniform); this is frontier red, not proof that every
dependent stub executed individually. Red log SHA-256:
3110d46729c9287b29313b1e1dd40d00735b2dac8367a3d8e8320d572cbbcd21.
An initial default-feature/offline dependency failure was not red evidence.

Before delegation Root reran the complete controlled shader regression after
the callback-boundary restoration: 13,580 original runs, zero mismatch/stubs/
errors; Pattern 13,932 records, compile 12,872 records and three-allocation
ownership test also pass. These do not pass the new public factory gate.

Root acceptance: immutable original --check; all 7,905 complete-record native
comparisons, zero stub/unexpected error counts, warnings/state/bit-identical
values; focused compiling semantic mutations covering constructor selection,
stop rewrites, transform/opacity/opaque dispatch, Sweep pre-stage and uniform
quantization/capacity order; existing producer/run tests and full Direct2D
feature test suite including real WARP. No tolerance or case filtering to
clear mismatches. Root performs independent review and all commands with
generated targets under ignored build directories, not the vendored crate.

Source file pins are in the fixture and m3i8-shader-dispatch-audit.md.
Additional original color.rs SHA-256:
3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d.
The inherited pins cover generic gradient, linear, radial, Pattern and pipeline.
Native acceptance remains open until Root records the full passing evidence.
