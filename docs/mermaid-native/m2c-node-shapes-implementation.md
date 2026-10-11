# M2c node-shape implementation submitted for root review

Implemented only new `src/scene/compile/node_shapes.rs` under the vendored
mermaid-rs-renderer plus this report. Authority is the complete pinned 0.3.1
`render.rs::shape_svg`, actual lines 5987–6347, SHA256
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.
The frozen contract initially extended the range into following test-module
lines; root owns that documentation correction.

Public `NodeShapePaints` has exactly the seven frozen resolved Paint fields.
`node_shape(&NodeLayout, &LayoutConfig, &NodeShapePaints, &[f64])` returns one
identity Group with ordered typed shapes, no clip, default layer and empty
metadata. All match branches and the actual default rectangle are implemented.
The helper does not resolve CSS or assign metadata/links.

Source f32 formulas, grouping, predicates and clamps precede two-decimal scalar
quantization. Stroke widths use default f32 Display/parsing; the Mindmap divider
alone uses two decimals. Negative emitted sizes/radii and widths are rejected,
while zero and emitted signed zero are retained. Relevant raw numeric inputs
must be finite. Polygon widths/heights remain coordinate operands, not an
invented positive-size constraint. Source-clamped intermediate overflow is not
blanket-rejected; nonfinite emitted geometry is rejected explicitly.

Circle paint priority remains start id, end id, empty label, ordinary paint.
DoubleCircle's inner inclusion predicate uses unquantized r-4>0, with source
background/None fill, background/outer stroke and 1.2/1.0 width. Inner circles
receive no outer dash. Cylinder order remains top ellipse, body rectangle,
bottom ellipse with None bottom fill; source radius/body-height clamps remain.
Subroutine order remains rectangle then left/right lines without outer dash.
Root explicitly froze effective fill None for those non-area lines.

MindmapDefault retains Move, four straight relative segments, four quadratic
corners and Close in source order. Relative operands are individually quantized
before accumulating absolute f64 coordinates. Literal-minus radius operands use
the source scalar spelling, rather than silently repairing any malformed scalar
result. Divider follows the path, uses y+h-raw_width.max(0.8), supplied divider
paint, Butt/Miter, no fill/dash and stroke opacity0.35. Ordinary shapes retain
Round/Round strokes, miter4, opacity1 and the supplied outer dash.

Every used emitted coordinate is checked for finiteness; dash entries must be
finite/nonnegative. Structured errors distinguish invalid numbers, geometry and
paint. Generic paint/resource validation remains SceneBuilder.finish's job.
Only scalar formatting/parsing is used: no SVG/XML/path strings, SVG parsing,
layout recomputation, text handling or native APIs.

No missing API prerequisites found. Root owns exports, oracle observations,
tests/mutations, builds/manifests, review and acceptance. No builds, tests or
commits were run; no callers, source layout/model, scene schema, backends,
expectations or Folia/math files were edited. This is submitted source work,
not full diagram parity, backend acceptance or production integration.
