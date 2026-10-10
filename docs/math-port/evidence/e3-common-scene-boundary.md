# E3 common-layout / scene / SVG boundary evidence

Status: source mapping only. No target implementation or interface is proposed
here, and no parity or acceptance claim is made.

Reference: MathJax-src 4.1.3 at commit
`fb987178c2d279a99b0db5ea02e751691703955e`. SHA256 values below are for the
checked-out source files. Upstream uses case-sensitive directory names
`ts/output/common/Wrappers/` and `ts/output/svg/Wrappers/` (capital `W`); the
base files are `ts/output/common/Wrapper.ts` and `ts/output/svg/Wrapper.ts`.
Both common and SVG have their own `Wrappers/math.ts`.

## Locked files

| Upstream file | SHA256 |
| --- | --- |
| `ts/output/common/Wrapper.ts` | `d1db6112db1cb3546658eb8868b5997a4f69e588632ecbe4e08a71949544a828` |
| `ts/output/svg/Wrapper.ts` | `3d886e96761e90c045d455ed3e246692e1f9feedac770adcc4b0d9dc4096f6b2` |
| `ts/output/common/Wrappers/math.ts` | `990a3f3b733a1d23b8001fdb1298afc3530445204b066e943983fc8085bb99d0` |
| `ts/output/svg/Wrappers/math.ts` | `7110be42cb256ed271a6c4c8117d49441a17e3ec75822bbbb5cd1d659c12af53` |
| `ts/output/common/Wrappers/mfrac.ts` | `bd11ae20e3f698656add3a38a4aba4385a8595ab4f8f64f5629eab40156ace9d` |
| `ts/output/svg/Wrappers/mfrac.ts` | `53a0dc9739c10e164a050328108e5751138b80a1aac9f6f3058e6faf23567ab1` |
| `ts/output/common/Wrappers/mo.ts` | `67c7ed182bc8a38139da98e4d42d0350d2386de29a7e5f95095b4161dcab1bcf` |
| `ts/output/svg/Wrappers/mo.ts` | `b982073346322e8bff645d39f94eac35c613f8b1e45e857de1fb6a2d88950873` |
| `ts/output/common/Wrappers/msqrt.ts` | `1af07e3067fc041346bac1a6020a49cd9841a02e26227f1f5bda90ca2b0f56f8` |
| `ts/output/svg/Wrappers/msqrt.ts` | `5cbc9e9725c71e64a08bf590129f49cc237d1de8a85ae80e23419da9832edc51` |

Line references below are one-based in these exact files.

## Shared wrapper state and measurement

In `common/Wrapper.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `CommonWrapper` constructor (lines 538–555) | Builds the wrapper from its factory, MML node, optional parent and `factory.jax.font`; initializes `bbox` to `BBox.zero()`, then gathers CSS styles/style data, variant, scale and spacing before wrapping children. Each child's wrapper is factory-dispatched. This is wrapper state initialization and measurement context, not SVG emission. |
| `wrap` (562–569) | Uses the injected wrapper factory and parent; records MML-node-to-wrapper association in the jax node map. It creates wrapper state only. |
| `getBBox` (579–587), `computeBBox` (633–642) | Lazy/cached measurement. `getBBox(save)` returns cached `bbox` when present; otherwise calls the concrete wrapper's `computeBBox`. Base `computeBBox` appends each child's `getOuterBBox`, cleans sentinel values, and may recurse after percentage child widths are resolved. The `save` flag matters for non-cached/stretch measurement. |
| `getOuterBBox` (595–606) | Starts from the measured BBox and, when `styleData` exists, copies it and adds per-side CSS padding, border widths and margins using `BBox.boxSides`. It consumes style arrays and BBox side metadata, and does not mutate the child box. |
| `getScale` (994–1048) | Reads parent `bbox.scale`; node attributes `scriptlevel`, `scriptsizemultiplier`, `fontsize`, `mathsize`, `scriptminsize`, explicitness of `mathsize`; and removed inline style font size. Writes absolute `bbox.scale` and parent-relative `bbox.rscale`. These values depend on prior inherited MML attributes. |
| `getSpace` (1053–1063), `getTeXSpacing` (1111–1127) | Chooses MathML or TeX spacing from jax option `mathmlSpacing`, whether the node is top embellished and has spacing attributes, MML TeX-class predecessor state, operator core attributes and lengths. Writes `bbox.L/R`; explicit operator `lspace/rspace` can override the computed TeX left glue. MathML spacing additionally examines neighboring embellished operators and scriptlevel (`getMathMLSpacing`, 1068–1103). |
| `getVariant` (925–968) | Resolves a wrapper's font variant from MML node attributes, inherited style and font services; this is part of common measurement/font selection, not SVG path emission. |

These methods establish inputs used downstream: a wrapper's current BBox,
outer BBox, parent-relative scale, left/right spacing, font variant, style data,
and child wrappers. Concrete wrappers may mutate/invalidate these as stretch
or percentage-width decisions change them.

## Fraction: common measurement, SVG placements and rule emission

In `common/Wrappers/mfrac.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `getTUV` (248–260) | Computes TeX numerator/denominator offsets from display flag, passed rule thickness `t`, `font.params.axis_height`, and `num1/num2`, `denom1/denom2`. |
| `getFractionBBox` (234–243) | Measures child `getOuterBBox()` values and combines them using each child's `rscale`; uses `getTUV`, axis, `pad`, and fixed `0.2` width contribution. This is common geometry measurement. |
| `getUVQ` (277–304) / `getAtopBBox` (267–272) | For a no-rule fraction, reads child outer boxes and scales, display mode, `num1/num3`, `denom1/denom2`, and `rule_thickness`; raises the numerator/denominator separation to the display/text minimum. `getAtopBBox` combines the boxes with those offsets and fraction padding. |
| `getBevelData` (322–343) / `getBevelledBBox` (311–317) | Reads scaled child dimensions, display mode, font axis, and computes slash height and offsets; bbox method combines numerator, synthetic slash operator, and denominator. |
| `isDisplay` (350–356) | Uses the node's `displaystyle` and `scriptlevel` attributes (`displaystyle && scriptlevel===0`), rather than a standalone external display boolean. |
| constructor (364–379) / `computeBBox` (384–407) | Constructor derives padding from `withDelims` and font `nulldelimiterspace`; bevelled path creates a synthetic slash `mo`, marks it symmetric/stretchable and asks it for an exact stretch. `computeBBox` reads `linethickness`, `bevelled`, display state, converts the thickness through `length2em`, chooses bevelled/atop/ruled measurement, cleans and handles child percentage widths. |

