# Root M2b — explicit edge-symbol presentation geometry

Root independently read pristine render.rs:5902-5975 and the sequence/general
callers at 814-839 and 1105-1145. Source render.rs SHA256 is
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
This packet extracts exactly edge_decoration_svg and arrowhead_svg into typed
scene geometry. It does not switch callers, implement markers or accept M2/M4.

## Sole implementation ownership and signatures

Sol may write only new src/scene/compile/edge_symbols.rs under the vendor and
docs/mermaid-native/m2b-implementation.md. Root owns compile/mod.rs, all source
callers/interfaces/manifests, tests, expectations/oracles, builds and commits.
No backend/native/Folia/math changes, no builds/tests/commits, no delegation.
Missing prerequisites must be reported before changing the frozen boundary.

Public functions in that module:

- arrowhead(point:(f32,f32),angle_deg:f32,stroke:Paint,stroke_width:f32)
  ->Result<SceneNode,SceneError>
- edge_decoration(point:(f32,f32),angle_deg:f32,
  decoration:crate::ir::EdgeDecoration,stroke:Paint,stroke_width:f32,
  at_start:bool)->Result<SceneNode,SceneError>

Input paint is already resolved by shared compilation. No CSS/XML/SVG string
parser, XML builder or raw SVG/path string is allowed in these functions.
Scalar formatting/decimal parsing for source quantization is allowed, as in
M2a. Generic paint/resource reference validation remains SceneBuilder.finish's
responsibility. Return ordinary typed Group/Shape nodes with empty metadata.

## Exact source behavior

All source arithmetic stays f32 until source-emitted numbers are quantized.
Translation, rotation angle and arrow polygon coordinates use source Rust
two-decimal formatting then f64 scalar parsing, preserving signed zero.
Stroke width uses source default f32 Display then f64 scalar parsing; direct
binary f32->f64 widening does not preserve the source SVG decimal value.

Root freezes rotation conversion to the quantized f64 angle.to_radians(), with
separate .cos() and .sin(), matching svgtypes 0.16.1's primary Transform::from_str
rotation conversion used by existing Folia usvg 0.47.0. Construct Affine
{a:cos,b:sin,c:-sin,d:cos,e:quantized_x,f:quantized_y}; translation times rotation.
Do not reduce modulo 360, re-quantize the matrix or use f32 trig. Huge finite
angles use this same conversion. Root oracle will compare against actual
source helper transform parsed independently by svgtypes, with f64 coefficient
tolerance 1e-12 and exact decimal translation/geometry/style values.

Reject nonfinite raw inputs as InvalidNumber and negative finite stroke widths
as InvalidPaint before returning a node. Zero width is legitimate and remains
zero. For arrow size preserve `(width * 1.5 + 4.5).clamp(5.5,10.0)` exactly in
f32; finite huge width can overflow the intermediate and still clamp to 10.
Do not reject that intermediate infinity or cap width to size. Compute half as
size * 0.52 in f32. Typed polygon points are (0,0),(-size,half),(-size,-half)
after source two-decimal quantization. Fill and stroke share the provided paint.

For decoration Diamond or DiamondFilled at the end only, add 180 in f32 before
quantizing angle. Start diamonds do not add it. Other decorations never do.
Source shapes/contours and their order:

| Kind | Shape geometry | Fill |
| --- | --- | --- |
| Circle | circle center(0,0), radius5 | None |
| Cross | M(-5,-5)L(5,5); M(-5,5)L(5,-5) | None |
| Diamond | polygon(0,0),(9,6),(18,0),(9,-6) | None |
| DiamondFilled | same polygon | supplied paint |
| CrowsFootOne | M(0,-6)L(0,6); M(5,-6)L(5,6) | None |
| CrowsFootZeroOne | nested group: circle(-4,0),r4 then M(4,-6)L(4,6) | None |
| CrowsFootMany | M(0,-6)L(0,6); M(0,0)L(8,-6); M(0,0)L(8,6) | None |
| CrowsFootZeroMany | nested group: circle(-4,0),r4 then M(4,0)L(12,-6); M(4,0)L(12,6) | None |

Every symbol has a transformed outer group and retains source inner groups.
Path/polygon strokes have round join/cap. Circle strokes use source SVG defaults:
butt cap, miter join, miter limit4. All strokes have empty dash, offset0, width
as above; all opacity fields and group opacity are1 and blend Normal. Paths use
NonZero, retain every independent Move/Line contour and no invented Close.

Caller extraction is later: preserve raw first/last routed endpoint (no dedupe),
existing edge_geometry endpoint angle, source start-arrow +180 performed at
caller only, no empty-route symbol, single-point angle0, source symbol order
after edge path and before labels. Decorations do not inherit edge dashes.
SVG <marker> expansion at render.rs:335-381 is excluded from this packet.

## Root acceptance

Root extracts and executes unchanged pristine helpers using pinned ir::
EdgeDecoration and locks. An independent root oracle parses only reference SVG
to typed observations, compares all8 kinds/endpoints and arrow size/half,
hierarchy/contour order/paints/caps/joins, decimal and signed-zero boundaries,
adjacent f32 clamp thresholds/default-display widths, noncommuting transforms,
huge finite coordinates/widths/angles and explicit invalid inputs. No expected
output is produced by the new implementation. Root adds compiling omission/
geometry/paint/rotation mutations, reviews the file, builds feature-off and
reruns relevant checks. Native WARP symbol checks supplement source observations
later. This packet alone cannot establish full diagram or Folia readiness.
