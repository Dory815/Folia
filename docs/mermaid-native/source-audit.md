# Mermaid native backend source audit

Date: 2026-10-11. Scope: read-only source audit and proposed contract. This document does **not** authorize implementation, establish parity, or approve production integration. The parent agent must freeze the interface and acceptance thresholds before the next stage. No Mermaid source, manifests, Folia integration, build files, math files, tests, or expectations were changed; no builds or commits were run.

## Provenance and baseline

Folia's `src/app-winui/third_party/usvg-normalizer/Cargo.toml` pins `mermaid-rs-renderer = "=0.3.1"`, with default features disabled. `Cargo.lock` records registry checksum `6cb5b469222ce6b9896edf0a969cf68464f5a028ef394fd6643f00bd943df117`. The cached `.crate` archive's SHA-256 matches that checksum.

The inspected source is `C:/Users/think book/.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/mermaid-rs-renderer-0.3.1`. Its package metadata names upstream `https://github.com/1jehuang/mermaid-rs-renderer`; `.cargo_vcs_info.json` names commit `2f993bd79a55235eb59a34d807852276ba25bea7`. This is the version to vendor/clone if implementation is authorized; no newer version or guessed repository was used. A clone was unnecessary for the audit. The cache does not contain `.cargo-checksum.json`; archive checksum verification is not a claim that every extracted cache file was compared to the archive. The inventory records hashes of inspected files.

The crate's default `cli` and `png` features are off in Folia. `fontdb`, `ttf-parser`, regex, JSON/JSON5, serde, and parser/layout/rendering remain dependencies. Existing PNG output is SVG rasterization through optional resvg/usvg and cannot serve as the native backend implementation.

## Established pipeline and the missing boundary

`src/lib.rs::render_with_options` calls `parse_mermaid_strict`, `compute_layout`, and `render_svg`. Strict parsing validates through `validator.rs`, then delegates to `parser.rs`; `ParseOutput` holds `Graph` and `init_config`. The high-level API currently does **not** merge `init_config` into `RenderOptions`; the CLI separately merges it. Preserve each entry point's existing behavior, and test this distinction explicitly.

`ir.rs` owns parsed semantics, styles, links, diagram kinds, and special diagram data. `layout/mod.rs` dispatches the existing algorithms, normalizes graph references, applies aspect-ratio adjustment, resolves edge labels, and finalizes bounds. `layout/types.rs` owns `Layout`, `NodeLayout`, `EdgeLayout`, `TextBlock`, and diagram-specific layout data. `Layout` is a useful intermediate, but is **not** a completed display list. `render.rs` still calculates visible geometry and label layout. Passing `Layout` separately to an SVG implementation and a D2D implementation would duplicate behavior and fail the required shared boundary.

The required final pipeline is:

```text
existing parser + validator -> existing graph -> existing layout
  -> shared scene compilation (all remaining geometry/text/style decisions)
  -> one immutable completed DiagramScene
       -> SVG export serializer
       -> Windows Direct2D drawing consumer
```

Both outputs consume the same scene instance when both are requested. A standalone output request may create a scene once. Neither backend may invoke Mermaid parsing/layout, calculate wrapping/routing/label collisions, or use a serialized SVG representation as input to the other backend. Folia production rendering must have one native route after acceptance; SVG remains an explicit export backend, with no feature flag, automatic SVG fallback, or old/new rendering switch.

## Supported family inventory

These are all 23 `DiagramKind` variants in the pinned source. Presence in the dispatch is evidence of the existing implementation, not a claim of upstream Mermaid feature completeness.

