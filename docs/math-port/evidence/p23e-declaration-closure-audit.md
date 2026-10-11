# P23e — MML tagged-node declaration closure audit

Read-only declaration audit of the current native `src/math/core/mml_tree` headers against `docs/math-port/p23b-node-binding-decision.md` and `docs/math-port/evidence/p23b-node-call-value-audit.md`. This lists the exact declarations that must agree with Root's freeze; it does not authorize changes or choose any signature beyond that decision. No code, fixture, build, test, or commit was changed or run.

## Declared tagged boundaries and their override closure

Root's frozen name is `NodeValue = tree::ChildValue<MmlNode>`. The alias belongs in `src/math/core/mml_tree/mml_node.ixx`, beside `Children` and `Factory`; it is not currently declared there. Every override in a virtual family must use the identical tagged type. The declaration closure currently using raw `MmlNode*` is:

| Declaration file and lines | Current declarations | Boundary from Root decision |
|---|---|---|
| `mml_node.ixx:61–62,71–72,75,79` | `bool notParent()`, `MmlNode* Parent()`, `core()`, `coreMO()`, `setTeXclass(MmlNode*)`, `inheritAttributesFrom(MmlNode*)` on `MmlNode` | `Property notParent() const`; the other returns/arguments use `NodeValue` exactly as frozen. `notParent` is Property rather than NodeValue because the MML style getter can return a falsey child value, while other implementations return booleans. |
| `mml_node.ixx:126–139` | `AbstractMmlNode` implementations of `notParent`, `Parent`, `core`, `coreMO`, `setTeXclass`, `inheritAttributesFrom` | Same virtual types; these are overrides, not a second compatibility API. |
| `mml_node.ixx:154–155` | `updateTeXclass(MmlNode*)`, `getPrevClass(MmlNode*)` | Both arguments become `NodeValue`. Their source truthiness test must see Missing/Null/Node before any required-node dereference. |
| `mml_node.ixx:179–181,188–190` | `AbstractMmlLayoutNode` and `AbstractMmlBaseNode` `core`, `coreMO`, `setTeXclass` | Return and setTeXclass argument become `NodeValue`. `core()` returns the direct slot value; `coreMO()` dereferences/delegates and may throw at that operation. |
| `mml_node.ixx:203–221` | `AbstractMmlEmptyNode` `notParent`, `Parent`, `core`, `coreMO`, `setTeXclass`, `inheritAttributesFrom` | Same tagged family. Empty-node `setTeXclass` returns the exact input; empty `inheritAttributesFrom` ignores its input, so these cannot normalize tags at entry. |
| `mml_nodes/maction.ixx:15–17` | `core`, `coreMO`, `setTeXclass` | All return values and the `setTeXclass` input become `NodeValue`; `selected()` is separately classified below. |
| `mml_nodes/menclose.ixx:15` | `setTeXclass` | Input and output become `NodeValue`. |
| `mml_nodes/mfenced.ixx:16` | `setTeXclass` | Input and output become `NodeValue`. |
| `mml_nodes/mfrac.ixx:14` | `setTeXclass` | Input and output become `NodeValue`. |
| `mml_nodes/mi.ixx:7` | `setTeXclass` | Input and output become `NodeValue`. |
| `mml_nodes/mo.ixx:46–47` | `setTeXclass`, virtual `adjustTeXclass` | Both methods take/return `NodeValue`; `MmlMo` can return explicit Null or forward the original previous value. |
| `mml_nodes/mpadded.ixx:13` | `setTeXclass` | Input and output become `NodeValue`. |
| `mml_nodes/mroot.ixx:14`, `msqrt.ixx:14` | `setTeXclass` | Input and output become `NodeValue`. |
| `mml_nodes/mrow.ixx:10–11,13` | `core`, `coreMO`, `firstNonSpace`, `lastNonSpace`, `setTeXclass`; `MmlInferredMrow::notParent` | `core/coreMO` and `setTeXclass` use `NodeValue`; first/last nonspace are the narrow Node/Null projections described below; `notParent` becomes `Property`. |
| `mml_nodes/mspace.ixx:6` | `setTeXclass` | Input and output become `NodeValue`; source returns the previous value exactly. |
| `mml_nodes/mtable.ixx:15`, `mtd.ixx:14`, `mtr.ixx:13` | `setTeXclass` | Each input and output becomes `NodeValue`. |
| `mml_nodes/tex_atom.ixx:13–15` | `notParent`, `setTeXclass`, virtual `adjustTeXclass` | `notParent` becomes `Property`; both TeX-class methods use `NodeValue`. `adjustTeXclass` delegates to the same Source operator-adjustment method as `MmlMo`. |
| `mml_nodes/mathchoice.ixx:12`, `mstyle.ixx:6`, `semantics.ixx:12` | `notParent` | Each returns `Property`. The first, inferred-row, TeXAtom, and semantics source implementations produce booleans; mstyle can return Missing/Null from its raw first-child falsey branch. |

