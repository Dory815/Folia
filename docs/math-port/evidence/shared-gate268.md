# Document-owned filter records: gate 268

Gate 268 built 13 targets with at most four compile jobs. All 2,213 registered
input hashes remained unchanged through the terminal build and evaluation.

InputJax and OutputJax now route their temporary filter argument records by
the actual document identity into module-owned trackers. Each private HTML
document holds its own issued tokens. Public calls remain untracked; closing
one document cannot retire another document's records. A failed close retains
its token and progress and prevents a new capture from starting. Partial
wrapper binding cleans up only its own newly allocated identity. HTML begin
rollback releases only TeX captures whose begin actually succeeded.

The complete Core suite passes 5,723 tests / 350,295 assertions. Host passes
484 / 3,477, including five new ownership/binding/rollback cases; output
foundation passes 116 / 1,334. Boldsymbol/Configmacros passes 7 / 69,
Begingroup 4 / 43, and Output Jax 22 / 101. Every execution exits 0.

The pinned 75-case profile passes common layout 75/75, complete SVG exact
bytes 150/150, paired Scene geometry 150/150 and four listener-position
pairs. Scene-only materialization and XML serialization counts are both zero.
The existing font table, instance extension and provider comparisons pass.

Across 192 renders, owner-bearing records grow from 2,548 to 29,812:
27,264 additional records, compared with 28,032 in gate 267. The 768-record
reduction equals four outer filter records per render. The cancellation probe
returns to its starting owner-bearing count, 2,043. Persistent macros, labels,
malformed input handling and results retained after Renderer destruction pass.

This is not bounded Context lifetime acceptance: allocation slots and MML
projection counts still grow. Native MML properties, serializer callbacks,
resident retired-record storage, platform raster acceptance and Folia's math
backend switch remain separate work. Mermaid remains paused.

Implementation commit: `8ff6f88`. Ignored execution provenance is under
`build/math-port/g268-*`; the terminal audit is
`build/math-port/final-evaluation-g268.json`, and profile evidence is in
`build/math-port/g268-current-acceptance/acceptance.json`.
