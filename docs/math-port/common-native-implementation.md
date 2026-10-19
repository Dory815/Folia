# Common native implementation contract

Status: Root froze ownership and the shared declaration model on 2026-10-20.
Canonical interfaces are authoritative; `common-contract-refresh/headers/` is
a historical ignored packet, not a source to overwrite integrated code from.
Common core, text/rows/math, fraction/radical classes and placement geometry,
scripts/limits, fence/action, recursive LinebreakVisitor, tables and containers
now compile together in their original partitions. Integrated component suite:
**263 tests / 32,397 assertions**, exit0,
`build/math-port/native-common-table-containers-shared-full.log`; final build
`native-common-math-percent-width-build.log`. The compact 288/192 runners now
also compare Source child origins, rules, overbars and placed heights exactly.
Four actual NewCM script trees use fixed Source values rather than formulas
recomputed from native results. Menclose remains an uncompiled candidate; its
56-case native SVG writer comparison is still required. Complete TeX input,
Renderer/Scene emission, final SVG and Folia integration remain unproven.

This contract reuses the current 43 polymorphic MML classes and arena, committed Tree factory/wrapper APIs, immutable NewCM FontRepository, Styles, StyleJson, LineBBox, and LengthContext. It preserves MathJax 4.1.3 Common responsibilities and method call order while replacing Context/Property/DOM object semantics with typed values. The pinned references are `.reference/MathJax-src/ts/output/common.ts`, `output/common/Wrapper.ts`, `output/common/WrapperFactory.ts`, and `core/Tree/Wrapper{,Factory}.ts`.

## Ownership and lifetime

`RenderRequest` owns `MmlDocument`; the document outlives `LayoutJob`. `LayoutJob` owns one `CommonOutputJax`; Jax stores the sole document borrow and LayoutJob's document accessor forwards to it. Jax owns the `CommonWrapperFactory` followed by the `Linebreaks` visitor, in that declaration order. Ordinary reverse member destruction therefore destroys the visitor before the factory and its wrappers. There is no explicit release/reset/retirement API. Factory borrows only Jax. Wrappers borrow their MML node, parent wrapper, and children; the generic Tree base owns the only `node` and `childNodes` storage. The factory owns the only wrapper `vector<unique_ptr<CommonWrapper>>`.

Jax is the single storage authority for request metrics, `CommonOptions`, font repository/selected face references, linebreak visitor, unknown-text cache, node-to-wrapper index, and table-wrapper pointer. `LayoutJob` stores none of those and no root pointer. It gets the root from the borrowed document when `build()` starts. `CommonWrapper::jax()` resolves through its Tree factory service to the factory's borrowed Jax. `CommonWrapperFactory` stores only its borrowed Jax and wrapper arena; it has no LayoutJob reference or parallel node index.

`CommonMtable::findContainer` alone sets the active Jax table pointer, matching Source. Factory `adopt` transfers unique ownership and calls `initializeSource()` exactly once on the fully constructed object. Factory inherits typed Tree `wrap` dispatch and does not index or append children. CommonWrapper's `wrap`/`wrapNode` retain Source's responsibilities: construct with the supplied parent or `this`, append to a parent child vector only when the caller explicitly supplied that parent, then register the completed wrapper in Jax's index. Base initialization appends its implicit-parent child-wrap results exactly once. This preserves Source Wrapper.ts:560 and avoids duplicated links. Failed initialization unwinds the owning LayoutJob; incomplete layout is not published.

The immutable process/profile `FontRepository` outlives a request. Jax borrows it and holds the selected `FontFaceData const&`. Wrapper owns only per-node state: public `parent`, `bbox` value, `bboxComputed`, `lineBBox` shared-value vector, optional `styles` and `styleData`, `removedStyles`, `variant`, and the shared-type stretch state. `bbox.scale/rscale` is the only scale authority; there is no wrapper scale field. No Context, Property, ObjectRef, DOM handle, font-data copy, duplicate metrics/options, duplicate child vector, or callback projection remains.

## Typed values and module direction

