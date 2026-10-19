# Common output and MathDocument service proposal

This proposal is intentionally not imported by the registered Core MathItem
module. The accepted `MathDocumentService` interface remains unchanged until
the canonical DOM owner and document graph are registered together.

Pinned `CommonOutputJax` consumes the same live `MathDocument` object in three
places: `setDocument(html)` stores it and assigns `adaptor.document = html.document`;
`getMetrics` iterates `html.math`; and `styleSheet` tests for and calls
`html.getStyles()` with `html` as receiver, then iterates the returned values.
These are independent observations of the source document object and must not
be backed by a copied metrics vector or reconstructed record.

The eventual typed service needs to expose:

* a stable identity for the actual document owner;
* its current document node as the canonical DOM tagged value, preserving
  Missing, Null, and node identity;
* its current MathJax MathList owner, with live iteration over actual MathItem
  identities and order;
* whether `getStyles` exists, plus invocation of that callable with the actual
  document as receiver and its actual iterator/result behavior.

The DOM node must resolve through the same canonical Lite/DOM node store used
by the adaptor. MathItem start locations must continue to resolve through the
actual location record and canonical node binding. Common output code must not
cache a second `document.node` or list representation. A MathDocument
implementation that cannot provide one of these services should report the
unsupported boundary explicitly instead of returning an empty list, Null node,
or empty style sequence.

No production Core interface, adapter, or acceptance claim is introduced by
this proposal. Registration requires the DOM owner, document owner, and their
Context identities to be available as one coherent graph.