In `svg/Wrappers/mfrac.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `toSVG` (141–159) | Creates the node's standard SVG wrapper and handles embellished output; reads `linethickness`, `bevelled`, and the same `isDisplay` result to select an SVG rendering branch. |
| `makeFraction` (167–205) | Uses child outer boxes, their `rscale`, `L/R/w`, fraction attributes `numalign/denomalign/withDelims`, common font parameters and `getTUV`. Calls child `toSVG` and `place` for numerator and denominator, then emits the SVG `<rect>` rule at axis height. Thus placement and SVG rule emission both occur in this SVG method, while the common BBox equations are separate. |
| `makeAtop` (212–238) | Consumes child outer boxes and their scales/spacing, padding, alignment attributes and common `getUVQ`; calls child `toSVG` and places them, with no rule element. |
| `makeBevelled` (245–261) | Consumes `getBevelData`, numerator/denominator outer boxes and scale, and the synthetic slash wrapper. Emits children then places numerator, slash, denominator. |

## Operator: common stretch state and SVG assembly

In `common/Wrappers/mo.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `canStretch` (662–675) | Checks node `stretchy`, operator text length/codepoint, requested direction, and font delimiter data; saves a delimiter/stretch record or `NOSTRETCH` in wrapper state. |
| `getVariant` (680–695) | Uses `largeop`, inherited `displaystyle`, explicit `mathvariant`, and `pseudoscript` to choose large/small operator or TeX variant. |
| `getStretchedVariant` (361–421) | Consumes requested H/D/width vector, exactness flag, node min/max sizes and `mathaccent` property, font `delimiterfactor/delimitershortfall`, delimiter character/size/stretch data. Selects a fixed size variant or assembly; on assembly it invalidates BBox and sets stretch BBox. Selection mutates wrapper fields `variant`, `size`, `stretch`, and glyph child BBox state. |
| `setDelimSize` (427–436) | Uses delimiter codepoint, selected size index, `font.getSizeVariant`, and optional `schar` remap; writes variant/size/delimiter character and invalidates glyph child BBox. |
| `getWH` (452–460) | Converts the requested dimension vector using `symmetric` and font `axis_height`; this yields the target dimension passed into size selection. |
| `getStretchBBox` (465–489) / `getBaseline` (494–526) | Uses delimiter min and HDW/hd metrics, direction, requested H/D, symmetric flag, axis height, and extender factor (for horizontal stretch where applicable); writes measured `bbox.h/d/w`. |
| `checkExtendedHeight` (531–538) | Uses optional delimiter `fullExt` extension/end sizes and rounds requested assembly dimension to a supported total. |
| `computeBBox` (603–618) | Measures the operator token through the base wrapper, applies symmetric vertical center adjustment based on `getCenterOffset`, and zeroes width for mathaccent operators under the source's stretch/size conditions. |

