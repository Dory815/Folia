# M4b read-only audit — typed input to pinned usvg text

Root review: checked the existing 12-file source hash manifest and Folia
adapter hash against the local pinned registry source. This report's private
field and narrow-facade findings inform the next interface packet; no proposed
API is frozen here. The existing COLR synthetic-SVG branch remains a required
replacement before native text can pass its no-SVG-bridge gate.

Status: proposed boundary for root to freeze, **not** an implementation or
shared-text acceptance. I inspected the cached `usvg` 0.47.0 source identified
by `shared-text-source-audit.json`; its registry archive SHA256 is
`d46cf96c5f498d36b7a9693bc6a7075c0bb9303189d61b2249b0dc3d309c07de`.
The file identities below are from that pinned source. Only this audit document
was written; no fork, manifest, test, build, or integration changed.

| Source file under `usvg-0.47.0/src` | SHA256 | Relevant lines |
| --- | --- | --- |
| `tree/text.rs` | `0bca06ea94d0a4a78809c32d75c448317ad3c7d413253f99cad5a55d820802f5` | 144–179, 184–459, 467–591 |
| `parser/text.rs` | `15843b5d4625e490ecdfcbcfd1f71372d6c5ea405f3f7aaff7b262549eebee53` | 89–363, 365–475, 481–563, 585–630, 692–909 |
| `parser/svgtree/text.rs` | `418b73948d1a819a8f3bf09f0a7f3c139707080a8cc76ef863d7c79402b25a41` | 12–101, 164–361 |
| `text/mod.rs` | `74bfea110c4622996573aeec7bc50b94183a7f8363f19e68e887f4a25b61b906` | 18–75, 85–189, 198–216 |
| `text/layout.rs` | `b626ca027f12b279b30df600a5a200ed30c0b3328d152176995665bdcc27b571` | 28–171, 199–380, 517–624, 859–1142, 1293–1515, 1587–1804 |
| `text/flatten.rs` | `a10db150354fea8d8619a6c22a03e7aaa612c652efd3e8143ad67c4a6b25bb82` | 16–180, 210–415 |
| `parser/paint_server.rs` | `b268ffb743931c240962b1cd4296b332b12bef14283de1d2aee123072c9003ee` | 554–758, 760–945 |
| `parser/converter.rs` | `178ce9e0b90a9e31fe0ff09e8ad5883daf5566da7dede5499ac9d0f927ad5938` | 29–127, 348–467 |
| `parser/options.rs` | `d8d43effeb3092c1ff9d3f53a4b04e6c9a9a144df4325b602639e82be799a1d6` | 13–120, 126–143 |
| `tree/mod.rs` | `cb11738534ce989b8583827d55e3e01096831ad40075f7c8351365da7ece35b5` | 23–42, 1026 onward, 1584–1672 |

## Exact source object and construction sequence

`Font` stores ordered `Vec<svgtypes::FontFamily>`, `FontStyle`, `FontStretch`,
`u16` weight and ordered `Vec<FontVariation { tag: [u8;4], value: f32 }>`;
all fields are `pub(crate)` (`tree/text.rs:144–149`). `TextDecorationStyle`
contains optional `Fill`/`Stroke`; `TextDecoration` contains separately styled
underline, overline and line-through (`285–325`). A `TextSpan` holds **chunk-local
UTF-8 byte** `start/end`, optional fill/stroke, paint order, `Font`,
`NonZeroPositiveF32` size, small caps, kerning flag, optical-sizing mode,
decoration, dominant/alignment baselines, an ordered baseline-shift vector,
visibility, letter/word spacing, optional textLength and lengthAdjust
(`331–350`). These fields cannot be initialized from an external crate.

`TextChunk` has optional absolute x/y, anchor, nonoverlapping spans, `TextFlow`
(`Linear` or `Path(Arc<TextPath>)`) and owned text (`tree/text.rs:481–529`).
`TextPath` has a private nonempty id, already resolved start offset and owned
path. `Text` stores id, rendering mode, per-Unicode-scalar dx/dy/rotate,
writing mode, chunks, absolute transform, four bounding boxes, flattened
group and feature-gated layouted spans (`576–591`). All of those fields are
`pub(crate)`; public methods at `594–716` only read them.

The parser constructor performs these steps in order (`parser/text.rs:100–147`):

1. Resolve x/y/dx/dy per descendant scalar, then rotate, then writing mode;
   collect chunks; choose text-rendering and id (empty inside markers).
2. Make `Rect::from_xywh(0,0,0,0)` and initialize all four bounds with that
   dummy; initialize `flattened=Box::new(Group::empty())` and `layouted=[]`;
   set `abs_transform=parent.abs_transform`.
3. Call `text::convert(&mut text, &state.opt.font_resolver, cache)`. On `None`,
   drop this text node. On `Some`, append `Node::Text` to the parent group.

