# Root M3 local native-consumer validation

Status: local typed-scene consumer checks pass. Full M3 backend parity and M4
Folia integration gates remain CLOSED. This is not acceptance of all Mermaid
families, scene compilation, SVG preservation or application integration.

Root independently reviewed all five backend implementation files against
m3-direct2d-contract.md, including the approved clip and glyph-paint spaces.
The optional Windows feature compiles with pinned windows 0.61.3 and
windows-numerics 0.2.0; portable feature-off compilation/tests still pass.
The native dependency/call graph consumes scene data, typed Direct2D geometry,
retained ttf-parser outlines and declared-raster WIC decoders. It contains no
Mermaid parsing/layout, SVG serialization/parsing, usvg/resvg/normalizer call,
second text shaping/layout or system-font substitution. glyph_svg_image is
only a retained font-table presence test with an explicit unsupported result.
Root removed one unused import; no algorithm correction was needed to pass
the initial native checks.

## Independent observations

Actual WARP software devices and bitmap readback run without windows or a
hardware GPU. Device creation/COM/HRESULT failures fail the tests; none skip.
All 10 native tests pass at terminal exit 0. Relevant coverage:

- Painter order, isolated group opacity applied once over overlap, and actual
  Blend Multiply with independent channel expectations.
- Caller target/affine restoration, borrowed frame ownership, canonical device
  mismatch diagnostics, malformed raster/numeric diagnostics and successful
  recording/drawing after a failed record attempt.
- Nonzero/even-odd two-contour fills, typed quadratic/cubic/elliptical arc
  interiors/direction, circle/ellipse/rounded rectangle/polygon/polyline fill.
- Absolute dash units, asymmetric three-element odd patterns, all-zero solid
  dashes and invisible zero-width strokes.
- Object-box gradient mapping/transform order, user-space gradients and
  pad/repeat/reflect spread. Expectations use native pixel centers rather than
  assuming an integer DIP sample always lies at its requested coordinate.
- Geometric group clips, noncommuting translated/scaled group transforms and
  root viewport clipping before completed viewBox mapping.
- Actual retained Arial A outline position/em size/Y direction at four DPIs;
  original TextNode anchor/font-size metadata intentionally disagrees with the
  completed run. CPU scene/font bytes/renderer are dropped before drawing.
- Both exact upstream 48x48 C4 PNGs decoded from owned bytes. An independent
  root PNG RGB oracle supplies pixel values and bilinear samples. Destination
  clipping truncates the completed intrinsic mapping without refitting.

The drawing cases run at 96/120/144/192 DPI where specified in the tests. Pixel
channel tolerance is 2; the font ink bounding-box tolerance is 2 physical pixels.
This is an initial primitive matrix, not a whole-image font/diagram comparison.
Fixed font source SHA256 is
b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a.

Root png-reference.cjs pins pristine render.rs SHA256
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4,
extracts the two source constants and independently reverses PNG scanline
filters after zlib decode. Fixture hashes are in tests/fixtures/
c4-image-reference.json. No WIC/Direct2D output generated the expected pixels.

## Mutation evidence and final commands

All 18 final mutations compile and fail Rust assertions (exit 101), with no
driver/process crash counted as detection. Root isolated copies retain pinned
locks and expectations. The original working source/tests/fixtures are checked
byte-for-byte after the run. Mutations cover fill rule, arc sweep, both curve
controls, rounded corners, gradient mapping/spread, absolute dash conversion,
actual odd-dash component omission, Multiply mode, run origin/em size/Y axis,
image clip/mapping, group opacity/clip transform and device preflight.

Two preliminary mutation runs were rejected because removing explicit odd-array
duplication survived pixel tests. The same happened after adding the asymmetric
three-element case: Direct2D already preserves that repetition in this tested
environment. This equivalent mutation is not counted as detected. The final
suite omits a real third dash component instead; it is caught. Earlier logs are
build/mermaid-m3-mutations.log and mermaid-m3-mutations-odd3.log.

With FOLIA_SCENE_TEST_FONT=C:/Windows/Fonts/arial.ttf:

- cargo test --locked --offline --no-default-features --manifest-path
  third_party/mermaid-rs-renderer/Cargo.toml --target-dir build/mermaid-native
  --test scene_contracts --test path_parity: 13+2 tests pass, exit 0;
  build/mermaid-m3-portable.log.
- node tools/mermaid-native/direct2d-mutations.cjs: 18 compiling mutants caught,
  exit 0; build/mermaid-m3-mutations-final.log and
  build/mermaid-direct2d-mutations/results.json.
- cargo test --locked --offline --no-default-features --features direct2d
  --manifest-path third_party/mermaid-rs-renderer/Cargo.toml --target-dir
  build/mermaid-native --test direct2d_warp --test scene_contracts
  --test path_parity: all 25 tests pass, exit 0; build/mermaid-m3-final.log.

## Remaining gates

Shared compile_scene and scene-consuming SVG output are still unimplemented;
only four source path helpers have local M2a acceptance. The 470 source SVG
observations over 23 families are a baseline, not proof that these families
produce accepted native scenes. Markers, shaping/fallback and family-specific
presentation extraction remain required. Full positioned text/CJK/combining/RTL/
retained color glyph checks, full SVG-reference pixels/options, broader stroke/
zero/arc cases, hardware/device-loss recreation and application lifetime/cache/
concurrency/cancellation checks remain open. Sideways runs and multiframe images
currently return UnsupportedFeature and require scoped source reachability and
policy validation before any full gate. Folia's production Mermaid dependency,
worker/cache and SVG drawing route have not been changed.
