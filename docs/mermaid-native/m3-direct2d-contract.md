# Root M3 — Windows consumer of the accepted shared scene

This packet implements the backend against the stable M1 scene interface in
parallel with remaining M2 scene compilation/SVG extraction. It is not permission
to wire Folia, and does not imply that all diagram compilation has been ported.
The backend must never parse Mermaid, invoke layout, serialize/parse SVG, use
usvg/resvg/roxmltree or load the normalizer. Generic native primitive conversion
is its responsibility; diagram placement, wrapping, shaping and marker expansion
are completed upstream in the scene.

## Owned files and public interface

Sol owns only new files under third_party/mermaid-rs-renderer/src/backends:
direct2d.rs (public lifecycle/record/draw/errors), direct2d/geometry.rs (typed
primitive -> D2D geometry), direct2d/paint.rs (brush/stroke/layer/effect handling),
direct2d/text.rs (positioned retained glyph appearances), direct2d/image.rs (WIC
decode/upload), plus docs/mermaid-native/m3-implementation.md. No other source,
manifest, expectation, test, build, production or commit mutations. Report missing
APIs/uncertain semantics before substituting a different behavior. Do not spawn.
Root controls cfg/exports/manifests/tests and approves each responsibility.

Root will add optional windows = =0.61.3 and windows-numerics = =0.2.0
in a cfg(windows) dependency section
and direct2d feature, with required Graphics::Direct2D/Common, Imaging and COM
APIs. Use cached pinned primary generated bindings; Windows/core APIs remain
strictly in this backend. Portable modules must continue building feature-off.

Public signatures (inside backends::direct2d):

- NativeRenderer::new(&ID2D1Device)->Result<NativeRenderer,NativeRenderError>
- NativeRenderer::record_scene(&mut self,&DiagramScene)
  ->Result<RecordedScene,NativeRenderError>
- RecordedScene::dimensions()->SceneDimensions
- RecordedScene::draw(&self,&ID2D1DeviceContext,&DrawOptions)
  ->Result<(),NativeRenderError>
- DrawOptions { transform: Affine, clip: Option<Rect> }

NativeRenderer creates and owns a separate device context from the supplied
device, used exclusively for recording completed native command lists/images.
RecordedScene owns the final device-bound native image/command list and all
required references, with read-only dimensions. Keep canonical IUnknown device
identity and reject drawing on a different device. Renderer/recorded objects
are thread-confined (e.g. PhantomData<Rc<()>>); CPU DiagramScene stays Send+Sync.
Do not implement thread-safe COM transfer or global resource caches. Each record
owns any resource cache; IDs from another scene must not collide or reuse fonts.

DrawOptions is a completed destination mapping/clip, not width/height layout
instructions. Native scene's root viewport transform/clip is already part of
recording. Draw combines original caller transform * options.transform using
the frozen column-vector convention, restores original transform on all exits,
and balances only its own optional clip. Caller owns its BeginDraw/EndDraw and
target; never replace the caller target or end its frame. Check synchronous
draw errors through Flush where needed; caller EndDraw remains authoritative.

DrawOptions.clip is in the caller's original local coordinate system: push it
under the original caller transform, then set the combined drawing transform.
The scene root clip is in completed output viewport coordinates: apply that clip
at identity in the recording context before applying viewbox_to_viewport.
ImageNode.destination is a local scene viewport/clip rectangle; push it under
the current parent transform before applying source_to_destination to intrinsic
pixels. No clipping-derived refit. Glyph image destination/transform follow the
same rule in glyph-local DIP coordinates relative to the final glyph origin.
Clip order is explicit and all pushes are balanced on error and success.

## Required semantics

Preflight all supported node/resource/numeric conversions before publishing
RecordedScene. Reject f64->f32 overflow with context, bad images, unsupported
features or native HRESULT failures; no panic/unwrap on caller data. Every enum
variant is implemented or returns UnsupportedFeature before success. Never
claim a record succeeded after silently dropping a primitive/paint/glyph.

Own-context BeginDraw/EndDraw and command-list Close failures are propagated.
On any partial-record failure balance layers/figures/clips and restore/detach
the recording target; do not publish or paint that partial output. Store HRESULT
and operation/context in structured NativeRenderError, distinguish device loss,
invalid numbers/resources and UnsupportedFeature. No automatic SVG fallback.

