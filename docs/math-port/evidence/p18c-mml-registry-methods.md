# P18c MML registry and factory methods

This is the implementation mapping for the pinned MathJax 4.1.3 files:

| Source | SHA-256 |
| --- | --- |
| `ts/core/MmlTree/MML.ts` | `c64bcfdf65452559995a92c13db75133b1500ef59efc2be5c9489ceacc4d4964` |
| `ts/core/MmlTree/MmlFactory.ts` | `5e7338cd3eed5a4002243b7371dc5098c6433ff4ba4345cb2d50cf0385b2abdb` |
| npm `cjs/core/MmlTree/MML.js` | `380332b7f0cc13104130affb65532ebca2ac214ccd897b93edda551cea443684` |
| npm `cjs/core/MmlTree/MmlFactory.js` | `d0c4e25237d04a9d21d12ee44776e46d30777e7f284db6f4f3ed1cc625b752d3` |

The reference checkout is at `fb987178c2d279a99b0db5ea02e751691703955e`.
`MML.ts` declares one object literal with 43 entries. `mml.cpp` seeds the
registry in that exact sequence. Keys below are the source
`Constructor.prototype.kind` values, and each native value is a fresh,
identity-bearing `classBinding<T>()` captured once in the registry.

| # | Source constructor | Key | Native class binding |
| ---: | --- | --- | --- |
| 1 | `MmlMath` | `math` | `classBinding<MmlMath>()` |
| 2 | `MmlMi` | `mi` | `classBinding<MmlMi>()` |
| 3 | `MmlMn` | `mn` | `classBinding<MmlMn>()` |
| 4 | `MmlMo` | `mo` | `classBinding<MmlMo>()` |
| 5 | `MmlMtext` | `mtext` | `classBinding<MmlMtext>()` |
| 6 | `MmlMspace` | `mspace` | `classBinding<MmlMspace>()` |
| 7 | `MmlMs` | `ms` | `classBinding<MmlMs>()` |
| 8 | `MmlMrow` | `mrow` | `classBinding<MmlMrow>()` |
| 9 | `MmlInferredMrow` | `inferredMrow` | `classBinding<MmlInferredMrow>()` |
| 10 | `MmlMfrac` | `mfrac` | `classBinding<MmlMfrac>()` |
| 11 | `MmlMsqrt` | `msqrt` | `classBinding<MmlMsqrt>()` |
| 12 | `MmlMroot` | `mroot` | `classBinding<MmlMroot>()` |
| 13 | `MmlMstyle` | `mstyle` | `classBinding<MmlMstyle>()` |
| 14 | `MmlMerror` | `merror` | `classBinding<MmlMerror>()` |
| 15 | `MmlMpadded` | `mpadded` | `classBinding<MmlMpadded>()` |
| 16 | `MmlMphantom` | `mphantom` | `classBinding<MmlMphantom>()` |
| 17 | `MmlMfenced` | `mfenced` | `classBinding<MmlMfenced>()` |
| 18 | `MmlMenclose` | `menclose` | `classBinding<MmlMenclose>()` |
| 19 | `MmlMaction` | `maction` | `classBinding<MmlMaction>()` |
| 20 | `MmlMsub` | `msub` | `classBinding<MmlMsub>()` |
| 21 | `MmlMsup` | `msup` | `classBinding<MmlMsup>()` |
| 22 | `MmlMsubsup` | `msubsup` | `classBinding<MmlMsubsup>()` |
| 23 | `MmlMunder` | `munder` | `classBinding<MmlMunder>()` |
| 24 | `MmlMover` | `mover` | `classBinding<MmlMover>()` |
| 25 | `MmlMunderover` | `munderover` | `classBinding<MmlMunderover>()` |
| 26 | `MmlMmultiscripts` | `mmultiscripts` | `classBinding<MmlMmultiscripts>()` |
| 27 | `MmlMprescripts` | `mprescripts` | `classBinding<MmlMprescripts>()` |
| 28 | `MmlNone` | `none` | `classBinding<MmlNone>()` |
| 29 | `MmlMtable` | `mtable` | `classBinding<MmlMtable>()` |
| 30 | `MmlMlabeledtr` | `mlabeledtr` | `classBinding<MmlMlabeledtr>()` |
| 31 | `MmlMtr` | `mtr` | `classBinding<MmlMtr>()` |
| 32 | `MmlMtd` | `mtd` | `classBinding<MmlMtd>()` |
| 33 | `MmlMaligngroup` | `maligngroup` | `classBinding<MmlMaligngroup>()` |
| 34 | `MmlMalignmark` | `malignmark` | `classBinding<MmlMalignmark>()` |
| 35 | `MmlMglyph` | `mglyph` | `classBinding<MmlMglyph>()` |
| 36 | `MmlSemantics` | `semantics` | `classBinding<MmlSemantics>()` |
| 37 | `MmlAnnotation` | `annotation` | `classBinding<MmlAnnotation>()` |
| 38 | `MmlAnnotationXML` | `annotation-xml` | `classBinding<MmlAnnotationXML>()` |
| 39 | `TeXAtom` | `TeXAtom` | `classBinding<MmlTeXAtom>()` |
| 40 | `MathChoice` | `MathChoice` | `classBinding<MathChoice>()` |
| 41 | `TextNode` | `text` | `classBinding<TextNode>()` |
| 42 | `XMLNode` | `XML` | `classBinding<XMLNode>()` |
| 43 | `HtmlNode` | `html` | `classBinding<HtmlNode>()` |

The non-lowercase keys are source data: `TeXAtom`, `MathChoice`, and `XML`.
`inferredMrow` and `annotation-xml` also retain their exact source spellings.
The native `kind()` definitions for the concrete nodes agree with those keys.
The registry uses a function-local `static const shared_ptr`; its handle cannot
be rebound through `MML()`, while its ordered map remains mutable through the
pointed-to object. Therefore the source class records remain shared by every
factory initialized from this registry.

`MmlFactory.ts` only adds the static `defaultNodes = MML` alias and a getter
returning `this.node`. The native declaration initializes its separately
rebindable `defaultNodes` handle from `MML()`. Its constructor forwards the
caller map and the selected injected/current defaults to the existing
`AbstractNodeFactory` constructor. An engaged map, including an empty one,
selects no defaults; `nullopt` uses injected class defaults when supplied, and
otherwise the current static handle. A null static handle raises `TypeError`
only on that last path. The getter delegates to `creatorView()`, preserving the
same per-factory live creator object on each read and its separation from
`nodeMap_`.

## CJS reconciliation

The installed npm package is under
`tools/mathjax-bundle/node_modules/@mathjax/src`. Its `ts/core/MmlTree/MML.ts`
and `MmlFactory.ts` match the pinned `.reference/MathJax-src/ts` files after
normalizing CRLF to LF (both files compare byte-for-byte after that newline
normalization). The package CJS `MML.js` was loaded directly with Node. It
reports 43 own keys, and every ordered `(key, constructor name, prototype.kind)`
tuple matches the table above, including `TeXAtom`, `MathChoice`, `XML`,
`inferredMrow`, and `annotation-xml`. The CJS `MmlFactory.js` confirms the
source lowering: `defaultNodes` aliases `MML`, and the getter returns `this.node`.

The CJS/native identity and construction oracle remains root-owned under the
P18c contract; this evidence records the direct registry export comparison and
does not claim overall P18c acceptance.