`MmlMstyle`'s compressed declaration is all on `mstyle.ixx:6`; the other line numbers above are from the checked-in declaration files. `AbstractMmlNode` and `AbstractMmlEmptyNode` are both declared in `mml_node.ixx`; there is no separate empty-node declaration module to update. The native current signatures are confirmed by the declaration inventory in the preceding table and `rg` over every `src/math/core/mml_tree/**/*.ixx` file.

## Extra declarations beyond the 31 child-consumer files

`src/math/core/mml_tree/operator_class.ixx:6` declares the shared free function `adjustOperatorTeXclass(MmlNode&, MmlNode*)`. It is not in the prior 31-file child-consumer inventory, but it is part of the tagged `adjustTeXclass` closure and must accept `NodeValue previous` while retaining its `MmlNode& receiver`. `tex_atom.ixx` and `mo.ixx` virtual overrides must agree with that input/result contract.

The three in-closure child-get adapters also currently erase direct indexed Get tags and are declaration dependencies of the frozen “Direct child helper Get returns NodeValue” rule:

| Declaration | Current signature | Source operation |
|---|---|---|
| `mo_host.ixx:11` | `MmlNode* moChild(MmlNode*, size_t)` | Direct childNodes indexed Get used by `mo.ts`; must retain Undefined/Null/Node until each caller tests or dereferences it. |
| `table_host.ixx:16` | `MmlNode* tableSlot(const Children&, size_t)` | Direct sparse slot read used by table/row code; the required-child adapter `tableChild` may dereference only at the original call site. |
| `multiscript_host.ixx:15` | `MmlNode* multiscriptSlot(const Children&, size_t)` | Direct slot read used by multiscript behavior; `multiscriptChild` is the required dereference layer. |

These helpers are already represented in the old consumer inventory, but their declarations are important because changing only virtual MML signatures leaves the same lossy pointer conversion at the data boundary. `arithmetic_host.ixx:13–18` instead exposes `MmlNode& arithmeticChild(...)`, which is an explicit required dereference helper and can remain a reference-returning helper if its callers preserve the Source access point. `tableNode`, `multiscriptNode`, `moRequired`, `tableChild`, `multiscriptChild`, and factory-create helpers likewise represent required dereference or fresh-product boundaries, not raw direct-Get results.

## Other pointer declarations and actual Source domains

The remaining pointer declarations do not all need the same conversion. This table classifies their Source result/argument domains so a broad replacement does not erase real projections or invent undefined cases.

