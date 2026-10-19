# Common wrapper call closure and native prerequisites

This is an implementation inventory for the pinned MathJax 4.1.3
`output/common` package. It records the current native boundary; it is not a
parity or build claim.

## Shared `CommonWrapper`

`ts/output/common/Wrapper.ts` owns class metadata (`kind`, `styles`,
`removeStyles`, `skipAttributes`, `BOLDVARIANTS`, `ITALICVARIANTS`) and the
wrapper lifecycle. Constructor order is observable: `super(factory,node)`,
parent/font/BBox assignment, then `getStyles`, `getStyleData`, `getVariant`,
`getScale`, `getSpace`, and finally child wrapping in source child order while
updating percentage-width state. The wrapper keeps the actual MML node, parent,
factory, child wrappers, cached BBox, line-BBox cache, stretch record, font,
styles, removed styles and style data.

The common algorithm surface is `wrap`, `getBBox`, `getOuterBBox`,
`getUnbrokenHD`, `computeBBox`, `getLineBBox`, `embellishedBBox`,
`computeLineBBox`, `getBreakNode`, `getChildLineBBox`, border helpers,
`setChildPWidths`, `breakToWidth`, `invalidateBBox`, `copySkewIC`, style and
variant resolution, scale/spacing, core/coreMO and scale walks, `getText`,
stretch, indent processing, alignment, width/alignment hooks, length and
Unicode helpers, `remapChars`, `mmlText`, `mmlNode`, `createMo`, variant-char
lookup and `html`. The class calls the actual MML tree/factory and output-jax
state; it must not infer a second node tree or measurement model.

The helper methods to assign individually are `breakTop`, `sideStyleSize`,
`addLeftBorders`, `addMiddleBorders`, `addRightBorders`, `explicitVariant`,
`getSpace`, `getMathMLSpacing`, `getTeXSpacing`, `isTopEmbellished`,
`coreRScale`, `getRScale`, `getAlignShift`, `getAlignX`, `getAlignY`,
`percent`, `em`, `px`, `getVariantChar`, and `createMo`. `computeBBox`,
`computeLineBBox`, and `getLineBBox` are extension points overridden by
multiple per-kind wrappers.

## Factory and per-kind call closure

`WrapperFactory.ts` extends the existing `AbstractWrapperFactory<MmlNode,...>`.
It supplies a Source-visible `jax`, class-level `defaultNodes`, and `Wrappers`
view over the same generic factory node registry. Per-kind classes must use
that registry's actual constructor identity and preserve Source inheritance.

`output/common.ts` is the real owner of the common output lifecycle. Its
constructor separates font options, runs the `AbstractOutputJax` constructor,
selects/initializes the real wrapper factory, connects `factory.jax`, creates
the style sheet/font, sets the font's math-spacing option, updates the actual
class `genericFont`, and creates unknown-glyph cache and linebreak visitor.
`toDOM` sets document/math/container/px-per-em, runs prefilters, creates the
node-to-wrapper map, resolves overflow and inline break marks, sets TeX class,
wraps the root, scales and calls subclass `processMath`, clears the node map,
then runs postfilters. `getBBox` has a separate root TeX-class/wrap/outer-BBox
path. Other important Source methods include `getLinebreakWidth`,
`markInlineBreaks`, `getStyles`, `html`/`text`, `fixed`, `measureText`,
`measureTextNodeWithCache`, `measureTextNode`, `cssFontStyles`, and
`getFontData`. These require the real OutputJax and scene owner; they cannot be
replaced by a standalone wrapper-only renderer.

The 27 common wrapper source files and their principal algorithm families are:

| Source files | Main family responsibilities |
| --- | --- |
| `math`, `mrow`, `mspace`, `maction`, `semantics`, `TeXAtom` | Root/container BBoxes, stretch traversal, line breaks, action selection and inherited TeX class behavior |
| `mi`, `mn`, `mo`, `mtext`, `ms`, `TextNode`, `mglyph`, `XmlNode` | Token variant/remapping, glyph lookup, text measurement, operator dictionary/stretch/break style, XML/foreign content |
| `mfrac`, `msqrt`, `mroot`, `menclose`, `mpadded`, `mfenced` | Fraction/root/enclosure geometry, dimension adjustment and generated delimiter/row children |
| `msubsup`, `munderover`, `mmultiscripts`, `scriptbase` | Script-list pairing/padding, movable limits, accent/core selection and script offsets |
| `mtable`, `mtr`, `mtd` | Table row/cell measurement, stretch negotiation, column/row width and spacing, labels, frame and line layout |

