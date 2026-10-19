# TeX formatter result boundary: gate 247

The direct Context `TeX.formatError` method and default `OPTIONS.formatError`
callback now return the existing canonical object for the typed formatter's MML
node. They previously called or received that formatter result and then threw
`Unimplemented` at conversion. No second formatter or new node identity is
introduced. Both adapters read `message` before `id`, matching Source's accessor
order; the existing compile-result exception contract is unchanged.

Four focused tests run inside the complete passing gate 247 Host suite
(455 tests / 3,185 assertions). They cover exact formatted text and
`data-mjx-error`, Source's `merror > inferredMrow > mtext > text` structure,
canonical return/factory identities, accessor order/receiver, a borrowed method
called against another TeX factory, and actual parser-error compilation through
custom formatter callbacks, including their receiver, arguments and exception
propagation. The test fixture does not equate native allocation ownership with
the node's separately observable Source `factory` field.

The independently executed witness
`tools/math-port/regressions/g244/format-error-source.cjs` checks pinned MathJax
4.1.3 TS/CJS hashes and the corresponding default/custom/borrowed formatter
behavior and exact inferred-row structure. Output:
`build/math-port/format-error-source-g247.json`.

All 2,204 registered gate inputs and the Host executable remained unchanged
through the successful run. This accepts the tested formatter boundary;
general Context lifetime, broader renderer/platform acceptance and replacing
Folia's math QuickJS path remain open.
