# Root decision — COLR appearance parity and shared preparation

This is a required policy for the next typed text/appearance interfaces; it is
not a declaration that the current scene or Direct2D consumer implements COLR.
Root read the complete pinned `usvg 0.47.0` `text/colr.rs`, the color/ordinary
glyph dispatch and batching in `text/flatten.rs`, and the relevant gradient,
clip/group, viewport and paint normalization. Source hashes and callback sites
are recorded in `evidence/m4c-colr-typed-painter-audit.md`. The audit's suggested
API and its proposed unsupported/fallback decisions are not accepted APIs.

## Authoritative comparison policy

The first native Mermaid path reproduces the existing Folia appearance from
the pinned Mermaid renderer and usvg normalizer. Its native shared preparation
must preserve the Source's deliberate reductions and observable quirks as well
as supported operations. It cannot substitute full OpenType COLR semantics for
that baseline, reject inputs the baseline accepts, or silently discard a
Source-supported scene operation. The following decisions are frozen by root:

| Existing Source behavior | Required shared typed result |
| --- | --- |
| Sweep paint only warns and emits no path | Record a Source sweep-omission provenance event and preserve that no-draw result. This is not a missing native implementation of a Source-supported paint. It must not request an outline fallback merely because it has no painted leaf. |
| Twelve unsupported Porter-Duff/composite modes map to normal | Preserve normal appearance and the original COLR mode as provenance. Do not invent an unsupported error for those accepted baseline glyphs. The sixteen modes actually emitted by Source require their exact resolved blends and isolated group semantics. |
| Linear gradient ignores the third COLR point | Preserve the two-point Source geometry. Retain ignored Source parameters in provenance where diagnostic comparison needs them; no backend reinterprets them. |
| Pad/repeat/reflect operates on SVG endpoints rather than first/last stops | Resolve that same endpoint domain once in shared preparation. Neither backend recomputes a COLR domain. |
| COLR appears before embedded SVG, raster and ordinary outline; COLR append does not flush a pending ordinary-outline batch | Preserve that selection and exact resulting appearance order, including overlap/opacity consequences. Semantic glyph order and source ranges remain separate provenance; consumers draw the completed appearance sequence. |
| Missing outline clears its path but retains the previous captured outline transform | Preserve the subsequent Source empty-path/clip/group outcome through typed normalization. Never reuse the previous geometry or invent a visible replacement. |
| Failed inverse for gradient starts from identity, then pre-concatenates the current transform | Preserve the original f32 operation stages and final Source-parsed numeric values, and record the failure. It is not permission to drop the gradient. |

`fontdb_colr` succeeds or returns None before later glyph alternatives. Typed
preparation must reproduce that exact selection outcome after Source-equivalent
normalization. A successful empty appearance remains distinct from None. A
Source absence/malformed input that returns None follows the Source's later
font-asset/raster/outline alternatives. A native implementation gap or unsupported
final scene operation is an explicit failed preparation and cannot take that
fallback route. No proposed audit rule saying every diagnostic must prohibit
Source fallback supersedes this distinction.

## File and layer responsibilities

1. The narrowly pinned usvg fork exposes existing native text structs, font
   matching, whole-chunk shaping/layout, and internal provenance. Its ordinary
   algorithms remain Source algorithms. It does not ingest a Mermaid SVG.
2. The shared COLR painter consumes the existing ttf-parser callbacks directly.
   It owns paths, nested clip/layer scopes and raw resolved paints after callback
   return; it cannot retain a borrowed Face or gradient iterator. A raw capture
   graph is preparation state and must not escape as a completed DiagramScene.
3. Shared appearance normalization reproduces the old numeric/stop/paint/clip/
   bounds stages directly. Scalar formatting/parsing to match shortest decimal
   numeric stages is permitted; SVG/XML/path-string manufacture and parsing is
   not. Stops use the Source's ordered adjustment algorithm, not global sorting.
4. A single immutable scene publishes final owned appearances, resource handles,
   original glyph/cluster/axis provenance and Source reductions. A nested glyph
   appearance tree, radial paint, full emitted blend set and isolation are
   required schema capabilities. Existing flat GlyphLayer geometry and only
   Normal/Multiply are insufficient; they do not pass the full text gate.
5. SVG and Direct2D consume that same completed tree/order/resources. They do not
   shape, select fonts, normalize COLR, reinterpret Source reductions or run the
   old synthetic-SVG branch. Direct2D code itself never invokes usvg. Reused
   portable Source text algorithms belong exclusively to shared preparation.

The new schemas and exact callable APIs require their own root-frozen packet
before implementation. This policy does not authorize arbitrary schema additions
by an agent. Actual embedded SVG font assets remain a distinct resource-format
decision under the font-asset packet; they cannot conceal a manufactured diagram
or COLR SVG bridge.

## Acceptance obligations

Only the independent oracle may invoke the unchanged Source's synthetic COLR
SVG path. Compare its final normalized tree, not just callback traces: paths and
f32 bits, transform order, colors/alpha/stops/spread, radial focal data, clip and
bounds validity, group isolation/blends, successful empty versus None outcomes,
and Source batching/appearance order. Cover all Source-reduced cases explicitly
alongside ordinary supported cases. Require actual WARP comparisons of all
sixteen emitted blends, gradient/clip/opacity nesting, interleaved color/ordinary
glyphs, variable/palette handling, ownership and failure behavior. Tests detecting
Source no-draw behavior must distinguish an intentional Source reduction from
an omitted supported operation. Neither this decision nor the read-only audit
accepts a shared text pipeline, a backend, or Folia integration.
