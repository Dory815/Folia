# LiteParser dispatch observations

This is source-only evidence from `tools/math-port/lite-parser-dispatch-oracle.cjs`.
The runner calls the pinned MathJax 4.1.3 CJS implementation directly. Its
TypeScript and CJS SHA-256 values are checked by the runner; observations are
written under ignored `build/math-port/lite-parser-dispatch-original.json`.

Every listed parser method is an ordinary writable, enumerable, configurable
prototype data property. The observed arities are `parseFromString(3)`,
`addText(3)`, `addComment(3)`, `closeTag(3)`, `openTag(4)`,
`addAttributes(3)`, `handlePCDATA(4)`, `checkDocument(2)`,
`getOnlyChild(2)`, `serialize(3)`, `serializeInner(3)`,
`allAttributes(3)`, `protectAttribute(2)`, and `protectHTML(1)`.

The runtime method dispatch is observable at each Source call site:

- `parseFromString` dynamically calls `addText`, `addComment`, `closeTag`,
  `openTag`, and `checkDocument`.
- `openTag` dynamically calls `addAttributes` and, for PCDATA elements,
  `handlePCDATA`; `checkDocument` dynamically calls `getOnlyChild`.
- `serialize` dynamically calls `allAttributes`, `protectAttribute` for
  each mapped attribute, and `serializeInner`. `serializeInner` dynamically
  calls `protectHTML` for text children and `serialize` for element children.
- The parser reads `SELF_CLOSING`, `PCDATA`, and `XMLNS` through the current
  constructor at their Source read points.

The `parse-live-parts-and-method-rebind` observation parses
`<p>original</p>tail`. Its overridden `openTag` replaces the supplied `parts`
array contents with `rewritten`, `</p>`, `after`, and rebinds `this.addText`.
The base parse loop then calls the rebound method for both text segments and
still dynamically calls `closeTag`; output text is `!rewritten` and `!after`.
This establishes that `parts` is the same live Array passed to `openTag` and
then consumed by the outer loop, and that later loop iterations re-read the
method from the receiver.

Likewise, serialization overrides are observed in source call order. The
Context-backed implementation must use the same parser instance, live method
properties, and one canonical parts-array identity; a copied vector or direct
typed call bypasses these Source observations.

Additional order probes are kept in `lite-parser-serializer-order-oracle.cjs`
and `lite-parser-check-document-order-oracle.cjs`. The serializer probe shows
that `x.name` is read/coerced before the current `protectAttribute` function is
captured, and that the function is captured before `x.value` is read. A value
getter that replaces the method therefore affects the next array callback, not
the current one. The `checkDocument` probe confirms that `adaptor.body(root)`
is evaluated a second time for the pre-node scan, after the `childNodes` method
lookup; the method and body must both remain live.

Pinned inputs: `.reference/MathJax-src/ts/adaptors/lite/Parser.ts` has SHA-256 `f3c15592f359cdca1a3c2729c230f8de862e8b67be5ae8f1d2d387e8482fe5d9`; installed `cjs/adaptors/lite/Parser.js` has SHA-256 `38874dadf30ee0bd60184dde9bdf29a1be9d45755893b2a025d8e1b0190ad8c0`.

`lite-parser-array-intrinsics-source-oracle.cjs` records the Array operations
used by the parser: `splice()` versus `splice(start)` versus an explicit
undefined delete count, sparse removed slots, inherited-index reads, and the
TypeError/partial effects when a later Set reaches an inherited getter-only
property. These observations motivated the Context `shift` and `splice`
implementations; no native run has yet verified them.
