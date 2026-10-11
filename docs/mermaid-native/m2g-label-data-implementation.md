# M2g source label data helpers — implementation report

Status: **awaiting root's independent acceptance**. This packet adds only two complete source functions and the public three-field `ErAttribute` data type. It does not integrate ER/Sankey rendering, text preparation, typed scenes, or a backend.

Pinned source: `.reference/mermaid-registry/mermaid-rs-renderer-0.3.1/src/render.rs`, SHA-256 `026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`. Frozen contract: `docs/mermaid-native/m2g-label-data-contract.md`.

| Source site | New native site | Exact operation preserved |
| --- | --- | --- |
| `render.rs:1968–1979`, `format_sankey_value(value: f32) -> String` | `third_party/mermaid-rs-renderer/src/scene/compile/sankey_labels.rs` | `(value * 100.0).round() / 100.0`; compare the absolute difference from `rounded_2.round()` strictly below `0.001` and format with `.0`; otherwise compute `(value * 10.0).round() / 10.0`, compare its difference from rounded_2 strictly below `0.001`, choose `.1` or `.2`. Original f32 operations and order, native Rust `format!`, signed zero, nonfinite and overflow behavior remain unchanged. |
| `render.rs:5465–5470`, `ErAttribute` | `third_party/mermaid-rs-renderer/src/scene/compile/er_attributes.rs` | The source fields `name: String`, `data_type: String`, `keys: Vec<String>` stay in their original order. The contract exposes them publicly and adds `PartialEq, Eq` to `Debug, Clone` for exact-field comparison; those are the only intentional declaration differences. |
| `render.rs:5472–5485`, title/body transition | `er_attributes.rs::parse_er_attributes` | First line trimmed or empty default; iterate from the second line; every divider enters/continues body; before the first divider, each nonblank trimmed line replaces the title. |
| `render.rs:5486–5519`, body token classification | Same function | Skip blank trimmed lines; `split_whitespace` with Rust Unicode semantics; strip only leading/trailing comma/semicolon before ASCII uppercase; collect PK/FK/UK in encounter order, including duplicates. For comma-containing uppercase token, collect recognized pieces and drop the **whole** original token when any is recognized. Unrecognized tokens retain their original, uncleaned spelling in `parts`. |
| `render.rs:5520–5533`, attribute construction | Same function | Drop rows with no parts; first part is `data_type` and remaining parts joined by one space form `name`; a one-part row has empty `data_type`; push the original key order; return `(title, attrs)`. |
| `render.rs:5431–5433`, `is_divider_line` | Existing `super::text_helpers::is_divider_line` | Both use `line.trim() == "---"`; no duplicate helper was introduced. |

No input normalization, f64 conversion, key deduplication/sorting, Markdown parsing, synthetic name/type, or SVG output was added. The new files are not exported or integrated by this packet; root owns those boundaries and the unchanged-source oracle, field/output comparisons, compiling mutation checks, focused tests, WARP matrix, and acceptance. Per contract, no build or test was run and no commit was made.

Question for root when integrating: the current `scene/compile` module must decide its eventual exports and call sites. This packet deliberately leaves `mod.rs` untouched. There is no known source-operation gap inside the two copied bodies; any mismatch revealed by the independent oracle should be resolved against the pinned source before acceptance.
