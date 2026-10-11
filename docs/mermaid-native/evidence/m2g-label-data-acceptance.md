# M2g root local acceptance — complete ER and Sankey label data

Root accepts complete `format_sankey_value` and `parse_er_attributes` from
pinned mermaid-rs-renderer 0.3.1 render.rs, SHA256
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.
Root froze `../m2g-label-data-contract.md` before Sol implementation, read all
bodies and independently compared native function text with unchanged Source:
the only difference is public visibility. ErAttribute adds public fields and
comparison derives; its three fields/order/data are unchanged. ER imports the
already accepted `is_divider_line`, which is not counted a second time.

`label-data-oracle.cjs` extracts and compiles the two unchanged Source functions,
Source struct and divider helper from the pristine pinned source. The isolated
reference crate imports no native implementation. Source hash/extraction hash
are recorded; --locked --offline and oracle --check reproduce the fixture.
The oracle supplies **2,790 Source cases**:

- **2,247 Sankey cases**: signed zeros, subnormals, infinities/NaNs, largest
  finite values and overflow, seven-neighbor bit ranges around rounding and
  branch boundaries, plus 2,048 deterministic arbitrary f32 payloads;
- **543 ER cases**: empty lines/bodies, title replacement, divider position,
  a first line that is itself a divider, repeated dividers, Unicode whitespace,
  punctuation and raw lexemes, ASCII case, mixed comma pieces, duplicate and
  ordered PK/FK/UK tokens, multiple attributes and title/body boundary cases.

The native test compares exact formatted strings, title, attribute count and
each name/type/key sequence. All **4,201 assertions** pass. Source output is the
only expected-value authority. Mutation review exposed missing first-line
divider and repeated-key cases; root added Source stimuli and regenerated
expectations, without changing implementation or inventing expected values.

All **25** independent isolated mutants compile and fail the unchanged Source
assertions with Rust test exit 101. They cover Sankey branching/rounding/sign/
precision and ER title/body/divider/case/punctuation/comma/duplicates/key/name/
attribute order. Build failures, timeout and OS crashes do not count. Working
source/manifests/fixtures are byte guarded. Results are
`m2g-label-data-mutations.json`; run/build logs stay in ignored directories.

Root ran the explicit ten native Rust targets with --locked --offline,
--no-default-features --features direct2d: scene_contracts, path_parity,
edge_symbols_parity, node_shapes_parity, text_presentation_parity,
edge_label_geometry_parity, architecture_icons_parity, c4_style_parity,
label_data_parity and direct2d_warp. **36 tests pass**, including all 10 actual
WARP tests. Arial font SHA256 remains
`b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a`.
Ignored matrix log: `build/m2g-mermaid-matrix.log`. The reference crate's own
lockfile is committed; the vendor dependency lock did not change.

Local function acceptance increases **32/70 → 34/70**. Complete ER/Sankey
orchestration, all diagram scenes, shared resolved text/font/COLR preparation,
SVG and Direct2D backend parity and Folia integration remain open. The original
upstream library target still lacks its 13 packaged comparison fixtures; the
explicit native matrix is not a claim that the whole upstream target ran.
The Folia Mermaid production drawing route remains unchanged.
