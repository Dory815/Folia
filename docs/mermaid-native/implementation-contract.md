# Root contract M1 — immutable shared scene, before emission extraction

Frozen 2026-10-11 by root. This is an implementation authorization for M1 only,
not acceptance of a Direct2D backend or permission to wire Folia.

## Pinned input and ownership

Vendor: third_party/mermaid-rs-renderer, version 0.3.1. All 55 src Rust files
were independently compared byte-for-byte with the checksum-verified registry
archive. Pristine root oracle is .reference/mermaid-registry/mermaid-rs-renderer-0.3.1.
VCS identity and checksum are in source-inventory.json. Do not change global
Cargo caches. Root's vendored manifest omits benchmark/dev/example auto targets;
production source and dependencies are otherwise the pinned baseline.

Sol owns only these new implementation files in M1:

- src/scene/mod.rs: exported scene structures and invariants, no algorithms.
- src/scene/geometry.rs: points, rectangles, affine transforms and typed paths.
- src/scene/paint.rs: resolved paints/strokes, gradients, layer blend modes.
- src/scene/text.rs: positioned text/glyph/resource structures, no font lookup yet.
- src/scene/builder.rs: structural construction and final validation.
- docs/mermaid-native/m1-implementation.md: source obligations and limitations.

Paths above are under the vendor except the report. Root owns lib.rs exports,
Cargo files, upstream render.rs, oracle fixtures/tests, builds, commits, review
and later integration. Sol must not spawn agents, edit expectations, change
parser/layout, alter upstream numerical behavior or add production wiring.

## Concrete shared interface

Public portable entry point after later M2 extraction will be
compile_scene(&Layout, &Theme, &LayoutConfig, Option<(f32,f32)>)
-> Result<DiagramScene, SceneError>. Later prepare_with_options performs the
existing strict parse and layout once, and returns Arc<DiagramScene>.
No compile_scene implementation is authorized in M1.

DiagramScene owns SceneDimensions (natural pixel width/height plus viewBox),
root viewport clip and aspect-ratio policy, ordered scene nodes and immutable
resources. Keep raw data private and provide read-only accessors after finish.
It must be Send + Sync without COM/Windows objects. Builder is mutable only
before finish(self)->Result<DiagramScene, SceneError>. A failed finish publishes
no partial scene. Public scene values are not serde/XML DOMs or generic bags.

Geometry uses f64 to retain emitted decimal coordinates, converting source f32
at extraction without recalculating layout. Point{x,y}, Rect{x,y,width,height},
Affine{a,b,c,d,e,f} follow SVG matrix coordinates. PathSegment has MoveTo,
LineTo, QuadraticTo, CubicTo, ArcTo (rx,ry,rotation_degrees,large_arc,sweep,end),
Close. Path owns segments and FillRule::NonZero/EvenOdd. No raw d string.
Shapes cover Rect with rx/ry, Circle, Ellipse, Line, Polyline, Polygon and Path.
Keep quadratic/arcs typed; backend conversion to D2D curves is allowed, diagram
placement/routing/collision is not. Pure affine composition/transformed-point
operations may live in geometry; preserve mathematical order explicitly.

SceneNode variants are Shape, Text, Image and Group. Every drawable stores
effective resolved PaintStyle. Group owns ordered children, affine transform,
optional typed geometric clip and LayerStyle (opacity, Normal/Multiply blend).
No RawSvg/RawXml/Unknown/Ignore node, parser or string-based draw escape hatch.
No second family-specific native scene/layout or independently positioned SVG
tree. Geometry in this tree is the single source consumed by both backends.

Paint is None, Solid(Rgba) or LinearGradient(GradientId). Gradient table carries
units (UserSpace/ObjectBoundingBox), affine transform, endpoints, ordered stops,
spread mode, resolved colors and stop opacities. Stroke carries paint,width,
dash vector/dash offset, cap, join and miter limit. Fill/stroke opacity and
overall opacity are distinct, finite and bounded. Rgba components are normalized
floats, never a CSS string. CSS/HTML color parsing belongs to M2's shared builder
adapter, not to Direct2D. Resource IDs are typed indices checked at finish.

