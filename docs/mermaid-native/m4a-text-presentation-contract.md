# Root M4a — source text presentation before shared font preparation

Pinned render.rs SHA256 026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
Root read the complete eight helpers at 4378–4578 and 4959–4983, plus
normalize_font_family (2207–2220) and is_divider_line (5431–5433).
This packet preserves their ten complete responsibilities, not shaping,
font selection, class/ER layout or a completed scene compiler.

## Ownership

Root owns text_presentation.rs/schema, exports, reference oracle, tests,
mutations, builds and acceptance. Sol may edit only
src/scene/compile/text_helpers.rs in the vendored renderer and
docs/mermaid-native/m4a-text-implementation.md. No render.rs/model/theme,
scene schema, backend, dependency/lock, test or production wiring edits;
no commits, builds or delegation. Report every source helper separately.

## Exact API and responsibilities

All functions are public. Inputs retain source signatures and argument order,
using crate::layout::TextBlock, crate::theme::Theme and LayoutConfig.
Rename source functions as follows (drop svg, retain their source arguments):
render_gitgraph_multiline_text -> gitgraph_multiline_text;
text_block_svg -> text_block;
text_block_svg_anchor -> text_block_anchor;
text_block_svg_with_font_size -> text_block_with_font_size;
text_block_svg_with_font_size_weight -> text_block_with_font_size_weight;
text_line_svg_with_font_size -> text_line_with_font_size;
text_line_svg -> text_line;
c4_text_svg -> c4_text.
Each returns Result<Vec<TextPresentation>, SceneError>. Single-source-element
helpers return a one-element vector, including zero-line ordinary blocks.
C4 returns one element per line, or an empty vector for zero lines.
normalize_font_family(&str)->String and is_divider_line(&str)->bool retain names.

Only scalar formatting/parsing is allowed. No SVG/XML/path creation or parser,
font shaping, diagram placement or SceneNode production. These requests feed
the one future common preparation stage, which produces the sole immutable
scene. They are never an alternative geometry path for either backend.

Preserve source f32 arithmetic order before every individual .2 decimal site.
Use f64 parsed from that decimal, or source f32 default Display for font size.
Reject nonfinite *emitted* numbers with InvalidNumber; source unused arguments
and zero-line C4 produce no synthetic errors. No added positive-size policy.
Preserve signed zero, original span order and empty/divider spans. Ordinary
zero-line blocks still produce an empty text element: the source has no guard.
Git splits only newline, including empty leading/trailing lines. Ordinary
dividers are exactly Rust trim()=="---", suppressed to empty content.
baseline bool changes only parent y; it does not set baseline attributes.
The _edge argument is unused. Weight omission checks trim but retains original
untrimmed weight. Use the exact source family normalization chain and fallback.

Text content is literal original Unicode, with no escape/unescape or whitespace
collapse. TextValue::Literal is for escaped source attribute strings;
AttributeLexeme is for the source's raw unescaped insertion. All ordinary
family/anchor/fill/weight are AttributeLexeme; Git fill is Literal, Git anchor
Literal("start"), family AttributeLexeme. C4 fill is Literal, anchor
Literal("middle"), C4Style retains its source raw CSS family and semantic
escaped weight, font-size in CSS px, with optional italic only when true.
Do not interpret paint/CSS strings here. The common resolver must later handle
entities, whitespace, CSS declaration injection and invalid attribute syntax;
local acceptance here makes no claim that those unresolved inputs render.
No source invalid markup is silently treated as a valid final scene.

Ordinary/Git attributes font-size uses Display. C4 font-size also Display;
source parent x/y and span dy use .2. C4 retains parent dominant middle AND
child alignment mathematical; span x absent. Ordinary/Git span x present and
baseline fields absent. text_line has Direct content and no spans/dy.

## Independent root acceptance

Extract unchanged helpers from the pristine pinned Rust source into an oracle
crate, run actual source emissions, and compare ordered elements, attribute
presence/provenance, numeric emission, CSS versus attribute fonts and literal
text/spans using independent XML observation. Include zero lines, blank lines,
Unicode/whitespace, divider variants, family quote chains, empty/raw weights,
baseline/anchor/color variants and boundary/overflow numeric inputs. Build
compiling deliberate mutants; they must fail unchanged assertions. Run focused
portable tests and existing relevant geometry tests before local acceptance.
Full text/font/COLR/SVG/D2D and Folia release gates remain closed.
