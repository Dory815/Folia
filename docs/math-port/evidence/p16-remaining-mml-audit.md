# P16 prerequisite audit: remaining scoped MML node classes

Read-only inventory of the concrete `ts/core/MmlTree/MmlNodes` entries that `docs/math-port/scope.json` still marks `not_started`, excluding `HtmlNode` details already recorded in `evidence/p15d-html-host-audit.md`. Sources are pinned MathJax 4.1.3. This report maps source behavior and dependencies; it does not choose an implementation boundary or claim acceptance.

## Inventory, profile reachability, and shared prerequisites

The eight non-HTML MML node sources are all still `not_started` in scope. Their SHA-256 values are the frozen manifest values:

| Source/class | SHA-256 | Source inheritance and arity | TeX-string profile evidence |
|---|---|---|---|
| `MmlNodes/maction.ts` / `MmlMaction` | `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895` | `AbstractMmlNode`; arity `1` means at least one child. | Reachable: the profile loads `action`; `ActionConfiguration.ts:35-67` creates `maction` for toggle and tooltip actions. |
| `MmlNodes/maligngroup.ts` / `MmlMaligngroup` | `4d8ecdae93aaff85887ec4eb09c8821811866a9e9b56e037ac61d0b6d81bf2b1` | `AbstractMmlLayoutNode`; inherits inferred-row arity `-1`; spacelike. | No TeX `parser.create('maligngroup',...)` caller found. MathML input constructs by MathML element kind, but that input jax is absent from the profile. |
| `MmlNodes/malignmark.ts` / `MmlMalignmark` | `f35d8b9edcf1afce67f74eef1dc1c04434f9caea944b3f3fd439d1aaf484cb61` | `AbstractMmlNode`; arity `0`; spacelike. | No TeX constructor call found; reachable through MathML element compilation only, which is outside the current TeX-only entry. |
| `MmlNodes/mathchoice.ts` / `MathChoice` | `e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76` | `AbstractMmlBaseNode`; exact arity `4`; `notParent=true`. | Reachable: base package `\mathchoice` creates it with display/text/script/scriptscript children (`BaseMethods.ts:2275-2287`). |
| `MmlNodes/menclose.ts` / `MmlMenclose` | `ffa4ab507744cb4f190312ca8dcd692474c333bee6e19c86709554aa9dad4333` | `AbstractMmlNode`; arity `-1` inferred row; `linebreakContainer=true`; ORD TeX class. | Reachable: profile enables base, cancel, enclose, and other packages; base/enclose/cancel callers construct it (`BaseMethods.ts:1431-1458`, `EncloseConfiguration.ts:50-62`, `CancelConfiguration.ts:47-74`). |
| `MmlNodes/mfenced.ts` / `MmlMfenced` | `64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397` | `AbstractMmlNode`; inherits arbitrary-child arity; INNER TeX class. | No TeX node creation call found; MathML input can reach it by element kind, but is not part of the checked-in input profile. |
| `MmlNodes/mglyph.ts` / `MmlMglyph` | `2c2d336067d3970c55f3fcdafb82b55d4339a2c11d69088c8c7eff4a6d953504` | `AbstractMmlTokenNode`; inherits token no-child arity; ORD TeX class. | No TeX constructor call found; MathML input is its visible creation route. Output also requires an image or font glyph path, not just node semantics. |
| `MmlNodes/mpadded.ts` / `MmlMpadded` | `b5aac516c32fb019ac5ace504f86312f3e0849e1faa44006567bac05815c7271` | `AbstractMmlLayoutNode`; inferred-row arity `-1`; `linebreakContainer=true`. | Reachable broadly: base, ams, bbox, cancel, mathtools, empheq, bussproofs and other loaded packages create it. Examples in `BaseMethods.ts:116,972,1002,1145-1215,1346,1449-1475`, plus `AmsMethods.ts:571-586`, `BboxConfiguration.ts:105`, `MathtoolsMethods.ts:284-890`. |

