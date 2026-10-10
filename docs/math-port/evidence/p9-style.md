# P9 style, error, and phantom node implementation ledger

Scope is limited to `MmlMstyle`, `MmlMerror`, and `MmlMphantom` source
methods. Source and emitted CommonJS hashes were checked against the pinned
4.1.3 checkout and the installed `@mathjax/src` package:

| Source | SHA-256 | Emitted CJS | SHA-256 |
|---|---|---|---|
| `ts/core/MmlTree/MmlNodes/mstyle.ts` | `7442e2885d2f4605d42f5b4472b3183f9cbadbd8ed5c0aeddf03b8f23c1d8616` | `cjs/core/MmlTree/MmlNodes/mstyle.js` | `49e13c923b88f6b41806c3f030a30edbfe8527beee6350e0e7067fc87707c488` |
| `ts/core/MmlTree/MmlNodes/merror.ts` | `a13174c3d2ea478562efb382e77cac194010f65111203d05df018e11506703bf` | `cjs/core/MmlTree/MmlNodes/merror.js` | `cd9b48970d5d0878d3806e8ff4e4f091d2db38f5021a63aa043acfc50d952d6a` |
| `ts/core/MmlTree/MmlNodes/mphantom.ts` | `b06531aeaf78ef1a0e78ebb4e94024079e37d0fadf1cbb8b8a68e56d52e46006` | `cjs/core/MmlTree/MmlNodes/mphantom.js` | `dae156634a5a2fd1076af9abd551df195463a2b6db029635b13e9809ddb566f3` |

## Member mapping

| Source member | Source behavior | Native implementation or binding |
|---|---|---|
| `MmlMstyle.defaults` (`mstyle.ts:37–48`) | Shallow-derived style defaults over `AbstractMmlLayoutNode.defaults`, with the listed inherited style values. | Root-owned `mstyleDefaults()` and `MmlMstyle::defaults()` binding in `mstyle_bindings.cpp`; this packet does not redefine static defaults. |
| `MmlMstyle.kind` (`52–54`) | Returns `mstyle`. | `MmlMstyle::kind()` in `mstyle.cpp`. |
| `MmlMstyle.notParent` (`59–61`) | Reads only child 0, then checks its child-list length for exactly one. | `MmlMstyle::notParent()` reads the same slots. The frozen host contract maps a null/missing/empty child 0 result to `false`; a null outer or nested child-list access remains a TypeError. |
| `MmlMstyle.setInheritedAttributes` (`66–75`) | Applies the source's default parameters (`attributes={}`, `display=false`, `level=0`, `prime=false`) before the first inherited write, then writes inherited `displaystyle`, then inherited `scriptlevel`, then invokes the superclass implementation. | `MmlMstyle::setInheritedAttributes()` calls the frozen `host::inheritanceDefaults` at entry, which supplies source defaults for Missing/undefined values only. It then preserves write order and calls `AbstractMmlNode::setInheritedAttributes()`. It reports a TypeError if the native attributes handle is null. |
| `MmlMstyle.setChildInheritedAttributes` (`82–114`) | Reads explicit `scriptlevel`; when non-null, calls its `.toString()`, checks `/^\s*[-+]/`, uses `parseInt`, adjusts the incoming level by primitive `+=` for relative values or replaces it for absolute values, and resets prime. It then processes explicit `displaystyle` with strict `=== true`, resets prime when present, applies raw non-null `data-cramped`, adds this node's attributes to inherited ones, and dispatches only child 0. | `MmlMstyle::setChildInheritedAttributes()` follows that source order and uses the root-frozen `host::methodString`, `host::relativeLevel`, `host::integer`, and `host::add` operations. `addInheritedAttributes` remains virtual. Child 0 is reread after inherited-attribute addition; empty/missing/null child 0 throws at the final dispatch. |
| `MmlMerror.defaults` (`merror.ts:36–38`) | Shallow copy of `AbstractMmlNode.defaults`. | Root-owned `merrorDefaults()` and binding. |
| `MmlMerror.texclass` (`43`) | Initializes the protected field to `TEXCLASS.ORD` after `super`. | `initializeClassFields()` writes with `initializeTexClass(TEXCLASS.ORD)`, using the frozen `TEXCLASS` object (not a C++ type) and the direct-storage hook specified by the class-binding contract. |
| `MmlMerror.kind` (`48–50`) | Returns `merror`. | `MmlMerror::kind()` in `merror.cpp`. |
| `MmlMerror.arity` (`57–59`) | Returns `-1`, indicating inferred row behavior. | `MmlMerror::arity()` in `merror.cpp`. |
| `MmlMerror.linebreakContainer` (`66–68`) | Returns `true`. | `MmlMerror::linebreakContainer()` in `merror.cpp`. |
| `MmlMphantom.defaults` (`mphantom.ts:36–38`) | Shallow copy of `AbstractMmlLayoutNode.defaults`. | Root-owned `mphantomDefaults()` and binding. |
| `MmlMphantom.texclass` (`43`) | Initializes the protected field to `TEXCLASS.ORD` after `super`. | `initializeClassFields()` writes with `initializeTexClass(TEXCLASS.ORD)`, using the frozen `TEXCLASS` object. |
| `MmlMphantom.kind` (`48–50`) | Returns `mphantom`. | `MmlMphantom::kind()` in `mphantom.cpp`. |

## Host adaptations and limits

The required `mstyle` conversions were already present in the root-owned
`leaf_host.ixx`: `methodString` models a direct `.toString()` call, `relativeLevel`
tests the source's leading whitespace/sign rule, `integer` delegates to the
JavaScript-compatible `parseInt`, and `add` preserves primitive string
concatenation as well as numeric addition. `Property` keeps `Missing`, null,
boolean, number, and string distinct, so explicit nullish checks and strict true
comparison remain available. No missing host primitive or interface declaration
blocked this packet.

The C++ API uses a boolean return for `notParent`, as required by the frozen
contract, even though JavaScript may return the falsey value `undefined` or
`null` from `childNodes[0] && ...`. Both are observed by callers as false. Native
TypeErrors identify the source operation but do not promise byte-for-byte
JavaScript engine error text. The typed value adapter does not model arbitrary
JavaScript proxies, property descriptors, or evaluated functions. Compiled
native virtual-node accessors and native callbacks are supported; their
identity and source-order effects remain part of the contract and are not
replaced with snapshots or generic coercions.

`tests/math/reference_leaf.cpp` contains the root-owned frozen executable
reference cases, including the `mstyle` raw relative levels/prime and
display/cramped cases. This packet did not edit or run that suite, and makes no
pass claim. The P9 fixture count and independent native acceptance remain with
the root reviewer.