`common:types` owns the closed shared values: `ExtendedMetrics` (including `std::string family`), `LinebreakOptions`, `CommonOptions`, owned measurement requests/results and Common forward declarations. Class-style metadata belongs to the wrapper partition. `CssFontData` is `{family:string, italic:bool, bold:bool}`; its final owner is the existing font_data module, matching Source FontData.ts (migration of that definition accompanies the pending CSS font metadata packet). Source `LinebreakOptions` fields are `inlineBreaks`, `width` string, and `lineleading`. `LengthInput` is `variant<double,u16string>` at the wrapper boundary; MML `AttributeValue` is resolved by its actual owning node/attribute method, not a universal conversion service.

`CommonOptions` is a typed record for the actual mathematical options: `scale`, `minScale`, `exFactor`, `mtextInheritFont`, `merrorInheritFont`, `mtextFont`, `merrorFont`, `mathmlSpacing`, `displayAlign`, `displayIndent`, `displayOverflow`, `linebreaks`, `htmlHDW` (`auto/use/force/ignore` closed values), and the named `skipAttributes`. It contains no font loader/path, backend service, wrapper factory, StyleJsonSheet, arbitrary map or cloned live configuration. Jax consumes the typed constructor input and owns the single live options value; the input config is not retained.

`StyleJson` remains the existing ordered recursive CSS value type and merge/format behavior in `util/style_json`; Common does not build a second style tree. Wrapper owns each node's `Styles` and `StyleData`. Jax's `addFontStyles`, `addWrapperStyles`, and `addClassStyles` collect shared stylesheet data; document insertion is Renderer/backend work.

Dependency direction follows existing partitions, with no Common/DOM cycle:

```text
common:types -> Tree factory value types, MML document primary, StyleJson,
                font_data and line_bbox (their owning named modules)
common:wrapper -> common:types, tree.wrapper, MML primary, bbox, Styles,
                  StyleJson, constants, LengthContext, font_data, line_bbox
common:wrapper_factory -> common:types, common:wrapper, tree.wrapper_factory
common:linebreak_visitor -> common:types, common:wrapper, tree visitor,
                           concrete wrapper-family declarations
common:output_jax -> common:types, wrapper, wrapper_factory,
                    linebreak_visitor, font_data, MML document, StyleJson
```

The wrapper/factory/Jax declaration cycle is broken with references to forward-declared classes in `common:types`. `CommonWrapper` derives from `tree::AbstractWrapper<CommonWrapper,mml::MmlNode>`. Factory derives from `tree::AbstractWrapperFactory<CommonWrapper,mml::MmlNode,WrapperBuildArguments>` (all three template arguments). That inherits the one ordered Tree creator registry; no second constructor map or runtime dispatch registry is allowed. `CommonWrapperFactory` remains extensible, not `final`. Configured catalog inputs supply creator and immutable class-style values together, keyed by each `NodeKind`; only the inherited Tree registry stores creators. Missing kinds throw rather than creating a generic wrapper. Default class data preserves Source `removeStyles`, `skipAttributes`, bold-variant and italic-variant maps.

The generic Common `defaultNodes()` is empty, as in pinned WrapperFactory.ts.
The native renderer supplies its concrete mathematical wrapper catalog explicitly;
it does not install SVG/CHTML class maps in Common. Jax and LayoutJob constructors
take that `CommonWrapperCatalog` plus a typed `LinebreakFactory` function pointer
(`unique_ptr<Linebreaks>(CommonWrapperFactory&)`), modeling the existing Source
wrapper/visitor constructor choices. The inputs are consumed once; the factory
retains the only creator registry and Jax owns the resulting visitor. No callback
receiver service or production no-op visitor is added. Complete renderer
integration must provide the full actual catalog and actual linebreak algorithm.

`InlineBreakPolicy` is a closed mathematical constructor input. Common defaults
to `opportunistic`; an SVG caller selects `forced`, whose forceInlineBreaks()
also respects the one live linebreaks.inlineBreaks option. Jax retains the sole
policy value. LayoutJob::build performs the Source mathematical entry sequence:
update root inherited overflow (preserving own/default layers), select width
when overflow is linebreak, reconcile old inline-marked/inline-forced state,
mark the actual first root row when enabled for inline math, run setTeXclass,
then construct wrappers. It creates no DOM and repeats no parser or inheritance
pass. The Common entry, text geometry and service-call tests do not stand in for
the not-yet-integrated actual LinebreakVisitor algorithm.

## Measurement boundary

Renderer owns `MeasurementTarget`; Common owns no DOM or measurement node. The interface is exactly two synchronous typed operations:

```cpp
ExtendedMetrics measureMetrics(MetricRequest const&);
UnknownBBox measureUnknownText(UnknownTextRequest const&);
```

Requests own their values and contain face/family, display and include-family inputs as applicable; unknown-text requests own UTF-16 text, selected variant and `CssFontData`. They contain no wrapper, MML, DOM, or callback. A missing/failed measurement is an explicit error; Common does not approximate unknown glyph dimensions. Jax owns the unknown-text cache and retains variant/text cache-key semantics.

## Declaration ownership and API surface

The ignored declaration mirrors are:

- `headers/types.ixx`: values and forward declarations only.
- `headers/wrapper.ixx`: actual CommonWrapper fields and retained math methods.
- `headers/wrapper_factory.ixx`: typed registry adapter and sole wrapper arena.
- `headers/output_jax.ixx`: request-local service owner and `LayoutJob`.

`LayoutJob` has exactly one owned `CommonOutputJax`, with document access forwarded to Jax's sole borrow. Jax declares `unique_ptr<CommonWrapperFactory>` before `unique_ptr<Linebreaks>` so destruction order is correct. Jax owns the `unordered_map<MmlNode*,CommonWrapper*>` index and borrowed `CommonWrapper* table`. Its factory accessor and CommonWrapper's `jax()` preserve the Source relationship. No `clearWrappers`, manual service retirement, duplicate factory in LayoutJob, table setter used by generic registration, root cache, or layout observer is added.

`CommonWrapper` retains the Source field names and method responsibilities. Tree supplies `node` and `childNodes`; Common supplies public `parent`, `bbox`, `lineBBox`, `bboxComputed`, `_breakCount`, optional styles/styleData, removedStyles, variant, and shared stretch state. `CommonWrapper` uses `bbox.scale/rscale` only. `stretch` is `variant<NoStretch,ActiveStretch>`, initially NoStretch just like Source Wrapper.ts:421 (`NOSTRETCH`); `ActiveStretch` holds only source codepoint and optional selected codepoint. The previously proposed UnresolvedStretch had no real consumer and incorrectly excluded nonstretching fraction siblings from row delimiter sizing. Delimiter direction comes from FontRepository; size selection remains in CommonMo. Per-node `Styles` are not duplicated in Jax.

`coreMO()` returns `CommonWrapper*`: Source MML's default core is the node itself,
which can be an ordinary row. Only callers requiring Mo-specific methods cast to
`CommonMo*` after establishing an operator. Narrowing the general lookup to Mo
would lose valid non-operator cores and break ordinary multiline rows.

Closed `CommonDiagnostic` values retain invalid explicit mathvariant warnings in
one Jax-owned vector. A record contains code, NodeKind and value, without a node
pointer or logging callback. The normal-variant fallback remains Source behavior.

`CommonWrapperFactory` derives from the exact typed Tree factory. It borrows only Jax and owns wrappers; its class-style catalog is value-only and keyed by `NodeKind`, beside the single inherited constructor registry. Its `adopt` transfers ownership and runs Source initialization. It inherits Tree `wrap` without index/list side effects. CommonWrapper owns the explicit-parent append and completed-wrapper indexing. Its `creationArguments` returns `{factory,node,parent}` typed borrows. Tree retains traversal behavior.

`CommonOutputJax` remains the named native math/layout service and CommonWrapper's `jax()` target. It is not derived from the current `core::AbstractOutputJax`, whose implementation is Context/options and DOM lifecycle. The public output-Jax lifecycle belongs to Renderer, which delegates mathematical services to this request-local Jax. Jax owns `ExtendedMetrics`, `CommonOptions`, `FontRepository const&`, selected face, factory, visitor, table, node index and unknown metric cache. It exports typed accessors for metrics/options/fonts/factory/visitor/index and preserves the Source math methods listed below.

## Method disposition

### CommonWrapper: keep as methods on the actual class

Retain and adapt the bodies of `initializeSource`, `getStyles`, `getStyleData`, `getVariant`, `explicitVariant`, `getScale`, and `getSpace`. Preserve the order: initialize bbox/cache state; call `getStyles → getStyleData → getVariant → getScale → getSpace`; then wrap MML children in source order. Derived class initialization calls the base lifecycle first.

