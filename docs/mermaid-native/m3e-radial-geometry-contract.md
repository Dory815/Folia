# M3e frozen shared radial geometry preparation

Root freezes a portable geometry preparation responsibility, preceding both
the SVG and Direct2D consumers. Source is tiny-skia 0.12.0
shaders/radial_gradient.rs (SHA256
3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c).
Use the locked tiny-skia-path 0.12.0 Point/Transform helpers directly: their
f32/f64 operation order is Source behavior. This is not a replacement geometry
formula or a Direct2D radial brush approximation.

Root owns radial_types.rs, module registration, dependency manifest, independent
actual-Source constructor observation shim, stimuli/fixtures/tests, mutations
and acceptance. Sol mermaid_geometry owns ONLY
third_party/mermaid-rs-renderer/src/scene/prepare/radial_geometry.rs and
docs/mermaid-native/m3e-radial-geometry-implementation.md. No builds, tests,
other agents, fixture changes, manifest changes, integration or commits by Sol.
Implementation starts only after root has frozen the Source fixture.

Frozen function signature (native helpers are portable tiny-skia-path types):
`pub fn prepare_radial_geometry(start_point: Point, start_radius: f32,
end_point: Point, end_radius: f32, stop_count: usize, mode:
crate::scene::paint::SpreadMode, transform: Transform) -> RadialGeometry`.
Types are defined by root in radial_types.rs. No new public API is delegated.
Root has generated 4,284 actual-Source observations before implementation:
tags None=1,887, SolidFirstStop=1,029, Radial=720, Strip=72, Focal=576;
36 thin-ring replacements. Frozen fixture SHA256 (LF bytes):
0887632703e3370d4b56e34d18d16451307ec9df1174e0265b95a8de81a9aa67.

prepare_radial_geometry takes resolved f32 circle centers/radii, the number of
already prepared stops, spread mode and the combined paint transform. Returns
None/SolidFirstStop/Conical; the latter retains Source Radial/Strip/Focal fields,
points-to-unit matrix, and a thin_ring flag requiring the later stop preparer
to replace stops with (0,first),(1,first),(1,last). SolidFirstStop retains the
original first stop's color at the caller, not a fabricated color in geometry.
The full stop array, units/href/bbox/alpha parsing, Gradient::new normalization,
stage selection, sampling, GPU effects and scene publication remain separate
required responsibilities. This packet cannot accept a completed paint or
advance the 70 Mermaid rendering-function count.

Implement all geometry decisions of Source new/new_radial_unchecked/create,
FocalData set/three predicates, map_to_unit_x/ts_from_poly_to_poly/from_poly2,
in the original order. Negative radii precede stop-count checks; a single stop
precedes transform/distance checks; thresholds are inclusive abs comparisons
at 1/32768 and 1/4096. Preserve NaN/infinity behavior without invented finite
guards. Keep the exact same-center early optimizations, Pad thin-ring case,
ordinary-threshold degeneracy, Strip, Focal swap, maps and extra scales.
No clamping of the focal point or fr to the outer circle is permitted.

Root oracle calls unchanged actual RadialGradient::new in an ignored copy with
read-only accessors for the resulting private fields. No branch algorithm may
be rewritten in the observation shim. Fixtures include threshold neighbors,
all branches, nonfinite radii/centers, single/empty stops, singular/affine
transforms and all three spreads. Compare projection tags, f32 bit patterns,
matrices and the observed thin-ring stop replacement exactly (NaN by category).
Root native replay, compiling assertion-failing mutations and the relevant
Rust regressions must pass before local acceptance. WARP and completed-paint
acceptance remain later gates; Folia must wait for complete Mermaid acceptance.
