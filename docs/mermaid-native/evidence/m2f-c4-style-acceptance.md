# M2f root local acceptance — complete C4 style selectors

Root accepts four complete source helpers c4_shape_colors,
c4_shape_font_family, c4_shape_font_size and c4_shape_font_weight from pinned
mermaid-rs-renderer 0.3.1 render.rs, SHA256
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
Root read their complete bodies and C4Config/C4ShapeKind, froze
m2f-c4-style-contract.md before Sol implementation, and independently verified
native bodies differ only in public visibility and qualified type spellings.
All 20 enum variants select their exact 100 config fields. No source config/IR/
layout/render.rs changes, normalization, SVG parsing or new layout was added.

The source oracle executes all four unchanged Rust functions extracted from the
pristine source, with pinned config/IR types. Its 2,380 observations cover:

- 20 default and 20 distinctly marked configurations;
- 2,000 isolated field mutations (100 fields across every 20-variant query);
- 160 raw string cases retaining whitespace, entities, quotes/CSS punctuation,
  NUL, Chinese/emoji and Arabic;
- 180 original f32 payload cases retaining positive/negative zero, signed
  subnormal values, infinities, positive/negative NaN payloads and largest finite.

Root test compares both colors, font family/weight and font-size bits exactly.
For 2,220 cases with distinguishable config storage, the actual source oracle
also identifies each returned string's original field by pointer/length; native
returns must borrow that exact config storage. Subsequent config updates must be
read live, including a new family and nonfinite font size. Shared paint/font/
text interpretation remains downstream; selectors do not sanitize raw input.
One test runs all 2,380 cases, with 34,421 successful assertions including the
case-count guard. The oracle --check reproduces fixtures and test stimuli.

c4-style-mutations.cjs independently built and detected all **107** mutants:
one wrong destination for every one of 100 selected fields, four string-trimming
mutants and three numeric normalization/rounding mutants. Every mutant compiled
and failed assertions with Rust test exit 101; compile errors and OS crashes
were excluded. Working source, manifests and fixtures were byte guarded and
unchanged. Results are m2f-c4-style-mutations.json; build/test logs are ignored.

Root ran the explicit nine native Rust targets with --locked --offline,
--no-default-features --features direct2d: scene_contracts, path_parity,
edge_symbols_parity, node_shapes_parity, text_presentation_parity,
edge_label_geometry_parity, architecture_icons_parity, c4_style_parity and
direct2d_warp. **35 tests passed**, including all 10 actual WARP tests. Font
fixture is the previously fingerprinted C:/Windows/Fonts/arial.ttf; no lock or
dependency changed. Ignored log: build/m2f-mermaid-matrix.log.
The restored registry archive still lacks the 13 comparison fixtures needed
by the broad upstream library test target; this does not claim that target ran.

This raises local function acceptance from 28/70 to 32/70. Complete C4 geometry/
orchestration, shared resolved text/font/COLR preparation, all diagram scenes,
full SVG/Direct2D backend parity and Folia integration remain open. The existing
Folia Mermaid production path is unchanged; this acceptance authorizes none of
those later gates by itself.
