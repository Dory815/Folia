# P12 MmlMo dependency audit

Read-only prerequisite audit for the pinned MathJax 4.1.3 source. Primary file
`.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mo.ts` has SHA-256
`2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605`.
No source, test, tool, interface, or contract file was changed; no build was run.

## MmlMo members and initialization

The class extends `AbstractMmlTokenNode` (line 47), declares no constructor,
and relies on JavaScript class-field initialization after the base constructor.
Its static declarations evaluate while the module initializes; the inherited
defaults spread is a one-time object copy, while the two dictionary statics
below are direct object aliases.

| Source member | Lines | Initialization or behavior |
|---|---:|---|
| `static defaults` | 52–79 | Creates a new ordinary property object by spreading `AbstractMmlTokenNode.defaults`, then sets the operator attributes (`form`, fence/separator, spacing, stretch, limits, accent, and linebreak/indent defaults). Later changes to the base defaults are not copied into this object. |
| `static MMLSPACING` | 81 | Direct reference alias to imported `MMLSPACING`; in-place changes are shared, later rebinding of either class field or module export separates them. |
| `static OPTABLE` | 86 | Direct reference alias to imported `OPTABLE`, with the same alias-versus-rebinding distinction. |
| `static pseudoScripts` | 92–108 | Builds the first of five MmlMo regex fields: a non-global, non-Unicode `RegExp` from the listed fragments joined with `''`; anchored start/end, one or more class members. |
| `static primes` | 110–118 | Builds the second regex field, another non-global, non-Unicode anchored regex from quote and U+2018–U+201F ranges. |
| `static opPattern` | 121 | Third regex field: `/^[a-zA-Z]{2,}$/`, ASCII letters only, no flags. |
| `static remapPrimes` | 126–139 | New ordinary numeric-key property object mapping quote code points to prime code points. Static reads are dynamic through `this.constructor`; inherited `Object.prototype` names remain visible for unmatched property lookups. |
| `static mathaccents` | 145–165 | Fourth regex field: builds an anchored non-global regex from the single-character accent list, joined with `''`; no Unicode flag. |
| `static mathaccentsWithWidth` | 169–177 | Fifth regex field: builds an anchored non-global regex for arrows and over/under parentheses/braces; no Unicode flag. |
| `_texClass` | 180 | Instance field initialized to `null`. |
| `get texClass` | 189–194 | If `_texClass === null`, calls `getOperatorDef(getText())[2]` each time; does not cache the result. Otherwise returns the stored value. |
| `set texClass` | 199–201 | Writes `_texClass`. |
| `lspace`, `rspace` | 207, 213 | Instance defaults of `5/18`; `checkOperatorTable` later replaces them from operator dictionary values divided by 18. |
| `get kind` | 218–221 | Returns `'mo'`. |
| `get isEmbellished` | 227–230 | Always returns true. |
| `coreParent()` | 235–252 | Resolves the registered `math` node class from the factory; follows embellished parents whose `coreMO()` is this operator, stopping before a `math` instance. Returns the outermost matched node or this. |
| `coreText(parent)` | 255–276 | Null returns `''`; embellished nodes use `coreMO().getText()`. Otherwise unwraps singleton `mrow`, `TeXAtom`, `mstyle`, `mphantom`, and `munderover` children while present, then returns token text or `''`. |
| `hasSpacingAttributes()` | 279–282 | Short-circuits `attributes.isSet('lspace') || attributes.isSet('rspace')`. |
| `get isAccent` | 286–313 | Uses `coreParent().parent`; branches by mover/munder/munderover and virtual `over`/`under` indices; then reads explicit parent accent key or this node's `accent`. Its explicit-parent branch returns the raw local attribute value when `getExplicit(key)` is undefined, despite the TS boolean annotation. |
| `setTeXclass(prev)` | 320–344 | Reads `form,fence` together with `attributes.getList`; if no explicit `texClass` and spacing attributes exist, returns null. Otherwise REL fences become OPEN for prefix or CLOSE for postfix, then delegates to `adjustTeXclass`. |
| `adjustTeXclass(prev)` | 347–406 | Applies previous-class and autoOP rules, stores `prevClass` and inherited script level, adjusts BIN classes at starts/after specified classes, and checks whether the embellished node is last in its parent. Mutations and early returns are source ordered. |
| `setInheritedAttributes(...)` | 413–430 | Defaults are `{}`/false/0/false. Calls `super` first, then reads `getText()` once and calls `checkOperatorTable`, `checkPseudoScripts`, `checkPrimes`, `checkMathAccent` in that order. |
| `getOperatorDef(mo)` | 433–455 | Gets explicit-form order; sets inherited `form` before reading class statics. Reads `CLASS.OPTABLE[form1][mo] || ...form2... || ...form3...` left-to-right by truthiness and returns the exact hit definition array. On miss sets `noDictDef`, reads `movablelimits`, matches `CLASS.opPattern`, then chooses fresh OPDEF(1,2,OP) or `getRange(mo)` plus `CLASS.MMLSPACING[range[2]]` and fresh OPDEF. |
| `checkOperatorTable(mo)` | 458–472 | Calls `getOperatorDef`; conditionally sets texClass from slot 2; enumerates own `Object.keys(def[3] || {})` and sets each inherited attribute; finally writes left spacing from slot 0, then right spacing from slot 1, divided by 18. |
| `getForms()` | 475–499 | Walks Parent/parent while embellished; uses the outer core and mrow `nonSpaceLength`, `firstNonSpace`, `lastNonSpace` to pick prefix/infix/postfix order. Returns a fresh three-element array. |
| `handleExplicitForm(forms)` | 504–512 | If `form` is set, reads it and returns `[form].concat(forms.filter(name !== form))`; otherwise returns the same input array. |
| `checkPseudoScripts(mo)` | 518–532 | Reads regex from dynamic constructor; on match checks outer core parent and msubsup/ msub kinds, sets `pseudoscript`, then writes inherited `lspace=0` followed by `rspace=0` if pseudo. |
| `checkPrimes(mo)` | 536–545 | Dynamic constructor regex; if matched, reads the dynamic remap object, maps UTF-16 code points through indexed lookup, converts with `unicodeString`, then writes `primes`. |
| `checkMathAccent(mo)` | 549–568 | Reads virtual `Parent`; exits if `mathaccent` property is defined, parent absent, or parent not munderover. Destructures `[base,under,over]`; checks base, under, then over paths and uses `isMathAccent` before `isMathAccentWithWidth`. The source's `isOver` expression at line 561 tests `under.coreMO() === this`, not `over.coreMO()`. Writes true or false only after the corresponding match. |
| `isMathAccent(mo=this.getText())` | 576–583 | Default expression calls `getText()` only when omitted; regex is read from dynamic constructor and tested by `String.match`. |
| `isMathAccentWithWidth(mo=this.getText())` | 587–594 | Same default-argument and dynamic-regex behavior for width-preserving accent characters. |

