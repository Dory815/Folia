# P18b — maction selection and projection members

Pinned source: `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/maction.ts`,
SHA-256 `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895`.
The actual pinned CJS is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNodes/maction.js`,
SHA-256 `eddcdd2111b4b7cf5ad72937b0a4ff7eb555b98308c6771a013a18121af8ce7`.

| Source member | Pinned TS lines | CJS implementation | Native mapping |
|---|---:|---:|---|
| `selected` | 50–56 | `Object.defineProperty(..., "selected", get)` at 50–58 | `MmlMaction::selected() const` reads the current attributes handle and `selection` first, then reads child-array length before numeric conversion, applies `Math.min(length, selection)` and `Math.max(1, result) - 1` with NaN propagation, rereads current children for the computed slot, and calls current `factory.create("mrow", {})` for missing/falsy slots. It returns a nullable pointer unchanged. |
| `isEmbellished` | 59–64 | getter at 59–65 | `MmlMaction::isEmbellished()` invokes `selected()` once, then reads the selected node's projection; a null selected pointer raises at that access. |
| `isSpacelike` | 66–71 | getter at 66–72 | `MmlMaction::isSpacelike()` invokes `selected()` once, then reads the selected node's projection; null fails at that access. |
| `core` | 73–75 | method at 73–75 | `MmlMaction::core()` captures one selected child and calls its `core()`; null fails only at the method access. |
| `coreMO` | 76–78 | method at 76–78 | `MmlMaction::coreMO()` captures one selected child and calls its `coreMO()`; null fails only at the method access. |

For `selected`, the TS order is `attributes.get('selection')`,
`childNodes.length`, numeric coercion inside Math.min/Math.max, current
`childNodes[index]`, then conditional zero-argument factory creation. The native
implementation keeps separate child-array reads for length and slot lookup.
Nonintegral, NaN, infinite, negative/out-of-range, and null slots do not become
integer indices; they take the factory path. No fallback node is attached or
cached. If the factory returns null, `selected()` returns null; each projection
raises at its own source-equivalent property or method access. The projections
call `selected()` exactly once and operate on that captured result.

Host adaptations: the source getter is represented by the existing virtual
`selected() const` method. Native child arrays are typed pointer vectors, so only
finite integral in-range indices address slots; JS property-key indexing of
fractional/NaN values consequently follows the missing-slot path. Existing
`TreeCallGuard` bounds recursive native projection chains. This report covers
these five members only; it does not claim complete `maction`, MML, or renderer
acceptance. No build or test was run for this packet.