| Native declaration | Pinned Source fact and observed domain | Closure note |
|---|---|---|
| `mml_node.ixx:81,141,223` `mError(...) -> MmlNode*` | `MmlNode.ts:987–1011` returns explicit `null` when already parented by `merror`, otherwise a newly created error node; `AbstractMmlEmptyNode.mError` at 1422–1428 returns explicit null. | Root has closed this narrow Node/Null domain: keep the pointer result and define `nullptr` as Source Null here. This is not a direct ChildArray Get and does not permit collapsing Undefined at a tagged boundary. |
| `mrow.ixx:11` `firstNonSpace()/lastNonSpace() -> MmlNode*` | `mrow.ts:128–149` filters with `if (child && !child.isSpacelike)` then returns a child or explicit `null`; never returns a hole's Undefined directly. | Root has closed this narrow Node/Null projection: `nullptr` represents Source Null at this result boundary. Child Gets before the truthiness test still need ChildValue semantics. |
| `mo.ixx:42` `coreParent() -> MmlNode*` | `mo.ts:235–249` ends with `embellished || this`; `this` guarantees a Node even when no parent qualifies. | Node-only fallback; pointer return is faithful. Its parent/coreMO reads still need the tagged boundaries above. |
| `mo.ixx:43` `coreText(MmlNode*)` | `mo.ts:255–274` first does `if (!parent) return ''`; otherwise it reads/delegates. Its declared source parameter is MmlNode, but runtime falsey Null/Undefined share the empty-string path. | The method returns text, not a node. A pointer parameter can express the method's immediate falsey projection, but a caller must not prematurely dereference a sparse child before this source guard. |
| `maction.ixx:12` `selected() -> MmlNode*` | `maction.ts:61–77` selects a child and returns `child || factory.create('mrow')`. | After that explicit truthy fallback the result is a Node under the pinned factory contract; raw pointer is appropriate. The input/indexed slot selection still has a tagged child boundary before this projection. |
| `mfenced.ixx:14–15` public `open`, `close` fields | `mfenced.ts:61–65` initializes both to `null`; `addFakeNodes` assigns a generated fake node or null (`:136–168`). | The locked implementation uses Node/Null. Preserve the source truthy checks and receiver read order; no normal Undefined return was found. These fields are not virtual helper returns. |
| `mfenced.ixx:20` `fakeNode(...) -> MmlNode*` | `mfenced.ts:180–189` creates TextNode, creates `mo`, assigns its class and parent, and returns that created node. | Fresh factory product; Node-only when factory succeeds. A failed factory must keep its existing error/required-node behavior rather than inventing a nullable result. |
| `mml_node.ixx:109,232,253` `copy() -> MmlNode*` (AbstractMmlNode, TextNode, XMLNode) | Source `AbstractMmlNode.copy`, `TextNode.copy`, and `XMLNode.copy` construct and return a fresh matching node. | Node-only fresh product under a successful factory call; raw pointer may remain. |
| `mml_node.ixx:129–130` MML `appendChild/replaceChild` overrides | Source `AbstractMmlNode.appendChild` returns the child's append result; `replaceChild` on inferred arity calls the inferred row and then returns `newChild`, while the ordinary branch returns `super.replaceChild(...)`, which also returns `newChild` (`MmlNode.ts:607–645`, `Node.ts:263–283`). The separate generic Empty override returns `oldChild` from `replaceChild` (`Node.ts:373–402`). | These use tagged arguments/results under the frozen generic Node boundary. Preserve MML's always-newChild return and Empty's distinct oldChild return; do not apply one replacement return rule to every class. The `node_types.ixx`/`node.ixx` declarations live outside `mml_tree` and must change in lockstep; no pointer bridge should remain. |
| `operator_class.ixx:6` `adjustOperatorTeXclass(..., MmlNode*) -> MmlNode*` | `mo.ts:347–404` returns the exact previous value in strict `TEXCLASS.NONE` path; other branches can return `this`. Previous is also truthiness-tested/dereferenced. | This is not a Node-only return: both input and output are NodeValue under the Root decision. |
| `table_host.ixx:45–49` and `multiscript_host.ixx:28–33` factory helpers returning pointer/reference | They immediately create a new product; the helpers preserve the raw result only until the same caller's required-node dereference. | Fresh product or explicit required factory result; they are not direct child Gets. Do not use them to normalize child values. |
| `table_host.ixx:45–49` `tableCreate(..., MmlNode* child)` and `multiscriptInsert(..., MmlNode* node)` | Current native helpers pass/store the already selected child/product. Their Source call paths operate after a branch/required dereference or create a fresh node; check each callsite before changing an argument type. | Do not widen solely because the helper is pointer-typed; preserve source caller's actual projection and partial mutation. Root's full graph freeze must decide if an individual call forwards raw direct Get before a guard. |

The pointer-return methods above are deliberately limited projections: `mError` and first/last nonspace return only Node or explicit Null in the audited source bodies. In contrast, Parent/core/coreMO/setTeXclass and direct indexed child Gets retain Undefined/Null/Node possibilities and follow Root's tagged boundary. This distinction prevents either an unnecessary signature expansion or an accidental global nullptr-to-Null collapse.

## Dependency and status boundary

The changes above are declaration closure only. `src/math/core/tree/node_types.ixx` and `node.ixx` own the generic `ChildValue<Product>` append/replace/remove signatures; `factory_types.ixx` owns `FactoryChildren`; wrapper declarations own child-array handles. They are required dependencies even though this report is focused on MML declarations. Root's decision says no mirrored parent pointer and all internal parent accesses must use the chosen typed binding, so MML implementations cannot keep a private `MmlNode*` cache beside it.

Nothing here changes concrete methods, ChildArray algorithms, test observers, or source acceptance. The previous 31-file inventory and its missing native NodeUtil downstream caller remain unchanged. This document does not claim the interface freeze or any MML migration has passed.

## Current native declaration fingerprints

These SHA-256 values identify the declarations audited above; they are fingerprints of the current native headers, not source-lock hashes or acceptance evidence.

