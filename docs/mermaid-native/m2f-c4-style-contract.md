# Root M2f — complete C4 source style selectors

Root read render.rs 4986–5140, C4Config's fields and C4ShapeKind's complete
20 variants. Pinned render.rs SHA256 is
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
This packet supplies the source style inputs to C4 scene compilation, alongside
accepted M4a text presentation. C4 geometry/orchestration and shared text
preparation remain separate unaccepted responsibilities.

Sol mermaid_geometry owns only
third_party/mermaid-rs-renderer/src/scene/compile/c4_style.rs and
docs/mermaid-native/m2f-c4-style-implementation.md. Root owns exports, build
manifests, source oracle, tests/mutations, review, acceptance and Folia wiring.
No changes to source config/IR/render.rs, scene schemas/backends, dependencies,
tests or unrelated files. No builds, commits or delegation. Report API gaps.

Implement four complete public functions with exactly the source names and
signatures, using existing crate::config::C4Config and crate::ir::C4ShapeKind:

```rust
pub fn c4_shape_colors(conf: &C4Config, kind: C4ShapeKind) -> (&str, &str);
pub fn c4_shape_font_family(conf: &C4Config, kind: C4ShapeKind) -> &str;
pub fn c4_shape_font_size(conf: &C4Config, kind: C4ShapeKind) -> f32;
pub fn c4_shape_font_weight(conf: &C4Config, kind: C4ShapeKind) -> &str;
```

Every variant maps to its exact five corresponding config fields: background,
border, font family, font size, font weight. Return borrowed string identities
and the original f32 bit pattern. No trimming, CSS/paint/font parsing, default
selection, validation, quantization or rejection of nonfinite/nonpositive
values here: the source helpers only select fields. Unused config values have
no effect. Read live config on every call; do not cache or derive external/DB/
queue styles from their parent variant. No SVG/XML/path serialization/parsing,
font lookup, wrapping or layout. The shared resolver owns interpretation later.

Root independent acceptance executes the four unchanged source Rust functions
extracted from the pristine pinned source. Compare all 20 variants over default
and uniquely marked configs, isolated field changes, Unicode/whitespace/raw
lexemes and exact f32 edge payloads (including NaN, infinities and signed zero).
Check returned strings borrow the selected fields and reflect subsequent config
changes. Require compiling field-routing/normalization/numeric mutants and the
existing native primitive/WARP matrix before local acceptance. This packet
does not accept a C4 diagram, a completed scene or either full backend.
