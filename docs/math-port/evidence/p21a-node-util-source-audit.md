# P21a `NodeUtil.ts` dependency and interface audit

This is a read-only source/interface audit for the pinned MathJax 4.1.3
`ts/input/tex/NodeUtil.ts`, SHA-256
`bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2`. No
native implementation or acceptance decision is included.

## Complete source object inventory

The source file exports one initialized object, `NodeUtil` (line 36), with the
`attrs` Set field and all 24 methods below. Ranges identify implementation lines,
not their preceding documentation comments.

| Member | Source lines | Behavior / native interface correspondence |
|---|---:|---|
| `attrs` | 37–49 | Mutable Set containing `autoOP`, `fnOP`, `movesupsub`, `subsupOK`, `texprimestyle`, `useHeight`, `variantForm`, `withDelims`, `mathaccent`, `open`, `close`. No NodeUtil static Set/object exists in native code. |
| `createEntity` | 57–59 | `parseInt(code, 16)` followed by `String.fromCodePoint`. Native `js_input::parseInt(..., Hex)` and `string_util::unicodeString(span<double>)` provide the two operations; invalid hex produces NaN, which `String.fromCodePoint` rejects with RangeError, matching the native helper. |
| `getChildren` | 67–69 | Returns the node’s original mutable `childNodes` array handle. Native `tree::FactoryChildren<MmlNode>` is a shared handle to `vector<MmlNode*>`; it preserves mutations through an existing handle, but not JavaScript slot/presence semantics. |
| `getText` | 77–79 | Delegates to `TextNode.getText()`. Native `TextNode::getText()` exists; MML token text is separately available on `AbstractMmlTokenNode`. |
| `appendChildren` | 87–91 | Iterates the supplied children and calls `node.appendChild(child)` for each, preserving virtual MML append behavior. `MmlNode::appendChild` exists. |
| `setAttribute` | 100–102 | Calls `node.attributes.set(attribute, value)`. Native `MmlNode::attributes()` returns the shared `Attributes` handle, which has `set`. |
| `setProperty` | 111–113 | Calls virtual `node.setProperty`. Present on `tree::Node` and implemented by `AbstractNode`. |
| `setProperties` | 121–140 | Snapshots `Object.keys(properties)`, then reads each value and mutates sequentially. Branches: `texClass` accessor setter then `setProperty`; `movablelimits` property write then `isKind('mo') || isKind('mstyle')` and attribute write; ignore `inferred`; otherwise classify with live `NodeUtil.attrs.has` and write property or attribute. Native pieces exist (`getPropertyNames`/`PropertyList::keys`, `setTexClass`, `setProperty`, `isKind`, `Attributes::set`), but no single adapter preserves object/Set receiver semantics or arbitrary property getters. |
| `getProperty` | 149–151 | Delegates to virtual `getProperty`. Present on generic Node. |
| `getAttribute` | 160–162 | Reads merged/inherited/defaulted `attributes.get`. Native `Attributes::get` exists. |
| `removeAttribute` | 170–172 | Calls `attributes.unset`, which deletes only the explicit attribute. Native `Attributes::unset` exists. |
| `removeProperties` | 180–182 | Spreads the rest arguments into virtual `node.removeProperty`. Native accepts a span of names, so callers need an ordered collected name sequence. |
| `getChildAt` | 191–193 | Direct array indexing; out-of-range and sparse reads yield JavaScript `undefined`, while explicit null yields null. Native child pointer access currently cannot distinguish those cases. |
| `setChild` | 202–208 | Captures `node.childNodes`, assigns at the numeric property key (growing/sparsifying as JS arrays do), then if child is truthy assigns `child.parent = node`. It does not clear the replaced child’s parent and does not call `appendChild`/`replaceChild`. |
| `copyChildren` | 216–221 | Captures the original children array once, reads its live length and indexed slots, then calls `this.setChild(newNode, i, child)` for each. Child array identity is retained across callbacks; the target child array is read afresh inside `setChild`. |
| `copyAttributes` | 229–234 | First aliases `newNode.attributes = oldNode.attributes`; then snapshots `Object.entries(oldNode.getAllProperties())` and calls `newNode.setProperty` for each entry. Attributes and generic properties are distinct stores. Native `setAttributes` preserves the shared `Attributes` pointer, and `getAllProperties` returns a shared `PropertyList` handle, but full ordering must retain the alias before the property snapshot/dispatch. |
| `isType` | 243–245 | Delegates to `node.isKind`. Present on generic Node/MmlNode. |
| `isEmbellished` | 253–255 | Reads the MmlNode `isEmbellished` property/getter. Native exposes virtual `isEmbellished()`. |
| `getTexClass` | 263–265 | Reads the virtual `texClass` accessor. Native `texClass()` returns raw `Property`, which is needed for the dynamic `mo` getter and raw values; reading backing `texclass_` would be wrong. |
| `getCoreMO` | 273–275 | Calls `coreMO()`. Present on MmlNode. |
| `isNode` | 283–287 | `instanceof AbstractMmlNode || instanceof AbstractMmlEmptyNode`; both branches matter. Native has separate RTTI families `AbstractMmlNode` and `AbstractMmlEmptyNode`, with the latter covering empty/Text/XML nodes. |
| `isInferred` | 295–297 | Reads `node.isInferred`. Native exposes virtual `isInferred()`. |
| `getForm` | 306–319 | First tests `isKind('mo')`; then gets forms from `MmlMo.getForms()` and iterates in returned order, invoking `this.getOp(mo, form)` and returning the first truthy definition or null. This is a NodeUtil-object dispatch, not `MmlMo::getOperatorDef`. |
| `getOp` | 329–331 | Has default `form='infix'`; directly reads `MmlMo.OPTABLE[form][mo.getText()] || null`. It does not compute forms, range fallback, operator defaults, or constructor-specific overrides. Native has `MmlMo::operatorTable()` and `operator_dict::OPTABLE()` backing the raw table, plus `MmlMo::getText()`, but the NodeUtil method/object binding is absent. |
| `getMoAttribute` | 341–351 | If `!mo.attributes.isSet(attr)`, checks `infix`, `postfix`, `prefix` by `this.getOp(mo, form)` in that order; reads optional definition slot `[3]` then `[attr]`, returning the first value not `undefined`. Otherwise returns `mo.attributes.get(attr)`. Native has `Attributes::isSet/get`, PropertyArray records for operator definitions, and property access helpers; it must keep this direct OPTABLE path and `this.getOp` dispatch. |

