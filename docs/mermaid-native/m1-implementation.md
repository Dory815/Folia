# M1 scene implementation report

Implemented against the frozen `implementation-contract.md`, 2026-10-11. This is a source implementation report for root review; no compile, tests, SVG parity, native rendering, or acceptance is claimed. No builds/commits were run. Only the five authorized new vendor scene modules and this report were written.

## Public construction and immutable result

The modules expose `SceneBuilder::new(SceneDimensions, RootViewport)`, `push_node`, `set_metadata`, typed `add_gradient`/`add_font`/`add_image`, and `finish(self) -> Result<DiagramScene, SceneError>`.

`DiagramScene` has private storage and read-only getters for dimensions, viewport, nodes, metadata and resource tables, plus checked typed resource lookup. Resource bytes are retained as `Arc<[u8]>`; the result holds no OS, COM, font-database, decoder, parser, SVG, or XML object. Nodes/specifications are publicly constructible before finishing; the immutable scene exposes no mutable access. `Arc<DiagramScene>` wrapping belongs to the later prepare API. M1 does not implement `compile_scene` or alter `lib.rs`.

`SceneDimensions` stores output width/height and numeric viewBox. `RootViewport` stores clip, original aspect-ratio policy and the **completed** viewBox-to-viewport affine transform. The constructor requires both instead of inventing an implicit fit transform. M2 must supply a mapping consistent with the dimensions and policy; M1 validates its numeric structure but does not recompute layout or compare it against a separately derived fit.

`Affine::compose(inner)` means `self * inner` in column-vector/SVG convention: apply `inner` first, then `self`. `transform_point` uses x'=a*x+c*y+e and y'=b*x+d*y+f. All geometry uses f64. Paths preserve absolute Move/Line/Quadratic/Cubic/Arc/Close segments and fill rule; no raw path string or parser exists. Rounded rectangles, circles, ellipses, lines, polyline/polygon and paths are typed. Clips are geometry plus a transform.

Paint types contain resolved normalized RGBA, linear-gradient references/table data, explicit stroke style, distinct fill/stroke/overall opacities, and normal/multiply layer blending. Groups own ordered children, transform, clip, layer style and metadata. Export metadata retains ids/classes/data suffix/value pairs/title/link/semantic text and typed marker/symbol/gradient identity. It cannot override geometry or paint.

## Text and images

`TextNode` retains original Unicode text, original `FontSpec`, resolved text origin, export anchor/baseline, style/metadata and final positioned glyph runs. Each run retains font resource id, em size, final baseline origin, UTF-8 source range, bidi level, sideways flag and glyphs with id/advance/offset/source cluster. Final run origins are authoritative; backend anchor adjustment is forbidden.

`GlyphAppearance` explicitly distinguishes ordinary `FontOutline`, retained ordered colored geometry layers, and retained image reference/destination/transform. M1 creates **no** outlines, font runs or color fallbacks. M2 must select a real resolved appearance for every glyph and preserve colored/fallback data. A `FontOutline` declaration is not evidence that arbitrary color fonts have been supported. Each geometry layer retains resolved paint, allowing a color glyph to refer to shared gradients. Whitespace/control shaping semantics and SVG text structure remain M2 text-service obligations.

`ImageResource` owns bytes, MIME enum (PNG/JPEG/GIF/WebP) and nonzero intrinsic pixel dimensions. `ImageNode` owns resolved destination and intrinsic-pixel-to-scene affine mapping, original aspect policy, style and metadata. Native decoding/upload is deferred. The completed transform controls drawing; the backend must not refit from the policy. Retained glyph images use a resolved destination/transform too.

## Validation and cleanup behavior

`finish` verifies before moving any ownership into the result. Failure returns one structured `SceneError` with field/node context; no partial scene escapes and no caller-data `unwrap`/panic is used.

Validation checks:

* finite coordinates/matrices, positive output size and usable positive viewBox; nonnegative rectangle sizes/radii/stroke widths/dash entries; finite rectangle extents, arc rotations, dash offset and glyph advances; zero-sized drawables, empty paths/point lists and empty scenes remain allowed;
* normalized RGBA and all separate opacities in 0..=1; gradient stop offsets in 0..=1 and nondecreasing order (equal offsets allowed), finite endpoints/transforms, valid gradient references and stroke miter limit >=1;
* nonempty font bytes/stable identity, conflicting duplicate font identities, font data/face-index parsing through the existing ttf-parser dependency, and ordinary FontOutline glyph IDs bounded by the face's number of glyphs; nonempty image bytes/nonzero intrinsic dimensions and valid font/image references including glyph appearance resources. No font lookup/shaping or image decoding occurs;
* positive font sizes/stretch, nonempty font-family list/tokens and weight 1..=1000; finite final origins/advances/offsets, ordered/in-bounds UTF-8 character boundary ranges, and glyph cluster containment within its run. Overlapping/reordered clusters and signed advances are allowed for real shaping. Empty clusters in a nonempty text source are rejected. Visible text must be covered by the union of nonempty glyph-run source ranges **and** glyph cluster ranges; an empty glyph run covering visible source is rejected. Whitespace and Unicode control scalars may remain uncovered and are retained in export text. Unicode format scalars such as ZWJ are not silently exempt; their shaped cluster must retain the source. Shaping fidelity beyond complete source coverage is an M2 obligation;
* visible source scalars require at least one positioned glyph. Visibility accounts for enclosing zero-opacity groups, fill/stroke alpha/opacity and stroke width. Transparent paint can retain unresolved semantic/export text; every supplied run/resource is still validated;
* typed retained glyph geometry is nonempty when selected and individually validated; retained image placements/references are validated;
* data attribute suffixes are nonempty ASCII letters/digits/hyphen/underscore/dot. Other suffixes return `UnsupportedFeature` rather than producing invalid serializer attributes. All current audited `data-` emission uses this subset;
* tree depth: root drawables count as depth 1; `MAX_SCENE_DEPTH=128` is fixed. Validation uses an explicit pending stack, propagates parent opacity and checks finite composed group/clip/image transforms. Deeper input returns `UnbalancedState` with context before descending further.

Ownership teardown is iterative in `builder::drop_nodes_iteratively`: pop a node, append a group's children to the explicit pending vector, then drop the now-empty group. `SceneBuilder::Drop` uses it for abandoned builders and rejected finish; `DiagramScene::Drop` uses it for completed scenes. An adversarial over-budget tree therefore does not recursively drop through all nested groups after validation fails. The builder checks the tree it owns; independently owned caller-created `SceneNode` values that never enter a builder remain ordinary Rust recursive values.

M1 permits empty gradients and all-zero dash patterns because those are legitimate source states; backends/shared adapters must implement their defined transparent/solid behavior explicitly. Singular finite transforms and zero-length primitives are retained rather than normalized away. Caller-owned image content is only checked structurally at this stage; malformed font data/face indices and out-of-range ordinary outline glyph IDs are rejected using ttf-parser. External draw options/transforms must be validated by the later backend. M1 does not promise encoded image validity, correct shaping/outline appearance of a numerically valid glyph, complete transformed curve-extrema bounds, or resource allocation limits beyond structural depth.

## Source obligations for M2/root review

Root owns exports, adversarial tests/builds and review. Required checks include Send+Sync, matrix composition order, field finiteness/negative sizes, all resource references, glyph cluster boundaries (including inside multibyte text), missing visible suffix/run/cluster coverage, malformed font bytes/invalid face indices/out-of-range glyph IDs, empty scenes/primitives, stop order, inherited text visibility, and rejected-depth cleanup/abandoned-builder cleanup. Source types alone are not passing evidence.

M2 must extract all final source geometry once, including typed marker/icon expansion/clips, radar/pie/Sankey/ER rendering-owned placement, C4 PNGs, inherited resolved paint/gradient/blend behavior, exact output viewBox/fit and completed image mapping. It must independently resolve text against root's separately frozen font/shaping policy and retain export semantics. The old 70-function/23-family/options corpus remains root's oracle. Every native draw must consume the same completed tree as SVG output, without layout, shaping, SVG serialization/parsing, ignored nodes, fake glyph geometry, or generic native approximations.

No interface conflict requiring deviation was found. Constructor and additional shaping/export fields concretize the contract within the authorized five files. Text resolution, fitting-policy consistency, resource decoding/limits and export serialization remain deliberately outside M1 and are explicit obligations rather than hidden fallback behavior. Implementation stops here for independent root review before M2.