Retain the geometry/cache/linebreak methods: `computeBBox`, `getBBox`, `getOuterBBox`, `getUnbrokenHD`, `getLineBBox`, `embellishedBBox`, `computeLineBBox`, `getChildLineBBox`, `getBreakNode`, `sideStyleSize`, `addLeftBorders`, `addMiddleBorders`, `addRightBorders`, `setChildPWidths`, `breakToWidth`, `invalidateBBox`, `copySkewIC`, `breakCount`, `breakTop`, and `fixesPWidth`. `getBBox()` returns cached `BBox&`; `getBBox(bool)` and `getOuterBBox(bool)` return values, so non-saving results cannot reference temporaries. Preserve `shared_ptr<LineBBox>` where Source aliases a child's line when no border adjustment is needed.

Retain MML/layout operations: `getMathMLSpacing`, `getTeXSpacing`, `isTopEmbellished`, `core`, `coreMO`, `coreRScale`, `getRScale`, `getText`, `canStretch`, `getAlignShift`, `getAlignX`, `getAlignY`, `processIndent`, `getWrapWidth`, and `getChildAlign`. They call actual document-owned polymorphic MML methods; Common does not reproduce MML dispatch with kind tests. `getAlignY(parentHeight,parentDepth,childHeight,childDepth,align)` keeps the Source quantity meanings.

Retain numeric/font/text operations: `percent`, `em`, `px`, `length2em`, `unicodeChars`, `remapChars`, and `getVariantChar`. Reuse `LengthContext`, immutable `FontRepository`, and exact selected variant/codepoint lookup. `getVariantChar` returns `optional<ResolvedFontGlyph>`; ordinary missing glyphs are handled explicitly by CommonText measurement. There is no fallback variant. Return vectors/records by value, not shared mutable arrays.

Retain `mmlText`, `mmlNode`, and `createMo` as CommonWrapper methods, using `MmlDocument`'s one factory, `NodeKind`, `AttributeSet` and child references. These helpers create MML values, never DOM nodes. Generated temporary wrappers remain owned by the factory arena for the job.

Retain `factory()`, `jax()`, `metrics()`, `containerWidth()`, `linebreaks()`, and typed `linebreakOptions()` accessors. `jax()` resolves through Tree's borrowed factory service. Metrics and linebreak options are const views into Jax. Container width inherits from parent according to Source, otherwise reads current metrics.

### CommonWrapperFactory: keep factory role

Keep `defaultNodes`, typed class registration/replacement, `getKinds`, `getWrapperClass`, `setWrapperClass`, `wrap`, and `adopt` where a native caller needs each. `FactoryClassMap<CommonWrapper,NodeKind,WrapperBuildArguments>` in Tree is the only creator registry. Metadata such as `styles`, `removeStyles`, `skipAttributes`, bold/italic variant maps remains closed value data, not callable constructor objects. `CommonWrapperClassData::commonDefaults()` supplies Source's default remove-style list, skip-attribute map and bold/italic variant mappings; family registrations override only their own values. No Context resolver/binder/construction scope/selected class or `Wrappers` reflective object remains.

### CommonOutputJax: keep mathematical/service responsibilities

Preserve `currentMetrics`, `currentLinebreakOptions`, `forceInlineBreaks`, `getLinebreakWidth`, `markInlineBreaks(MmlNode&)`, `unmarkInlineBreaks(MmlNode&)`, and the linebreak visitor service. Preserve typed `getMetricsFor`, `measureText`, `measureTextWithCache`, `measureTextNodeWithCache`, `cssFontStyles`, `getFontData`, `addFontStyles`, `addWrapperStyles`, and `addClassStyles`. Common continues choosing font/variant/style inputs, caching unknown measurements and returning geometry inputs. The typed MeasurementTarget replaces only DOM-based measurement execution.

`table` remains Jax-owned and is assigned only by `CommonMtable::findContainer`. Jax owns the sole MML-to-wrapper index; CommonWrapper's wrap method registers the completed wrapper. The visitor is Jax-owned and borrows the Jax-owned factory; it stores no `std::any` or receiver lookup table.

`getLinebreakWidth` reads `document().root()->attributes().lookup(u"maxwidth")`
before falling back to options width. Jax keeps computed `containerWidth_` in ems,
distinct from `metrics.containerWidth` in pixels. `pxPerEm()` derives
`metrics.ex / face.parameters.required("x_height")` (Source common.ts:437),
and LayoutJob selects the linebreak width before wrapper construction. No second
root or metrics copy is introduced. `data-semantic-added` is an MML attribute
(Source common.ts:590), already represented by AttributeSet, not a missing
NodeProperties field.