In `svg/Wrappers/mo.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `toSVG` (141–189) | Reads node `symmetric`, `largeop`, break style and mathaccent property plus common stretch direction/size. It can ask common `getStretchedVariant([])`, creates standard SVG nodes, selects assembly or ordinary glyph output, applies center/accent transforms, and appends child SVG. This method coordinates scene decisions with XML output. |
| `stretchSvg` (194–203) / `getStretchVariants` (210–217) | Reads delimiter assembly parts/variants and measured BBox; resolves each part variant through `font.getStretchVariant`; dispatches vertical or horizontal assembly. |
| `stretchVertical` (224–239) | Consumes the top/extender/bottom/middle codepoint+variant arrays and measured BBox `h/d/w`; calls part-placement helpers and divides extender placement around a middle glyph when present. |
| `addTop` (317–322), `addBot` (382–387), `addMidV` (395–401) | Read each selected part's glyph metrics/path through `getChar/addGlyph`; position top and bottom against target H/D, center parts within W, and align middle on `font.params.axis_height`. They emit SVG glyphs while determining endpoint extents for assembly. |
| `addExtV` (333–373) | Consumes extender glyph metrics and target H/D, endpoint overlap T/B, and W. Applies `VFUZZ`, computes visible viewport height `Y`, vertical glyph scale `s=1.5Y/(h+d)`, and viewBox offset `y=(s(h-d)-Y)/2`; emits a nested `<svg>` with `x=(W-w)/2`, `y=B-D`, `width=w`, `height=Y`, `viewBox=[0,y,w,Y]`, and a scaled glyph. That nested SVG viewport supplies clipping; it is not represented by the outer common BBox alone. |

## Radical: common measurement and SVG placements

In `common/Wrappers/msqrt.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `combineRootBBox` (227) | Base square-root hook is an empty method; indexed-root subclasses supply the root-box combination. |
| `getPQ` (232–243) | Reads stretched surd BBox, font `rule_thickness`, `surd_height`, `x_height`, and node `displaystyle`; returns clearance `p` and overbar gap `q`. |
| `getRootDimens` (248–253) | Base square-root returns four zero metrics; indexed-root subclasses provide index dimensions. |
| `rootWidth` (258–260) | Returns `1.25` to leave room for the surd. |
| `getStretchedSurd` (265–274) | Reads radicand outer BBox and the same font/node values, computes target surd extent, and requests exact vertical stretch from a synthetic `mo` using `[surdH - depth, depth]`. |
| constructor (283–288) | Creates synthetic square-root `mo`, marks it vertically stretchable and initializes its selected stretch from the radicand. |
| `computeBBox` (293–308) | Consumes stretched surd BBox and radicand outer BBox, `getPQ`, font rule/surd parameters, and `getRootDimens`; combines surd and radicand into a measured box and resolves child percentage widths. |
| `invalidateBBox` (313–316) | Invalidates the wrapper tree plus synthetic surd glyph child so later stretch measurement is recomputed. |