## Native state and protocol observations

The current native snapshot used for this comparison is identified here so
root can re-check the audited declarations/implementations independently:

| Native file | SHA-256 | Relevant lines |
|---|---|---|
| `src/math/core/tree/node_types.ixx` | `5f278b03d99bc40026b46fb38698d1ab4ca0dfc9fc65fa143ad7a1e9879e165a` | 11–14 (`parent`, `childNodes` storage). |
| `src/math/core/tree/factory_types.ixx` | `25542be82bc00641c5f5880362aaae8f8c022b635df0914011bff33f10ba4033` | 35 (`FactoryChildren` shared vector alias). |
| `src/math/core/tree/node.ixx` | `0be5adbf63239fb4bcbfd459f9267d59c10a3f713aae2c1e202cf64888ec7fd1` | 46–61 property methods; 71–113 child mutation/index behavior. |
| `src/math/core/tree/property.ixx` | `3c49619f8600b15f7738a8ea2f253a35594c75785aef97163f998bc592c07701` | 18–50 `Property`/storage; 53 onward PropertyList; 229–255 PropertyArray and live slot/spread operations. |
| `src/math/core/mml_tree/mml_node.ixx` | `36f47c27f389ac337c8f08dd890d066ba362f8ed9e51ea9ad53cad999ceeba3e` | 12 `AttributeList`; 57–83 MmlNode methods; 92 onward AbstractMmlNode and empty/token families. |
| `src/math/core/mml_tree/attributes.ixx` | `473a07a74349c985f16b4dcc60c927a785aad7d7eb04896d32e8ce54aa3a3d78` | 33–39 storage layers; 50–105 get/set/isSet APIs. |
| `src/math/core/mml_tree/mml_nodes/mo.ixx` | `608146e0720ffdecaee92cfa218e38f506391cfc78f6b517afa1cb8ba5c5b204` | 10–11 operator lookup hook; 22–24 static operator table; 40–50 texClass/getForms declarations. |
| `src/math/core/mml_tree/mml_nodes/mo_lookup.cpp` | `46b7ca32780edcbb46bedcaa3220cc95ec4c16b3afa84c5b6ebefaacba9912ed` | 44–82 constructor-specific `getOperatorDef`; 115 onward `getForms`. |
| `src/math/core/mml_tree/mml_nodes/mo_bindings.cpp` | `c0727bba412e644458765816f9cd0696695f470c8a129416f11a3847ba52434c` | 31–42 static table/spacing binding and `MmlMo::operatorTable()`. |
| `src/math/core/mml_tree/operator_dictionary.ixx` | `e790872c81cca0ec1919215d58392415510423c20326c6f336bbf4b9461831c6` | 7–15 `OperatorDef`, `PropertyArray`, `OPTABLE` declarations. |
| `src/math/util/js_numeric_input.ixx` | `f27bc4eed4a0b5120fb17d7bbc937c62ed7bb7a124d98e0da0aac8b38878dc5f` | 7–12 parseInt host declaration and radix enum. |
| `src/math/util/string.ixx` | `662e628055a087429beb9a80581698f454d2ff8399572bc28bdf348a8c86f93b` | 8–12 `unicodeString` declaration. |
| `src/math/util/string.cpp` | `2fd81b4393fdc3eb9c157c09ae4928e3658e3bbc51cc39c6741e6a44728c638e` | 50–59 parse/range helper; 88–98 `unicodeString` validation and encoding. |

