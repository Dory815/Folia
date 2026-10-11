# Root M4a local acceptance — ten complete text presentation helpers

Root reviewed every line of render.rs::normalize_font_family,
render_gitgraph_multiline_text, text_block_svg, text_block_svg_anchor,
text_block_svg_with_font_size, text_block_svg_with_font_size_weight,
text_line_svg_with_font_size, text_line_svg, c4_text_svg and is_divider_line
against Sol's implementation and the root-owned typed presentation schema.
The pinned render.rs SHA256 is
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
Each complete extracted function hash is recorded in the reference fixture;
the reference also executes unchanged escape_xml, not a native implementation.

One precursor request model feeds the future shared preparation stage. It
retains source f32 arithmetic and scalar decimal emission, optional attributes,
ordered direct text/spans, empty/divider lines, source raw lexical versus
escaped attribute values, ordinary attribute fonts versus C4 CSS fonts,
dominant/alignment baselines, raw untrimmed weights and italic presence.
It does not produce a second geometry model consumed by either backend.
No SVG/XML is constructed or parsed in the runtime helpers; scalar decimal
format/parse is the only serialization there. Original character content is
retained until the common resolver applies source-compatible normalization.

Independent actual-source Rust oracle: 400 cases, 369 valid ordered DOM
observations and 31 nonfinite-emission errors. Every invalid observation has
the independent diagnostic "nonfinite source emission"; native helpers return
InvalidNumber. XML/DOM decoding occurs only in the test and reference crates.
The portable test compares parent/child order, content, exact numeric emission,
all attributes and omissions, CSS font units/style suffix, normalization and
divider decisions. Additional assertions verify typed lexical provenance,
direct-versus-spans ownership, repeated/absent span x, and all baseline fields.
Malformed lexical strings are retained as unresolved requests in a separate
native boundary witness; this is not acceptance of their final rendering.

The matrix includes zero/one/multiple lines; trailing/leading empties; Unicode,
entities, combining text, bidi characters and line endings; quote/whitespace
family chains; blank and untrimmed weights; independent explicit/theme sizes;
baseline/anchor/color/italic/edge variants; signed zero, rounding boundaries,
large/negative values, overflow and unused nonfinite intermediates. Ordinary
zero-line blocks emit an empty parent; zero-line C4 emits nothing and checks
no unused coordinates. Root independently corrected the earlier audit's claim
that ordinary zero-line blocks were omitted before implementation dispatch.

24 isolated deliberate mutants compiled and failed unchanged assertions
(Cargo exit 101 with explicit FAILED assertion results, no crash). These cover
all ten responsibilities and source omissions, forwarding, whitespace, signed
decimal precision, default-display versus binary widening, baselines and typed
provenance. Source/fixture bytes were verified unchanged after the run. Exact
results: m4a-text-presentation-mutations.json; ignored build directory contains
individual compile/test logs. --check reproduced the complete source fixture.

Final relevant matrix: 31 tests passed, zero failures/ignored, including 10
actual Direct2D WARP tests plus scene, path, edge, node-shape and text checks.
WARP uses the pinned Arial font SHA256
b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a;
the first combined run omitted its required environment variable, then the
complete matrix was rerun with the hash-checked fixture and exited 0.
Portable text tests also independently passed with direct2d disabled.
Only roxmltree 0.21.1 was added as a test dependency; the existing package lock
changes by one root dependency entry, with no runtime dependency upgrades.

This advances locally accepted render responsibilities from 7/70 to 17/70.
Class/ER/mixed painter emission, shared XML/entity/CSS interpretation and
inherited defaults, font selection/fallback, bidi/shaping, variable fonts,
COLR/font SVG/raster appearances, immutable scene completion and SVG export
still require their own implementation and source/native acceptance. The
existing Direct2D consumer cannot yet consume these raw text requests.
No full text/backend/Folia gate is passed and Folia has not switched rendering.
