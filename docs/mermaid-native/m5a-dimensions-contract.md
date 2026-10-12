# M5a Root: one shared diagram measurement

The user requires Luna for Mermaid delegation. This is a small extraction on
the direct parser/layout -> completed scene -> SVG/Direct2D route, governed by
the scope correction in architecture.md. No raster-engine work belongs here.

Root assigns only the following write scope to Luna:

- new src/scene/compile/dimensions.rs;
- src/scene/compile/mod.rs: export the new dimensions module;
- src/render.rs: replace the existing dimension definition/helper/body with
  a re-export and one thin delegating wrapper;
- docs/mermaid-native/evidence/m5a-dimensions-implementation.md.

All source paths above are in third_party/mermaid-rs-renderer. Do not change
other rendering decisions, public entry routing, manifests, tests, expected
records, text preparation, Direct2D, Folia or previously accepted leaf helpers.
Do not build, test, commit or spawn. Root independently reviews and validates.

The shared public type is DiagramDimensions, with exactly the six original
f32 fields and the original aspect_ratio / viewbox_aspect_ratio methods. Retain
Debug, Clone, Copy, PartialEq and Serialize. The shared public function is:

    measure_layout_dimensions(&Layout, &LayoutConfig, Option<(f32,f32)>)
        -> DiagramDimensions

Move the original fit_dimensions_to_preferred_ratio helper, four sequence
padding constants and measure_svg_dimensions algorithm without numeric or
branch changes. Do not cast to f64, sanitize previously accepted raw values,
alter ordering or merge kind/data branch conditions. The completed scene
compiler will convert the final measurement into SceneDimensions and resolve
the root viewport transform separately; that conversion is outside this packet.

The SVG-facing public API remains the same through:

    pub use crate::scene::compile::dimensions::DiagramDimensions as SvgDimensions;
    pub fn measure_svg_dimensions(...) -> SvgDimensions {
        crate::scene::compile::dimensions::measure_layout_dimensions(...)
    }

This is one measurement implementation with an existing public name alias,
not a second sizing model or compatibility fallback. Existing SVG rendering
calls the wrapper. Other sequence constant uses, if present, must import the
same shared constants with crate visibility instead of duplicating values.

Root acceptance compares the extracted type/methods/helper/constants/body
against the pre-extraction source (allowing only the specified names/imports),
builds the portable library and runs existing renderer tests. This accepts only
the shared measurement extraction, not complete scene compilation, text,
SVG/Direct2D parity or Folia integration.
