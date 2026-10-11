# Root M2e — complete Architecture icon presentation

Root read the complete pristine render.rs::architecture_icon_svg, lines
2633–2813, render source SHA256
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
This packet owns one entire source function, including every exact-name,
substring and fallback branch. Whole-diagram orchestration and shared text
resolution remain separate responsibilities.

Sol may edit only third_party/mermaid-rs-renderer/src/scene/compile/architecture_icons.rs
and docs/mermaid-native/m2e-architecture-icons-implementation.md. No edits to
exports, schema, source renderer, manifests, tests/oracles/expectations,
backend, production wiring, commits or delegation. Root owns those and acceptance.

API: public enum ArchitectureIconItem { Shape(SceneNode), Text(TextPresentation) };
public struct ArchitectureIconPresentation { pub items: Vec<ArchitectureIconItem> };
both derive Debug, Clone, PartialEq. Export
`architecture_icon(icon_type: Option<&str>, w: f32, h: f32,
fill: &Paint, fill_lexeme: &str) -> Result<ArchitectureIconPresentation, SceneError>`.
The shared caller supplies the resolved paint and original raw unescaped source
attribute lexeme. Shapes consume the paint; lambda text retains AttributeLexeme
for common lexical resolution, with no backend consumption of this intermediate.

Keep source f32 arithmetic and exact ASCII lowercase, branch order and substring
predicates. Compute cx=w/2, cy=h/2, r=min(w,h)*.35, sw=max(w*.02,1.5).
Quantize each emitted geometry scalar and stroke width at its source .1 site;
lambda font size is .0. No geometry/SVG/XML/path string generation or parsing.
Scalar decimal formatting/parsing is allowed. Exact internet/globe emits four
shapes; substring internet/gateway/api-gateway emits three. Server and its
server/ec2/compute aliases emit three alternating rounded rectangles/indicator
circles. Exact database/disk emits rectangle, two ellipses, two lines;
database/aurora/rds/dynamodb/db/disk/storage/s3/glacier substrings emit only
rectangle and two ellipses. Cloud exact/substring emits one typed closed path:
Move, four QuadraticTo, Close, with original individually quantized coordinates.
Do not follow the source comment claiming these are cubic curves.
Lambda substring emits a TextPresentation with .1 x/y, middle anchor, central
dominant baseline, serif family, .0 font size, Direct("λ"), no italic/weight.
Use Literal for constant anchor/family and AttributeLexeme for fill_lexeme.
Fallback emits rounded square, horizontal line, vertical line. Preserve order.

Ordinary shapes have no fill, resolved fill paint as stroke, .1 width,
Round cap/join, miter=4, no dash, all opacities=1 and default metadata.
Server indicators have fill paint and no stroke. Fixed rectangle radii are
2 or 4 as the source specifies (both rx/ry). Use NonZero paths. Do not clamp
negative source sizes or silently discard invalid shapes. Diagnose nonfinite
emitted geometry/width/font size and negative emitted size/radius/font size
with structured SceneError. No arbitrary upper cutoffs.

Root acceptance executes the unchanged pinned function, comparing every branch,
overlapping aliases/priority, case/non-ASCII names, None/unknown, dimensions,
all ordered geometry/style scalars and lambda text lexical attributes.
Include decimal neighbors and invalid/nonfinite/overflow cases. Require compiling
semantic mutations to fail unchanged tests, portable scene validation and WARP
regression matrix. This helper alone cannot pass full scene/text/SVG/D2D gates
and does not authorize Folia integration.