## Dependency behavior and identities

`OperatorDictionary.ts` (SHA-256
`9afac6e3a8f3b778247a5a678c529b9edddb8172850df7384511bf61dfbffd37`) defines
`OperatorDef` as a four-slot array and
`RangeDef` as four required slots plus an optional fifth string slot (lines
30–32). `OPDEF` (line 37 onward) creates
a fresh four-slot result and defaults only missing texClass/properties.
`getRange` (lines 159–177) reads global `OPTABLE.infix[text]`, then prefix,
then postfix by `||`; a hit produces a fresh `[0,0,def[2],'mo']`. On a range hit
it returns the exact row object from `RANGES`; a miss creates a fresh REL/mo
tuple. `MMLSPACING` and `OPTABLE` are mutable ordinary objects/arrays, not
immutable values.

MmlMo and `getRange` intentionally use different tables: `getOperatorDef`
looks up `this.constructor.OPTABLE` and fallback spacing through
`this.constructor.MMLSPACING`, whereas `getRange` uses module-level
`OPTABLE` and `RANGES`. Therefore subclass static replacement can change the
first/fallback lookup without changing the Unicode-range path. A dictionary
hit in `getOperatorDef` returns the original definition array; `getRange`
does not return that definition on a table hit. An applicable Unicode range
returns its original row identity. The subsequent OPDEF creates a fresh array.