The parser collector is also substantive. It skips invisible descendants and
invalid/nonpositive font-size spans while still advancing its source character
counter (`parser/text.rs:184–235`). For each text child, it resolves source
font/paint/baseline/spacing fields before iterating characters (`226–304`). A
new chunk starts at the first scalar, at a scalar with absolute x or y, or
around textPath; a new style span starts at the first scalar from a new text
child; later characters extend its byte end (`306–361`). Empty tspans create
neither chunk nor dy movement. Position lists clamp each descendant attribute
list to that descendant's scalar count, and nested values override earlier
ones (`692–737`). Rotate has different last-value propagation (`748–774`).
`svgtree/text.rs:12–37,164–361` first normalizes XML whitespace across nested
text nodes. Typed input must arrive **after equivalent lexical/entity/CSS,
unit and whitespace resolution**, or the fork adapter must explicitly reproduce
those semantics. Passing raw Mermaid strings straight into `TextChunk` would
skip source behavior.

## Semantics the typed boundary must carry

The input must preserve, in source order: normalized descendant text and
source-scalar provenance; the exact chunk boundaries; chunk optional x/y,
anchor and linear/path flow (resolved path id, transform and start offset if
used); chunk-local UTF-8 span ranges; all `TextSpan` properties above; text id,
rendering mode, writing mode, absolute transform and global per-scalar dx/dy/
rotate. The global position arrays may include characters skipped by the
parser collector, while `layout_text` advances `char_offset` by retained chunk
text (`parser/text.rs:221–235,359–360`; `text/layout.rs:218–232,366`); preserve
the constructed arrays and chunks rather than repairing that source behavior.
Validate scalar count, every UTF-8 boundary, span ordering/coverage and finite
resolved values before constructing the internal types. Preserve source skip
outcomes for invalid font size/empty text; do not invent a visible placeholder.

Font conversion is more than a family string: parser/text.rs:397–475 resolves
style/stretch/weight, explicit variation axes, then conditionally appends wght,
wdth, ital and slnt mappings, and falls back to `Options::font_family` if the
family parse is empty. `FontOpticalSizing::Auto` is the default
(`tree/text.rs:260–275`); shaping adds opsz only when the face has that axis
and no explicit opsz was supplied (`text/layout.rs:1411–1445`). The typed
resolved input must retain **final ordered axes** or call an extracted shared
font-normalization routine, never append them a second time. The parser's
relative `bolder`/`lighter` weight rules are at `parser/text.rs:585–630`.

Baseline fields are independent: `NoChange` inherits parent dominant baseline
at `parser/text.rs:245–256`; nested baseline-shift values are collected
nearest-first, with all-Baseline entries removed (`829–866`), then applied in
reverse order (`text/layout.rs:591–624`). Font metric shifts for dominant and
alignment baselines remain in `ResolvedFont` (`layout.rs:1718–1784`). Layout
applies writing mode, letter spacing, word spacing, textLength adjustment,
anchor, relative shifts and rotation in that order (`layout.rs:223–273,
981–1142,545–580`). `textLength=SpacingAndGlyphs` and text-on-path retain
their distinct handling. These are source algorithms, not values to recompute
in Mermaid or a backend.

## Font ownership and unchanged conversion/postpass

`FontResolver` callbacks receive `&mut Arc<fontdb::Database>` and may append
fonts via `Arc::make_mut`; additions must be monotonic (`text/mod.rs:18–68`).
Default selection queries ordered families plus Serif fallback (`85–139`);
fallback scans database faces with an exclusion list and character check
(`147–189`). `layout_text` caches selected `Font -> Arc<ResolvedFont>` for the
text, then shapes each **whole chunk for each span** and merges clusters
(`layout.rs:199–216,859–979`). `shape_text` handles whole-run fallback,
missing glyphs, LTR paragraph bidi, variable axes and kerning/small-caps
features (`1293–1515`). The adapter must use the same `FontResolver` and the
same `Arc<Database>`/cache, and retain the final database with the result.

`converter::Cache::new(Arc<Database>)` owns the mutable database plus outline,
COLR, SVG, raster, opsz and paint-server caches (`parser/converter.rs:45–127`).
Normal SVG conversion starts `Tree.fontdb` and `Cache.fontdb` from
`Options.fontdb`, calls conversions, clears retained resource caches, runs the
paint postpass, then assigns the possibly enlarged `cache.fontdb` to the tree
(`348–460`). Folia's normalizer currently supplies Arial, a system-loaded
database and specific generic families (`src/app-winui/third_party/
usvg-normalizer/src/lib.rs:149–180`, hash
`a4c8e5d02e40b50bda733f2b86699bf4363bc67380209f55085f479832a7fbe2`).
Reusing only the original base `Options.fontdb` after a callback would leave
returned glyph font IDs unresolved. A typed session should own one cache and
publish its final `Arc<Database>` with the prepared text batch.

