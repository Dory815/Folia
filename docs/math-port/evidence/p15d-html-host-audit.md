# P15d prerequisite audit: HTML/XML host boundary

This is a read-only source and interface inventory for the pinned MathJax 4.1.3 tree and the checked-in Folia TeX-to-SVG profile. It does not propose an implementation or claim that HTML/XML support is accepted.

## Profile reachability

`docs/math-port/scope.json` pins `tools/mathjax-bundle/entry.cjs` (SHA-256 `a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9`). It constructs only a TeX input jax and SVG output jax (`entry.cjs:1-4,51-78`); it does not instantiate `MathMLCompile` or a MathML input jax. The enabled TeX packages include both `html` and `texhtml` (`entry.cjs:59-70`), which are different facilities:

* `html` is loaded as a TeX syntax package. It does not itself create `HtmlNode` or call an XML/HTML adaptor. The corpus's HTML class/id cases exercise TeX attributes and SVG-wrapper propagation (`reference-corpus.json` entries `html_class_and_id_attributes`), not an HTML payload node.
* `texhtml` is also registered, but its parser option defaults to `allowTexHTML: false` (`ts/input/tex/texhtml/TexHtmlConfiguration.ts:90-99`, SHA-256 `e31f1539452eba33048d8e7bcf87fcf9690f23be1412799ddf2a6f43beab4a3b`). Its `TexHTML` method returns `false` before scanning/parsing markup when the option is false (`:38-52`). The frozen default-policy case records literal-TeX rendering, no HTML node, and must retain the false option (`docs/math-port/reference-corpus.json:1306-1330`). Thus positive raw-TeX HTML construction is not reached by the current profile.

The option is intentionally host-configurable. If a caller enables `allowTexHTML`, `TexHTML` scans a serialized `<tex-html>` span, errors with `TexError(TokenNotFoundForCommand,...)` if the terminator is absent, trims the payload, parses it through the saved HTML document adaptor, and builds an `html` node (`TexHtmlConfiguration.ts:44-86`). Package preprocess/postprocess hooks save and then clear that adaptor (`:126-135`); the HTML handler also installs the `includeHtmlTags` serialization hook, using `innerHTML` and marker counts to preserve nesting (`:100-124`). The default-profile fixture therefore proves policy, not absence of this configurable host boundary.

## Pinned node methods and observable host behavior

Source identities: `ts/core/MmlTree/MmlNode.ts`, SHA-256 `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`; `ts/core/MmlTree/MmlNodes/HtmlNode.ts`, SHA-256 `1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665`.

| Source member | Required host call / behavior |
|---|---|
| `XMLNode.getXML` (`MmlNode.ts:1508-1510`) | Returns the exact stored XML object reference. It does not clone or normalize it. |
| `XMLNode.setXML` (`:1517-1524`) | Stores the supplied object and adaptor references, including explicit null; returns `this`. The default adaptor is null only for an omitted/undefined argument. |
| `XMLNode.getSerializedXML` (`:1529-1531`) | Calls `this.adaptor.serializeXML(this.xml)` directly. No null check or copy precedes the call; null adaptor failure is observable. |
| `XMLNode.copy` (`:1536-1541`) | Creates a factory node first, calls the original adaptor's `clone(xml)`, then calls `setXML(clone)` without forwarding the adaptor. The new node owns the cloned object identity and has a null adaptor; later serialization therefore fails. The source ordering also means a factory or clone failure happens before the next step. |
| `XMLNode.toString` (`:1547-1549`) | Returns the fixed string `XML data`; it does not call the adaptor. |
| `HtmlNode.kind` (`HtmlNode.ts:38-40`) | Returns `html`; used as the registered MML node kind. |
| `HtmlNode.getHTML` (`:45-47`) | Returns `getXML()` by identity, only type-cast in TypeScript. |
| `HtmlNode.setHTML` (`:54-68`) | Calls `adaptor.getAttribute(html,'data-mjx-hdw')` in a `try`; the result is ignored. Any exception causes `adaptor.node('span',{},[html])`; then the selected object and same adaptor are stored via `setXML`, and the node itself is returned. If adaptor/getAttribute fails and `node` also fails (including null adaptor), the second exception propagates. No filtering/sanitization is performed here. |
| `HtmlNode.getSerializedHTML` (`:72-74`) | Calls `adaptor.outerHTML(xml)` directly, returning serialized markup. Null adapter errors propagate. |
| `HtmlNode.textContent` (`:79-81`) | Calls `adaptor.textContent(xml)` directly; returned text feeds token `MmlNode.getText` (`MmlNode.ts:1053-1067`) for children with a `textContent` member. |
| `HtmlNode.toString` (`:88-90`) | Calls `adaptor.kind(xml)` then returns `HTML=<kind>...</kind>`. A missing adaptor or kind failure propagates. |
| `HtmlNode.verifyTree` (`:96-102`) | Reads `parent`; if parent exists and `!parent.isToken`, calls virtual `mError('HTML can only be a child of a token element', options, true)` and returns. Parent token children are permitted. The check itself needs no adaptor. |

