# M3g root freeze — complete f32 gradient stop preparation

This packet is shared portable preparation, before either SVG or Direct2D.
Root freezes declarations, exact responsibilities, actual Source observations,
tests, mutations and acceptance. Sol owns only
`third_party/mermaid-rs-renderer/src/scene/prepare/gradient_stops.rs` and
`docs/mermaid-native/evidence/m3g-gradient-stops-implementation.md`.
No other edits, API changes, tests/builds, expectations, commits or delegation.
No backend or Folia wiring is authorized by this local packet.

## Source and complete responsibilities

Pinned tiny-skia 0.12.0 `src/shaders/gradient.rs` SHA256
`c8a87c87acb85764d5e13c95fd5f66531d24e2c145513e84adfa372d9a878847`:
`GradientStop::new` (31–37), `Gradient::new` (51–103),
`Gradient::apply_opacity` (261–267).
Pinned `src/color.rs` SHA256
`3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d`:
`Color::from_rgba` (240–249), `Color::apply_opacity` (332–334),
`Color::is_opaque` (340–342). These three color dependencies are included
explicitly; other color methods and complete pipeline remain required later.
The six exact methods, all branches and field assignments must be mapped.

Use the existing pinned tiny-skia-path 0.12 `NormalizedF32`, `Scalar` and
`Transform` value/numeric services. Do not introduce a renderer dependency.
NormalizedF32 maps nonfinite raw positions to zero; Scalar::bound uses
`max.min(value).max(min)`, so NaN opacity bounds to one before multiplication.
Do not replace this with one generic clamp for positions and opacity.
Keep f32 arithmetic and inclusive 1/4096 near-equality; no f64/byte rounding.

Root declarations live in `gradient_stop_types.rs`. `StopColor` has four
normalized f32 channels with Source constructor/opaque/opacity methods;
`PreparedGradientStop` has normalized position and StopColor;
`PreparedGradient` owns exactly one stops vector, spread, shader transform,
points-to-unit transform, opaque flag and uniform flag. Its accessors expose
the completed preparation to both future consumers. There is no second
derived stop store or SVG-shaped IR. The scene's existing f64 resolved paint
inputs remain an upstream input responsibility; explicit conversion and final
scene-resource wiring are not accepted by this packet.

Source Gradient::new precondition is at least two stops; zero/one-stop dispatch
and thin-ring stop replacement belong to the radial caller already inventoried,
not to this method. Retain Source's debug assertion. Preserve endpoint dummy
insertion ordering, original first/last colors, equal/descending positions,
first-dummy loop start zero, original uniform step, forced final position one,
all opaque scans and re-computation after each opacity. Do not sort/deduplicate
stops, silently skip input, or update uniform flags during opacity.
Both transforms and mode are retained verbatim without validation/reordering.

## Independent acceptance frozen before implementation

Root actual Source observer executes unchanged six methods and numeric helpers
in an ignored pristine copy. Only read-only observer/exports are appended;
every `.rs` file is checked against original plus exact append before/after.
Adapter dependencies contain no mermaid native crate. The fixture has
**3,050 cases: 3,030 valid, 20 invalid colors**, SHA256
`77016e8df5759cbebc131910c3883eb9a9b0f4df4ccb947378a52203d4bccca8`.
It records constructor output before normalization, initial/each-opacity stops,
both flags, spread, both transform bit arrays and rejected color construction.
Coverage includes nonfinite/negative/subnormal/signed-zero offsets, descending
and equal stops, endpoint dummies, inclusive threshold neighbors, 2–17 stops,
all spread modes, repeated opacity, finite/nonfinite transforms, and all four
invalid channel slots. No native output contributes any expectation.

Root requires focused exact-bit native comparisons, source method/branch review,
effective compiling mutants killed by assertions, and the final explicit native
test matrix including actual WARP tests. A compiling placeholder/red baseline
is not acceptance. This packet advances neither the 34/70 Mermaid rendering
function count nor full paint/sampling/GPU/SVG/Direct2D/Folia gates.