| Family | Existing layout route | Existing final rendering route |
|---|---|---|
| Flowchart (`flowchart`, `graph`, supported edge-only detection) | flowchart | general graph, rounded edges, layered node labels |
| Class | flowchart | general graph, class compartments and markers |
| State (`stateDiagram` variants) | flowchart | general graph, notes, state font scaling |
| Sequence | sequence | general sequence boxes, frames, activations, numbers, notes, footboxes |
| ER | flowchart | general graph plus ER table/attribute/badge rendering |
| Pie | pie or configured error scene | pie slices, label arrangement, legend, title |
| Mindmap | mindmap | general graph, basis curves, mindmap shapes |
| Journey | journey | dedicated journey rendering |
| Timeline | timeline | dedicated timeline rendering |
| Gantt | gantt | dedicated task/status/grid/label rendering |
| Requirement | flowchart | dedicated requirement rendering |
| GitGraph | gitgraph | dedicated branch/commit/tag rendering |
| C4 (`C4`, Context, Container, Component, Dynamic, Deployment) | c4 | dedicated C4 shapes/boundaries/relations/images |
| Sankey | sankey | dedicated gradients, links, label collision adjustment |
| Quadrant | quadrant | dedicated quadrants, axes, points |
| ZenUML | sequence | sequence rendering |
| Block | block | general graph rendering |
| Packet | flowchart | general graph rendering |
| Kanban | kanban | general graph rendering |
| Architecture | architecture | dedicated icons/groups/junctions/edges |
| Radar | radar | dedicated radar series extraction/scaling/polygons/legend |
| Treemap | treemap or configured error scene | general graph with treemap colors/labels |
| XYChart | xychart | dedicated axes/grid/series/title rendering |

All default/configured routes and error scenes require coverage. `DiagramData` has Graph, Sequence, Pie, Quadrant, Gantt, Sankey, GitGraph, C4, XYChart, Timeline, Journey, and Error variants; some families share Graph and therefore cannot be enumerated using `DiagramData` alone.

## Geometry and presentation still embedded in SVG emission

All locations below refer to pinned `src/render.rs`. The machine-readable inventory lists every top-level function with its source line. The entire module must be audited during extraction; the following groups cover its rendering responsibilities.

| Source region / functions | Responsibility that must move to the shared scene stage |
|---|---|
| 39-204, dimension helpers | natural canvas, sequence padding, requirement padding, C4/gitgraph viewBox, mindmap extrema, preferred ratio and explicit dimensions |
| 206-1614, `render_svg_with_dimensions` | root background, diagram dispatch, painter order, inherited styles, subgraphs, lifelines, sequence frame corners/tab geometry/dividers, notes, activations, overlay layering, labels, nodes/footboxes |
| 1616-1751, path helpers | polyline conversion, rounded-corner quadratic curves and mindmap basis cubic curves |
| 1771-1966, label rectangles/gaps | label background visibility, segment/rectangle distance and collision-related rendering decisions; endpoint-label fallback positions also remain in emission |
| 1981-2143, Sankey | per-rank label collision adjustment, value labels, cubic control points, gradient coordinates, opacity and multiply blend groups |
| 2145-2230, errors/font normalization | fixed error paths and transforms, text coordinates, inherited CSS/error colors, normalized font-family stacks |
| 2231-2430, requirements | edge path/marker selection, label backgrounds, box/header/divider/text positions |
| 2431-2631, radar | series read from node label strings, axis extraction, normalization/max value, 700x700 chart geometry, polar positions, rings, polygons, colors and legend. Existing `layout/radar.rs` uses 680x680/290-radius intermediate constants; do not draw those as the final chart or silently fix this discrepancy in the extraction |
| 2633-3010, architecture | exact built-in icon paths (including lambda text), service/group icon placement and edge styling; unknown custom icon currently gets a neutral component glyph |
| 3011-3310, pie | full-circle special case, wedge arc paths, percentage/category label choice and collision arrangement, leader lines, legends, metric calls and ring strokes |
| 3311-3475, quadrant | rectangles, labels, point radius/position and axis presentation |
| 3476-3710, Gantt | status fills/strokes, task bars, milestone geometry, text fit/placement (including metric calls), section bands and ticks |
| 3711-3820, XYChart | axis/title rotations, grid and tick text, bars and line paths |
| 3821-4099, timeline/journey | event/card paths, connector lines, participant icons/scores, label baselines and weights |
| 4100-4410, GitGraph | branch tracks, commit-specific symbols, paths, tag shape/hole, transforms and multiline label geometry |
| 4411-4578, text helpers | exact baselines, per-line offsets, anchors, divider suppression, line-height calculation, weights and font stack |
| 4580-5140, C4 | shapes (person/database/queue/etc.), embedded person PNGs, fixed symbol/marker definitions, boundary opacity/dashes, relation curves, text baseline/weight/font selection |
| 5142-5800, class/ER text | class compartment positions, ER label reinterpretation, row/header bands, column widths, attribute/key badges, divider lines; metric calls in ER rendering |
| 5889-6351, links/decorations/shapes | URL/title/target export metadata, triangle/diamond/crow-foot/etc. decorations and transforms; exact numeric rectangle/circle/double-circle/stadium/roundrect/cylinder/subroutine/hexagon/parallelogram/trapezoid/asymmetric/mindmap geometry |