The checked-in entry is TeX input plus SVG output (`scope.json` profile hash `a5a03eed8a10872b9b435b9e602523a08a9f574e21cc5636e8377f0222811ac9`; `entry.cjs:1-4,51-78`). Thus `maction`, `MathChoice`, `menclose`, and `mpadded` are live semantic prerequisites for profile macros. `maligngroup`, `malignmark`, `mfenced`, and `mglyph` are registered MathML node kinds but no direct TeX creator was found in the inspected input source; their MathMLCompile route remains outside this profile. The registration map in `MML.ts:41-72,99-130` maps all eight class prototype `kind` values to constructors.

Shared source dependencies are `MmlNode.ts` (scope accepted base, `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`), `Attributes.ts` (accepted, `c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a`), and `Tree/Node.ts` (accepted, `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0`). `MmlFactory.ts` is still `not_started` (`5e7338cd3eed5a4002243b7371dc5098c6433ff4ba4345cb2d50cf0385b2abdb`), although it is needed for each `factory.create` call and the kind-to-class registration identity. Current native modules have generic MML base classes and shared attributes, but there are no native declarations/implementations for these eight classes in `src/math/core/mml_tree/mml_nodes` at this audit snapshot.

Every `static defaults` declaration below creates a fresh object with object spread when the class is initialized; it is stable per constructor thereafter and copies base properties shallowly. This is distinct from assigning the base `defaults` object by identity. Mutable defaults/attribute handling and `INHERIT` sentinel semantics flow through the shared Attributes implementation; these node files do not deep-copy nested values.

## Per-class source behavior

### `MmlMaction`

Members: `defaults` (lines 36-41), `kind` (45-47), `arity` (54-56), `selected` (61-65), `isEmbellished` (70-72), `isSpacelike` (77-79), `core` (84-86), `coreMO` (91-93), `verifyAttributes` (98-110), `setTeXclass` (114-123), `nextToggleSelection` (127-138), and `setChildInheritedAttributes` (141-157). Defaults are a fresh shallow spread plus `actiontype:'toggle'`, `selection:1`.

`selected` reads raw `attributes.get('selection')`, clamps using `Math.max(1, Math.min(childNodes.length, selection)) - 1`, returns that child or creates a new `mrow` through the factory. With zero children or a non-index result this can create a new row each getter call; `isEmbellished`, `isSpacelike`, `core`, and `coreMO` each invoke the getter and delegate. `verifyAttributes` calls `super` first, then reads actiontype and, if it is not strict string `'toggle'` and `selection` is explicitly present, unsets it. `setTeXclass` first reads actiontype and checks child slot 1; for tooltip it lays out child 1 with null predecessor before retrieving `selected`; then selected child receives the original predecessor and `updateTeXclass(selected)` runs. `nextToggleSelection` string-coerces the selected attribute through `parseInt`, clamps the parsed result to at least 1, compares it with live child count, wraps to 1 when larger, and writes the resulting number (including NaN if parseInt produces NaN). Inheritance lowercases actiontype; non-tooltip delegates to superclass. Tooltip path independently sends child0 the supplied attributes/display/level/prime, and child1 `(attributes,false,1,false)`; missing slots are skipped with optional chaining.

### `MmlMaligngroup` and `MmlMalignmark`

`MmlMaligngroup` members: defaults (`:37-40`) with `groupalign: INHERIT`, kind (`:45-47`), `isSpacelike` (`:54-56`, true), and child inheritance (`:63-74`). It reads all of its effective attributes via `attributes.getAllAttributes()`, merges them into the incoming inherited-attribute list with `addInheritedAttributes`, then calls the layout superclass hook. Child propagation therefore uses the returned merged handle/value, not the original list.

`MmlMalignmark` members: defaults (`:36-39`) with `edge:'left'`, kind (`:44-46`), arity (`:53-55`, 0), and `isSpacelike` (`:62-64`, true). It has no class-specific factory or visitor behavior in this file.