| Declaration | SHA-256 |
|---|---|
| `src/math/core/mml_tree/mml_node.ixx` | `36f47c27f389ac337c8f08dd890d066ba362f8ed9e51ea9ad53cad999ceeba3e` |
| `src/math/core/mml_tree/operator_class.ixx` | `d30508af542b8d8888d190b79b426e022e6eb94cfe91e93b1b765df06ba98337` |
| `src/math/core/mml_tree/mo_host.ixx` | `3b86da07cd2672b54c71aa5bb910cae581dfae80c389f949a259f13c94d45645` |
| `src/math/core/mml_tree/table_host.ixx` | `382d4b842df2a9d52eb6b6adf42db7ba5c9ac02d64f56fecb4749d0f8ffb8925` |
| `src/math/core/mml_tree/multiscript_host.ixx` | `481e775cce3c176d62cb17edb90954013adff6f618cbea98a6b16a2b884e828e` |
| `src/math/core/mml_tree/mml_nodes/maction.ixx` | `018e35bc5f4904960c9957e7314b404519502d6fe9dae464dc4f6a0e66f34783` |
| `src/math/core/mml_tree/mml_nodes/menclose.ixx` | `4a1472c0228f9405f543acd6a54a381c3ad24ed592f2542202be018f3e966c88` |
| `src/math/core/mml_tree/mml_nodes/mfenced.ixx` | `4bd2c0db772c3c2bb3de16eb527fa884024822e8d7c9f87198f1ddcadb146fd1` |
| `src/math/core/mml_tree/mml_nodes/mfrac.ixx` | `ba90e65da9e5b2df1886d97c17b4c2c074ad2d182d91da7e6e1044a1368732d2` |
| `src/math/core/mml_tree/mml_nodes/mi.ixx` | `45a82ae4c54d4277c1c8fe204e5c64f0993dc7cb68c567f1eadcfeeb1c1c6491` |
| `src/math/core/mml_tree/mml_nodes/mo.ixx` | `608146e0720ffdecaee92cfa218e38f506391cfc78f6b517afa1cb8ba5c5b204` |
| `src/math/core/mml_tree/mml_nodes/mpadded.ixx` | `464d80b6f1e45d8c6c8c05c6e70fc3fa53ed5bd5ba3463be5af9d780ba53d5` |
| `src/math/core/mml_tree/mml_nodes/mroot.ixx` | `80abc54742d61a28cea4faee4462d4b516f8d6b828e057a7328a2af5b632a1be` |
| `src/math/core/mml_tree/mml_nodes/mrow.ixx` | `2dd2a919efaf0daf71274199dececfdb9382c5bfb5d21b0f90277d2a5be4429a` |
| `src/math/core/mml_tree/mml_nodes/mspace.ixx` | `da9084a735606cb7a71e6211827461a0396838ef8e1c2e71dbd8e79e421e3d84` |
| `src/math/core/mml_tree/mml_nodes/msqrt.ixx` | `db618e4830257839cc5b065067dd9a76aeac6a0b849eacfb0c8d54579172b224` |
| `src/math/core/mml_tree/mml_nodes/mtable.ixx` | `92440779983d7c1b597d44a6344bfa9de404fb1b23abb45d300a257acf5b8d48` |
| `src/math/core/mml_tree/mml_nodes/mtd.ixx` | `8002f400b7cd18c90d098bc6bef4608b0b28b66a983cd5dd7f4f73b69db31861` |
| `src/math/core/mml_tree/mml_nodes/mtr.ixx` | `36bc2a9573b003f6dd6852a077ab26bb08044721893491e270a09d1f3c78e3cc` |
| `src/math/core/mml_tree/mml_nodes/tex_atom.ixx` | `db0d2b49ef14d667374282da0fcdc42c269b7c3672ce7c1c02829d84fff6693d` |
| `src/math/core/mml_tree/mml_nodes/mstyle.ixx` | `1aeed5ca23de4f3e65c3affeadcfe36a441ccc94049c6200765d10e77f7fa918` |
| `src/math/core/mml_tree/mml_nodes/mathchoice.ixx` | `dc2517a48856b650bd1c4306339422963c598452d62631cd183f9acc126254ba` |
| `src/math/core/mml_tree/mml_nodes/semantics.ixx` | `b00643dcca8c424b255994c3899d831cdecbf901c9ea2ccc49dab47ae05b451a` |

## Pinned Source references

| Pinned source | SHA-256 | Signatures/behavior cited |
|---|---|---|
| `.reference/MathJax-src/ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` | Parent explicit-null initialization and generic tree methods. |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` | `Parent`, core/coreMO, class helpers, setTeXclass, inheritAttributesFrom, mError, empty-node overrides. |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mo.ts` | `2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605` | `coreParent`, `coreText`, and operator TeX-class input/return. |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mrow.ts` | `af58f0c1aa39a71d6c3dc5addf358a8cefeecb3f76cb483ec940f3a7ea72b4b9` | `firstNonSpace`, `lastNonSpace`, inferred-row `notParent`, `setTeXclass`. |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/maction.ts` | `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895` | Selected-child falsey fallback and delegated core values. |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mfenced.ts` | `64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397` | Fake-node product and nullable open/close source fields. |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mstyle.ts` | `7442e2885d2f4605d42f5b4472b3183f9cbadbd8ed5c0aeddf03b8f23c1d8616` | Raw falsey `notParent` child return. |
| `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/TeXAtom.ts` | `bad114224392b94852456a4636b85ad90a5a2c96b63e6c7911d155db299f7028` | TeXAtom `notParent`, set/adjust TeX class. |
