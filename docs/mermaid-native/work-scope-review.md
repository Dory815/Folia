# Root review of Mermaid scope

The work was not all necessary for the requested Direct2D backend. Root expanded
the task into reproducing tiny-skia internals, incorrectly making CPU raster
implementation packets prerequisites for a native GPU drawing backend. That
expansion consumed effort without completing the main diagram route. It was
Root's scope/architecture decision, not an agent acceptance claim to rely on.

Directly useful work:

- Pinned mermaid-rs-renderer and its existing parser/layout; original output
  observations give comparison material across its 23 supported families.
- Immutable DiagramScene, SceneBuilder and portable typed geometry, paints,
  ordered groups, transforms/clips, text glyph runs and retained resources.
- Extracted path, node shape, endpoint decoration, label geometry/presentation,
  Architecture icon, C4 style, ER/Sankey label and dimension helpers. These are
  partial compiler building blocks, not completed family scene lowering.
- The Direct2D consumer in src/backends/direct2d*: native geometries/brushes,
  strokes/clips/groups, positioned font outlines/retained appearances, WIC
  images, device-bound recording/drawing, error and lifetime handling. Eleven
  actual WARP tests provide primitive-level evidence, not whole diagrams.

Excess work for this request:

- Source-exact tiny-skia radial/stops/gradient-program preparation and precision
  projection beyond what Direct2D brush creation needs.
- CPU highp/lowp pipeline compilation/execution, register/mask operations,
  pattern sampling and public shader dispatch reproduction.
- Blitter blend policies, borrowed destination/mask/coverage surfaces, storage
  allocation contracts and the pending blitter-constructor observer.
- Internal implementation parity and very large mutation/observation matrices
  for those CPU packets. Their local correctness does not make them necessary
  or establish Mermaid backend acceptance.

General paint/text appearance decisions belong upstream where required, but
that does not require recreating the raster engine of the old SVG-to-PNG path.
The existing Direct2D files neither import scene::prepare nor call SVG output,
usvg or resvg. Additional raster packets have stopped and are not prerequisites
for the main backend work. Their existing files are preserved pending cleanup.

Current main gap: public render_with_options/render_strict still terminate at
Layout -> render_svg. Complete Layout -> DiagramScene lowering, shared text
resolution and the parallel scene SVG serializer are absent; the native
consumer is not yet fed by the full Mermaid path. Folia is not wired to it.
Root must finish that route and independently test complete diagrams through
both backends before integration. Future Mermaid agents are Luna; Root owns
architecture, bounded contracts and acceptance.
