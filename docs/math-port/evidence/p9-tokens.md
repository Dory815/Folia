# P9 token and space-node implementation evidence

This evidence covers only the five source implementations owned by the P9
token packet. The source is pinned MathJax 4.1.3 at commit
`fb987178c2d279a99b0db5ea02e751691703955e`. Root owns frozen interfaces,
defaults, primitive bindings, fixtures, build, and acceptance. This ledger does
not claim native acceptance.

## Member ledger

| Source member | Native body or root binding | Source-order behavior / adaptation |
|---|---|---|
| `MmlMi.defaults` (`mi.ts:41-44`) | Root `miDefaults()` / `MmlMi::defaults()` binding | Fresh shallow family table is initialized from token defaults before class registration. |
| `MmlMi.operatorName`, `MmlMi.singleCharacter` (`mi.ts:48-54`) | Root live `Pattern&` bindings | C++ callbacks point at root's exact non-Unicode UTF-16 grammars; the implementation calls the current callback by reference, never substitutes code-point/wchar matching. |
| `MmlMi.texclass` field (`mi.ts:58`) | `initializeClassFields()` → `initializeTexClass(ORD)` | Applied after superclass initialization by `classBinding`; writes the underlying field directly and avoids virtual setter dispatch during initialization. |
| `MmlMi.kind` (`mi.ts:63-65`) | `kind()` | Returns `"mi"`. |
| `MmlMi.setInheritedAttributes` (`mi.ts:73-86`) | `setInheritedAttributes(...)` | Applies this method's own source defaults for missing display/level/prime (`false/0/false`) at entry; explicit null/raw nonmissing values remain unchanged. Then calls token/base inheritance with the original attribute handle and normalized arguments. Next it gets text and runs the live single-character pattern. Only on match does it inspect incoming `attributes.mathvariant` using JS truthiness; if falsy, it sets inherited mathvariant to italic on this node. No explicit value is overwritten. An empty native Pattern binding raises TypeError; the pinned raw-TeX profile uses the frozen compiled callback. |
| `MmlMi.setTeXclass` (`mi.ts:91-111`) | `setTeXclass(previous)` | Calls `getPrevClass` first, then reads text. Only UTF-16 `.length > 1` proceeds to the live operator-name pattern. It then checks node mathvariant by strict string equality with `normal`, and tests `autoOP` then `texClass` for source `undefined`/native Missing. On success it invokes the virtual TeX-class setter with OP, then assigns `autoOP=true`; setter overrides therefore observe the runtime assignment. It returns this node and does not call a generic token classification algorithm. An empty native Pattern binding raises TypeError. |
| `MmlMn.defaults` (`mn.ts:36-39`) | Root `mnDefaults()` / defaults binding | Fresh shallow copy per root contract. |
| `MmlMn.texclass` field (`mn.ts:43`) | `initializeClassFields()` → `initializeTexClass(ORD)` | Direct post-super field write. |
| `MmlMn.kind` (`mn.ts:48-50`) | `kind()` | Returns `"mn"`. |
| `MmlMtext.NONSPACELIKE` (`mtext.ts:36`) | Root live `NONSPACELIKE()` reference | Retains the mutable list identity; query uses explicit-presence semantics through `Attributes::hasOneOf`. |
| `MmlMtext.defaults` (`mtext.ts:41-44`) | Root `mtextDefaults()` / defaults binding | Fresh shallow copy per root contract. |
| `MmlMtext.texclass` field (`mtext.ts:48`) | `initializeClassFields()` → `initializeTexClass(ORD)` | Direct post-super field write. |
| `MmlMtext.kind` (`mtext.ts:53-55`) | `kind()` | Returns `"mtext"`. |
| `MmlMtext.isSpacelike` (`mtext.ts:64-69`) | `isSpacelike()` | Runs token `getText`, then exact root `host::onlySpaces` ECMA whitespace predicate. If the regex-equivalent check fails, short-circuits without reading attributes. Otherwise reads the current Attributes handle and returns false if any NONSPACELIKE property is explicitly present, regardless of value. Null Attributes raises TypeError at that source access. A shared TreeCallGuard bounds virtual text getter re-entry. |
| `MmlMs.defaults` (`ms.ts:36-42`) | Root `msDefaults()` / defaults binding | Fresh shallow copy per root contract, with `lquote` and `rquote` values initialized by root. |
| `MmlMs.texclass` field (`ms.ts:45`) | `initializeClassFields()` → `initializeTexClass(ORD)` | Direct post-super field write. |
| `MmlMs.kind` (`ms.ts:50-52`) | `kind()` | Returns `"ms"`. |
| `MmlMspace.NONSPACELIKE` (`mspace.ts:36-46`) | Root live `NONSPACELIKE()` reference | Retains the mutable list identity; width remains deliberately excluded. |
| `MmlMspace.defaults` (`mspace.ts:48-64`) | Root `mspaceDefaults()` / defaults binding | Fresh shallow copy per root contract; root owns exact default values. |
| `MmlMspace.texclass` field (`mspace.ts:66`) | `initializeClassFields()` → `initializeTexClass(NONE)` | Direct post-super field write. |
| `MmlMspace.setTeXclass` (`mspace.ts:71-73`) | `setTeXclass(previous)` | Returns the exact incoming pointer unchanged and does not change either TeX-class field. |
| `MmlMspace.kind` (`mspace.ts:78-80`) | `kind()` | Returns `"mspace"`. |
| `MmlMspace.arity` (`mspace.ts:87-89`) | `arity()` | Always zero. |
| `MmlMspace.isSpacelike` (`mspace.ts:96-99`) | `isSpacelike()` | Reads current Attributes once for explicit `linebreak`; short-circuits false when explicitly present, including null/false/zero. Otherwise invokes virtual `canBreak()`. Null Attributes throws at `hasExplicit`. |
| `MmlMspace.hasNewline` (`mspace.ts:105-113`) | `hasNewline()` | Reads `linebreak` first, then invokes virtual `canBreak()`. Only after it succeeds does it strictly compare the saved value with `newline` or `indentingnewline`; values are not coerced. |
| `MmlMspace.canBreak` (`mspace.ts:118-124`) | `canBreak()` | Reads a fresh Attributes handle for `hasOneOf(NONSPACELIKE)`; if none are present, reads Attributes again for width, applies root `host::string` then exact ECMA `host::trim`, and rejects only when the first trimmed UTF-16 unit is `-`. Empty/missing/null/false/zero widths follow JS String coercion. Each null-Attributes failure occurs at the corresponding source access. |

