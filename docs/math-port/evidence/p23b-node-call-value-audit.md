# P23b node-call value boundary audit

Read-only audit of node-valued arguments/results across the pinned MathJax
4.1.3 Source closure (`fb987178c2d279a99b0db5ea02e751691703955e`, 246 Source
files in `scope.json`). This supplements `p23b-node-value-return-audit.md` by
following `setTeXclass`, `getPrevClass`, `updateTeXclass`,
`inheritAttributesFrom`, and direct/logical parent values through their callers
and current native overrides. It does not edit native code or declare a
migration accepted.

## Source value facts

The declared TypeScript interfaces (`MmlNode.ts:224,258`) say `MmlNode`, but
runtime values are governed by the actual expressions. `Tree/Node.ts:41–48`
is the interface declaration, not runtime initialization. The actual
AbstractNode class initializes parent to explicit Null at line 166.
appendChild/replaceChild assign a Node, while removeChild assigns explicit
Null (lines 263–295). Visitors/filters also assign Null explicitly. Ordinary
pinned construction/detachment yields Null or Node; an external public-field
assignment can supply Undefined. These facts are distinct.

### `setTeXclass(prev)` has a tagged input and a tagged result

| Source method | Source expression/result | Value boundary and consumer effect |
|---|---|---|
| `AbstractMmlNode.setTeXclass`, `MmlNode.ts:694–697` | Calls `getPrevClass(prev)`, then returns `this.texClass != null ? this : prev`. The loose null check covers null and undefined texClass; the false branch returns the exact original `prev`. | Input `prev` is passed into a truthiness test in `getPrevClass`; output can be the receiver Node or the exact Undefined/Null/Node input. |
| `AbstractMmlEmptyNode.setTeXclass`, `MmlNode.ts:1383–1385` | `return prev;` without a test. | Exact Undefined/Null/Node passthrough. This is the clearest counterexample to a pointer-only result. |
| `AbstractMmlLayoutNode.setTeXclass`, `MmlNode.ts:1153–1157` | `prev = this.childNodes[0].setTeXclass(prev); updateTeXclass(this.childNodes[0]); return prev;` | The child result, including Undefined/Null, is propagated. Child receiver is a direct indexed Get and throws for Undefined/Null before invoking the method. The update is a fresh childNodes[0] read after callback. |
| `AbstractMmlBaseNode.setTeXclass`, `MmlNode.ts:1199–1221` | Saves child result only on the embellished/`mi` path; it ultimately returns `result || this`. Other child calls use explicit Null and ignore results. | Result is Node after the `||` projection; selected child and fallback are truthiness projections. The input `prev` is passed unchanged to `getPrevClass` and possibly to the base call. |
| `MmlMrow.setTeXclass`, `mrow.ts:154–183` | Fence branch sets `prev = null`, then each for-of child result replaces `prev`; ordinary branch likewise reassigns `prev` after every child. Returns final `prev`. | After any child returns Undefined or Null, the next child's call receives that exact tag. A hole yielded by for-of is Undefined and fails on `.setTeXclass`; a child may mutate/rebind `this.childNodes`, but the captured iterator array remains active with live length. |
| `MmlMaction.setTeXclass`, `maction.ts:114–122` | Optional tooltip call uses explicit Null and ignores its result; selected Node returns a value assigned to `prev`, then `updateTeXclass(selected)`, then exact `prev` returned. | Selected uses its own falsey fallback, but its `setTeXclass` result can still be Undefined/Null and propagates to the caller. |
| `MmlMfenced.setTeXclass`, `mfenced.ts:80–103` | `prev` is successively assigned each truthy open/child/separator/close call result; returns exact final value. | Starting input can be Null/Undefined/Node; a delegated result may change its tag. Optional fields/slots are tested before call. |
| `MmlMenclose.setTeXclass`, `menclose.ts:76–80` | Assigns child-0 result to `prev`, updates from a fresh child-0 read, returns `prev`. | Direct child receiver must be a Node; return preserves child result tag. |
| `MmlMo.setTeXclass` / `adjustTeXclass`, `mo.ts:320–340,347–404` | With undefined `texClass` and spacing attributes, returns explicit Null. Otherwise returns `adjustTeXclass(prev)`. `adjustTeXclass` returns `prev` unchanged for strict `texClass === TEXCLASS.NONE`; other branches use `if (prev)` then access/write its fields and finally return `this`. | Input is a raw value: falsey Undefined/Null bypasses dereference and sets `prevClass=NONE`; a truthy non-node would fail on property access. Output is Null, exact `prev`, or receiver Node depending on strict NONE and spacing path. `TEXCLASS.NONE` is a strict sentinel, not a truthy/falsy test. |
| `MmlMpadded.setTeXclass`, `mpadded.ts:62–70` | Non-vbox delegates and returns superclass result unchanged. Vbox path calls `getPrevClass(prev)`, sets own class, calls child with explicit Null, and returns `this`. | Delegated return can be Undefined/Null/Node; vbox path is Node-only after child call succeeds. |
| `MmlMSpace.setTeXclass`, `mspace.ts:71–73` | `return prev;` | Exact Undefined/Null/Node passthrough. |
| `MmlMfrac`, `mroot`, `msqrt`, `mtd`, `mtr`, `mtable`, `MmlMi`: `mfrac.ts:83–89`, `mroot.ts:82–87`, `msqrt.ts:79–83`, `mtd.ts:99–103`, `mtr.ts:137–143`, `mtable.ts:209–215`, `mi.ts:91–105` | These implementations call `getPrevClass(prev)`, perform child calls with explicit Null where present, then return `this`; child results are ignored. | Successful result is Node; `prev` still must be a tagged value because `getPrevClass` reads it iff truthy. Child values are independently dereferenced at the exact call. |
| `TeXAtom`, `TeXAtom.ts:87–90,97–104` | Calls child zero's setTeXclass with explicit Null and ignores its result, then returns adjustTeXclass(prev). Its adjustTeXclass binding is replaced by the original MmlMo implementation. | Retains the exact tagged result of operator adjustment; strict NONE can forward prev unchanged. This is not a return-this method. |

