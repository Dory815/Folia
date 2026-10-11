# P15 prerequisite audit: semantics and HTML-in-MathML nodes

This is a source audit only. It does not decide which source behavior may be omitted from the native port.

## Locked source and scope

The pinned MathJax source is `.reference/MathJax-src` (4.1.3). SHA-256 values:

| Source | SHA-256 | Relevant lines |
|---|---|---|
| `ts/core/MmlTree/MmlNodes/semantics.ts` | `e609dfc3d7510a996a87804e3929cd4f53a5aa060003e351f45f1fc5dcfb6e96` | classes at 31–130 |
| `ts/core/MmlTree/MmlNodes/HtmlNode.ts` | `1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665` | class at 32–103 |
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` | base child inheritance and verification |
| `ts/core/MmlTree/MML.ts` | `c64bcfdf65452559995a92c13db75133b1500ef59efc2be5c9489ceacc4d4964` | imports 66–72; registry 84–136 |
| `ts/input/tex/texhtml/TexHtmlConfiguration.ts` | `e31f1539452eba33048d8e7bcf87fcf9690f23be1412799ddf2a6f43beab4a3b` | parser method 38–86; package options/hooks 90–120 |

The `MML` object registers `semantics`, `annotation`, `annotation-xml`, and `html` as distinct kind-to-constructor entries. The scope manifest includes the semantics node file (entry around `docs/math-port/scope.json:889`) and `HtmlNode.ts` (around line 395) as unimplemented port entries. `MML.ts` itself depends on both `semantics.ts` and `HtmlNode.ts`. These are source-level factory availability facts, independent of whether a given TeX input reaches a node.

## `semantics.ts` members and source behavior

The file declares three classes; its comments have stale copy/paste descriptions, so the actual declarations and executable members govern:

| Class | Parent | Own fields/members | Consequence |
|---|---|---|---|
| `MmlSemantics` (lines 32–68) | `AbstractMmlBaseNode` | static `defaults` is a fresh shallow object spread from base-node defaults with `definitionUrl:null`, `encoding:null`; getters `kind='semantics'`, `arity=1`, `notParent=true` | Uses base-node behavior. The `notParent` flag makes parent/embellished-node traversal skip this wrapper. Generic child verification still applies with minimum arity 1; no local verification override exists. |
| `MmlAnnotationXML` (lines 70–101) | `AbstractMmlNode` | static `defaults` is a fresh shallow object spread from generic MML defaults plus `definitionUrl:null`, `encoding:null`, `cd:'mathmlkeys'`, `name:''`, `src:null`; getter `kind='annotation-xml'`; protected `setChildInheritedAttributes()` is an empty override | It does not propagate inherited attributes to its children. Its inherited generic `verifyTree`/`verifyChildren` behavior remains in force. |
| `MmlAnnotation` (lines 103–130) | `MmlAnnotationXML` | static `defaults` is another shallow spread (fresh object, same property values); instance field `properties={isChars:true}` (fresh object per instance); getter `kind='annotation'` | Inherits the annotation-XML no-op inheritance hook and generic verification. `properties` is node metadata, separate from the static attribute defaults. |

No class in this file defines a constructor, child-array field, `verifyTree`, `verifyChildren`, `setTeXclass`, or error method. Initialization, parent/children identity, and generic verification are inherited; they should not be reimplemented as per-class special cases. `MmlSemantics` derives from the base-node branch while the two annotation classes use the ordinary MML-node branch. The `semantics` minimum arity is one; the annotations do not override the generic arity contract.

## `HtmlNode.ts` dependency surface

`HtmlNode<N>` extends `XMLNode` and has these own executable members:

| Member | Source order and behavior |
|---|---|
| `kind` (lines 38–40) | Returns `html`. |
| `getHTML()` (45–47) | Returns the current `getXML()` result by cast; no cloning or serialization. |
| `setHTML(html, adaptor=null)` (54–68) | First calls `adaptor.getAttribute(html,'data-mjx-hdw')` inside a broad `try`. Any thrown error triggers `adaptor.node('span',{},[html])`; it then calls `setXML(html,adaptor)` and returns that result. The adaptor's default is actually `null`; missing/null adaptor therefore throws in `getAttribute`, enters the catch, then throws again on `adaptor.node`. Non-attribute errors from `getAttribute` also take the wrapping branch. |
| `getSerializedHTML()` (72–74) | Directly calls `this.adaptor.outerHTML(this.xml)`; no null check or local error conversion. |
| `textContent()` (79–81) | Directly calls `this.adaptor.textContent(this.xml)`. |
| `toString()` (88–91) | First calls `this.adaptor.kind(this.xml)`, then returns `HTML=<kind>...</kind>`. The `kind` lookup must happen before string construction; absent adaptor/XML errors are not caught. |
| `verifyTree(options)` (96–102) | Reads current `parent`; only when parent is truthy does it read `parent.isToken`. If parent exists and is not a token, calls `mError('HTML can only be a child of a token element', options, true)` then returns. Otherwise returns without generic superclass verification, child traversal, or arity checking. |

Its inherited XML methods retain `XMLNode` behavior and state (`xml` plus adaptor). The current native boundary has `XMLNode`, `XmlRef`, and `XmlAdaptor` with `clone` and `serializeXML` (`src/math/core/mml_tree/mml_node.ixx:232–251`), but no HTML adaptor operations `getAttribute`, `node`, `outerHTML`, `textContent`, or `kind`, nor an `HtmlNode` subtype/member binding. The existing XML adaptor is therefore not by itself evidence that the HTML methods are covered.

## Raw-TeX reachability and profile facts

The checked-in profile eagerly loads `TexHtmlConfiguration` from `tools/mathjax-bundle/entry.cjs:41` and lists the `texhtml` package in its TeX package array at lines 60–66. That establishes package registration, not successful HTML parsing. In the pinned configuration:

* `TexHtmlConfiguration.OPTIONS.allowTexHTML` defaults to `false` (lines 92–99), with a comment explicitly requiring opt-in because raw HTML insertion is unfiltered.
* `HtmlNodeMethods.TexHTML` immediately returns `false` when `parser.options.allowTexHTML` is false (lines 44–49); with the checked-in default, serialized `<tex-html>` text is consequently left to the TeX parser rather than creating an HTML node.
* If enabled, the method scans for a matching `</tex-html>` marker (including the nested marker form), trims the enclosed HTML, obtains the adaptor from package data, parses HTML, returns on zero child nodes, wraps multiple nodes in a `div`, creates an `html` node, calls `setHTML`, puts that node under `mtext`, pushes it, and returns true (lines 50–86). This route depends on an HTML document/adaptor, not just TeX parsing.
* The package preprocessor stores `data.document.adaptor` in package data; its postprocessor resets that entry to null (lines 105–120). This is a stateful lifetime boundary for any HTML node that survives parsing.

The frozen profile corpus has `texhtml_texhtml_default_policy` in `docs/math-port/reference-corpus.json:1306–1330`, classified `profile_disabled`. Its note records that this default case emits literal markup over ten SVG fragments and does not construct an HTML node. It is a required observed profile outcome, not evidence that the enabled HTML branch is reached or irrelevant. Core `HtmlNode` registration in `MML.ts`, the enabled-option path in `TexHtmlConfiguration`, and any explicit host/factory construction remain separate reachable interfaces to account for.

## Output consumers and unresolved native boundary

The common semantics wrapper reads child 0 for its bounding box (`ts/output/common/Wrappers/semantics.ts`, `computeBBox`) and selects break count from either the embellished core or child 0. The SVG semantics wrapper renders only child 0 when children exist. SVG annotation currently delegates to the superclass with a source FIXME to render plain text and returns its existing bbox; annotation-XML's SVG wrapper declares foreign-object styles. These are output-wrapper responsibilities, not members of `semantics.ts`; their class-kind dispatch is another dependency if matching these nodes in SVG output.

Current native MML types implement the generic `MmlNode`/`AbstractMmlNode`/`AbstractMmlBaseNode`/`XMLNode` branches, but no native `MmlSemantics`, `MmlAnnotationXML`, `MmlAnnotation`, or `HtmlNode` class is present in `src/math/core/mml_tree`. Existing `XMLNode::toString()` returns a generic `XML data` string, whereas `HtmlNode::toString()` derives the exact element kind through the adaptor. Root-owned interfaces still need to determine how these source identities, fresh defaults/property objects, specialized inheritance/verification overrides, and HTML adaptor calls are represented; this audit does not choose an omission or fallback.

## Source uncertainty

The pinned files establish the exact branch behavior above. They do not establish that the raw-TeX user API can set `allowTexHTML`, preserve the adaptor after the postprocessor clears package data, or invoke arbitrary DOM operations. The current native typed boundary's supported host operations and lifecycle must be checked against the profile contract before assigning those behaviors. The stale class comments naming `MmlMroot` are documentation errors, not evidence of different class identity.
