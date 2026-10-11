# Root M2g — complete source label data helpers

Pinned render.rs SHA256:
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.
Root read complete `format_sankey_value` (1968–1979), `ErAttribute` and
`parse_er_attributes` (5465–5533). These are presentation data responsibilities,
separate from text shaping, diagram layout, scene painting and backends.

Sol mermaid_geometry owns only
`third_party/mermaid-rs-renderer/src/scene/compile/sankey_labels.rs`,
`third_party/mermaid-rs-renderer/src/scene/compile/er_attributes.rs`, and
`docs/mermaid-native/m2g-label-data-implementation.md`. Root owns exports,
manifests, source oracle, tests/mutations, acceptance and Folia integration.
No source render/config/IR change, schema/backend edit, build, commit or
additional delegation is authorized for this packet.

```rust
pub fn format_sankey_value(value: f32) -> String;
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ErAttribute {
    pub name: String,
    pub data_type: String,
    pub keys: Vec<String>,
}
pub fn parse_er_attributes(lines: &[String]) -> (String, Vec<ErAttribute>);
```

Port both complete Source bodies. Sankey preserves the exact f32 operation
order, round behavior, strict 0.001 comparisons and 0/1/2 decimal formatting,
including signed zero, nonfinite values and overflow. Do not trim trailing
zeros, clamp, prevalidate or switch to f64. ER preserves title replacement
before the first divider, all dividers, Rust Unicode trim/split_whitespace,
ASCII-only uppercase, end comma/semicolon stripping, recognized PK/FK/UK
ordering and duplicates, and the Source rule dropping an entire comma token
if any recognized key piece occurs. Parts retain their original token spelling.
Import the accepted `super::text_helpers::is_divider_line`; do not duplicate it.
No Markdown parser, key sorter/deduplicator, synthesized name/type or SVG path.

Root acceptance executes the unchanged extracted functions/struct from the
pristine registry source; fixtures never derive expected values from native
code. Compare exact strings and all ER fields/order. Cover threshold and
rounding neighbors, random f32 bit patterns, nonfinite/zero values, empty and
missing bodies, multiple titles/dividers, Unicode whitespace and keys,
mixed punctuation, comma groups, duplicate keys, and raw lexemes. Require
compiling detected mutants, focused tests and the existing native WARP matrix.
This packet accepts two additional source functions only, not complete ER or
Sankey diagrams, text preparation, shared scenes or either backend.