In `svg/Wrappers/msqrt.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `addRoot` (155–162) | Base square-root implementation returns zero; the method is an SVG hook overridden by the `mroot` wrapper. |
| `toSVG` (169–203) | Reads synthetic surd and radicand wrappers, optional root child, surd BBox, radicand outer BBox, `getPQ`, font `surd_height`, and parent `bbox.scale`. It creates standard SVG nodes, emits surd and radicand SVG, obtains root offset from `addRoot`, places surd and radicand, and emits the top rule `<rect>` in the final SVG container. |

## Math wrapper: line measurement and display container properties

In `common/Wrappers/math.ts`, `getWrapWidth` (148–152) returns the local
measured width for nested math, or `containerWidth / jax.pxPerEm` at the top.
`computeBBox` (157–171) delegates common child measurement, then, only for a
top-level displayed equation with `overflow="linebreak"`, compares to
container width, asks the first child to `breakToWidth`, and updates the math
BBox from that child. The inputs are MML attributes, display state, wrapper
parent, jax metrics and the child line-breaking implementation.

In `svg/Wrappers/math.ts`, `handleDisplay` (186–207) consumes display alignment
and shift from `getAlignShift`, the math BBox `pwidth`, and possibly the output
jax table's outer BBox; it sets container `justify/width` and `jax.minwidth` or
`jax.shift`. `toSVG` (214–222) delegates tree SVG output, then for block display
sets the output container's `display` attribute and calls `handleDisplay`.
`setChildPWidths` (227–237) delegates width assignment using top-level
containerWidth converted by `pxPerEm`. These are host/output layout effects,
not glyph geometry formulas.

## Base SVG wrapper: positioning and XML/path emission

In `svg/Wrapper.ts`:

| Method | Ownership and inputs consumed |
| --- | --- |
| `toSVG` (146–149) | Emits the wrapper's ordinary node: tries embellished output, otherwise creates standard SVG nodes then appends child SVG. |
| `addChildren` (191–201) | Emits children in sequence, gets each child's outer BBox, places its DOM at accumulated x plus `L*rscale`, and advances by `(L+w+R)*rscale`. This is common measured geometry consumed by SVG placement. |
| `standardSvgNodes` (211–219), `createSvgNodes` (225–234) | Creates per-parent `<g data-mml-node=...>` roots and then applies style, scale, border, color and MML attributes. If there are multiple parents (e.g. line fragments), it creates corresponding nodes. |
| `handleHref` (242–267) | Reads `href`, MML embellished state and BBox/line BBox to emit anchor wrappers and transparent hitbox `<rect>` geometry. |
| `handleStyles` (272–292), `handleScale` (297–304), `handleColor` (311–348) | Emit CSS style, border/padding/margin offset, relative scale and explicit foreground/background. `handleScale` uses `bbox.rscale / 1000` because path coordinates are in 1000-unit font space; color uses explicit MML color attrs, while background rectangles consume line/outer boxes. |
| `place` (536–552) | Consumes requested x/y, wrapper `dx`, `bbox.rscale`, DOM transform already on the element, and node ID/BBox behavior; writes SVG `translate(...)`. Numeric serialization uses `fixed` (797–799), which multiplies by 1000 before jax formatting. |
| `placeChar` (626–667) | Consumes codepoint, x/y, parent SVG, selected font variant, buffered unknown text and font's resolved char metrics/data. For known path data it emits a path/use node and returns font width; unknown glyphs become text measured through jax; character expansion data recursively emits glyphs. |
| `charNode` (696–701), `pathNode` (708–710), `useNode` (718–728) | Choose `<path>` versus cached `<use>` based on SVG `fontCache` option, variant and codepoint. `pathNode` emits path `d`; `useNode` references the font-cache id. |

## Boundary observations from these methods

Common methods own BBox measurement, inherited scale and spacing, and
stretch-state selection; they depend on already-resolved MML attrs and the
font service. SVG wrapper methods consume these boxes and state to position
children, create glyph/rule elements, assemble stretched delimiters and
serialize attributes/transforms. Delimiter extension clipping specifically
depends on nested SVG viewport and `viewBox` values created by `addExtV`.
The requested source files contain both placement and XML operations in some
SVG methods (`mfrac.makeFraction`, `mo.addExtV`, `msqrt.toSVG`); the separation
above records actual method behavior rather than claiming upstream already
has a separate backend-neutral scene object.

No target implementation, scene class, integration rule or build result is
included in this evidence note.

## Exhaustive executable-member inventory

This inventory includes every executable class/mixin member and top-level
factory/helper in the ten files listed above. Type-only interfaces repeat some
method signatures without bodies; the entries below point to their
implementations. Fields and static data are recorded separately because they
are persistent state/metadata rather than methods. Categories: **M**
measurement/state, **P** placement/fragment
geometry, **A** glyph/rule assembly, **E** SVG/XML emission, **U** shared utility
or host adaptation, **Mix** performs more than one of these. Dependencies are
the direct inputs/services read by the listed methods.

### `ts/output/common/Wrapper.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| `addStyles` 355–357; static metadata `kind`, `styles`, `removeStyles`, `skipAttributes`, `BOLDVARIANTS`, `ITALICVARIANTS` 272–354 | E/U | Registers/defines CSS and variant/style policy; static data is consumed by style/variant methods and SVG subclass. |
| top-level constants `SMALLSIZE`, `MOSPACE` 69–70; `MathMLSpace` 78–88; exported `SPACE` 94–101 | M/U | `MathMLSpace` clamps dictionary/non-dictionary MathML spaces by scriptlevel using `SMALLSIZE` and `MOSPACE`; `SPACE` maps standard TeX em-space lengths to MathML spacing names. Consumed by `getMathMLSpacing`/`getTeXSpacing`. |
| getters `jax` 433–435, `adaptor` 442–444, `metrics` 451–453, `containerWidth` 460–462, `linebreaks` 470–475, `linebreakOptions` 485–492, `fixesPWidth` 498–500, `breakCount` 505–520 | M/U | Factory/jax services, MML node/parent and children, linebreak options, cached break count. They expose state/services or derive breakpoint count. |
| `breakTop` 527–531; constructor 538–555; `wrap` 562–569 | M/U | MML linebreak-container/parent chain; factory, node, parent, font and node map. Constructor obtains styles, variant, scale and spacing before dispatching child wrappers. |
| `getBBox` 579–587; `getOuterBBox` 595–606; `getUnbrokenHD` 613–627; `computeBBox` 633–642 | M | Cached BBox, `save` flag, virtual per-node measurement, child outer boxes, CSS style data and percentage-width callbacks. `getOuterBBox` copies before adding margins/borders/padding. |
| `getLineBBox` 651–676; `embellishedBBox` 684–688; `computeLineBBox` 696–698; `getBreakNode` 706–722; `getChildLineBBox` 729–744 | M/P | Break count, linebreak cache, child line boxes, embellished operator core, node/child indices, line-leading and wrapper outer BBoxes; finds and measures fragment boundaries. |
| `sideStyleSize` 750–755; `addLeftBorders` 760–763; `addMiddleBorders` 768–772; `addRightBorders` 777–780 | M | StyleData border/padding/margin indexed by BBox side; adds side extents to line BBoxes. |
| `setChildPWidths` 792–814; `breakToWidth` 819–821; `invalidateBBox` 828–837; `copySkewIC` 844–857 | M | Parent/child BBoxes, width and clear/recompute flags, pwidth; linebreak engine; invalidation bubble; first/last child skew, accent dx and italic correction. |
| `getStyles` 865–877; `getStyleData` 882–920 | M/U | Explicit CSS `style` text and common removed-font-style list; convert border, padding and margin lengths using BBox side order, font rule thickness, scale and jax px conversion. Stores parsed styles and edge arrays. |
| `getVariant` 925–968; `explicitVariant` 978–989; `getScale` 994–1048 | M/U | MML token kind and explicit/inherited `mathvariant`, font family/weight/style, `variantForm`, font variant registry, CSS styles, parent scale, scriptlevel/size attrs and lengths. Chooses font variant and absolute/relative scale. |
| `getSpace` 1053–1063; `getMathMLSpacing` 1068–1103; `getTeXSpacing` 1111–1127; `isTopEmbellished` 1133–1138 | M | `mathmlSpacing` option, embellished/core MO and neighboring node, TeX-class predecessor state, `form`, explicit l/rspace, `noDictDef`, scriptlevel and length conversion; writes BBox L/R. |
| `core` 1145–1147; `coreMO` 1152–1157; `coreRScale` 1162–1170; `getRScale` 1175–1183; `getText` 1188–1198; `canStretch` 1204–1215 | M/U | MML core and node-map, parent wrapper chain, cumulative `rscale`, token text, embellished core, direction and shared delimiter stretch state. |
| `getAlignShift` 1220–1230; `processIndent` 1240–1271; `getAlignX` 1279–1285; `getAlignY` 1295–1309 | P | MML indent/alignment attrs, display mode and jax options, container width, BBox dimensions/L/R/rscale and alignment strings. Produces line/table positions. |
| `getWrapWidth` 1315–1317; `getChildAlign` 1323–1325 | M/P | Child BBox or per-node alignment; defaults to child's intrinsic width and left alignment. |
| `percent` 1336–1338; `em` 1344–1346; `px` 1353–1355; `length2em` 1363–1380; `unicodeChars` 1387–1405; `remapChars` 1411–1413 | U/M | Length utility constants, font rule thickness, node scale, jax pxPerEm and font variant SMP map. Converts units and codepoints; subclasses override remapping. |
| `mmlText` 1419–1423; `mmlNode` 1431–1441; `createMo` 1450–1461 | U/M | Current MML node factory, requested kind/properties/children or text; synthetic MO inherits attrs, parent and wrapper registration. |
| `getVariantChar` 1468–1474; `html` 1487–1489 | M/E/U | Font `getChar(variant,codepoint)` with unknown marker/default options; HTML creation delegated to jax. |