`edge_geometry.rs` is already shared for endpoint tangents, marker inset and related geometry. Preserve and extend that responsibility instead of copying its formulas into a D2D backend. `write_output_svg` is file output only; `write_output_png` remains the optional SVG raster export, outside the native rendering path.

The emitted visual primitives are rect/rounded rect, circle, ellipse, line, polygon/polyline, and paths with lines, quadratic/cubic curves, and elliptical arcs; groups carry translate/scale/rotate transforms. SVG defs include markers, C4 symbols, and Sankey linear gradients/stops. Preserve markerUnits/viewBox/reference-point scaling/orientation, not just marker silhouettes. C4 symbol definitions appear in output; the renderer has no `<use>` emission. Links/title/id/class/data attributes are export metadata, distinct from drawing, and must remain available to SVG serialization.

The renderer emits fill/stroke colors including CSS HSL values, stroke widths/dash arrays/caps/joins, fill/stroke/global opacity, even-odd fill rule, inherited group/font styles, and Sankey `mix-blend-mode: multiply`. A D2D implementation cannot treat unknown colors as black, turn gradients into solids, omit blend isolation, drop dashed strokes, or ignore a primitive. Resolve effective styles before backend consumption. Unsupported values must return a classified diagnostic before publishing a partial scene/drawing.

No explicit `clipPath`, `clip-path`, mask, `foreignObject`, radial gradient, or filter emission was found in pinned `render.rs`. The root SVG viewport and default aspect-ratio fitting still impose coordinate mapping/clipping semantics, and marker viewports may impose clipping. Native acceptance must check these, including strokes/labels at and outside bounds. Future scene clip commands require support or an explicit error, never a silent no-op. Do not claim arbitrary SVG feature support from this bounded source inventory.

## Text and image risks to freeze

`layout/text.rs` wraps labels and stores lines/approximate width/height. It measures at a baseline font size of at least 16 for regular labels, can use fast metrics, and applies guards and grapheme/CJK/emoji rules. `text_metrics.rs` lazily loads system fonts through fontdb/ttf-parser, caches selected font bytes under the user's cache directory, and sums advance widths; this is not a complete shaping engine. Several final render functions call it again.

The old Folia path lets usvg resolve and shape SVG text, using system fonts with Arial/sans, Times New Roman/serif, Cascadia Code/monospace, languages en/zh-CN. Native text must preserve the shared scene's wrapping, anchor positions, baselines, weights and fallback intent. A D2D draw call must not reflow a text block or calculate a new diagram width. Moving raw `<text>` strings to `DrawText` with a rectangle would change baselines and shaping; it is not sufficient parity evidence.

**Unresolved contract decision:** choose the shared text service and specify whether scenes hold positioned glyph runs, resolved glyph outlines, or finalized per-line text recipes with a shared shaping/anchor resolver. All wrapping/anchor/baseline placement belongs before backend dispatch. Plain-text recipes that defer independent measurement to each backend do not satisfy the strongest geometry-parity contract. Preserve SVG's text/export structure unless the parent explicitly accepts conversion to paths; silently outlining every exported text element would change existing export behavior. Freeze font files/fallback and complex-script/emoji policy for deterministic comparison. The current approximate metrics are a baseline behavior to preserve, not permission to invent another approximation.

C4 embeds two fixed PNG person icons (`render.rs:4580-4581`), emitted at 4717. A native backend needs a typed raster resource (bytes, format, dimensions, interpolation/alpha) and a Windows image decoder; it must draw these images and handle decode failure. Architecture icons use numeric paths/text rather than network icon retrieval. Existing unknown-icon behavior should be retained exactly by shared scene compilation, not replaced with a new native-only fallback. Fixed path constants must become typed geometry; no runtime SVG path/document parsing on the Direct2D route.