Shapes: preserve ordered rect/rounded rect, circle, ellipse, line, polygon/
polyline and typed Path. Sink state preserves multiple subpaths/Close, source
fill rule, quadratic/cubic and elliptical arc semantics (including zero radius
and degenerate endpoints). Rectangle radii use SVG primitive clamping semantics;
do not replace asymmetric rx/ry with one radius. Generic path conversion can
use D2D arcs/quadratics, not resampling arbitrary curves into polylines.

Paint: normalized RGBA, separate fill/stroke/node/group opacity, stroke cap/join/
miter limit, dash lengths/offset, gradients/stop order/units/transform/spread.
D2D custom dash units are stroke-width multiples; convert from scene lengths
without changing the pattern, preserve odd-pattern repetition and the all-zero
solid case. Zero-width stroke is invisible, never a divide-by-zero operation.
Group/node opacity composites the entire group/drawable once, including fill/
stroke overlap, rather than independently multiplying all descendant alpha.
Geometric clips and their transforms remain ordered/balanced. Native normal
layers and multiply groups must preserve isolation/painter order. Multiply uses
native D2D effect/composition with completed foreground/background images; never
approximate it using a color tint or per-brush alpha. Root will compare against
actual source Sankey blend output. Object-bounding-box gradient mapping needs
correct geometric bounds. Root approves ID2D1Geometry::GetBounds(None) on the
already-converted drawable geometry as a generic native primitive query, not
diagram layout. Use local untransformed fill bounds excluding stroke, then
bounding-box mapping composed with the gradient transform in SVG order. Zero
bounds retain SVG's degenerate paint semantics. Text paint bounds span the full
text drawable, never a separately reanchored box or per-glyph gradient reset.
If a shared algorithm beyond that query is needed, report to root before
duplicating it in backend-specific layout code. An explicit diagnostic
for not-yet-supported generic scene features is allowed during implementation,
but cannot pass the final full-feature gate.

Text: iterate positioned glyphs in stored order, with run origin + accumulated
explicit signed advances + glyph offsets. bidi_level/anchor/baseline are retained
metadata; do not independently reorder, shape, re-anchor or wrap. FontOutline
uses the retained face/index and glyph ID, actual ttf-parser outlines converted
to native geometry, scaled by em_size/units_per_em with font Y axis inverted.
Outline extraction is glyph resource conversion, not text shaping. Preserve
quadratic/cubic contours. Empty whitespace outlines are legitimate; missing
visible unsupported appearances report a diagnostic. No DrawText boxes, guessed
advances, system font replacement or ad-hoc glyph rectangles. Glyph Geometry
coordinates are local DIPs transformed relative to the current glyph baseline
origin; Image appearances carry a completed local destination/transform with
the same origin. FontOutline uses the enclosing text's effective paint and full
text drawable bounds, so its gradient is not restarted per glyph. Retained
GlyphLayer.style is an independently resolved layer-local paint: user-space
gradients move with glyph origin * layer.transform, and object-box gradients
use that layer's local geometry bounds under the same transform. Text outer
opacity composites all appearances once. Handle retained color geometry/image
appearances explicitly.
Sideways/other unsupported run modes must diagnose, never be ignored; all current
source chart rotations are group transforms, not reshaping requests.

Images: WIC decode retained bytes and convert to premultiplied RGBA/BGRA for
upload. Check decoder/dimensions/format conversion/upload results and all frame
ownership. C4's two PNGs are required. Image source_to_destination is the
completed intrinsic-pixel -> local scene transform, aspect policy is export
metadata; no new fit operation. Preserve alpha/interpolation and style opacity.
No file/URL/image network fetch or SVG image decoder.

## Independent root acceptance (still required)

Root owns WARP/D3D11/D2D offscreen tests, source SVG/reference pixels and expected
values. Feature-off portable build, Windows feature build, all shapes, contours,
transforms, clips, gradients/dashes, alpha overlap, multiply, fonts/CJK/combining/
RTL/color appearances and C4 images at 96/120/144/192 DPI and multiple destination
mappings; no blank-image success. Inspect bounds and painter ordering and catch
deliberate geometry/style/text omissions with compiling mutants. Verify caller
state, resource drop, invalid image/font/conversion/HRESULT paths, scene/device
mismatch, device recreation and lifetime. Root fixes issues at their responsible
boundary and records acceptance. Agent completion is not acceptance. Full M2
23-family compilation/SVG parity and these native tests must pass before Folia
FFI/worker/cache/editor/PDF integration and app build.