The full Source override inventory found by scanning all `ts/**/*.ts` is:
`AbstractMmlNode`, `AbstractMmlLayoutNode`, `AbstractMmlBaseNode`,
`AbstractMmlEmptyNode`, `maction`, `menclose`, `mfenced`, `mfrac`, `mi`, `mo`,
`mpadded`, `mroot`, `mrow`, `mspace`, `msqrt`, `mtable`, `mtd`, `mtr`, and
`TeXAtom`. Classes with no override use the applicable inherited body. No
additional concrete `setTeXclass` declaration appeared in the pinned tree.

The root entry points found in Source are `common.ts:465,491`, both passing
explicit Null to `math.root.setTeXclass`; and the Common `mo` wrapper at
`output/common/Wrappers/mo.ts:556`, passing `this.multChar.node` (a Node) to a
next operator. Internal node results feed sibling calls in mrow, mfenced,
layout/base, maction, and menclose as described above. Other call results are
deliberately ignored, including the extra-child/tooltip cleanup calls with
explicit Null. Therefore both the virtual parameter and return must retain
Undefined/Null/Node, and every callsite must preserve whether it forwards,
ignores, or projects the value.

### `getPrevClass(prev)` and `updateTeXclass(core)`

`MmlNode.ts:716–721` implements `if (prev) { this.prevClass = prev.texClass;
this.prevLevel = prev.attributes.get('scriptlevel'); }`. It returns no value.
Undefined and Null both skip all reads; a Node triggers ordered `texClass`,
`attributes`, then `get('scriptlevel')` accesses. A truthy non-node would
throw, so “not a node” cannot be pre-normalized to null. Its callsites are
base `setTeXclass`, layout/base/empty-derived overrides, and concrete classes
listed in the override inventory (`rg setTeXclass` at the pinned tree).

`MmlNode.ts:703–710` implements `if (core)`, assigns this.prevClass from
core.prevClass, then assigns this.prevLevel from core.prevLevel (each read is
followed immediately by its write), clears core through the
right-associative `core.prevClass = core.prevLevel = null`, then reads and
copies `core.texClass`. Its parameter is truthiness-tested; Undefined/Null
skip the whole body, Node performs the ordered transfers. The method is void.
Callers are layout/base, `maction`, `menclose`, `mfenced`, and `mrow`
(`MmlNode.ts:1153–1157,1204–1208`; `maction.ts:118–121`;
`menclose.ts:77–80`; `mfenced.ts:99–101`; `mrow.ts:179–181`). The core
argument may be the result of direct `core()` and so can be Undefined/Null;
the original guard is the source-level projection.

### `inheritAttributesFrom(node)`