Markers and icons must later expand to ordinary ordered geometric instances
using their existing units/ref point/orientation/viewBox/clip rules at scene
compilation. SVG export metadata can retain marker/definition identities;
Direct2D must not repeat expansion or position them differently. In M1 store
typed exported metadata only; no marker algorithm is authorized yet.

Text stores original Unicode text/export semantics, original FontSpec,
resolved origin/baseline/anchor and immutable positioned glyph runs. Each run
references an owned FontResource (font bytes, face index, stable identity),
glyph IDs, advances, offsets, UTF-8 source cluster ranges and em size. Backend
never shapes, wraps or computes anchor offsets. Glyph resources must retain
fallback/color glyph images or geometry when used; do not promise arbitrary
color fonts by representing every glyph as a black outline. Text with no
resolved runs is rejected for a nonempty visible string. M1 defines structures
and validation only; M2 text resolution will be frozen separately after root
baseline font/parity evidence. Never substitute approximate DrawText boxes.

Image references owned encoded image bytes with a MIME enum, intrinsic pixel
dimensions and resolved destination rectangle/aspect mapping. Native resource
decoding/upload is backend-specific; positioning/fit is shared. Preserve C4's
two embedded PNGs. Do not fetch remote resources or add new network behavior.

ExportMetadata retains id, classes, data attributes, title, link URL/target
and semantic text where applicable. These are metadata, not a raw attribute
map able to override geometry/paint. Metadata never drives native geometry.
SVG source text/links must remain available, with final serializer policy
reviewed independently during M2. Do not silently discard them.

SceneError is structured: InvalidNumber, InvalidGeometry, InvalidPaint,
InvalidResource, UnresolvedText, UnsupportedFeature, UnbalancedState, with
context. Finite coordinates, nonnegative sizes/radii/widths, usable viewBox,
gradient stop order, valid cluster ranges/UTF-8 boundaries, valid font/image/
gradient refs and recursion budget are verified by finish. Empty scenes and
zero-length primitives may be legitimate; do not manufacture a rectangle or
claim success for unsupported geometry. No panic/unwrap on caller data.

## Subsequent responsibilities (not authorized by M1)

M2: move every final geometry decision in upstream render.rs to shared scene
compilation, preserving all 70 functions/23 families/options and painter order.
Split per-family responsibilities under scene/compile, shared paths/text/shapes
under scene/compile helpers; render.rs remains API facade. Existing parser,
layout and edge_geometry remain authoritative. SVG serializer under
backends/svg.rs consumes scene only, preserves explicit export behavior.

M3: backends/direct2d.rs is cfg(windows)+explicit direct2d feature. Root will
pin windows=0.61.3, select only needed APIs and test WARP. Context is borrowed
for draw_scene(&DiagramScene,&ID2D1DeviceContext,DrawOptions)->Result; resource
cache is tied to device identity. DrawOptions specifies destination transform
and clip, not diagram layout. Save/restore target/transform/clips/blend state
on every return; propagate HRESULT, never swallow EndDraw/Close/device loss.
No SVG/usvg/normalizer path in direct2d's dependency/call graph.

M4: root's explicit backend acceptance then Folia integration. Opaque versioned
FFI handles, balanced retain/release, CPU-only preparation worker, native
resource creation on draw thread, cache/generation/error/PDF behavior required.
Root writes and validates the Folia boundary, never a feature-flag fallback.

## Root acceptance for M1

Root independently compiles without default features/Windows dependencies,
tests geometry order and structural invariants, resource bounds, invalid values,
text clusters, empty scenes and Send+Sync. Adversarial tree depth must fail with
a diagnostic, not stack overflow. Review every field against source audit.
Agent report is not a pass. M1 does not establish SVG parity, native pixels or
backend readiness. Full acceptance retains architecture.md's requirements,
with root-owned frozen baseline for all families/options and actual WARP pixels
at 96/120/144/192 DPI, multiple sizes/themes and fonts, state/lifetime failure
tests and deliberate geometry/paint/text/clip omissions.
