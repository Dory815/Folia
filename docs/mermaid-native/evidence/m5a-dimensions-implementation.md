# M5a implementation notes — shared diagram measurement

Implemented only the four files authorized by `m5a-dimensions-contract.md`:

- Added `src/scene/compile/dimensions.rs` with the six-field `DiagramDimensions` type, its two original ratio methods, the original preferred-ratio fitting helper, all four sequence padding constants, and the original `measure_svg_dimensions` algorithm exposed as `measure_layout_dimensions`.
- Exported `dimensions` from `src/scene/compile/mod.rs`.
- In `src/render.rs`, replaced the former public type with the requested `SvgDimensions` re-export and replaced the original measurement body with a thin delegating wrapper. SVG emission still uses the same wrapper at its existing call site.

The algorithm was copied from Root's `build/m5a-render-before.rs` snapshot. The only semantic names/access changes are `SvgDimensions` → `DiagramDimensions`, `measure_svg_dimensions` → `measure_layout_dimensions`, and sequence constants made `pub(crate)` for shared reuse. No numeric constants, branch conditions, arithmetic, order, f32 types, or raw values were changed. The sequence padding constants were used only by this extracted measurement body in the pinned source; there were no other constant references to redirect.

No build or tests were run, per the contract. No acceptance claim is made; Root should independently compare the extraction and run its specified validation. No uncertainty remains about the scoped extraction; conversion from `DiagramDimensions` to final scene dimensions/root viewport remains outside M5a, as specified by the contract.