FontFaceData's implemented immutable CSS metadata consists of `cssFamilyPrefix`,
`cssFontMap` and `styles` in the existing font_data owner. Its `getFamily` and
`getCssFont` methods retain Source's exact prefix and default-font behavior.
These do not replace glyph metrics or require repeating the completed glyph
table audit; their own Source metadata comparison is required.

### Move DOM lifecycle to Renderer/backend

Move `typeset`, `toDOM`, `createNode`, `setScale(DOMNode,wrapper)`, `getInitialScale`, `setDocument`, document/adaptor binding, `getMetrics(MathDocument)`, `getMetricMaps`, `getTestElement`, DOM `measureMetrics`, `html`, `text`, `unknownText`, `measureTextNode(DOMNode)`, filter orchestration, and insertion of stylesheet content into a document. These are renderer/backend responsibilities. Common imports no DOM/SVG modules and creates no XML/HTML nodes. SVG serializes the completed Scene and owns backend spelling/escaping/attribute order; it performs no math placement or layout. Renderer creates one LayoutJob, obtains typed metrics through MeasurementTarget, freezes the completed geometry into Scene, and invokes SVG.

`styleSheet` behavior is split: Common's typed style collection methods remain; Renderer/backend owns document insertion. Any layout `getBBox` role is fulfilled by the LayoutJob build from the MML document and returns an owned BBox/Scene result, not by creating DOM.

## Linebreaking

The `Linebreaks` base is a typed service with protected borrowed
`CommonWrapperFactory& factory_`, constructor taking that reference, a virtual
destructor, `factory() const noexcept`, and pure virtual
`void breakToWidth(CommonWrapper&,double)`. Test implementations may reject
unexpected calls; production must supply the actual linebreak algorithm.

Native Tree Visitor retains the existing `AbstractVisitor` owner and methods.
Its frozen template is `AbstractVisitor<Product,Kind,Argument=monostate,Result=void>`.
`VisitorFunction` is `function<Result(Product&,Argument const&)>`; kind handlers
are one typed map in this class. A templated constructor takes the existing
`FactoryServices<Product,Kind,CreationArguments>&`, a typed handler table and
`VisitorChildren<Product>` accessors (`size(Product&)`, `get(Product&,size_t)`
returning a borrowed Product pointer). It snapshots only handlers for registered
factory kinds; set/remove handler retain their Source roles. `visitTree` forwards
to virtual `visitNode`; `visitDefault` traverses the live child count and forwards
the same typed argument, returning `Result{}` for nonvoid. Null child inputs are
invalid, never sparse-array simulation. Empty handlers select default traversal.
The old reflection-only `methodName`/MethodLookup/VisitorMethod.truthy/value and
`std::any` surfaces are removed. Common LinebreakVisitor will use this actual
base with `Argument=size_t,Result=void`, not a second dispatch mechanism.

Keep `Linebreaks` and `LinebreakVisitor` as the source-aligned request service. Preserve `addWidth`, `processBreak`, `pushBreak`, `breakLineToWidth`, `createState`, `borderLR`, `moPenalty`, `getPrevious`, `mspacePenalty`, `mtextPenalty`, and the visitor responsibilities for embellished operators, mo/mspace/mtext/mrow/inferred-row/fraction/radicals/scripts/fences/actions. Replace the current `std::any` arguments/results and `MethodLookup` dynamic receiver dispatch with typed visitor state and actual wrapper-family dispatch. No second algorithm or generic callback/variant invocation service. Visitor reads the one Jax `LinebreakOptions`; wrappers do not cache a second copy.

## Ordered implementation assignments

### Shared fraction and radical geometry (2026-10-20 next integration contract)

Keep mathematical placement on the actual Common family classes. Move only
the numerical portions of Source SVG `makeFraction/makeAtop/makeBevelled` and
radical `toSVG/addRoot` into those existing family partitions. They consume
the existing wrapped children, selected Mo, immutable face, and current BBox;
they must not accept a second public scalar input model or own another tree.

