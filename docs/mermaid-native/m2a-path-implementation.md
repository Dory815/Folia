# M2a presentation path implementation report

Implemented only `third_party/mermaid-rs-renderer/src/scene/compile/paths.rs` and this report under the frozen `m2a-path-contract.md`. Root owns module declarations, expectations, tests, builds, review and commits. No builds/tests/commits or changes to upstream rendering, parser, layout, manifests, M1 modules, Folia, or math were performed by this agent. This report is a source mapping, not path/backend acceptance evidence.

## Verified source mapping

Read all four pinned `src/render.rs` helpers completely. Actual source starts are `points_to_path:1616`, `rounded_polyline_path:1638`, `basis_curve_path:1695`, and `dedupe_points:1753`. Vendor and pristine oracle `render.rs` independently read back with identical SHA-256 `026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.

The new module exposes the four exact public Result-returning signatures in the contract. Typed paths use `FillRule::NonZero` and MoveTo/LineTo/QuadraticTo/CubicTo. It contains no SVG/XML/path strings, string-based commands, path parsing, new routing, or backend-dependent branches.

* `dedupe_points` validates all caller coordinates first, then compares each point against the **last retained point** using strict x/y `abs(previous - current) < 1e-3` with source short-circuit order. It returns original f32 coordinates without quantization, including signed zero. It does not compare only adjacent original points, merge by distance, or round before comparison.
* `points_to_path` handles an empty path, explicitly returns a MoveTo-only path for one retained point, then emits exactly one LineTo per subsequent retained point. Order/endpoints are retained.
* `rounded_polyline_path` validates radius even for empty/fallback paths. Its <=2-point or finite radius<=0 fallback calls `points_to_path` on the already deduped list, preserving the source's second dedupe. Interior vectors, hypot lengths, unit directions, cross/dot, and before/after coordinates stay f32. It preserves the <=1e-3 degenerate-corner skip; cross.abs()<=1e-3 or dot<-0.95 straight/U-turn branch; radius.min(in_len*0.45).min(out_len*0.45) trim; trim<=0.5 straight branch; and ordered LineTo(before), QuadraticTo(cur,after), final LineTo(last). No degenerate corner is manufactured or endpoint curve substituted.
* `basis_curve_path` explicitly handles zero/one/two retained points. For >=3 it emits the initial `(5*p0+p1)/6` LineTo; each interior cubic's `(2*x0+x1)/3`, `(x0+2*x1)/3`, `(x0+4*x1+x)/6` coordinates (and corresponding y formulas); raw f32 x0/y0/x1/y1 update order; the final cubic's repeated-last-point `(x0+5*x1)/6`; and final LineTo(x1,y1). No rewritten/simplified arithmetic or alternate spline is used. Quantized points never feed back into the next formula.

## Numerical boundary

`quantized_coordinate` first rejects a nonfinite f32 emitted coordinate, formats **one scalar** using Rust `format!("{value:.3}")`, parses that number to f64, and verifies the result is finite. This is the contract's explicit source-emission precision adapter, not path serialization. Signed zero is preserved by formatting/parsing. No scale-and-round approximation, f64 layout arithmetic, clamping, or marker adjustment is introduced.

The original source emits at precisely these conversion sites: retained MoveTo/LineTo points, rounded before/control/after points, basis initial/control/end points and its final endpoint. Intermediate source values and dedupe output are not quantized.

Nonfinite input coordinates/radius and nonfinite **emitted** coordinates produce `SceneError::InvalidNumber` with input/function/segment/component context, and no Path is returned. Empty-path success remains empty geometry. Finite negative/zero radius retains straight fallback.

The initial contract wording had an ambiguity about intermediate-only overflow. This was reported before relying on a changed behavior, and root froze the clarification in the committed contract: preserve nonfinite intermediate predicates if source emitted geometry remains finite. The implementation therefore does not blanket-reject vector/length/cross/dot/trim intermediates. Opposite finite f32 extremes can overflow dedupe subtraction to infinity and retain both original finite points because the epsilon predicate is false. Rounded arithmetic may overflow internally yet take a source straight fallback with finite output; it remains accepted. A rounded/basis coordinate actually emitted as NaN/infinity is rejected at its scalar conversion. This policy does not change source formulas or hide invalid emitted geometry.

## Root acceptance obligations

Independently generated pristine-source expectations must compare raw dedupe results and typed segment kind/order/every coordinate, including signed zero, initial/final segments, skipped degenerate corners, epsilon boundaries, cross/dot/trim thresholds, U-turns, finite negative radius, scalar quantization ties, opposite f32 extremes, emitted arithmetic overflow and seeded random finite cases. Deliberate curve/endpoint/formula/threshold mutants must fail unchanged root assertions. Root may compile and test the new module through its owned declarations without modifying oracle expectations to fit this implementation.

M2a extracts only four presentation helpers. The remaining source rendering decisions, shared text resolution, scene compilation, SVG output, Direct2D consumption, native pixels/resources and Folia integration are unchanged and unaccepted. The implementation stops for independent root review before the next packet.