The class's own static and instance declarations are data, not executable
members: static policy and variant maps are at 272–354; constructor-initialized
and persistent fields are at 362–426. `StringMap` and `StyleData` are type-only
records at 53–71; the wrapper/factory/child wrapper generic contracts are
type-only declarations at 174–270. They define the maps, four-side CSS arrays,
and generic wrapper services referenced above.

The inherited fields are also part of the state contract: static output `kind`,
CSS `styles`, style/attribute skip policies and variant maps; instance
`factory`, `parent`, `childNodes`, `dom`, `removedStyles`, `styles`, `styleData`,
`variant`, `bbox`, `bboxComputed`, `_breakCount`, `lineBBox`, `stretch`, and
`font` (lines 272–426). Accessors and helpers above read or update this state.

### `ts/output/svg/Wrapper.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| static `kind`, `styles` inherited, `borderFuzz` 117–122; fields `dx`, `utext`, `font` 127–137 | E/M | SVG wrapper DOM roots, border fuzz, CSS offset, buffered unknown text and SVG font service. |
| `toSVG` 146–149; `toEmbellishedSVG` 157–186; `addChildren` 191–201 | Mix | Parent SVG nodes, MML embellished/break style, child wrappers, child outer BBox L/w/R/rscale and existing transforms. Creates fragment nodes or places each child at accumulated x. |
| `standardSvgNodes` 211–219; `createSvgNodes` 225–234; `handleHref` 242–267 | E/Mix | MML kind/attrs, parent list, `dom`, CSS/style/scale/color/attributes, href, line/outer BBox and `dx`; creates `<g>` roots, anchors and transparent hitboxes. |
| `handleStyles` 272–292; `handleScale` 297–304; `handleColor` 311–348 | E/Mix | Styles/styleData, padding/border/margin, `bbox.rscale`, explicit color/background attrs, embellished and line BBoxes; writes CSS/transforms and background rectangles. |
| `handleBorder` 353–403; `addBorderSolid` 414–435; `addBorderBroken` 448–492 | E/P | Per-edge CSS border widths/styles/colors, margin, `dx`, line/outer boxes, multi-fragment edge flags, borderFuzz and adaptor; emits polygon/line elements for solid/dashed/dotted sides. |
| `handleAttributes` 501–527 | E | Explicit MML attrs, defaults, `skipAttributes`, existing SVG attrs and class list; copies eligible attrs/classes to each output root while filtering event/property/default attrs. |
| `place` 536–552; `handleId` 564–589; `firstChild` 597–615 | P/E | Coordinates, `dx`, `bbox.rscale`, current DOM transform, id attr and measured height; mutates SVG transforms/child structure for browser id alignment. `firstChild` skips id-alignment text and hitbox rectangles. |
| `placeChar` 626–667; `addUtext` 676–688 | Mix | Codepoint/x/y/parent/variant/buffer, resolved font character metrics/options, SVG data path or character expansion, unknown-text buffer, jax unknown-text and text measurement service. Emits paths/text and returns advances. |
| `charNode` 696–701; `pathNode` 708–710; `useNode` 718–728 | E | FontCache option, variant, hex code, path data, cache-generated id and XLink option; selects `<path>` versus `<use>`. |
| `drawBBox` 735–757; `html` 770–772; `svg` 780–782; `text` 788–790; `fixed` 797–799 | E/U | Outer BBox/style border and adaptor; create nodes through jax; numeric formatting multiplies MathJax em coordinates by 1000 before jax rounding. `drawBBox` is debug output. |

