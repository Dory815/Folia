# ParseUtil pinned-source coverage

`tools/math-port/parse-util-original-oracle.cjs` runs the actual pinned CJS
ParseUtil, KeyValueDef, KeyValueTypes, TeX, ParseOptions, TexParser and MML
factory/node implementations. It verifies the pinned TypeScript clone and CJS
hashes against the MathJax scope lock. The generated original-only corpus is
`parse-util-original-source.json`; these observations are not native acceptance.

| Source responsibility | Original observations |
| --- | --- |
| `KeyValueDef` constructor, static `oneof`, predefined verifier/converter callbacks and `KeyValueTypes` own descriptors | `key-value-builtins-and-oneof-real-callbacks` |
| `cols` | `cols-numbers-and-empty` |
| `fenced`, `fixedFence`, `mathPalette` with real ParseOptions/MML | `fenced-fixedfence-mathpalette-real-node-trees` |
| `fixInitialMO` initial whitespace, TeXAtom and empty cases | `fixinitialmo-real-arrays-holes-and-texatoms` |
| `internalMath` fallback lexer, delimiters, raw Unicode, error, level/font and live callback path | `internalmath-default-and-box-delimiters`; `internalmath-level-font-and-live-callback` |
| `internalText` whitespace and definition forwarding | `internaltext-whitespace-and-definitions` |
| `underOver`, `checkMovableLimits` | `underover-movable-limits-and-stack-shape`; `checkmovablelimits-source-property-and-form` |
| `setArrayAlign` parser and non-parser branches | `setarrayalign-parser-and-no-parser-branches` |
| `substituteArgs`, `addArgs`, macro and buffer errors | `macro-substitute-addargs-limit-and-errors` |
| `checkMaxMacros`, `checkEqnEnv` | `macrocount-error-ids-and-equation-nesting` |
| `copyNode`, `mmlFilterAttribute`, `getFontDef` | `copy-node-real-tree-and-node-lists`; `mmlfilter-fontdef-and-getfontdef-liveenv` |
| `keyvalOptions`, internal `readKeyval`/`readValue`/`removeBraces` | `keyval-options-types-filter-errors-and-l3keys` |
| `isLatinOrGreekChar` | `latin-greek-normalization-character-samples` |

The pinned-source corpus now includes big fences and colored node trees,
escaped internal-math delimiters plus `ref`/`eqref`, `copyNode`'s live
`addNode` calls and falsey `in-lists` values, `fixInitialMO`'s captured length
with live indexed reads, and `keyvalOptions`' current getter and method
receiver. Remaining useful original witnesses are a truly embellished
`munderover`, inherited/prototype and symbol-key behavior for `keyvalOptions`.
The actual enabled profile mostly passes ordinary parser records, typed MML
children, and plain literal key maps; broad JS object reflection is not implied
by this corpus.

Native implementation boundary: the actual `textmacros` package installs an
`options.internalMath` callback that constructs a real `TextParser` and
returns its MML nodes. The current native parser graph has no TextParser
identity/allocation binding or Context-array-to-`mml::Children` identity
bridge, so the native method must report this as unimplemented until that
cross-module binding exists. The native TexParser constructor similarly
rejects an ObjectRef environment instead of copying its properties; its normal
internal `make()` path accepts the canonical Property graph and is independent
of that constructor bridge. Neither boundary is accepted by the Source-only
oracle or should be counted as complete package parity.
