# Root M2a — source presentation paths, typed geometry

Prerequisite: M1 independently builds and passes 13 root tests with Folia's
existing dependency resolutions. M1 is a structure/invariant acceptance only,
not SVG/Direct2D or production acceptance.

Sol may create only third_party/mermaid-rs-renderer/src/scene/compile/paths.rs
and docs/mermaid-native/m2a-path-implementation.md. Root owns module declarations,
tests, manifests, reference outputs and integration. No other writes/builds/
commits/agents. Read pinned src/render.rs functions completely: points_to_path
at 1616, rounded_polyline_path at 1639, basis_curve_path at 1697, dedupe_points
at 1751 (confirm actual line numbers; names/verified file hash are authoritative).

These are presentation geometry responsibilities already in upstream rendering,
not new edge routing. Preserve all f32 operations, expression/order/thresholds,
last-retained-point deduplication, first/last segments, skipped degenerate
corners, near-straight/U-turn branches, trim/radius/clamp thresholds, basis
initial/interior/final cubic formulas and command order. Never simplify routes,
fix source quirks, or use a new spline algorithm. Existing layout and
edge_geometry are authoritative and untouched.

Exact public functions in the new module:

- dedupe_points(&[(f32,f32)]) -> Result<Vec<(f32,f32)>,SceneError>
- points_to_path(&[(f32,f32)]) -> Result<Path,SceneError>
- rounded_polyline_path(&[(f32,f32)],f32) -> Result<Path,SceneError>
- basis_curve_path(&[(f32,f32)]) -> Result<Path,SceneError>

All Path values use typed MoveTo/LineTo/QuadraticTo/CubicTo and NonZero fill rule.
Empty inputs preserve an empty path; one/two-point source cases are explicit.
No XML/SVG/path d string is emitted or parsed. Do not serialize points into a
path then parse it back. A scalar numeric-format adapter is allowed to reproduce
the source's three-decimal output quantization: source f32 arithmetic first,
then format exactly one coordinate with Rust's {:.3} and parse that number to
f64. This is a number conversion, never SVG/path serialization. Preserve signed
zero. Both final backends will consume these quantized scene coordinates.
Naive round(x*1000)/1000 need not match the source's tie behavior and is not
authorized as an approximation. Dedupe returns original f32 values without
quantization. Quantization occurs only at corresponding source emission sites.

Reject nonfinite input coordinates/radius and nonfinite computed coordinates
with InvalidNumber context. Source would serialize invalid numeric SVG for
such inputs; the frozen M1 finite-scene boundary instead reports an error and
publishes no geometry. Finite negative radius preserves the source straight
fallback, not a radius error. Reject nonfinite emitted geometric coordinates;
preserve nonfinite intermediate predicates when source output geometry remains
finite. Opposite finite extreme points may overflow dedupe's subtraction to
infinity, making its less-than epsilon predicate false: retain those original
finite points exactly. Never add a blanket arithmetic overflow guard or clamp
to arbitrary viewport coordinates. Rounded/basis paths reject only when their
actually emitted coordinates are nonfinite.

Root independently extracts unchanged functions from the verified pristine
source for reference execution. Root fixtures compare command types/order and
every numerical coordinate to source emitted paths, plus raw deduped points;
they cover epsilon/radius/trim/dot/cross boundaries, duplicates, degenerate and
U-turn paths, signed zero, unusual ranges and seeded random finite cases.
Nonfinite/overflow behavior is independently checked as the explicit typed
boundary. Mutants dropping curves/endpoints or changing formulas/thresholds
must compile and fail unchanged assertions. Passing M2a does not extract the
other 66 render functions or allow native/Folia wiring. Root authorizes the
next packet only after reviewing this source mapping and evidence.