## Proposed exact responsibility/file boundary (requires parent approval)

Implementation location has not been authorized. In a fork pinned to the above source, the following is a proposed allowed-file map, not an instruction to edit the global Cargo cache:

| Proposed crate file(s) | Responsibility |
|---|---|
| `src/scene/mod.rs`, `src/scene/types.rs` | immutable `DiagramScene`, output canvas + viewBox mapping, resource table, ordered typed commands, metadata, typed errors; no Windows API or SVG source |
| `src/scene/compile.rs` | one dispatch over the existing Layout + resolved Theme/LayoutConfig; finishes all remaining drawing/layout decisions once |
| `src/scene/geometry.rs`, `src/scene/styles.rs`, `src/scene/text.rs`, `src/scene/resources.rs` | shared typed numeric geometry, resolved paints/strokes/blending, finalized text runs/metadata, typed fixed PNG resources; no XML formatting |
| `src/scene/graph.rs`, `sequence.rs`, `requirement.rs`, `architecture.rs`, `radar.rs`, `pie.rs`, `sankey.rs`, `quadrant.rs`, `gantt.rs`, `xychart.rs`, `timeline.rs`, `journey.rs`, `gitgraph.rs`, `c4.rs`, `node_shapes.rs`, `node_labels.rs` | extract the corresponding existing render decisions; share routines across families; no second parser/layout implementation |
| `src/render.rs` | existing public SVG/dimension/output facade; delegates scene construction and SVG consumption, preserving public behavior |
| `src/backends/mod.rs`, `src/backends/svg.rs` | scene -> SVG only; XML escaping, numeric formatting, export ids/classes/links/text structure/defs; no diagram geometry decisions |
| `src/backends/direct2d/mod.rs`, `geometry.rs`, `paint.rs`, `text.rs`, `images.rs`, `error.rs` | Windows-only direct primitive/resource consumption; same scene in, D2D commands out, HRESULT/error propagation; COM ownership/render state guards; no Mermaid IR/Layout imports |
| `src/lib.rs`, optionally `src/edge_geometry.rs` | expose preparation/scene/SVG/native APIs and keep shared geometric helpers authoritative |
| `tests/scene_suite.rs`, `tests/backend_parity_suite.rs`, `tests/direct2d_suite.rs`, a Windows rendering harness | independent structural/geometry and real D2D rendering evidence; retain existing tests/fixtures/expectations unchanged |
| `Cargo.toml`/lock, only in a later explicitly allowed stage | feature/Windows target dependencies and new test declarations once parent freezes APIs/dependencies |

Proposed API shape: `prepare(input, options, canvas_override) -> Result<Arc<DiagramScene>, PrepareError>`; `svg::render(&DiagramScene) -> Result<String, SvgError>`; `direct2d::draw(&DiagramScene, &NativeRenderContext, destination_transform) -> Result<(), NativeRenderError>`. Names and Rust COM binding/dependency choices remain to freeze. Direct2D receives a caller-owned context; CPU scene preparation and device resource creation have distinct lifetimes. Never pass a raw Rust trait object/Arc across a C ABI. A later Folia bridge should expose versioned opaque scene handles, explicitly balanced retain/release, stable size/error access, and an accepted Windows draw/resource interface.

Scene commands need numeric paths, resolved fills/gradients, explicit strokes, positioned text, embedded images, transforms, clips and groups with opacity/blend semantics. No `RawSvg`, `UnknownElement`, backend-specific callback, family-specific draw shortcut, or unhandled `_ => {}` branch. Markers must be finalized into directly drawable numeric geometry (or typed reusable geometry + completed transforms) before D2D. The SVG backend may preserve reusable marker/export metadata from that same scene. Retain painter order and grouping, including multiply composition and error-style text stroke.

## Current Folia integration facts and later replacement boundary