`SVGWrapper` inherits the common wrapper's MML, BBox and factory state; its
additional persistent fields and static policy are listed at 117–137. The
`N`, `T`, `D`, SVG adaptor and SVG font aliases are type-only declarations near
the file header; they do not add runtime methods.

State includes `dom` (one root per fragment parent), `dx` for border/padding,
the pending `utext` buffer, and `SvgFontData`. This file handles SVG visibility
and hit-testing affordances through background/hitbox/border geometry and CSS
attributes. Its base methods do not implement general bidi direction; the
stretch `direction` used in MO is the delimiter direction enum. In the scoped
files, explicit viewport clipping is implemented by stretchy extender nested
SVGs; there is no generic wrapper clip-stack method.

### `ts/output/common/Wrappers/math.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| `getWrapWidth` 148–152 | M/P | Parent presence, local BBox width, or top-level `containerWidth / jax.pxPerEm`. |
| `computeBBox` 157–171 | M/P | Base child measurement, top-level/display state, `overflow="linebreak"`, container width, child `breakToWidth` and child BBox. |

`CommonMath` and `CommonMathClass` at 57–120 are type-only mixin contracts.
The executable top-level `CommonMathMixin` factory is at 124–173; it composes
the base wrapper with the two methods inventoried above.

### `ts/output/svg/Wrappers/math.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| static `kind` 130; static `styles` 135–178 | E | Wrapper dispatch kind and CSS for zero-width break markers/newlines and embedded zero font data. |
| `handleDisplay` 186–207; `toSVG` 214–222; `setChildPWidths` 227–237 | Mix | Display alignment/indent shift, `bbox.pwidth`, jax container/table/minwidth/shift, block-display attribute, containerWidth and pxPerEm; emits host container attributes and delegates percentage widths. |

`SvgMathNTD`/`SvgMathClass` at 53–104 are type-only contracts. The executable
`SvgMath` factory expression is at 107–239; it composes the SVG math wrapper from the
common math mixin and SVG base wrapper. Static `styles` is data rather than a
method.

### `ts/output/common/Wrappers/mfrac.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| `getFractionBBox` 234–243; `getTUV` 248–260 | M | Numerator/denominator outer boxes/rscale, display, rule thickness, font axis and TeX numerator/denominator params, padding. |
| `getAtopBBox` 267–272; `getUVQ` 277–304 | M | Child outer boxes/scale, display, font `num1/num3`, `denom1/denom2`, rule thickness; adjusts minimum no-rule separation. |
| `getBevelledBBox` 311–317; `getBevelData` 322–343 | M | Child outer boxes/scales, synthetic slash wrapper, display, font axis; derives slash height, delta and baseline offsets. |
| `isDisplay` 350–356 | M | Node `displaystyle` and `scriptlevel`. |
| constructor 364–379; `computeBBox` 384–407 | M | `withDelims`, `bevelled`, `linethickness`, font nulldelimiter/rule params, child percentage widths and length conversion; may initialize a symmetric stretchable slash MO. |
| `canStretch` 412–414; `getChildAlign` 419–424; `getWrapWidth` 429–437 | M/P | Fraction is nonstretchable; attributes `bevelled`, per-child `numalign/denomalign`, `linethickness`, child outer BBox or fraction measured width. |

The data members `bevel` and `pad` (lines 77–82, 222–227) hold the synthetic
slash wrapper and delimiter padding used by the above methods.
`CommonMfrac`/`CommonMfracClass` at 60–194 are type-only contracts. The
executable `CommonMfracMixin` factory is at 198–439 and combines the common
fraction implementation with its base wrapper.

