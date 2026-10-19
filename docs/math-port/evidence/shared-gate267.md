# Native math validation: gate 267

Gate 267 built 13 targets in the existing `build/core` directory with at most
four compile jobs. Build and evaluation completed with exit code 0. All 2,212
registered input hashes remained unchanged throughout evaluation.

| Executable or focused selection | Executed tests | Assertions |
| --- | ---: | ---: |
| Core, complete | 5,718 | 350,271 |
| Host, complete | 479 | 3,453 |
| Output foundation, complete | 116 | 1,334 |
| Boldsymbol/Configmacros | 7 | 69 |
| Begingroup | 4 | 43 |
| Output Jax | 22 | 101 |
| Token walk Source cases | 1 | 780 |
| Constructor ACCENTS live entries | 1 | 21 |
| TextNode raw string field | 1 | 3 |

All passed. The three previously failing Core families now pass in both
focused runs and the complete Core run. The complete Core run took about
282 seconds and exited normally without creating its optional crash dump.
The package compile fixture now constructs its canonical Source-visible
MathItem instead of using an unbound native substitute.

The pinned MathJax source is commit
`fb987178c2d279a99b0db5ea02e751691703955e` (4.1.3). The existing 75-case
profile passed common layout 75/75, full SVG exact-byte comparison 150/150,
paired Scene geometry 150/150, and four listener-position pairs. The 150
records cover continuous and fresh sessions. Scene-only measurement made
zero SVG materialization and zero XML serialization calls. Four static font
sources, font-instance extension data, and the provider's 40 modules,
80 requests, and 24,646 glyphs also passed their existing comparisons.

Request-owner checks passed cancellation, malformed input, persistent macro
and label/reference state, and retained results after Renderer destruction.
Across 192 renders, owner-bearing record growth fell from 28,416 in gate 262
to 28,032 in gate 267: 384 fewer records. The MathDocument fix captures its
private conversion reducer and every private MathItem, including both the
main item and Bussproofs' scratch item. Public retained reducers and items
remain live in the focused regression.

Context storage is still unbounded. Allocation slots and MML projection
counts still increase; MML projection counts match gate 262. The new Source
constructor properties-birth capture passes its focused ownership tests,
but the actual renderer creates nodes through native NodeFactory/MmlFactory
and bypasses that Source constructor observer. No reduction of the renderer's
MML projection growth is claimed.

Ignored execution records are under `build/math-port/g267-*`; the terminal
input audit and execution summary are
`build/math-port/final-evaluation-g267.json`. Profile results are in
`build/math-port/g267-current-acceptance/acceptance.json`. These are finite
test/profile results, not a claim of all MathJax features or completion of
the Folia integration. Folia's math backend has not been switched yet.