`scriptbase.ts` is a shared superclass used by script wrappers, not an
additional registered MathML kind. `XmlNode` is the only common wrapper found
to call the adaptor directly (`getAttribute`); it also uses `nodeSize`,
`nodeBBox`, and HTML-node access. Ordinary common wrappers create HTML through
`jax.html`; they obtain MathML children and attributes from MML nodes.

## Existing native dependencies

The repository already has `BBox` (`src/math/util/bbox.ixx`), `Styles`,
`StyleJsonSheet`, `FontData`/`FontDataClass`, MML node/attribute/factory APIs,
the generic core wrapper/factory interfaces, and the Lite adaptor protocol.
`BBox` has the Source fields and core `empty`, `clean`, `append`, `combine`,
`rescale`, `updateFrom`, and `copy` operations used by wrapper equations.
`FontData` exposes variants, characters, delimiters, parameters, remapping,
CSS font data and dynamic-file state. `Attributes` exposes explicit/inherited/
all-attribute reads needed by wrapper setup.

`LineBBox.ts` has no integrated native counterpart yet; an initial isolated
source port is in `src/math/output/common/line_bbox.ixx/.cpp`, awaiting
registration and comparison. It retains `indentData` handle identity on
`copy`, stores raw attribute `Property` values in indent tuples, and mutates
the appended child as Source does. `LinebreakVisitor.ts` has no native
counterpart.
There are no `CommonWrapper`, `CommonWrapperFactory`, `CommonOutputJax`, common
per-kind wrappers, or scene modules in `src/math/output` yet.

## Open integration boundaries

`AbstractDOMAdaptor` currently names Lite-specific `LiteNodeRef` values rather
than a generic Source `N/T/D` family. `LiteAdaptor` uses one tagged Lite node
identity for elements, text and documents. The Common packet must settle this
at the existing adaptor boundary while retaining that one Lite graph; wrapper
code must not add a parallel DOM model.

`CommonOutputJax` needs an agreed real owner for `math`/metrics, options,
adaptor, font instance, wrapper factory, linebreak service, node map and
`html`. Existing `FontData` and MML factory APIs can supply their typed
dependencies, but no common OutputJax/scene contract exists yet. In
particular, MML wrapper identity to wrapper identity must be one mapping owned
by the output job, and SVG/scene code must consume the completed placements
rather than recompute them.

The wrapper factory remains one actual persistent factory identity. Native
wrapper products belong to a per-operation `LayoutJob` arena owned by that
factory's active output operation, not the generic factory's lifetime product
vector. The job owns the temporary node-to-wrapper map and wrapper arena;
finishing it clears the map and destroys wrappers. A completed scene retains
only owned glyph outlines, styles, text, geometry and other required resources,
never raw pointers into the job. This is the C++ lifetime mapping of the
Source's garbage-collected wrappers, not a second factory or layout map.

The persistent `CommonOutputJax` owns its Source options, current font, wrapper
factory, style sheet, linebreak service, metrics/caches and actual DOM
adaptor. The active job borrows the current MathItem/document/container and
owns the temporary math pointer, `nodeMap`, wrapper arena and scene-build
state. It must reproduce all Source state writes and read points in `toDOM`,
`getBBox`, filters and wrapper calls; it is not a reduced service for only the
current 18 samples.

The complete Source fields are `styleJson`, `document`, `math`, `container`,
`table`, `pxPerEm`, `font`, `factory`, `linebreaks`, `containerWidth`,
`nodeMap`, `testInline`, `testDisplay`, and `unknownCache`, plus inherited
OutputJax options/adaptor/document services and class statics `NAME`, `OPTIONS`,
`commonStyles`, and `genericFont`. The Source method inventory is
`setAdaptor`, `addExtension`, `typeset`, `createNode`, `setScale`,
`getInitialScale`, `toDOM`, `processMath`, `getBBox`, `getLinebreakWidth`,
`markInlineBreaks`, `markInlineBreak`, `unmarkInlineBreaks`, `getMetrics`,
`getMetricsFor`, `getMetricMaps`, `getTestElement`, `measureMetrics`,
`styleSheet`, `addFontStyles`, `addWrapperStyles`, `addClassStyles`,
`insertStyles`, `setDocument`, `html`, `text`, `fixed`, `unknownText`,
`measureText`, `measureTextNodeWithCache`, `measureTextNode`, `cssFontStyles`,
and `getFontData`; `forceInlineBreaks` is a virtual getter. Base behavior,
abstract subclass behavior, and per-operation mutations must be represented
separately.

The line-break path depends on the full `LinebreakVisitor` protocol and
per-wrapper overrides, not only `LineBBox`: it reads line-leading, display,
container width, penalties, operator break style, indentation, and mutable
breakpoints, then invalidates affected BBoxes. This is a separate required
Source owner before width-constrained wrapper parity can be claimed.