### `ts/output/svg/Wrappers/mfrac.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| static `kind` 129; `toSVG` 141–159 | Mix/E | Wrapper dispatch kind; embellished break handling, standard SVG nodes, `linethickness`, `bevelled`, display and common font/attribute state; dispatches to the selected fraction emitter. |
| `makeFraction` 167–205 | Mix | Child outer boxes and L/R/rscale, `numalign/denomalign/withDelims`, font params, common TUV equations, child `toSVG/place`; emits rule rect. |
| `makeAtop` 212–238 | P/E | Child outer boxes/scales/spacing, alignment, padding and common UVQ; emits and places children without a rule. |
| `makeBevelled` 245–261 | P/E | `getBevelData`, child extents/rscale, synthetic slash SVG wrapper; emits and places three parts. |

The SVG subclass stores the SVG-typed bevel child (field at line 134); it reuses
the common measurement implementation through `CommonMfracMixin`.
`SvgMfracNTD`/`SvgMfracClass` at 51–103 are type-only contracts; the executable
`SvgMfrac` factory expression is at 105–263 and composes the SVG subclass with the common
fraction mixin.

### `ts/output/common/Wrappers/mo.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| getters `breakCount` 294–296, `embellishedBreakCount` 301–303, `embellishedBreakStyle` 308–310 | M/P | Core MO and embellished parent/child break state. |
| `protoBBox` 315–336; `getAccentOffset` 341–345; `getCenterOffset` 350–356 | M/P | Stretch direction/size and delimiter variant, node explicit fence/TeX class/text, null-delimiter option/font space, skew/IC, font axis, measured bbox. |
| `getStretchedVariant` 361–421; `setDelimSize` 427–436; `getSize` 441–447; `getWH` 452–460 | M | Requested dimension/exactness, node min/max/symmetric/mathaccent, font delimiter factor/shortfall, delimiter size/assembly/schar data and size variant service; updates selected size/variant and invalidates child boxes. |
| `getStretchBBox` 465–489; `getBaseline` 494–526; `checkExtendedHeight` 531–538 | M | Delimiter HDW/hd/min/fullExt, direction, requested H/D, symmetry, font axis/extender factor and MathML spacing option. |
| `setBreakStyle` 543–559; `getBreakStyle` 564–576; `getMultChar` 581–586 | M/P | MML linebreak attrs/properties and parent embellished state; optional synthetic multiplier MO for invisible times; adjusts next node TeX class. |
| constructor 593–598; `computeBBox` 603–619; `computeLineBBox` 624–626; `moLineBBox` 631–657 | M/P | MO accent/stretch state, break style and line leading, inherited wrapper initialization, center adjustment, mathaccent width, line-fragment attrs and outer/line boxes. |
| `canStretch` 662–675; `getVariant` 680–695; `remapChars` 700–715 | M/U | Operator stretchy attr/text/direction/delimiter data; largeop/displaystyle/mathvariant/pseudoscript; prime property and remap table keyed as accent or MO. |

The interface also declares the actual class state consumed here: `size`,
`isAccent`, `breakStyle`, `multChar`, `embellishedBreakCount` and
`embellishedBreakStyle` (interface lines 80–105; class fields 274–289), plus
helper signatures `protoBBox`, line BBox, offsets, stretch selection/metrics,
break style and multChar operations (112–195). The SVG subclass uses these
same fields and methods.
`CommonMo`/`CommonMoClass` at 62–247 are type-only contracts; `CommonMoMixin`
at 250–717 is the executable class factory. It composes the shared operator
logic with the supplied wrapper base. Persistent state is initialized in that
factory's class body and listed above.

### `ts/output/svg/Wrappers/mo.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| static `kind` 136; `toSVG` 141–189 | Mix | Wrapper dispatch kind; common stretch direction/size, `symmetric`, `largeop`, mathaccent, break style and fragment roots; selects fixed glyph vs assembly, transforms centering/accent, emits child SVG. |
| `stretchSvg` 194–203; `getStretchVariants` 210–217 | A/E | Delimiter assembly parts, resolved variants from SVG font, measured BBox and stretch direction. |
| `stretchVertical` 224–239; `stretchHorizontal` 246–263 | A/P | Ordered delimiter part/variant arrays, full BBox h/d/w and endpoint/middle extents; chooses one extender or two around middle and places parts. |
| `getChar` 271–279; `addGlyph` 289–306 | M/E | SVG font glyph metrics/options and variant; DOM fragment roots and `placeChar` for requested codepoint and coordinates. |
| `addTop` 317–322; `addBot` 382–387; `addMidV` 395–401 | A/E | Glyph height/depth/width, target H/D/W and font math axis; emits end/middle glyphs and returns extents for vertical assembly. |
| `addLeft` 410–412; `addExtH` 422–460; `addRight` 468–472; `addMidH` 480–485 | A/E | Horizontal part widths, extender width/height, `HFUZZ`/`VFUZZ`, target W and endpoint/middle widths; creates nested `<svg>` viewport/viewBox for horizontally scaled extender and emits/places pieces. |
| `addExtV` 333–373 | A/E | Vertical extender metrics and target H/D/T/B/W; applies `VFUZZ`, computes visible Y, vertical scale and viewBox crop; nested SVG viewport clips the scaled glyph. |