`MmlNode.ts:889–898` first reads `node.attributes` without a guard, then
`displaystyle`, `scriptlevel`, conditionally `mathsize` after `isSet`,
`getProperty('texprimestyle')`, and invokes `setInheritedAttributes` with a
fresh defaults record and the resulting prime boolean. A missing/Null input
throws on the first member access; Node proceeds in this order. The empty-node
override at `MmlNode.ts:1410` ignores the input completely, including Null.
Source callers include `a11y/complexity/collapse.ts:609`,
`output/common/Wrapper.ts:1456`, and common `menclose`/`mfenced` wrappers
(`output/common/Wrappers/menclose.ts:507`, `mfenced.ts:185`). These actual
callers pass MML nodes, but the virtual boundary must preserve the original
argument until the concrete body decides to dereference or ignore it.

## Parent values and consumers

The ordinary `parent` field is a Source value, not a nullable-pointer model:
`AbstractNode` initializes it to explicit Null (`Tree/Node.ts:166`), it is set
to Node on append/replace, and it is set to explicit Null by remove and
several visitors/filters. The MML declarations at `MmlNode.ts:423,1240` only
refine the TypeScript type; they do not replace that runtime initialization.
Pinned construction/editing paths therefore expose Null or Node. Because the
field is publicly mutable in JavaScript, an extension/caller can still assign
Undefined; that value must not be silently normalized if it reaches a native
boundary.
MML
`Parent` (`MmlNode.ts:575–581`) captures that value, repeats the virtual
`notParent`/`Parent` path only while truthy, and returns the current exact value.
`AbstractMmlEmptyNode.Parent` (`1313–1315`) directly returns `parent`. Thus
Undefined and Null both terminate the loop but remain distinct results.
`notParent` itself may also be falsey raw child value on `mstyle.ts:59–61`, so
the test must not replace source results before the loop.

| Source access | Value handling in Source | Current native consumer |
|---|---|---|
| `Tree/Node.ts:166,263–295`, `LegacyMmlVisitor.ts:50`, `FilterUtil.ts:249`, `NodeUtil.ts:202–208` | Constructor initializes Null; append/replace assign Node; remove and the cited visitors/filters assign explicit Null. Append/replace/remove preserve their source partial-write ordering. | `src/math/core/tree/node.ixx:33–106`, `node_types.ixx:12,17–25`; parent is currently pointer-valued and cannot distinguish an external Undefined overwrite from Null. |
| `MmlNode.ts:575–581,671–689,987–1010` | `Parent` returns Node/Undefined/Null; `childPosition` uses truthy parent walk and can return index or explicit Null; `mError` truthy-checks direct parent, then dereferences it for replacement. | `mml_node.cpp:235–307,459–471`; `mml_node.ixx:61–62`. |
| `MmlNode.ts:1313–1315`; `mo.ts:288–304,477–494,521–525,550–562` | Empty Parent directly returns the stored value. `isAccent` and checks often truthy-test; `getForms` compares `firstNonSpace/lastNonSpace` by identity; `checkPseudoScripts`/`checkMathAccent` use logical Parent; coreParent walks direct parent. | `mo_classes.cpp:15–58`, `mo_lookup.cpp:119–128`, `mo_scripts.cpp:34–81`, `operator_class.cpp:50–63`. |
| `MmlNodes/math.ts:128`; `HtmlNode.ts:97`; `mtr.ts:90,114–121`; `mtd.ts:85–92`; `mmultiscripts.ts:179–180,223–224` | Conditional checks collapse falsey parent before member access; table alignment and error methods dereference after their own checks. | `mml_branches.cpp`, `mtr.cpp`, `mtd.cpp`, `mmultiscripts.cpp`; see exact native inventory below. |
| `MmlNodes/mathchoice.ts:82`; `mtable.ts:188–189`; `mfenced.ts:188` | Direct parent method dereference/replacement or assignment; no common null projection. | `mathchoice.cpp:34–50`, `mtable.cpp:118–120`, `mfenced.cpp:166`. |
| Enabled TeX callers: `base/BaseConfiguration.ts:126,144`; `centernot/CenternotConfiguration.ts:76`; `FilterUtil.ts:103,143,209–216,249,338`; `bussproofs/BussproofsUtil.ts:79,153,165,177,188,216,219,273,381,386,413,546,557`; `empheq/EmpheqUtil.ts:180`; `ParseOptions.ts:292`; `NodeUtil.ts:206` | Mixture of truthy checks, strict identity, direct chained dereference, parent traversal, assignment, and property helper argument passing. For example centernot's six-link chain throws at the first missing receiver; `FilterUtil` tests `!parent` before kind checks; bussproofs next/previous helpers dereference parent repeatedly. | Most of these Source helpers do not yet have a corresponding native profile implementation. Native graph migration must not assume every upstream parent access has already been ported. |