Pinned generic `Node.ts` (SHA-256
`532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0`, lines
30–31, 41–49, 54–86, 159–197) defines ordinary properties separately from
attributes, a mutable `childNodes: N[]`, and a mutable `parent`. Native
`src/math/core/tree/node_types.ixx` stores `childNodes` as
`shared_ptr<vector<Product*>>` and `parent` as a raw pointer. This is an
identity-bearing live container, but has no per-index presence bitmap: a
`nullptr` slot cannot distinguish an array hole/undefined from explicit null.
There is no native numeric array-property API for negative/fractional/NaN
indices, sparse extension, or `length`/slot distinction. Direct vector resizing
can represent gaps as null pointers, but does not retain JavaScript hole
identity. This matters to `getChildAt`, `setChild`, `copyChildren`, and enabled
TeX code that directly pushes null child slots.

Pinned `MmlNode.ts` (SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`, lines
39–40, 138–230, 443–459, 467–490, 497–505) defines `AttributeList` as
origin/value tuples and MML child/property/accessor behavior. Native
`mml_node.ixx` carries `AttributeList=Property`, virtual property-valued
`texClass()`/`setTexClass()`, shared `Attributes`, and typed child pointers.
`MmlMo::texClass()` is dynamic and its setter targets its own `_texClass`;
`AbstractMmlEmptyNode::setTexClass()` throws. Thus property writes must dispatch
through these accessors in source order. Native property storage is independent
from the shared `Attributes` object, matching the source distinction.

Pinned `Attributes.ts` (SHA-256
`c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a`, lines
39–69, 75–103, 110–120, 151–189, 222–245) implements four live layers
explicit→inherited→defaults→globals. The native `Attributes` API has matching
`set`, `unset`, `get`, `getExplicit`, `hasExplicit`, `setInherited`,
`getDefault`, `isSet`, and shared layer handles. For `getMoAttribute`, `isSet`
checks explicit plus inherited only, while `get` can return inherited/defaulted
values; the dictionary scan happens only when neither explicit nor inherited
values are set.

Pinned `Types.ts` (SHA-256
`924d0b48bf7b5fdb0ccc995171fbdfb001bc3ac51a0af1409bb8c1d953430927`, lines
28–41) types TeX `Args` as `boolean | number | string | null`; the native
`Property` graph additionally tracks Missing and identity-bearing objects for
the host semantics already accepted elsewhere.

## Unicode/entity boundary

`createEntity` is lines 57–59 of NodeUtil.ts. Its actual operation is
`String.fromCodePoint(parseInt(code, 16))`, not a fixed-width hex decoder.
`js_numeric_input.ixx` provides `parseInt(u16string_view, Hex)` with JavaScript
prefix/sign/trailing-nondigit parsing and binary64 result. `string.ixx` exposes
`unicodeString(span<const double>)`, corresponding to pinned `ts/util/string.ts`
(SHA-256 `024a2ac6f96a314884f2bd71a913060c06fb75d46cbf6c94e15f4e71643a7084`,
lines 67–69), whose implementation in `string.cpp`
checks finiteness, integer-ness and `0..0x10FFFF`, then encodes BMP units or a
surrogate pair. `String.fromCodePoint(NaN)` throws `RangeError: Invalid code
point NaN` (and an undefined argument likewise throws); this was independently
checked with Node 24.13.0. Since `parseInt` returns NaN for invalid hex,
`createEntity` also throws in that case. The native `unicodeString` rejects
NaN and out-of-range/infinite points with the corresponding range failure, so
the existing `parseInt(Hex)` plus a one-element `unicodeString` span matches
the relevant conversion behavior. There is no missing `fromCodePoint` host
operation identified by this audit.

## Operator table boundary

Pinned `MmlNodes/mo.ts` (SHA-256
`2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605`, lines
81–87, 189–201, 320–340, 433–450) declares static `MmlMo.OPTABLE`, `getForms`,
and an instance `getOperatorDef`. Native `MmlMo::operatorTable()` retains a
Property handle to `operator_dict::OPTABLE()`, and `getForms()`/`getText()` are
present. However, `MmlMo::getOperatorDef()` intentionally uses captured
constructor services, form fallback order, `noDictDef`, operator-name
fallback and Unicode range defaults. NodeUtil.getOp must instead directly
look up the static `MmlMo.OPTABLE[form][text]` and collapse falsy values to
null. Reusing `getOperatorDef()` would change both the value and side effects.

Pinned `OperatorDictionary.ts` (SHA-256
`9afac6e3a8f3b778247a5a678c529b9edddb8172850df7384511bf61dfbffd37`) is the
static dictionary data/lookup source; native `operator_dictionary.ixx` exposes
`OPTABLE`, and operator definitions are identity-bearing `PropertyArray`
values. `NodeUtil.getMoAttribute` reads definition slot 3 and then the named
attribute, skipping only Missing/undefined, not null or other falsy values.
The inherited `getOp` lookup is called as `this.getOp` from both `getForm`
(NodeUtil.ts:313) and `getMoAttribute` (line 344); a native implementation that
hardwires a different Mo lookup bypasses this object-method dispatch.

## Enabled-profile call domain

The `rg` inventory over pinned `ts/input/tex/**/*.ts` finds NodeUtil references
throughout the enabled base path and enabled packages: base methods/items,
NodeFactory, Stack, ParseUtil/ParseOptions, FilterUtil, plus ams/amscd,
boldsymbol, bussproofs, color, centernot, html, mathtools, physics, unicode and
textmacros. Representative callsites confirm the data requirements:

| Caller | Source hash | Relevant source lines |
|---|---|---|
| `input/tex/base/BaseMethods.ts` | `7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda` | 810, 824–826 call `createEntity`/`setChild`, including an explicit null child slot; 647, 654 use child copies; 658–665 exercise core operator property routing. |
| `input/tex/NodeFactory.ts` | `bf20e8d4f7ed43450df42d65556ccbceea1469541b214c37544f180aa205c840` | 83 calls `setProperties` on newly created MML nodes. |
| `input/tex/FilterUtil.ts` | `dc821e2483fcd0820159a13d984b2bf299952e8c9f40418d72ff4c183d93053a` | 113, 142 copy attributes; 164 form lookup; 233 forwards a child array to append. |
| `input/tex/mathtools/MathtoolsMethods.ts` | `ad6562e23ba65cafdb480afc33387982c7f7b14a2886ae7844dfbe64ec44e918` | 218 iterates a returned live child list; 919 appends two null slots through that same list. |
| `input/tex/html/HtmlMethods.ts` | `128fcaa123ed0ea01c1ce228c1038120e6ecee90ce770b1814eedb986fd573a8` | 157–158 copy children then alias/copy attributes. |
| `input/tex/Stack.ts` | `36be7583b14111961367e9e3f1dd5911456d235b096a2fcee62f438d37c7e11a` | 89 classifies stack content using the two-family `isNode` check. |

The TeX profile in `docs/math-port/scope.json` (SHA-256
`53e1bef466ae717954c692123b133d889921aaf4b89ceaff21e37730a0c22ee3`, package
entries beginning at lines 9–12) enables these package paths;
this audit does not assume arbitrary JavaScript callers or mutable monkeypatches
are part of the raw-TeX interface. It records the source `this` dispatch
because it is observable in the pinned exported object itself.

## Concrete native gaps for root

1. There is no native `NodeUtil` object/module, no declaration for its mutable
   `attrs` Set, and no object method receiver through which `this.getOp` and
   `this.setChild` dispatch can be represented.
2. The child array handle is live, but its dense `vector<MmlNode*>` has no hole
   versus explicit-null distinction or JS numeric property-key/length protocol.
   Source callers directly push null slots and can observe indexed undefined.
3. `createEntity` can compose the existing `parseInt(...,Hex)` and
   `unicodeString` operations; invalid hex produces NaN and both JavaScript
   `String.fromCodePoint` and the native helper reject it. No conversion-host
   gap is identified here.
4. `MmlMo` native table data and helpers exist, but NodeUtil requires a separate
   raw static `MmlMo.OPTABLE[form][getText()] || null` path. The similar-looking
   `MmlMo::getOperatorDef` has extra dynamic-constructor lookup, form fallback,
   metadata mutation, and range/default behavior and is not interchangeable.
5. Attribute/property storage and accessors are present, including readonly
   empty-node setters and raw `Property` TeX class values. A full port must
   preserve the sequential `setProperties` branch order and shared-attribute
   identity in `copyAttributes`; bypassing them with backing fields would lose
   source dispatch and partial mutation behavior.
6. `isNode(item: any)` accepts arbitrary JS values at source level. Native
   RTTI can distinguish the two MML families once given a typed object, but
   the current MmlNode pointer API has no general value/object receiver that
   can also safely classify primitives and unrelated object identities.