The horizontal path has the same output-level clipping obligation as the
vertical path: `addExtH` emits a nested SVG with computed x/y/width/height and a
horizontal-scale viewBox. Specifically, it reduces endpoint widths by
`HFUZZ`, uses `X=W-L-R`, `Y=h+d+2*VFUZZ`, `s=1.5*X/w`, and `D=-(d+VFUZZ)`;
the nested viewport is `(width=X,height=Y,x=x+L,y=D)` and its viewBox is
`[(s*w-X)/2,D,X,Y]`. `stretchHorizontal` positions those extenders between
left/right glyphs, or splits them around the midpoint glyph using its returned
edge positions. The class is built over `CommonMoMixin`, so the
selection/BBox state remains common while both assembly directions are emitted
here.
`SvgMoNTD`/`SvgMoClass` at 54–109 are type-only contracts; the executable
`SvgMo` factory expression is at 112–487 and composes the SVG operator subclass with
`CommonMoMixin`.

### `ts/output/common/Wrappers/msqrt.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| getters `base` 203–205 and `root` 210–212; fields `surd` 217, `surdH` 222 | M | Base/root child indices and synthetic radical MO/target dimension. |
| `combineRootBBox` 227; `getPQ` 232–243; `getRootDimens` 248–253; `rootWidth` 258–260 | M | `combineRootBBox` is empty in base sqrt; `getRootDimens` returns `[0,0,0,0]`; `rootWidth` returns `1.25`; `getPQ` reads surd box, font rule/surd/x-height and displaystyle. |
| `getStretchedSurd` 265–274; constructor 283–288 | M | Radicand outer box, displaystyle, rule/surd/x-height parameters, synthetic stretchy radical operator; asks common MO for exact required stretch. |
| `computeBBox` 293–308; `invalidateBBox` 313–316 | M | Surd BBox, radicand outer BBox, getPQ/root dimensions, font params and child percentage widths; invalidates synthetic surd child when the radical layout changes. |

`CommonMsqrt`/`CommonMsqrtClass` at 60–176 are type-only contracts. The
executable `CommonMsqrtMixin` factory is at 179–318 and composes the shared
radical measurement with a supplied wrapper base.

### `ts/output/svg/Wrappers/msqrt.ts`

| Implemented member(s), source lines | Category | Direct inputs/dependencies |
| --- | --- | --- |
| static `kind` 139; field `dx` 144; `addRoot` 155–162 | P/E | Wrapper dispatch kind and SVG offset; base square-root hook returns zero and is overridden for indexed roots. |
| `toSVG` 169–203 | Mix | Synthetic surd BBox, radicand outer BBox, optional root wrapper, common `getPQ`, font surd height, `bbox.scale`, `addRoot` result and parent DOM. Emits root SVG nodes, places surd/radicand and emits overbar rect. |

`SvgMsqrtNTD`/`SvgMsqrtClass` at 52–112 are type-only contracts. The executable
`SvgMsqrt` factory expression is at 115–205 and combines the SVG radical wrapper with
`CommonMsqrtMixin`.

## Explicitly located neighboring responsibilities

* Fragment and linebreak state is present in `common/Wrapper.ts` (`breakCount`,
  `getLineBBox`, `getBreakNode`, style-edge helpers) and `common/Wrappers/mo.ts`
  (`breakStyle`, `multChar`, MO line BBoxes). SVG fragment handling is in
  `svg/Wrapper.ts::toEmbellishedSVG` and `svg/Wrappers/mo.ts::toSVG`, which may
  create/remove one of multiple fragment roots according to break style.
* CSS visibility/clipping that is in these files is specific: SVG math's CSS
  hides break markers with `clip-path` (`svg/Wrappers/math.ts:135–178`), and
  stretchy MO extenders use nested SVG viewports in `svg/Wrappers/mo.ts`.
  The base wrapper exposes hitbox/background/border geometry and copies eligible
  explicit attrs/classes; it has no general clip-stack or opacity renderer.
* `DIRECTION` in the common wrapper imports is the stretchy-delimiter direction
  used by `canStretch`; these files do not implement a general bidi-direction
  algorithm. `unknownText` and its measurement are delegated from SVG wrapper
  `addUtext` to the output jax; the wrapped methods supply the buffered text and
  variant.

Every method body in the ten source files is now represented above or in the
earlier detailed measurement/placement tables. Interface/type declarations,
static policy data and state fields are listed separately where they have no
method body. This is an inventory for root's scene-boundary review, not a
frozen interface or an implementation recipe.