`MmlNode.ts` (SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`) supplies
the inherited token/node API: factory, `getText`, property and attribute
access, `Parent`, `parent`, childNodes, `coreMO`, `isKind`, `isToken`, and
`setTeXclass`. `mrow.ts` (SHA-256
`af58f0c1aa39a71d6c3dc5addf358a8cefeecb3f76cb483ec940f3a7ea72b4b9`) provides
the three positional helpers used by `getForms`. `munderover.ts` (SHA-256
`b9bf04f77922c50044c4d99b3093882cab9b35ec36203f20e1e999997f9adf82`) provides
virtual `under` and `over` indices and concrete Mover/Munder/Munderover
identity. `Tree/Node.ts` provides `PropertyList` dynamic property semantics.
`BaseMappings.ts` (SHA-256
`08dada0ed7fab0134da2c3a341e30637959ebb730512835b9482d7c78d72adae`)
registers `mmlToken` at line 739; `BaseMethods.ts` (SHA-256
`7336240c8f9c5037f9fdcd5a84ea079312854f1ef3f406aeb5a8b6a8ff765cda`)
implements `MmlToken` at line 1073, creates the requested token kind with
`parser.create('node', kind)`, and validates `node.isToken`. This is part of the
configured base TeX package and supplies an actual profile path to MmlMo.
`util/string.ts` (SHA-256
`024a2ac6f96a314884f2bd71a913060c06fb75d46cbf6c94e15f4e71643a7084`) provides
the UTF-16 `unicodeChars`/`unicodeString` transformations used by prime
remapping.

All operator-regex construction uses array `.join('')` during static class
initialization. The pattern flags are absent, so JavaScript's non-Unicode
UTF-16 regex mode applies. `String.match` is used without global/sticky flags;
there is no `lastIndex` state. In the pinned Node 24 runtime, the anchored `$`
does not match before a final LF or CRLF (`/foo$/.test('foo\n')` and the CRLF
case are both false); it does match `foo`. Character classes and ranges must
be preserved exactly, including the source regex spelling and the exact
U+20D0–U+20E1 accent members. The imported utility `quotePattern` is not used
to produce these expressions.

## Current native declarations and gaps

`src/math/core/mml_tree/operator_dictionary.ixx` now declares `OperatorDef` as
`PropertyArray`, `RangeDef` as `Property`, and module-level OPDEF/getRange,
MMLSPACING, and OPTABLE. Its implementation preserves the fresh-vs-shared
array distinctions above. `PropertyArray` and `PropertyList` share storage.
The P11 follow-up has since closed the host `has` and prototype cases recorded
by its earlier review; this audit does not count those historical observations
as current native gaps.

There is currently no `MmlMo` class declaration or implementation under
`src/math/core/mml_tree/mml_nodes`. `mml_node.ixx` provides the abstract
inherited methods and MmlNode virtual interface but has no `MmlMo` fields,
static bindings, constructor-class metadata for its statics, or overrides for
the members listed above. Existing `MmlMrow` and `MmlMover`/`MmlMunder`/
`MmlMunderover` native classes provide portions of the traversal dependencies;
they do not provide `MmlMo`'s dispatch or behavior. Native `operator_dict`
module-level aliases do not establish the separate mutable `MmlMo.OPTABLE` and
`MmlMo.MMLSPACING` class fields, subclass lookup, or copy-versus-rebind rules.

No native regex binding for these five MmlMo patterns is declared. The string
utility module supplies UTF-16 and code-point conversion, but does not provide
the required non-Unicode JavaScript `RegExp`/`match` behavior. This report
records the dependency gap only; it does not propose an implementation or
acceptance boundary.

## Runtime probes of source quirks

The following probes ran in fresh Node processes against the installed pinned
MathJax 4.1.3 CJS implementation under
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs`. Small mock subclasses
were used only where the requested internal class is not exported through the
public bundle; the relevant MmlMo methods themselves were the installed CJS
methods, not native reimplementations.

* **Undefined is a cached `_texClass` value.** I supplied an `infix` table
  object with a proxy `get` trap for the inherited key `toString`. On its first
  read the inherited `Object.prototype.toString` function was a truthy hit,
  and `checkOperatorTable('toString')` assigned its missing slot 2 to
  `_texClass`, making that field `undefined`. A subsequent `texClass` read did
  not re-query the table (one proxy read total); `texSpacing()` returned the
  empty spacing result. Forcing `_texClass` to `null` instead made `texSpacing`
  re-run lookup (second proxy read), return REL class `3`, and produce
  `thickmathspace`. Thus optional-number storage that merges undefined with
  the initial null state changes observable lookup count and spacing.
* **The getter is stateful.** In a miss case with the real `getOperatorDef`,
  two reads of `texClass` while `_texClass` remained `null` produced two
  inherited-`form` writes and two `noDictDef` property writes. The getter's
  source invokes `getOperatorDef` on every such read, which is incompatible
  with a const-only getter contract unless mutation is represented elsewhere.
* **`isAccent` can return a raw value.** With an `munder` parent and no explicit
  parent `accent` value, the getter returned the child attribute's original
  string `"yes"` and number `17` unchanged. When the parent's explicit lookup
  returned `false`, the getter returned its initial `false`, even though the
  child's own accent property was true. This matches the source branch's
  control flow and its runtime value despite the TypeScript `boolean`
  annotation.
* **The `isOver` test reads `under.coreMO()`.** With a `munderover` mock where
  only `over.coreMO()` was this operator, `checkMathAccent` did not mark the
  node as an accent. The source expression compares `under.coreMO()` to this
  node; substituting `over` would change behavior.

The inherited-name scenario is reachable through the actual configured TeX
profile, not only direct core construction. Base `mmlToken` is registered by
`BaseMappings.ts:739` and implemented by `BaseMethods.MmlToken` at lines
1073–1088. The root's reproducible dependency probe is
`node tools/math-port/mo-dependency-probe.cjs`; it invokes the locked
`reference-profile.cjs`, inspects actual `mo` nodes in the TeX input post-filter,
and renders through the profile's SVG output. It reports both
`\mmlToken{mo}{toString}` and `\mmlToken{mo}{constructor}` with
`stored_type: "undefined"`, `stored_null: false`, `getter_undefined: true`,
and `svg_has_mo: true`. The script also asserts strict end behavior for final
LF and CRLF against the pinned Node runtime. This is dependency evidence, not
native MmlMo acceptance. Separately, `\operatorname{toString}` creates an `mi`
(the AMS handler at lines 359–380 removes `autoOP` for a single `mi`); that
narrower route does not negate the `mmlToken` witness.
