# M3b Direct2D blend consumption — implementation report

Status: **locally accepted with M3c; full backend open**. Evidence is in evidence/m3bc-source-blend-acceptance.md. Sol's initial packet changed only `third_party/mermaid-rs-renderer/src/backends/direct2d/paint.rs` and `src/backends/direct2d.rs`. It consumes the root-declared 16-value `scene::paint::BlendMode`; no parser, text, SVG or application code is changed.

Pinned Source vocabulary: `usvg 0.47.0 src/tree/mod.rs:207–224`, SHA-256 `cb11738534ce989b8583827d55e3e01096831ad40075f7c8351365da7ece35b5`. Native constants below are from the locked `windows 0.61.3` `Win32/Graphics/Direct2D/Common/mod.rs:27–50`, SHA-256 `75ba7d5478064184b35a2fc64b357e509cb199a384f7e3dd07f20ea69433cc2c` (`third_party/mermaid-rs-renderer/Cargo.lock:839–840`).

| Source `BlendMode` | Native consumption | D2D integer |
| --- | --- | ---: |
| Normal | Record background then foreground through existing `draw_image` source-over composition; no Blend effect | — |
| Multiply | `D2D1_BLEND_MODE_MULTIPLY` | 0 |
| Screen | `D2D1_BLEND_MODE_SCREEN` | 1 |
| Overlay | `D2D1_BLEND_MODE_OVERLAY` | 11 |
| Darken | `D2D1_BLEND_MODE_DARKEN` | 2 |
| Lighten | `D2D1_BLEND_MODE_LIGHTEN` | 3 |
| ColorDodge | `D2D1_BLEND_MODE_COLOR_DODGE` | 9 |
| ColorBurn | `D2D1_BLEND_MODE_COLOR_BURN` | 5 |
| HardLight | `D2D1_BLEND_MODE_HARD_LIGHT` | 13 |
| SoftLight | `D2D1_BLEND_MODE_SOFT_LIGHT` | 12 |
| Difference | `D2D1_BLEND_MODE_DIFFERENCE` | 18 |
| Exclusion | `D2D1_BLEND_MODE_EXCLUSION` | 19 |
| Hue | Private Source kernel through M3c | — |
| Saturation | Private Source kernel through M3c | — |
| Color | Private Source kernel through M3c | — |
| Luminosity | Private Source kernel through M3c | — |

`paint::blend(context, background, foreground, mode, location)` replaces the former multiply-only helper. Its match is exhaustive over the 16 root-declared variants. D2D has no `D2D1_BLEND_MODE_NORMAL`; the Normal arm uses `record` and two ordered `draw_image` calls. Eleven separable modes create `CLSID_D2D1Blend`, write the matched enum integer to `D2D1_BLEND_PROP_MODE`, set background as input 0 and foreground as input 1, and obtain the output. Root's independent comparison rejected the original built-in nonseparable effects with 24 differing samples; the final four arms dispatch to the private M3c Source shader. Native HRESULT forwarding and input order remain required. Source expectations were not changed to accept the original failure.

`NativeRenderer::compose` retains its Normal batching: ordered Normal images accumulate, and `normal_batch` draws the prior background followed by that batch. On each non-Normal node it flushes the batch, calls the shared `blend` helper with the completed background and that node's image, then continues in input order. This keeps the same flush and accumulation sequence that previously handled Multiply, now for every other resolved mode. The renderer's existing group recording, opacity, clip and resource lifetimes are untouched.

Native API issue and acceptance limit: matching Direct2D enum names does not itself establish pixel equivalence to the pinned usvg/resvg blend formulas, especially under alpha, color-space conversion, opacity and nonseparable hue/saturation/color/luminosity modes. Root must compare controlled WARP pixels against the independent Source oracle. This packet does not implement or claim group auto/isolate semantics, COLR preparation, font SVG resources, full renderer parity or Folia integration. Per contract, no build, tests, oracle, expected-pixel changes or commit were made here.