* `media/MermaidRenderer.cpp` loads `folia_svg_normalizer.dll`, obtains normalizer create/destroy plus Mermaid render/buffer-free symbols, owns that DLL/context on a dedicated worker, and returns `MermaidSvg {renderId, svg, error, width, height}`. `folia_mermaid_render` in Rust validates UTF-8, uses default modern theme or dark theme, renders SVG, calls `usvg::Tree::from_data`, and exports normalized SVG/dimensions. Statuses are -1 invalid ABI arguments, 0 success, 1 render/parse/normalization failure with message, 2 panic boundary result; release profile uses panic=abort, so process-aborting panics cannot be promised caught.
* Worker key is source + separator + dark flag; queue deduplicates requests and discards oldest pending requests at 128. Result admission is 16 MiB/64 entries with FIFO eviction (hits do not update order); errors also cache. `Clear()` increments generation, drops pending/cache, and rejects completed older-generation results. It does not cancel an in-flight parse/layout. Destruction sets stopping, clears completion, wakes and joins; no persistent device COM resource is carried by the Rust result.
* `EditorSurfaceRenderer` coalesces completion and dispatches UI invalidation. Completion/lifetime behavior and callbacks already enqueued when a surface is destroyed need explicit testing; suppressing future callbacks alone is not evidence that queued captures are safe. `ResetDocumentCaches` does not currently call `mermaid.Clear()`; the source/theme key still prevents content confusion, but later scene invalidation/lifetime policy must be explicit.
* `EditorDocumentBlockPreparer::ApplyMermaidPresentation` gates on SVG context support, detects non-source code blocks with case-insensitive `mermaid` language, queues UTF-8 source plus dark variant, retains source text if pending/error, previews the diagram when caret is inside the code block, and replaces displayed code with an inline diagram otherwise. Errors are stored in worker results but this path does not expose a Mermaid error diagnostic UI.
* Preparation scales down (never up) to available content width, preserving intrinsic ratio; previews use content width minus 16 DIPs. `EditorDocumentRenderPass` positions/centers the placeholder and previews. Source positions/document editing are unchanged; the diagram is a rendering resource.
* `EditorSurfaceRenderer.Document.cpp` passes each Mermaid SVG to `EditorSvgPainter`, which requires `ID2D1DeviceContext5`. `SvgDrawingCache` in `src/platform/windows/editor/rendering/editor_svg_drawing.ixx` creates an SVG document, records `DrawSvgDocument` into an `ID2D1CommandList`, and uses a separate context to avoid altering the active paint target/transform/clips. This entire SVG construction route is forbidden for the native Mermaid backend.
* That drawing cache has a 24 MiB/128-entry estimated admission budget with LRU order and conservative size estimates. It checks device identity, dimensions, exact source, DPI, antialias, blend and unit mode; a new device clears old resources. DIPs and surface scale (`96 * surfaceScaleX/Y`) enter at the D2D edge. Native cache tests must cover the corresponding effective scene/resource inputs rather than assume render id alone is sufficient.
* Theme changes clear SVG drawings/prepared content and increment theme revision; Mermaid currently receives only light-vs-nonlight, not custom theme colors. Preserve this baseline unless parent explicitly changes theme behavior. Surface `EndDraw` handles `D2DERR_RECREATE_TARGET` by resetting targets; native resource reconstruction must be tested, not inferred.
* PDF export records `DrawDocument` into page command lists and waits for blocks with `pendingMermaid` (among other assets). Replacing the screen painter changes this route too. Real PDF/print parity, pending failures, page clipping and export cancellation need acceptance. There is no separate Folia diagram SVG-export UI found in the C++ integration; the crate's public SVG/file export is the existing SVG behavior that must remain supported alongside native output.
* `NormalizeMermaidSvg` is declared/defined in content utilities, but no call site was found; current normalization already happens in the Rust FFI. Do not retain/reintroduce this helper as a native fallback.

No application production wiring belongs in the first backend stage. Root owns changes to the FFI DLL, Mermaid worker/result types, prepared-content structs, native painter/cache, PDF route, dependency/build packaging and notices **after independent backend acceptance**. No math resource/helper/controller files should be touched as collateral work.

## Acceptance evidence that must exist before production integration