### `MathChoice`

Members: defaults (`:39-41`, fresh spread of base-node defaults), kind (`:46-48`, exact capitalization `MathChoice`), arity (`:55-57`, exactly 4), `notParent` (`:64-66`, true), and `setInheritedAttributes` (`:74-84`). It is intentionally replaced during inherited-attribute propagation and should not remain in the finished tree.

Selection is `0` when `display` is truthy; otherwise it is `Math.max(0, Math.min(level,2)) + 1`. There is no integer conversion; a fractional or NaN property key misses the array and triggers `factory.create('mrow')`. The method then dereferences `this.parent`, calls `parent.replaceChild(child,this)`, and only after replacement calls `child.setInheritedAttributes(attributes,display,level,prime)`. A detached MathChoice fails at parent access before child inheritance; replacement side effects remain if inheritance throws.

### `MmlMenclose`

Members: defaults (`:36-39`) plus `notation:'longdiv'`; protected `texclass=ORD` field initializer (`:44`); kind (`:51-53`); arity (`:60-62`, inferred row); `linebreakContainer` (`:69-71`, true); and `setTeXclass` (`:76-80`). It lays out `childNodes[0]` with the passed predecessor, stores the returned predecessor, updates its own TeX class from that same child, then returns the child result. The direct index/dereference has no local empty-child guard; inferred-row construction/verification is an upstream invariant, and malformed state can throw. The class initializer order matters because the base constructor invokes virtual hooks before derived field initialization in the observed CJS construction sequence recorded in `evidence/p8b-normal.md`.

### `MmlMfenced`

Members: defaults (`:42-47`) with `open:'('`, `close:')'`, `separators:','`; protected `texclass=INNER` (`:52`); mutable fake-node storage `separators=[]`, `open=null`, `close=null` (`:57-68`); kind (`:70-72`); `setTeXclass` (`:80-106`); inherited-child hook (`:111-124`); `addFakeNodes` (`:129-177`); and `fakeNode` (`:180-194`). The source stores fake delimiters separately from actual `childNodes`.

`setChildInheritedAttributes` calls `addFakeNodes` before inheriting to the array `[open,close].concat(separators)` in that order, skipping nulls, then calls the superclass hook for real children. `addFakeNodes` obtains open/close/separators together with `attributes.getList`, removes ASCII space/tab/LF/CR by regex, creates open first, then separators, then close. It repeats the final separator UTF-16 code unit until separator string length is at least `childNodes.length-1`; iterates a `slice(1)` snapshot of children, skips falsey entries, and advances the separator index only for truthy entries. It appends created separators to the existing field array and does not clear prior fake fields, so repeated calls can retain/append prior nodes. Each delimiter gets `mo` properties; open and close use fence/form plus OPEN/CLOSE TeX classes. `fakeNode` factory-creates a `text` node and calls `setText`, factory-creates `mo` with that text child/properties, sets `texClass`, then manually assigns `parent=this`.

`setTeXclass` first calls `getPrevClass(prev)`, processes existing open, child0, then for `i=1` to the captured current child length processes separator `i-1` and child `i`, then close. It calls `updateTeXclass(open || child0 || close)` only if either open or close is absent. Thus fake-node creation normally precedes the class pass through inheritance, but direct invocation before that leaves fields null; no implicit `addFakeNodes` occurs in `setTeXclass`.

### `MmlMglyph`

Members: defaults (`:36-47`) add `alt:''`, `src:''`, `index:''`, `width:'auto'`, `height:'auto'`, `valign:'0em'`; protected ORD field (`:49`); kind (`:54-56`); and `verifyAttributes` (`:61-77`). Verification gets `src`, `fontfamily`, and `index` together from `attributes.getList`. If `src === ''` and either `fontfamily === ''` or `index === ''`, it calls `mError('mglyph must have either src or fontfamily and index attributes',options,true)` and does not call the superclass verifier. Otherwise it calls `super.verifyAttributes(options)`. Checks are strict empty-string comparisons over the post-default raw values. Rendering the accepted node still needs an image or font-backed glyph facility in the output wrapper; this class does not resolve resources.