These methods require host object identity to survive `setHTML`/`setXML`/`getHTML` and token text extraction. Serialization is a separate host operation and can fail after a successful copy because the source omits the adaptor when copying.

## Creation and consumption caller paths

`MathMLCompile.ts` (SHA-256 `2bb1f7258daf68fa6d51ea585b08312c1bc407719bd6711651369db1d5b68349`) has a separate input route, not active in `entry.cjs`:

* `compile` builds then verifies/inherits/walks the MML tree (`MathMLCompile.ts:102-108`).
* `makeNode` calls the configured DOM adaptor for `kind`, attributes, and classes, then factory lookup/creation (`:119-140`).
* `unknownNode` creates an HTML node only when both `factory.getNodeClass('html')` and `options.allowHtmlInTokenNodes` are truthy (`:179-189`); that option defaults false (`:47-59`). This path is separate from `allowTexHTML`.
* In `addChildren`, an `annotation-xml` parent wraps each non-comment/non-text child DOM object as `factory.create('XML').setXML(child, adaptor)` (`:275-307`). The XML object is retained as the child; `MmlAnnotationXML` deliberately has an empty inherited-attribute hook (`MmlNodes/semantics.ts:70-101`).

If a MathML input route is added to the native profile, `MathMLCompile` itself expects additional adaptor calls: `kind(node)` then strips a namespace prefix (`:119-127`); `getAttribute(node,'data-mjx-texclass')` uses an empty-string fallback (`:122-124`); `allClasses(node)` is iterated for `MJX-TeXAtom-*`/fixed-limits class behavior (`:128-135`); `allAttributes(node)` drives ordered attribute processing (`:201-235`); `childNodes(node)` is iterated and each child's `kind` distinguishes comments, text, `annotation-xml` content, and recursive MathML nodes (`:275-307`); text nodes are read with `value(child)` (`:318-334`). The misplaced-child check rereads `childNodes(child)` and branches on its live `.length` (`:292-305`). These operations are separate from the narrower TeX `texhtml` parser calls above. The current entry profile selects TeX input only, so this is an identified host gap for a future MathML-input profile, not an active call in today's Folia wrapper.

Consumers include the generic visitors and the SVG output path:

* `SerializedMmlVisitor.visitXMLNode` calls `getSerializedXML`; `visitHtmlNode` calls `getSerializedHTML` (`SerializedMmlVisitor.ts:56-71`). `MathMLVisitor` calls `getXML`/`getHTML`, then `cloneNode(true)` and appends that clone into the output document (`MathMLVisitor.ts:63-76`). `JsonMmlVisitor.visitXMLNode` returns `getXML()` directly, retaining the object (`JsonMmlVisitor.ts:77-82`). `LegacyMmlVisitor.visitXMLNode` passes `getXML()` to `MML.xml` (`LegacyMmlVisitor.ts:63-68`). The base `MmlVisitor` declares visit hooks but does not perform adaptor calls (`MmlVisitor.ts:98-112`).
* SVG registers `SvgHtmlNode`, which inherits `SvgXmlNode` (`output/svg/Wrappers/HtmlNode.ts:58-71`, source hash `902225b6605078eea19a893fa355875edb7adc5c5312924097e0c8d335b76b45`). `SvgXmlNode` is built on `CommonXmlNodeMixin` and its `toSVG` emits a `foreignObject` containing `getHTML()` (`output/svg/Wrappers/semantics.ts:273-349`, SHA-256 `6a68dd39649fcf05407dd12a700a294c78e9be140b7ceefe518523efae4bdb6e`).
* That common wrapper's reachable methods use a broader DOM surface (`output/common/Wrappers/XmlNode.ts`, SHA-256 `aa03369e8f9e4b36154c9e3d4b7ad9bed94fa0713c8ead00467a289e42654dea`): `computeBBox` reads `data-mjx-hdw` or measures live DOM (`:228-235`); `getHTML` clones the original XML, gets font styles, reads HDW and may wrap/adjust it (`:240-254`); `getHDW` calls `getAttribute` (`:263-268`); `measureXmlNode` clones content, creates `mjx-xml-block`, `mjx-baseline`, and `mjx-measure-xml` nodes, appends them beneath a live parent/container, calls `nodeBBox` twice, then removes temporary nodes (`:302-325`). SVG `toSVG` additionally calls `append`, constructs an SVG `foreignObject`, and uses `fixed`/`getBBox` (`semantics.ts:312-335`). This is the MathJax SVG HTML embedding/measurement route, not a path exercised by ordinary TeX glyph SVGs.

