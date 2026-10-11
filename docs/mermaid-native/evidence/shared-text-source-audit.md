# Root shared-text source audit and next architectural boundary

Status: independently reviewed source findings; no shared text implementation or
full text acceptance. Root and the user-requested Sol agent read pinned usvg
0.47.0 and Folia's current normalizer. Source identities are in the adjacent
shared-text-source-audit.json; the cached registry archive matches Folia's lock
checksum. This audit does not authorize replacing the font or scene contracts.

## Existing behavior that must survive

Mermaid's TextBlock is wrapped text plus measured dimensions. Its existing
text_metrics font lookup/approximations control diagram layout. Shaping later
must not feed different measurements back into that layout. The current Folia
normalizer separately shapes through usvg, with Arial default, the owned system
font database, generic Times New Roman/Arial/Comic Sans MS/Impact/Cascadia Code,
languages en/zh-CN, and default resolver callbacks. Its default WriteOptions
outlines text, with coordinate and transform precision 8. Thus Folia already
displays outlined labels; raw Mermaid `<text>` is an earlier source stage.

usvg shapes whole chunks across spans (text/layout.rs::shape_text), preserving
the source bidi, fallback, ligature and combining-cluster behavior. Shaping
each styled span separately would change that behavior. The source paragraph
bidi direction is explicitly LTR; language options are not directly passed as
shaping language. Cluster advance uses the source maximum glyph advance rather
than a sum. Font fallback order and the source whole-chunk replacement/partial
missing-glyph merge conditions must remain exact.

Font variation coordinates, including automatic optical size, and font metric
baseline calculations are source behavior. Mermaid normal/GitGraph multiline
helpers, C4 dominant/alignment baselines and dy/textLength spacing, class/ER
spans and badges, and diagram-specific direct text emissions all need explicit
resolved input lowering. Numeric values are quantized at their own source
emission sites before relative positioning is accumulated. Empty emitted tspans
apply no dy: parser/text.rs::resolve_positions_list limits assignments to actual
descendant characters, and chunks are constructed while iterating characters.
Mermaid's outer start_y calculation still includes the original line count.

text/mod.rs::convert first calls layout_text, installs positioned spans and
metrics bounds, then flatten. flatten batches consecutive ordinary outlines
into a single path per span. Painting each glyph separately can alter alpha and
stroke overlap. Appearance priority is COLR, font SVG, PNG raster, then outline;
source COLR insertion does not flush the ordinary span builder, whereas SVG and
raster insertion do. Keep the source ordering, even where it is unexpected.

parser/paint_server.rs::update_paint_servers is a required typed postpass for
text paints. It resolves original spans, layouted spans, decoration paths and
flattened paths using the parent text's metrics bounding box. Per-glyph ink
bounds or a postpass using only outline bounds would change gradient behavior.

## Why the current scene text cannot be called complete

M1 FontResource lacks variation axes, and current FontOutline conversion would
read the default outlines. Per-glyph appearances also miss source outline
batching. Existing flat color layers/linear paints/Normal-Multiply groups cannot
represent every source COLR gradient/clip/composite. Text metadata lacks the
full resolved chunk/span/character-position semantics. Legitimate non-rendering
characters need explicit coverage/provenance, rather than invented empty paths.
Existing public usvg positioned output omits original cluster byte indices,
advances and bidi levels; those cannot be reconstructed from glyph substrings.
The earlier WARP tests accept ordinary retained glyph conversion locally, not
these missing full text semantics.

## Root-selected direction and responsibilities to freeze next

Use a checksum-pinned narrow usvg source fork to expose its existing typed text
algorithms. Copying only rustybuzz calls would omit positioning, fallback,
variation, metrics and appearance ordering. The fork is a shared preparation
service; Direct2D remains a consumer of completed portable scene geometry and
must never invoke this service or usvg itself.

The required flow is source layout -> typed text presentation -> shared font
selection/shaping/positioning -> completed typed appearances -> immutable scene
-> SVG and Direct2D. No Mermaid diagram SVG is serialized or parsed in that
flow. Source SVG parsing remains confined to independent reference oracles.

The narrow input boundary must carry ordered font families/style/stretch/weight/
variation axes; complete span ranges/paints/font size/kerning/optical size/
decorations/baselines/spacing/textLength; chunk x/y/anchor/Unicode/text flow;
character dx/dy/rotate/writing mode and transform; and an owned font context.
Derived bounds and glyph positions are results, not caller approximations.
Validation must check UTF-8 boundaries, scalar counts, ranges, finite numbers,
resource lifetime and source-compatible empty/missing-font outcomes.

Proposed file responsibilities, requiring an exact API/acceptance packet before
implementation:

- Mermaid resolved_text: source text emission -> resolved typed chunks and
  original semantic/export metadata; no font lookup or new wrapping.
- Shared font_context: immutable owned fonts/generic family policy and identity;
  preserve resolver-added fonts and source lookup order.
- Fork typed-text adapter: build native source structs and call existing source
  layout, with explicit diagnostic/provenance hooks; no diagram SVG parser.
- Shared glyph_appearance: source outline batching, retained variation geometry,
  direct typed COLR painter, embedded font assets and source paint postpass.
- Shared scene construction: publish completed ordered appearances plus semantic
  and glyph provenance. Both backend consumers use those same appearances.
- SVG serializer: export prepared geometry and retained semantic/link metadata.
  It cannot ask a browser to reshape raw labels independently of Direct2D.

Font SVG assets are actual SVG bytes stored in a font, distinct from a diagram
serialization bridge. Their resource resolver/font policy needs its own frozen
packet. Existing usvg COLR conversion serializes an artificial SVG then reparses
it (flatten.rs::colr); that bridge must be replaced by a typed painter before
the full native text gate. Merely exposing convert unchanged would retain it.
There is no authorization here to silently exclude color fonts or report a
complete text backend while that bridge or schema gaps remain.

## Required independent acceptance

Compare pristine Mermaid emission processed by unchanged pinned usvg with the
typed path, using identical owned font bytes/index/order and fixed policies.
Cover multilingual bidi/ligatures/marks, whole-chunk fallback and styled spans,
font weights/variations/optical size, baseline/anchor/dy/textLength, empty spans,
default-ignorable characters, source metrics/paint bounds, color/raster/font SVG
appearances, batching/painter order and overlapping opacity/stroke. Detect
compiling omissions, verify source/provenance, and exercise actual WARP output.
Neither the audit nor ordinary ASCII outline tests passes this acceptance.
