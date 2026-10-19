# SVG XML import leaf-node correction (G169)

The G168 failure is caused by an unconditional `childNodes(source)` in `SvgOutputJax::importXmlSvgNode()`. Import recurses from a real TeXHTML `<b>` element to its text child; `LiteAdaptor.childNodes()` is defined for `LiteElement`, while `LiteText` exposes `value` and `parent` but no `children`. The current `childNodesArray()` routes through `LiteNodeStore::field(node, "children")`, which throws `unsupported Lite owner field` for Text/Comment nodes.

The candidate keeps the element recursion and skips child enumeration for text and comment leaves. This covers both the G168 output-render failure and the helper-return fixture, which contains both text and comment descendants. The production fixture is an existing formal test and must remain unchanged; this patch changes only `src/math/output/svg/output_jax.cpp`.

Evidence:
- G168 profile comparison fails only for `texhtml-explicitly-enabled` (continuous and fresh-session group, index 52), whose actual HTML payload includes a `<b>` element with a text child.
- `.reference/MathJax-src/ts/adaptors/lite/Element.ts` defines `children` on `LiteElement`.
- `.reference/MathJax-src/ts/adaptors/lite/Text.ts` defines a distinct `LiteText` type; it has no `children` member.
- `.reference/MathJax-src/ts/adaptors/liteAdaptor.ts` declares `childNodes(node: LiteElement)` and spreads `node.children`.
- `src/math/output/svg/output_jax.cpp` currently queries `childNodes` before branching on `#text`/`#comment` and then handles their `value` as leaf semantics.

Apply check (from repository root):

```powershell
git apply --check tools/math-port/candidates/svg-xml-import-leaf-g169/svg-xml-import-leaf-g169.patch
```

No build or test was run for this candidate.
