# Common layout, completed scene and SVG implementation contract

Root owns architecture, upstream observations, registration and independent
acceptance. This contract preserves the full locked Folia profile in scope.json.
It does not reduce the profile to the current parser or layout samples.

## Source and file responsibilities

| Native location under src/math | Responsibility and upstream source |
| --- | --- |
| output/common/output_jax.ixx/.cpp | CommonOutputJax from output/common.ts: actual font instance, metrics, styles, wrapper factory, linebreak services, node map and output lifecycle |
| output/common/wrapper.ixx/.cpp | CommonWrapper from output/common/Wrapper.ts: actual MML association, styles, variants, scales, spacing, lazy BBox, child wrapping and common measurement |
| output/common/wrapper_factory.ixx/.cpp | Common WrapperFactory and actual per-kind registrations over the existing generic factory |
| output/common/line_bbox.ixx/.cpp, linebreak_visitor.ixx/.cpp, notation.ixx/.cpp | Corresponding complete Source utility responsibilities |
| output/common/wrappers/<kind>.ixx/.cpp | Each of the 27 corresponding common Wrappers files; Source BBox/stretch/script/table equations and family dependencies |
| output/scene/scene.ixx/.cpp, builder.ixx/.cpp | Backend-independent completed geometry, semantic paint, grouping, clipping, links and their ownership; no second math measurement model |
| output/scene/wrapper.ixx/.cpp and wrappers/<kind>.ixx/.cpp | Placement and paint portions of corresponding output/svg/Wrapper and Wrappers files lifted to a common scene consumer; preserve a member-level Source mapping |
| output/svg/serializer.ixx/.cpp | Consume the completed scene and Source serialization metadata, materialize actual Lite output nodes and serialize SVG |

The Source SVG wrappers mix placements with SVG emission. Preserve their
equations and branching, but implement each placement algorithm once in the
scene wrapper family. Source common measurement stays in the common family.
The native SVG consumer may not recompute child positions or stretch geometry.
No parallel per-kind Direct2D layout algorithms will be needed: its future
consumer will draw this exact completed scene in the platform layer.

## State and dependency boundaries

The common output family is one named module `folia.math.output.common` with
`types`, `wrapper`, `wrapper_factory` and `output_jax` partitions. Forward
declarations belong to `types`; implementations import the complete partitions
they consume. Do not create competing declarations or duplicate output state to
work around Source's wrapper/output-jax dependency cycle. Existing BBox, FontData,
Styles and LineBBox modules remain separate dependencies.

The persistent wrapper factory adopts each product into one active LayoutJob's
unique_ptr arena. Its MML-node-to-wrapper map and the output jax's nodeMap refer
to the same map, with borrowed wrapper pointers. Parent/child/factory back edges
are borrowed within that job. If a Source output reference survives an operation,
retain the corresponding job owner explicitly; do not introduce an ownership
cycle through shared wrapper/map aliases. Scene resources survive independently.

Use the existing actual MathItem and MML nodes, BBox, Styles, FontData and generic
wrapper/factory infrastructure. There is one font variant/glyph/delimiter graph
and one MML-node-to-wrapper mapping owned by the output job. Read sparse glyph
tuple slots and raw option values at the same Source operations; do not add a
flattened glyph-metrics copy or a second cache that changes identity/lifecycle.

CommonOutputJax uses the real DOMAdaptor protocol supplied by the Lite packet
for Source host metrics and styles. Its ordinary Folia defaults are em=16,
ex=8, containerWidth=1280, exFactor=.5, NewCM plus the three configured font
extensions, displayOverflow=linebreak and inline linebreaks enabled. Actual
caller values and all options must reach the corresponding Source operations;
these defaults must not replace explicit values or styles.

The scene contains resolved transforms and child placements, glyph identity and
outline references, rules, background/border geometry, text runs, clipping and
semantic metadata needed by the profile. Coordinates remain full precision in
Source math units; the consumer performs explicit unit/orientation conversion.
Source SVG rounding belongs to serialization, not layout. Preserve grouping,
draw order, visibility, ids/links, fragment and linebreak state. Define all
reachable primitive kinds before accepting a renderer; glyphs alone are not
the scene contract.

Glyph outlines may retain original font path spelling for exact SVG export and
decoded path commands for a native drawing consumer. Decode font outline data
once at the font/geometry boundary. Never parse generated SVG XML to recover
geometry. Common BBox measurement must remain independent of SVG path/cache
metadata. Unknown text, XmlNode, mglyph, texhtml and worker/async dependencies
need explicit real reachability and implementation gates, not empty output.

Retained scene resources own their needed data after the temporary layout job
ends. No scene edge may dangle into a destroyed factory or Context. Returning
SVG strings to Folia is an API adapter over this result, not a second layout.
The public TeX input continues to come from the authoritative inline/block
source; rendering does not change the editor's source coordinate or history.

## Original observations and acceptance

Root's common-layout-original-oracle.cjs executes the pinned original with all
38 configured packages and the actual NewCM/font extensions. It validates the
locked entry, 59 common/SVG wrapper TS files, seven principal executable hashes
and package versions. The initial 18-case baseline is preserved byte for byte.
The expanded 75-case corpus validates all 299 executable dependency hashes
before module execution, then verifies the actual loaded closure exactly.
It observes compiled MML, wrapper BBoxes, variants, child associations, place/
placeChar calls and primitive creation before serialization, plus final SVG.
Two whole results agree exactly; SVG/errors/loads/MML/warnings agree when layout
tracing is disabled (the compile observer remains enabled). There are 73 SVG
results and two actual Lite-host action exceptions, retained as Source behavior.
This is Source evidence, not native layout or renderer acceptance. Intended
package labels do not establish complete handler/method coverage.

Acceptance requires exact Source tree/attributes before layout, per-member
Source inventory and original-driven observations, and a native full pipeline
observer operating on real production objects. Compare computed BBox, scale,
spacing, glyph/variant/stretch decisions, placements, paint geometry and final
SVG. Formatting-only normalization must be separately declared and may not hide
numeric or structural differences. Tests/faults must detect incorrect scales,
child offsets, delimiter assembly, rule geometry, paint order and namespace/
escaping errors. Include display/inline, actual width-driven linebreaking,
scripts, fractions/atop/bevel, roots, operators, tables, text/styles, links,
malformed TeX and every enabled package.

Root accepts a stable compiled snapshot with focused original comparisons and
the ordinary core runner. Folia switches the math path only after full required
profile parity; remove the math QuickJS bridge/assets/dependencies then, retain
unrelated Mermaid work, and verify actual inline/display rendering and errors
with the Windows editor/app build. Do not introduce old/new flags or fallbacks.
