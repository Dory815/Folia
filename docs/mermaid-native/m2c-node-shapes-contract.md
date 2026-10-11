# Root M2c — complete ordinary node-shape emission

Root read the complete pinned render.rs::shape_svg, lines 5987–6347,
SHA256 026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
This is one complete source responsibility, independent of unresolved shared
text shaping. All source match branches and the source default branch are in
scope. Architecture/C4 icons and diagram orchestration are separate packets.

## Ownership and API

Sol may edit only third_party/mermaid-rs-renderer/src/scene/compile/node_shapes.rs
and docs/mermaid-native/m2c-node-shapes-implementation.md. Root owns exports,
builds/manifests, reference inputs/observations, tests, mutations and acceptance.
No edits to render.rs, source layout/model, scene schema, platform backend,
expectations, production wiring, commits, or sub-agent delegation.

The module exports NodeShapePaints and

`node_shape(node: &NodeLayout, config: &LayoutConfig,
paints: &NodeShapePaints, dash: &[f64]) -> Result<SceneNode, SceneError>`.

NodeShapePaints contains public Paint fields `fill`, `stroke`, `background`,
`primary_text`, `primary_border`, `line`, and `divider_line`. The shared caller
will resolve fill/stroke from node overrides and theme defaults; divider_line
from node.style.line_color or effective stroke; the other fields from theme.
This helper receives resolved paints and dash lengths. It neither parses CSS
nor examines colors as strings. The original NodeLayout determines numeric
stroke widths, shape, id/label state, and coordinates. Return one ordered Group
with identity transform, no clip, default layer/export metadata, holding the
source's shapes in source painter order. Metadata/link assignment belongs to
the later scene caller.

## Required behavior

Preserve every f32 source operation, order, min/max/clamp and predicate before
the source's individual two-decimal emission sites. Quantize only each emitted
scalar with its original format precision; default-display stroke widths use
the shortest f32 decimal, not direct binary f64 widening. Source relative path
operands in MindmapDefault are quantized individually before accumulating
absolute typed coordinates. Keep paths/curves typed; no SVG/XML/path-string
creation or parsing. Scalar decimal formatting/parsing is permitted.

Source rectangles/fork/actor radii, diamond/hexagon/parallelogram/trapezoid/
asymmetric polygons, circles, stadium, round rectangle, cylinder, subroutine,
mindmap quadratic path/divider, and the exact default rectangle are required.
Do not replace the source default branch with UnsupportedFeature: that branch
is an actual supported source decision for remaining NodeShape values.

Circle state selection must preserve priority: __start_ id, __end_ id, then
label_empty (all source lines trim to empty), then ordinary fill/stroke. Use
source paint roles, including DoubleCircle's background inner fill/stroke and
its 1.2/1.0 width choice. Include inner circle only when unquantized r-4>0;
no inheritance of outer dash onto the inner circle. Cylinder order is top
ellipse, body rectangle, bottom ellipse; bottom fill None. Subroutine order
is rectangle then two lines, and those lines do not receive outer dash.
Subroutine lines have effective fill None: source omits the fill attribute,
but its inherited/default fill has no area on a Line. The root observation
compares effective non-area paint separately from inherited SVG attributes.

Ordinary shape stroke cap/join are Round/Round; default miter 4, opacity 1.
Mindmap divider uses Butt/Miter defaults, source line width quantized to two
decimals, opacity 0.35, no fill/dash; preserve its position computed using
unquantized stroke_width.max(0.8). General width defaults to node override
or 1.0, source f32 display precision. Preserve radius/height clamping exactly
where source performs it. Backend primitive clamping remains backend work.

Reject non-finite input/output geometry, negative emitted sizes/radii/widths,
invalid dash values and overflow with structured SceneError. Do not silently
drop a shape or repair invalid caller values. Source finite intermediate
overflow may legitimately be removed by its own min/max/clamp; judge completed
geometry, with exact source predicates, rather than adding arbitrary cutoffs.

## Root acceptance

Reference oracle extracts unchanged shape_svg from pristine checksum-pinned
source and executes it with real NodeLayout/LayoutConfig. Compare ordered typed
shapes, paths, paints, stroke/cap/join/dash/opacity and exact emitted numeric
values across every NodeShape/default branch, id/label states, node overrides,
dimensions, cylinder clamps, inner-circle thresholds, mindmap corner/divider
settings and decimal boundary/relative-operand cases. Invalid source output
must produce an explicit diagnostic, never a published invalid scene.

Root checks finished SceneBuilder validation, compiling mutations that omit
geometry/style/order, existing portable tests and actual WARP tests. Helper
acceptance does not pass complete diagram/Common-text/SVG/Direct2D gates and
does not authorize Folia integration.