### `MmlMpadded`

Members: defaults (`:36-44`) add blank width/height/depth, `lspace:0`, `voffset:0`; kind (`:48-50`); `linebreakContainer` (`:55-57`, true); and `setTeXclass` (`:62-70`). It tests `!this.getProperty('vbox')`; falsey values delegate directly to superclass. Truthy vbox path calls `getPrevClass(prev)`, sets `texClass=ORD`, calls `childNodes[0].setTeXclass(null)`, and returns `this`. There is no local missing-child check. Attribute parsing/geometry is not done in this semantic node method; exact output handling belongs to its separate common/SVG wrapper sources.

## Shared dependencies and known host boundaries

* The nodes use generic `MmlNode`/`AbstractMmlNode`/`AbstractMmlLayoutNode`/`AbstractMmlBaseNode` semantics, dynamic attributes, parent/child mutation, tree verification and TeX-class methods. Arity meanings (`0` no children, positive `1` minimum-one, `-1` inferred row, `Infinity` unrestricted) come from `MmlNode.ts:560-570` and the base-node overrides (`:1132-1142`).
* `maction.selected`, `MathChoice`, and `mfenced.fakeNode` call `factory.create` during normal getters/hooks. This depends on the unported `MmlFactory` and on class registration; errors or side effects can occur before later mutations/child visits.
* `INHERIT` for maligngroup comes from `Attributes.ts`; attribute-list identity, explicit/unset behavior, `getList`, `getAllAttributes`, and inheritance merging use that shared subsystem.
* No direct DOM API appears in these eight node implementation files. Host/output requirements are downstream: `maction`, `menclose`, `mfenced`, `mglyph`, and `mpadded` have common and/or SVG wrappers in `ts/output/common/Wrappers` and `ts/output/svg/Wrappers`; their measurement, glyph/image, paint, and scene behavior is not implemented by the semantic classes. `maligngroup`/`malignmark` have no direct TeX producer in the searched profile and normally depend on MathML table structure.
* `HtmlNode` is another scope `not_started` entry, but its adaptor calls and profile gate are in `evidence/p15d-html-host-audit.md`; it is omitted from the eight-class member inventory here to avoid duplicating that assigned audit.

## Source/caller file hashes inspected

| File | SHA-256 |
|---|---|
| `ts/core/MmlTree/MML.ts` | `c64bcfdf65452559995a92c13db75133b1500ef59efc2be5c9489ceacc4d4964` |
| `ts/core/MmlTree/MmlFactory.ts` | `5e7338cd3eed5a4002243b7371dc5098c6433ff4ba4345cb2d50cf0385b2abdb` |
| `ts/input/tex/base/BaseMethods.ts` | `7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda` |
| `ts/input/tex/action/ActionConfiguration.ts` | `0e647387a28c524bd2d4b7a57758b78bfc95555297874ecf3e46da8a1d163b1d` |
| `ts/input/tex/enclose/EncloseConfiguration.ts` | `318e8e6a80dc40b2fc12441dc597fcc493e54ed241f10105adb36c57e200c2d8` |
| `ts/input/tex/bbox/BboxConfiguration.ts` | `0e004e3b4c22c5935c12fc4609962e84dcba41fdb6935cb7d1685553aab18867` |
| `ts/input/tex/cancel/CancelConfiguration.ts` | `684b21b3eb7eab7ea80e225dc57c5690594cac01cc425e9da41bad3ce26ac5d7` |
| `ts/core/MmlTree/Attributes.ts` | `c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a` |
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |

The audit did not enumerate every wrapper method or validate rendering parity; it establishes only source semantics and caller reachability needed before root freezes the next native packet.
