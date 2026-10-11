# M2b implementation submitted for root review

Implemented only `third_party/mermaid-rs-renderer/src/scene/compile/edge_symbols.rs`
under the frozen `m2b-edge-symbols-contract.md`. Source authority is pristine
mermaid-rs-renderer 0.3.1 `render.rs:5902–5975`, SHA256
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.

The public `arrowhead` and `edge_decoration` signatures match the contract and
return typed ordinary Group/Shape nodes. All eight EdgeDecoration variants are
handled exhaustively. Crow's-foot zero variants retain their identity inner
group with circle before path. Every path retains its independent Move/Line
contours, uses NonZero, and adds no Close. Polygons retain source point order.

Arrow size and half use the exact source f32 expressions. End diamonds alone add
180 in f32. Scalar two-decimal formatting/parsing reproduces translation,
rotation and arrow coordinate emission, including signed zero. Stroke width
uses default f32 Display/parsing rather than binary widening. Rotation uses
quantized f64 angle.to_radians(), separate cos/sin, and translation-times-rotation
coefficients without angle reduction or matrix quantization. Finite huge arrow
widths retain source overflow-then-clamp behavior.

Arrowheads and filled diamonds share the supplied resolved paint between fill
and stroke. Other shapes have None fill. Path/polygon strokes use round cap/join;
circles retain butt/miter defaults. Width is retained, including zero; all dash
arrays are empty, offsets zero, and miter limits four. Shape opacity fields and
group opacity are one, groups blend Normal, clips are absent and metadata empty.

Nonfinite point coordinates, input angles and widths return InvalidNumber;
negative finite widths return InvalidPaint before returning any node. Generic
paint and gradient-reference validation remains SceneBuilder.finish's job.
Private helpers format/parse only scalar numbers; there is no XML/SVG/path
string generation or parsing, CSS parsing, route computation, font handling,
native API use, marker definition expansion or caller switching.

Prerequisites and ambiguities: none found within the frozen packet. Root owns
module declaration/export, independent pristine-helper oracle, tests, mutations,
builds, review, acceptance and subsequent callers. No tests, builds or commits
were run by the implementing agent. Existing callers, scene interfaces,
manifests, expectations, backend and Folia files were not edited. This report
records submitted implementation obligations, not accepted diagram parity or
production readiness.