Keep `text::convert`'s order unchanged: `layout::layout_text` fills positioned
spans and metric bounds, then `flatten::flatten` builds appearances and stroke
bounds (`text/mod.rs:204–216`). `flatten` orders overline, underline, glyphs,
line-through and batches consecutive ordinary outlines into one path; COLR,
font-SVG, raster, outline priority and transforms are at `flatten.rs:48–180`.
The source COLR branch currently obtains a tree through an artificial SVG
serialization/reparse in `flatten.rs:378` and `text/colr.rs`; exposing
`convert` unchanged does **not** satisfy the eventual no-SVG-bridge text gate.
Font-embedded SVG glyphs are separate assets and need their own resource policy.

`parser/paint_server.rs::update_paint_servers(group: &mut Group,
context_transform: Transform, context_bbox: Option<Rect>, text_bbox:
Option<Rect>, cache: &mut Cache)` is already public **inside a private parser
module** (`554–560`). Its Text arm updates original chunks, positioned spans,
decoration paths and flattened paths using `text.bounding_box`
(`660–755`). `process_paint` resolves objectBoundingBox and context transforms
through the same cache (`887–924`). Calling the postpass only on flattened
glyph paths or using individual glyph ink boxes would change gradients. The
typed adapter can put its `Node::Text` in an internal `Group`, call this exact
postpass and extract the node; it must preserve the surrounding context
transform/bbox and resource identity supplied by shared scene preparation.

## Smallest fork seam proposed for root review

Add one public `typed_text` facade **inside** the checksum-pinned usvg fork.
It can access the existing `pub(crate)` fields and `converter::Cache` without
making `Text`, `TextSpan`, `Font`, `TextPath` or cache fields public. Keep the
source `layout_text`, `convert`, `flatten` and `update_paint_servers` bodies
authoritative. Proposed signatures, with input/output definitions frozen only
after root review:

```rust
pub struct TypedTextSession { /* owns converter::Cache and output provenance */ }
impl TypedTextSession {
    pub fn new(fontdb: Arc<fontdb::Database>) -> Self;
    pub fn prepare(
        &mut self,
        input: ResolvedTextInput,
        resolver: &FontResolver<'_>,
        paint_context: TypedPaintContext,
        trace: Option<&mut dyn TextTraceSink>,
    ) -> Result<TypedTextOutcome, TypedTextError>;
    pub fn into_fontdb(self) -> Arc<fontdb::Database>;
}
pub enum TypedTextOutcome { Prepared { text: Text, provenance: TextProvenance },
                            SourceDropped { stage: TextDropStage } }
```

`ResolvedTextInput` should own already resolved parser-equivalent chunks and
span styles plus the Text-level fields listed above; it must not contain XML
or Mermaid SVG. A fork-internal `build_text(input) -> Result<Text, ...>` should
perform the exact dummy-bound initialization from `parser/text.rs:123–141`.
If the root chooses to accept hierarchical typed fragments instead of already
split chunks, extract the character append/split core at
`parser/text.rs:306–361` into one crate-internal function used by both the SVG
parser and typed adapter; do not maintain two hand-written chunk algorithms.
No blanket widening of source field visibility or public exposure of
`converter::Cache`/`parser::State` is needed. The session can invoke the
unchanged `text::convert` and private-module paint postpass internally.

`TextTraceSink` should observe, without changing decisions: input scalar and
chunk/span byte ranges; primary font selection and each fallback request,
returned font ID and face identity; UTF-8 cluster start/end, glyph ID,
advance, offset, bidi level and final transform; skipped/missing characters;
variation list including automatic opsz; baseline/spacing/anchor adjustments;
appearance kind and ordered outline batching; layout/flatten/postpass drop
stage; metric and stroke bounds; paint server id, units and resolved bbox.
The current public `PositionedGlyph` exposes ID/text/font/transform but hides
glyph/cluster/span transforms and does not expose original cluster byte index,
advance or bidi level (`layout.rs:28–48`). `Glyph`, `GlyphCluster`, `ByteIndex`
and `ResolvedFont` are crate-private (`layout.rs:175–189,1587–1804`). Capture
provenance during shaping and cluster conversion, before these values are
discarded at `layout.rs:338–348`, rather than reconstructing it from rendered
glyph strings. Existing `Option<()>` failures in convert/layout/flatten lose
which stage failed; the wrapper needs a sidecar diagnostic path while retaining
the source's drop/omit behavior. Diagnostics should identify invalid typed
input separately from source-compatible no-font/empty-bbox omission and from
unsupported appearance or paint resolution; no partial scene should publish.

Open root decisions: exact owned `ResolvedTextInput` schema and whether chunk
splitting is done by the typed lowerer or the extracted shared collector;
how typed paint-server resources/context are supplied to the internal cache;
where a fully typed COLR painter replaces the source SVG bridge; how font-SVG
assets and output glyph provenance are retained in the portable scene. These
are required before implementation or full shared-text acceptance.
