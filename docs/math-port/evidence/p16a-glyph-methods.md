# P16a source mapping: `MmlMglyph.verifyAttributes`

Pinned upstream is MathJax 4.1.3 `ts/core/MmlTree/MmlNodes/mglyph.ts`, SHA-256 `2c2d336067d3970c55f3fcdafb82b55d4339a2c11d69088c8c7eff4a6d953504`. This packet implements only `verifyAttributes`; root-generated declarations, defaults, `kind`, and `texclass` initialization remain in `mglyph.ixx` and `mglyph_bindings.cpp`.

## Source-member mapping

| Source member | Upstream lines | Native mapping |
|---|---:|---|
| Class declaration / parent | 31–33 | `MmlMglyph : AbstractMmlTokenNode` in `mglyph.ixx`. |
| `defaults` | 36–44 | Root-generated fresh shallow copy of token defaults with `alt:''`, `src:''`, `index:''`, `width:'auto'`, `height:'auto'`, and `valign:'0em'`. |
| `texclass` field | 49 | Root-generated class-field phase initializes the node to `TEXCLASS.ORD`. |
| `kind` | 54–56 | Root-generated getter returns `mglyph`. |
| `verifyAttributes(options)` | 61–76 | Implemented in `mglyph.cpp`, with the source reads, branch, diagnostic call, and qualified superclass call described below. |

## `verifyAttributes` correspondence

The source first reads `this.attributes`, then invokes its `getList` once with names in source order: `src`, `fontfamily`, `index` (`mglyph.ts:62–66`). The native method captures the current `attributes()` pointer once and calls `getList({"src", "fontfamily", "index"})` in that same order. If the typed native attributes pointer is null, it raises a TypeError at that dereference boundary rather than silently treating it as empty.

The invalid test is strict source equality: `src === '' && (fontfamily === '' || index === '')` (`:67`). Native code compares the raw `Property` values to an empty-string `Property`; false, null, missing/undefined, zero, arrays, and objects are not string-coerced. The raw matrix in the frozen oracle covers combinations of all those values.

On the invalid branch, source calls `this.mError(exactMessage, options, true)` (`:68–72`) and does not call superclass verification. Native code passes the same `PropertyList` handle through `ErrorOptions(options)`, uses the exact message and `shortError=true`, and returns without calling the superclass. The `(void)` only discards the return, matching source use as a statement. On the valid branch, source calls `super.verifyAttributes(options)` (`:73–75`); native code qualifies `AbstractMmlTokenNode::verifyAttributes(options)`, avoiding virtual redispatch and preserving the established token superclass checks.

The method is nonrecursive and adds no call guard, image lookup, font loading, fallback glyph, or other behavior. The root-owned frozen actual-CJS oracle is the authority for expected results; this report does not claim test execution or packet acceptance. Root owns independent review, build, mutation replay, and acceptance.
