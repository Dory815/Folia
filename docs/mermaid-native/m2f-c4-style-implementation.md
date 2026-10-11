# M2f C4 style selectors — awaiting root acceptance

Implemented four complete source selectors in
`third_party/mermaid-rs-renderer/src/scene/compile/c4_style.rs`, using existing
`C4Config` and all 20 existing `C4ShapeKind` variants. Authority is pristine
`mermaid-rs-renderer` 0.3.1 `render.rs` SHA256
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.

| Function | Pristine source | New module |
| --- | ---: | ---: |
| `c4_shape_colors` | 4986–5057 | 6–77 |
| `c4_shape_font_family` | 5059–5086 | 79–106 |
| `c4_shape_font_size` | 5088–5111 | 108–131 |
| `c4_shape_font_weight` | 5113–5140 | 133–160 |

The 155 source lines map one for one to module lines 6–160. A line-by-line
comparison found no differences after making the four functions public and
replacing fully qualified `crate::config::C4Config` and
`crate::ir::C4ShapeKind` spellings with imports. Every branch returns its
corresponding live config field. The color, family and weight functions return
borrowed `&str` values; font size returns the original `f32` value without
arithmetic, conversion or validation. No value is trimmed, parsed, normalized,
cached or defaulted here.

API gaps: `scene/compile/mod.rs` does not yet export `c4_style`, so callers
cannot access these public functions until root adds that export. C4 scene
orchestration has not been connected to them, and the shared paint/font/text
resolver remains separate. The existing config and enum provide all required
fields and variants; no further type change was needed.

Root owns independent source expectations, tests, builds, review and local
acceptance. I did not edit exports, manifests, source rendering, tests or
backends, and did not build, test or commit. This submission does not establish
a completed C4 scene or SVG/Direct2D backend parity.
