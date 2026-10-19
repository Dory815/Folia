# LiteParser `checkDocument` order witness

`tools/math-port/lite-parser-check-document-order-oracle.cjs` invokes the pinned
MathJax 4.1.3 CJS implementation and verifies both TypeScript and CJS hashes.
It writes the exact result under ignored `build/math-port/`.

For a body containing a doctype comment followed by one `html` element, the
observed call sequence includes:

1. `adaptor.body(root)` to obtain the input to `getOnlyChild`;
2. the parser's current `getOnlyChild` method;
3. a separate `adaptor.childNodes(adaptor.body(root))` evaluation for the
   pre-node doctype scan (the body is read again after the `childNodes` member
   lookup);
4. the comment's `type` assignment, then `adaptor.kind` for the selected node
   and each child;
5. `head` and `body` assignments, `root` assignment, and `adaptor.remove`;
6. fresh `root.body` / `root.head` reads around `adaptor.parent`, followed by
   conditional `append` and `insert` calls.

The second body read is observable and must remain live across getters or method
rebinding. The property writes target the document's actual own fields. The
probe is source evidence only; native parity remains pending Root's build and
comparison.
