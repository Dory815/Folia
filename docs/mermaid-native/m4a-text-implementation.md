# M4a text presentation submitted for root review

Sole implementation file is
`third_party/mermaid-rs-renderer/src/scene/compile/text_helpers.rs`; this report is
the only other file written. The helper consumes root's unchanged
`text_presentation.rs` schema. Authority is pristine mermaid-rs-renderer 0.3.1
render.rs SHA256
`026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4`.

| Complete source member | New public member | Preserved obligation |
| --- | --- | --- |
| normalize_font_family, 2207–2219 | normalize_font_family | Exact split/trim/single-quote-trim/double-quote-trim/filter/join and sans-serif fallback. |
| render_gitgraph_multiline_text, 4378–4409 | gitgraph_multiline_text | Newline-only split, parent y+size, Start anchor, repeated x, zero/relative dy, empty lines; Literal color and anchor. |
| text_block_svg, 4411–4431 | text_block | Source delegation, theme size, Middle anchor, false baseline and unused edge flag. |
| text_block_svg_anchor, 4433–4453 | text_block_anchor | Source delegation with supplied anchor and false baseline. |
| text_block_svg_with_font_size, 4455–4499 | text_block_with_font_size | Count-based initial y, supplied size, ordered repeated-x spans, divider suppression and retained empty parent for zero lines. |
| text_block_svg_with_font_size_weight, 4501–4550 | text_block_with_font_size_weight | Same numeric/content rules, omit trim-empty weight while retaining untrimmed nonempty lexical weight. |
| text_line_svg_with_font_size, 4552–4568 | text_line_with_font_size | Direct literal text content, explicit size/anchor/fill, no spans or dy. |
| text_line_svg, 4570–4578 | text_line | Same direct presentation with theme size. |
| c4_text_svg, 4959–4983 | c4_text | One element per line, raw CSS family, semantic escaped weight/fill, CSS px size, parent Middle plus child Mathematical baseline, absent span x, optional italic only when true. |
| is_divider_line, 5431–5433 | is_divider_line | Exactly Rust trim()=="---". |

All eight element helpers retain source signatures/argument order and return
Result<Vec<TextPresentation>, SceneError>. Single-element helpers return exactly
one element even for zero-line ordinary blocks; C4 returns an empty vector when
there are zero supplied lines. No SceneNode is created.

All source arithmetic remains f32 before each emission site. Positions and dy
use two-decimal formatting/parsing; font sizes use default f32 Display/parsing.
Signed zero is retained. Nonfinite emitted numbers produce InvalidNumber, with
no new positive-size constraint or validation of unused values. In particular,
zero-line C4 emits nothing and accepts otherwise unused nonfinite numeric
inputs. A baseline=true block does not reject an unused nonfinite total-height
intermediate; a single-span block/Git helper does not reject unused line-height
when only zero dy is emitted. Zero-line ordinary blocks still evaluate their
source parent fields and preserve the source's empty text element.

Characters remain literal original Unicode with no escape/unescape/collapse.
Ordinary family/anchor/fill/weight preserve AttributeLexeme provenance. Git fill
and anchor are Literal, its family AttributeLexeme. C4 preserves C4Style with
normalized raw CSS family and semantic weight, Literal fill/anchor, parent
dominant middle and child alignment mathematical. Baseline Boolean changes
only ordinary parent y; it emits no baseline fields. Empty/divider spans remain
ordered requests for later common resolution, not invented absolute lines.

No prerequisite/schema ambiguity found within this frozen packet. Still open:
the single shared lexical/entity/attribute/CSS resolver (including invalid
markup and CSS injection), inherited defaults and whitespace/chunk lowering,
font database/selection/fallback policy, pinned typed usvg API, bidi/shaping and
variable-font preparation, nonrendering/color/SVG-font/COLR/raster appearances,
final immutable scene conversion and export semantics. Class/ER presentation,
measurements and source family callers remain later packets. SVG/native/Folia
release acceptance remains closed.

No builds, tests, oracle changes, mutations or commits were run. No schema,
exports, dependencies, render.rs/model/theme, backend, Folia or expectation files
were edited. Root owns independent observations, tests, review and acceptance;
this report makes no completed rendering/parity claim.