## Source fingerprints

| Pinned TypeScript source | SHA-256 |
|---|---|
| `ts/core/MmlTree/MmlNodes/mi.ts` | `3bbddaeac72d2c0f2e5d8a0440c28188f98d1bf1481a0ec6872e103a50f146c2` |
| `ts/core/MmlTree/MmlNodes/mn.ts` | `5eaf9e5c7e0222303e98757aeed6a1d909ff08560591fd7a6985220d70e0522c` |
| `ts/core/MmlTree/MmlNodes/mtext.ts` | `66e24a47505714c0d67b334a8c73c6e85dbfcac1f920913eef9f664546f2403f` |
| `ts/core/MmlTree/MmlNodes/ms.ts` | `71b28a279461524fce585c4e57960d5586bd04a5f52379c66c04e4b1f82cdaf0` |
| `ts/core/MmlTree/MmlNodes/mspace.ts` | `69961341e3323cc3af8721f9d800190e2051057b11cba5558ef1940e0deefdd4` |

## Host boundary and remaining review

The native class binding invokes `initializeClassFields()` only after
`initializeMml()`, matching derived TypeScript field initialization after
`super()`. The direct `initializeTexClass` operation is essential because
initial field writes do not invoke virtual accessors. `Pattern`,
`NONSPACELIKE()`, Attributes, and Properties remain live root-owned objects;
the leaf bodies do not copy or replace their shared identities. Text length and
regex inputs use UTF-16 units. No parser, registry, rendering code, or fallback
is introduced here.

The root-frozen CJS-generated `tests/math/reference_leaf.cpp` fixture was not
edited or executed by this agent. Root still owns source review, filtered/full
builds, resource-boundary checks, and acceptance. `Pattern` is a typed compiled
matcher binding rather than a JavaScript RegExp value union: an empty native
callback is an invalid host binding and raises TypeError. Callable replacement
remains live, as required; arbitrary `RegExp` replacement/coercion is outside
the raw-TeX host contract.
