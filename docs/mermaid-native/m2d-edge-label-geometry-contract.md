# Root M2d — exact edge-label background geometry

Root read complete pristine render.rs lines 1770–1966, SHA256
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
This packet extracts LabelRect::from_center and the ten consecutive geometry
functions from edge_label_background_visible through on_segment. It belongs
to shared scene compilation before either backend; no diagram layout, parser,
paint resolution, text shaping or backend drawing belongs here.

Sol owns only third_party/mermaid-rs-renderer/src/scene/compile/edge_label_geometry.rs
and docs/mermaid-native/m2d-edge-label-implementation.md. Root owns exports,
fixtures, unchanged-source reference crate, tests, mutations, manifests,
review, counts and acceptance. No other changes, builds, commits or delegation.

Public API: LabelRect { pub x:f32, pub y:f32, pub width:f32, pub height:f32 }
derives Debug, Clone, Copy, PartialEq. Its public from_center takes six f32
arguments in exact source order and returns Self. EdgeLabelKind enum has
Center, Start, End and derives Debug, Clone, Copy, PartialEq, Eq.
All ten functions retain their source names, exact f32/tuple/reference argument
types and return types, but become public. Use crate::ir::DiagramKind directly.
No new input validation, quantization, f64 widening or SceneError conversion.

Preserve each entire source body, operation order, f32 min/max/clamp behavior,
NaN/infinity/signed-zero behavior and strict/inclusive comparison choices.
Early returns are source behavior: invalid-sized/short inputs show backgrounds;
short polyline returns infinity; distance <= 1e-6 returns exactly zero; squared
segment length <= 1e-9 uses endpoint distance. Center thresholds depend on
Flowchart, Sequence, Requirement and other kinds; Start/End separately dispatch.
Rectangle edges are inclusive. Collinear tests use abs < 1e-6 and bounding
boxes expanded by epsilon; only afterward do opposite-orientation tests run.
Keep corner order and all segment/point candidates in minimum-distance loops.
No SVG/XML/path serialization or runtime dependency on the pristine renderer.

Root acceptance must run extracted unchanged source bodies from the verified
registry source, covering all 23 DiagramKind values, all label kinds, threshold
neighbors, tangencies, crossings, endpoint/degenerate cases, all helper APIs,
empty/single/multi-point polylines, negative/zero rectangles and nonfinite/signed
zero values. Compare exact f32 bits (NaN category separately), not approximate
geometry or only final background booleans. Compiling deliberate threshold,
candidate, corner, comparison and forwarding omissions must fail unchanged
assertions. This packet cannot pass the complete Mermaid or text/backend gates.