1. **Frozen baseline:** verified 0.3.1 input/source/options/fonts, untouched fixture expectations and pre-extraction SVG outputs generated independently. Full SVG output behavior includes root attributes, text structure, metadata, markers, links, fixed images and numeric precision. Any permitted difference must be declared and reviewed in advance, never accommodated by rewriting goldens after a mismatch.
2. **Complete corpus:** every 23-family fixture directory plus variants: directions, nesting/subgraphs, each NodeShape, every arrow/decoration/end-label combination, frame/activation/footbox/note type, class/ER compartments, all C4 subtypes/shapes/relations, custom styles and links, blank/zero data, pie/treemap normal and configured error modes, unknown icon baseline, multiple series, Gantt statuses/milestones, dense collision cases, long/multiline/Unicode labels and boundary overflow. Exercise all serialized configuration fields, including family options, preferred ratio, explicit dimensions, use-max-width, fast metrics, supplied themes, CLI init-config merge and high-level non-merge behavior. An options field inventory must drive coverage; defaults alone cannot establish it.
3. **Single-result proof:** counters/tracing show one parse + one layout + one scene compile for a prepared result reused by both backends. Static dependency checks plus runtime spies show zero SVG serialization/parsing/usvg/normalizer invocation and zero layout recomputation on native drawing. Repeated draws/scale/DPI/device recreation use the same immutable scene. SVG exports consume it too.
4. **Semantic/geometric evidence:** independent comparison of canvas/viewBox, painter order, every transformed primitive, marker tangent/ref/scale, gradient stops/space, fill rules, dashes/caps/joins, inherited style/opacity/blend, clipped bounds and text lines/baselines/anchors against the old SVG. Round-trip scene serialization is useful for evidence only; do not serialize SVG as an implementation bridge. No missing element, forced generic shape, substituted solid fill, or hidden error passes.
5. **Actual Windows raster parity:** deterministic WARP/offscreen harness renders old normalized SVG reference and new D2D consumer with identical fonts/backgrounds at 96/120/144/192 DPI, native/small/large destinations, light/dark and explicit custom themes; hardware smoke test as well. Save input/options/scene diagnostics/reference/native/diff pixels, compare independently frozen thresholds for geometry/stroke and text regions, and visually review complete families. Existing `test_editor_svg_drawing.cpp` supplies a useful real WARP/readback pattern; it is not evidence for native Mermaid yet. Include viewport/marker clipping and Sankey multiply/gradient composition. Text checks include CJK, combining marks, supplementary-plane characters, emoji/ZWJ and complex scripts/fallback, weighted fonts and missing fonts. Check large diagrams for precision/overflow.
6. **Failure completeness:** parse errors, malformed directives/UTF-8, unsupported native commands/style/paint/image format, missing font/decode/resource failure, HRESULT and panic/abort policy produce classified failure and no partial success. A deliberately injected unknown command verifies an explicit error. Preserve supported baseline parser behavior without adding a separate parser or source-text exceptions.
7. **Resource/concurrency evidence:** scene/opaque-handle ownership survives cache eviction while retained by preparation/export; CPU worker owns no UI-affine device resources; thread/COM rules are explicit. Verify reference counts/destruction, balanced transform/target/layer/clip restoration on all failures, Close/EndDraw HRESULTs, bounded cache admission with actual accounting estimates, cache key correctness, clear/device identity/DPI changes, lost target/device recovery without re-layout, rapid edit/theme churn, stale generations, queued eviction, shutdown and already-enqueued callback safety. Cancellation may reject stale results, but in-flight work cancellation behavior/limits must be documented and tested.
8. **Later Folia acceptance owned by root:** editor display/preview/source selection, large/nested documents, asynchronous completion/pending/error behavior, repeated resize/scroll/theme changes, device loss, print/PDF export and cancellation, unchanged saved Markdown, dependency restore/build and required core/platform tests. No production backend switch is accepted before items 1-7 pass independent review.

## Audit outcome / decisions still required

The source supports a direct native backend, but the backend-neutral result is not yet implemented. The largest extraction risks are radar's rendering-owned data/geometry, Sankey/pie's rendering-owned label collision placement, ER's rendering-owned table columns and badges, C4 raster resources/marker semantics, and font/shaping equivalence to current usvg output. Multiply blending and viewport clipping must be handled explicitly.

Before implementation: parent must freeze the implementation root and exact allowed files, scene schema/public API/ABI strategy, shared text service/fallback policy, color/style resolution and blend semantics, diagnostics/cancellation/resource lifetime policy, SVG compatibility tolerance, and numerical/raster acceptance thresholds/corpus. This audit stops at that boundary; no acceptance claim or integration authorization is implied.