The exact native C++ parent/Parent consumer search additionally found
`mml_empty.cpp:18`, `mml_node.cpp:235–238,297–307`,
`operator_class.cpp:50`, `mo_classes.cpp:15–58`, `mo_lookup.cpp:119–123`,
`mo_scripts.cpp:34–81`, `mmultiscripts.cpp:39,117`, and
`mtable.cpp:118–120`; the corresponding declarations are in
`mml_node.ixx:61–62,126–127,203–204` and `mo.ixx:42`. `notParent` overrides
also exist on inferred mrow, MathChoice, semantics, TeXAtom, and mstyle; their
truthiness/boolean contract remains separate from parent value storage.

The current native implementations of those ancestry flags are
`mml_node.cpp:233`, `mml_empty.cpp:17`, `mrow.cpp:138`,
`mathchoice_bindings.cpp:11`, `semantics_bindings.cpp:14`,
`tex_atom_bindings.cpp:13`, and `mstyle.cpp:14`; declarations are in
`mml_node.ixx:61,126,203`, `mrow.ixx:13`, `mathchoice.ixx:12`,
`semantics.ixx:12`, `tex_atom.ixx:13`, and `mstyle.ixx`. Source's
`mstyle.notParent` is not Boolean-only: `mstyle.ts:59–61` can return the raw
falsey child value (Undefined/Null), while the native binding currently
returns `bool`. MathChoice/Semantics/TeXAtom/inferred-row return booleans as
their pinned source bodies specify.

## Native override and caller inventory

Current native virtual signatures are `MmlNode* setTeXclass(MmlNode*)`,
`MmlNode* Parent()`, `MmlNode* core()/coreMO()`, and
`void inheritAttributesFrom(MmlNode*)` (`mml_node.ixx:61–79`). Current
`getPrevClass` and `updateTeXclass` also take raw pointers (`:154–155`). The
implemented `setTeXclass` overrides are:

| Native implementation | Source override / exact source lines | Return/call behavior that the pointer signature loses |
|---|---|---|
| `mml_node.cpp:311–315` | `MmlNode.ts:694–697` | Returns exact `prev` when no texClass; can be Undefined/Null. |
| `mml_empty.cpp:41` | `MmlNode.ts:1383–1385` | Returns `prev` unchanged, including Undefined. |
| `mml_branches.cpp:124–132` | `MmlNode.ts:1153–1157` | Forwards child's exact return, then performs a fresh core read. |
| `mml_branches.cpp:146–173` | `MmlNode.ts:1199–1221` | Final `result || this` is Node, but input and internal base result remain tagged until projection. |
| `maction_behavior.cpp:24–46` | `maction.ts:114–122` | Forwards selected child's return unchanged. |
| `menclose.cpp:13–27` | `menclose.ts:76–80` | Forwards child result unchanged. |
| `mfenced.cpp:33–66` | `mfenced.ts:80–103` | Accumulates and returns latest delegated result. |
| `mfrac.cpp:13–26` | `mfrac.ts:83–89` | Returns this after child calls that ignore child results. |
| `mi.cpp:35–49` | `mi.ts:91–105` | Returns this; getPrevClass still consumes input. |
| `mo_classes.cpp:102–104` | `mo.ts:320–340` plus `adjustTeXclass:347–404` | Can return explicit Null or exact previous value; pointer signature erases distinction. |
| `mpadded.cpp:13–25` | `mpadded.ts:62–70` | Super path forwards exact result; vbox path returns this. |
| `mroot.cpp:12–23`; `msqrt.cpp:12–21` | `mroot.ts:82–87`; `msqrt.ts:79–83` | Return this after required child calls. |
| `mrow.cpp:106–135` | `mrow.ts:154–183` | Returns final child's exact result; rebinds `prev` at each child call. |
| `mspace.cpp:52–54` | `mspace.ts:71–73` | Returns previous value unchanged. |
| `mtd.cpp:23–32`; `mtr.cpp:96–107`; `mtable.cpp:143–153` | `mtd.ts:99–103`; `mtr.ts:137–143`; `mtable.ts:209–215` | Successful return is this, while input is observed by getPrevClass; direct child results are ignored. |
| `tex_atom.cpp:14–20` | `TeXAtom.ts:87–90` and shared `adjustTeXclass` source in `mo.ts:347–404` | Returns the shared operator adjustment result; input may be returned exactly for strict NONE. |