## Adaptor surface compared with native XML interface

The native `src/math/core/mml_tree/mml_node.ixx:232-251` currently has only:

* `XmlObject` as an opaque polymorphic base and `XmlRef = shared_ptr<XmlObject>`;
* `XmlAdaptor::clone(XmlRef) -> XmlRef` and `serializeXML(XmlRef) -> string`;
* `XMLNode` stores `XmlRef` plus a raw/non-owning `XmlAdaptor*`; `setXML`, `getXML`, `getSerializedXML`, `copy`, and `toString` are implemented in `mml_empty.cpp:70-95`.

Therefore XML clone/serialization calls have named native hooks, but there is no native HTML subtype or HTML-specific adaptor operation. Missing operations for `HtmlNode` itself are `getAttribute`, HTML `node` construction, `outerHTML`, `textContent`, and `kind`. Missing operations on enabled `texhtml` creation are `parse`, `body`, `childNodes`, and `innerHTML`; these require preserving DOM node references and parse failures/empty-vs-many child behavior. The SVG XML/HTML wrapper additionally needs `append`, DOM `parent`, `nodeBBox`, `remove`, and cloning/DOM node creation; the broader HTML handler paths refer to document/body/head and stylesheets. This report identifies the calls only; it does not decide whether those can be supplied by one service or multiple services.

Native `XMLNode::copy` currently creates the output node, checks a null adaptor and throws a manually synthesized TypeError, calls `clone`, and sets only the cloned XML (`mml_empty.cpp:74-93`). The visible identity/adaptor omission matches the source for a valid adaptor. The native typed `XmlRef` boundary cannot by itself model arbitrary input-node kinds or adaptor-specific errors; no concrete adaptor implementation was found under `src/math` (`rg XmlAdaptor` only finds this interface and the XMLNode calls).

## Suggested witnesses for the root-owned gate

The following cases would distinguish default policy from live host behavior without changing this audit's scope:

1. Default TeX `<tex-html>...</tex-html>` case: assert the existing exact literal rendering and no `html` MML node while `allowTexHTML` remains false.
2. Explicit `allowTexHTML=true` positive cases for one child, zero children, multiple children, nested markers, missing terminator, existing `data-mjx-hdw`, and a throwing `getAttribute` followed by successful/failing `node('span',...)`. Assert original/wrapped object identity and adaptor retained.
3. XMLNode cases with an instrumented adaptor: `setXML`/`getXML` identity; serializedXML argument identity; copy call order; clone result identity; output adaptor null after copy; later serialization's null failure.
4. `HtmlNode` direct cases: exact `outerHTML`, `textContent`, `kind` call logs; token-vs-nontoken verification; null adaptor and thrown adaptor methods.
5. If MathML input or MathML SVG wrapper is included in the actual native product profile, separately witness `annotation-xml` child identity and the SVG measurement route's temporary append/measure/remove order. Those are not exercised by the checked-in TeX entry profile, so they should not be inferred from the raw-TeX default fixture.

The current evidence establishes that the default Folia TeX-to-SVG path does not require the HTML host surface for ordinary formulas. It also establishes that the registered `texhtml` option can make the HTML path reachable and that the MathJax MML/SVG source has a larger XML/DOM dependency surface. The profile's current disabled case must not be treated as an acceptance test for that enabled path.