`CommonMfrac::layoutGeometry()` returns an owned `FractionGeometry`: numerator
and denominator points, an optional slash point and an optional rule rectangle.
The ruled/atop/bevelled calculations remain named family methods matching the
Source operations. Use existing pure `scene::Point/Rect` values; no DOM nodes,
SVG formatting, Scene builder, backend handle, font lookup copy, or generic
geometry callback. The result is a value for the future Scene-freezing caller,
not another mutable cache beside BBox.

`CommonMsqrt::layoutGeometry()` returns an owned `RadicalGeometry`: surd/base
points, an optional degree point, overbar rectangle, placed height and placed
surd height. Its virtual `addRoot(RadicalGeometry&, BBox const&, double height)`
returns the horizontal root offset; the default sqrt case returns zero and
Mroot overrides it using its existing `getRootDimens`. Both use the same actual
child wrappers and existing surd selection as measurement. Preserve the
Source distinction between Common measured height (rule thickness) and SVG
placement height (surd height times wrapper scale); do not unify them to make
tests simpler. Menclose's projected-base constructor change is a separate
narrow edit and must not overwrite this work.

Acceptance extends the existing compact 288/192 runners in place: compare all
Source child placements by role and rule/overbar coordinates and dimensions,
plus the existing BBox and layout quantities. Preserve signed extents, exact
floating-point operation order, child rscale/scale distinctions and the original
fixture hashes. Establish each coordinate against Source's actual enclosing
transforms; do not apply a blanket SVG y-flip to already mathematical values.
The same completed values later feed both SVG and Direct2D through Scene.
These methods alone do not prove Renderer or Folia integration.

### Stroke paint required by notation Source output (2026-10-20)

The existing backend-independent `scene::Paint` owns optional `line_cap` and
`line_join` values. Use the closed enums `StrokeCap { butt, round, square }` and
`StrokeJoin { miter, round, bevel }`; absence preserves inheritance and is not
the same as an explicit default. Append these members after the existing Paint
members to preserve aggregate initialization order. The SVG writer serializes
these values; it does not infer them from a notation name or perform layout.
Common notation sets phasorangle and longdiv caps/joins from their actual pinned
Source handler behavior. Compare all 56 notation observations, including absent
and explicit paint values, and verify writer spelling with owned Scene inputs.
Do not revive the inactive ShapePaint/Context path to express these fields.

### Root percentage width finalization (2026-10-20)

Move the numerical `SvgMath.setChildPWidths` override (Source
`output/svg/Wrappers/math.ts:227`) to the actual `CommonMath` class. This is
container-width layout, with no DOM dependency: root math always forwards
`metrics.containerWidth / jax.pxPerEm`, nested math forwards the caller's width,
and both preserve the Source `clear=false` argument. The ordinary root
`getBBox()` request invokes it through existing CommonWrapper computation.
The SVG writer and future Direct2D consumer must not independently determine
percentage widths. LayoutJob prepares/wraps; the layout consumer requests the
root BBox before freezing geometry. Tests mirror this real request, rather than
calling table setters by hand.

1. Root reviews and freezes the four ignored declaration mirrors; no method-body work before this interface review.
2. Implement CommonWrapper and CommonWrapperFactory in their existing module partitions, adapting old C++ method bodies to typed MML/Tree, existing Styles/StyleJson/LineBBox/LengthContext/FontRepository, and the field/ownership contract above. Prove factory adoption and exact initialization order with real MML wrappers.
3. Implement CommonOutputJax typed request state, MeasurementTarget calls/cache, inline break marking and style/font service methods. Move DOM lifecycle and DOM creation into Renderer/backend rather than retaining a second compatibility path.
4. Adapt the existing LinebreakVisitor to typed dispatch while preserving its Source state transitions and all visitor cases. Add Source witness cases for inline and block marking, width calculation, penalties and line geometry.
5. Restore CommonMo/operator stretching, then actual mrow/space/text/table wrappers, then CommonMfrac/CommonMsqrt/CommonMroot in their canonical family modules. Preserve source constructor/initialization order and have real factory-created objects own the existing equations; no production scalar helper API.
6. Integrate Renderer → LayoutJob → Scene → existing SVG writer, then run the unchanged full profile: 38 TeX packages, NewCM/extensions, 40 modules, 80 provider requests, 24,646 glyphs. No profile narrowing and no second layout pass.

No canonical source, CMake, build, native execution, or git operation is part of the declaration packet.