Further current consumers include all internal calls in `mml_node.cpp`,
`mml_branches.cpp`, `mrow.cpp`,
`mfenced.cpp`, `maction_behavior.cpp`, `menclose.cpp`, `mpadded.cpp`,
`mfrac.cpp`, `mroot.cpp`, `msqrt.cpp`, `mtd.cpp`, `mtr.cpp`, `mtable.cpp`,
`mo_classes.cpp`, `mi.cpp`, `mspace.cpp`, and `tex_atom.cpp`. The prior
P23b child-return report lists the rest of the 31 ChildArray consumers; the
call-value migration additionally affects the methods and wrappers above even
when they do not directly index a child array.

## Boundary that must be frozen by Root

The Source evidence requires one node-value type at every virtual
`setTeXclass` input/output and for node-valued helper results, with explicit
Null and Undefined tags; exact method sites decide whether to forward, ignore,
truthiness-project, compare identity, or dereference. `getPrevClass` and
`updateTeXclass` must accept the same value type so callers cannot erase the
tag before their source truthiness checks. `Parent` and direct `parent` need
the same tagged storage and reads. `inheritAttributesFrom` must preserve the
tag until each override chooses its source behavior: the ordinary body
dereferences, while AbstractMmlEmptyNode ignores its argument.

Only narrowly Node-only values may remain `MmlNode*`: `coreParent()`'s
`embellished || this` fallback; `MmlMaction.selected` after its own
`child || factory.create(...)` projection; and ordinary constructor/factory
results before they enter a tagged virtual boundary. The Source-backed
`setTeXclass` virtual itself cannot be pointer-only because Empty and mspace
return the input unchanged and mo can return explicit Null. Parent's ordinary
initialized value is explicit Null; the typed binding must additionally retain
an external Undefined assignment or a controlled observer accessor result,
rather than reconstructing a lost tag from a null pointer. No code/API change
is authorized by this audit.

No build, test, generated fixture, or runtime experiment was run. This is
source and current-native boundary evidence only; Root owns declaration
freezes, red fixtures, native migration, and acceptance.

## Pinned source hashes

| Source file | SHA-256 |
|---|---|
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `ts/core/MmlTree/MmlNodes/mrow.ts` | `af58f0c1aa39a71d6c3dc5addf358a8cefeecb3f76cb483ec940f3a7ea72b4b9` |
| `ts/core/MmlTree/MmlNodes/mo.ts` | `2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605` |
| `ts/core/MmlTree/MmlNodes/maction.ts` | `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895` |
| `ts/core/MmlTree/MmlNodes/mfenced.ts` | `64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397` |
| `ts/core/MmlTree/MmlNodes/mfrac.ts` | `4ded5fc9922d462d02818523d676c30299d08cfe7794067bd4e20950c8fccfc7` |
| `ts/core/MmlTree/MmlNodes/mroot.ts` | `b4b8f9deddb22efe7580067d8845d7cb225046e244eb93da8b0e931eff76e99a` |
| `ts/core/MmlTree/MmlNodes/mi.ts` | `3bbddaeac72d2c0f2e5d8a0440c28188f98d1bf1481a0ec6872e103a50f146c2` |
| `ts/core/MmlTree/MmlNodes/mpadded.ts` | `b5aac516c32fb019ac5ace504f86312f3e0849e1faa44006567bac05815c7271` |
| `ts/core/MmlTree/MmlNodes/mspace.ts` | `69961341e3323cc3af8721f9d800190e2051057b11cba5558ef1940e0deefdd4` |
| `ts/core/MmlTree/MmlNodes/msqrt.ts` | `8d522b96f39f35c170fe12ab7e59e0999c65551335466010522280bfa3570b7e` |
| `ts/core/MmlTree/MmlNodes/mtable.ts` | `8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda` |
| `ts/core/MmlTree/MmlNodes/mtr.ts` | `ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f` |
| `ts/core/MmlTree/MmlNodes/mtd.ts` | `8b888aec4069818407079e84c42247495e04bd93ff52a3fd28bff65ca7635cfb` |
| `ts/core/MmlTree/MmlNodes/TeXAtom.ts` | `bad114224392b94852456a4636b85ad90a5a2c96b63e6c7911d155db299f7028` |
| `ts/core/MmlTree/MmlNodes/menclose.ts` | `ffa4ab507744cb4f190312ca8dcd692474c333bee6e19c86709554aa9dad4333` |
| `ts/output/common.ts` | `3686a8e803e26ae59d03c096ac2e6e5b1ab1332a5c4bcaa083cb9a02c2c61096` |
| `ts/output/common/Wrappers/mo.ts` | `67c7ed182bc8a38139da98e4d42d0350d2386de29a7e5f95095b4161dcab1bcf` |
