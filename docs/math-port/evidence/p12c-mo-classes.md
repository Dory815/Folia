# P12c MmlMo class-method port evidence

This file maps the five methods implemented in
`src/math/core/mml_tree/mml_nodes/mo_classes.cpp` to the pinned MathJax 4.1.3
source. Upstream source: `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mo.ts`,
SHA-256 `2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605`.
The upstream MathJax Consortium copyright and Apache-2.0 notice are retained in
the implementation file; the full license is in `src/math/LICENSE-MathJax.txt`.

| Native member | Upstream member and lines | Source behavior mapped |
| --- | --- | --- |
| `MmlMo::coreParent` | `coreParent`, 235–249 | Fetches the `math` constructor once from the current factory; walks the outer embellished/coreMO chain and tests actual constructor identity through `host::moInstance`; returns the last qualifying wrapper or this node. |
| `MmlMo::coreText` | `coreText`, 255–274 | Null input returns empty text; embellished input immediately follows `coreMO().getText()`; otherwise unwraps singleton `mrow`/`TeXAtom`/`mstyle`/`mphantom` and all `munderover` nodes while child 0 is present, then reads text only for tokens. |
| `MmlMo::isAccent` | getter `isAccent`, 286–313 | Starts with boolean false; checks the core parent's parent; chooses `accent`/`accentunder` through mover/munder/munderover child core checks and virtual `over()`/`under()` indices; an explicitly present parent attribute leaves the initial false untouched, otherwise returns this node's raw `accent` Property. |
| `MmlMo::setTeXclass` | `setTeXclass`, 320–340 | Reads form and fence together, then the `texClass` property; preserves the missing-class plus spacing early return; for truthy fence and REL class applies prefix OPEN/postfix CLOSE before delegating to adjustment. |
| `MmlMo::adjustTeXclass` | `adjustTeXclass`, 347–405 | Captures raw class and previous class; strict NONE early return; preserves `autoOP`, previous-class and inherited-level writes; applies leading/trailing BIN normalization; performs the embellished ancestor walk and last-child check, retaining prior writes if malformed ancestry throws. |

## Host boundary and observable adaptations

All five methods enter `TreeCallGuard<MmlNode>`. Recursive `coreParent()` from
`isAccent()` therefore consumes the same tree-call budget as its callers.
`mo_host.ixx` supplies actual-constructor `moInstance`, checked child access,
required-node and virtual attribute access helpers. Null `childNodes`, required
children, null attributes, and malformed parent state are converted into typed
TypeErrors rather than native undefined behavior. MmlNode and its children,
attributes, class getters/setters, `core()`, `coreMO()`, `isKind()` and
`isEmbellished()` remain the shared native graph; this implementation adds no
alternate tree representation.

`isAccent()` validates the result of virtual `coreParent()` before reading its
parent pointer. In the trailing-BIN rule's second branch, the chained source
assignment is right-associative: `this.prevClass` is written before
`prev.texClass`; the native implementation preserves that order.

Raw values are retained for the source's `prevClass`, inherited
`scriptlevel`, `accent`, and class-property paths. The only synthesized class
values are the literal source constants (`ORD`, `OPEN`, `CLOSE`, and `NONE`).
The native runtime's fixed tree guard and typed failures are host safety
boundaries; no TeX-specific fallback or behavior normalization was introduced.

Root owns frozen JavaScript-oracle fixtures, builds, mutation checks, and final
acceptance. This evidence records source mapping only and does not claim that
the packet has passed those checks.
