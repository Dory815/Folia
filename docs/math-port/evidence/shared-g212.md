# Gate212 native MathJax evaluation

The existing `build/core` tree compiled all 61 targets with `--parallel 4`.
The build and completed evaluation kept all 2,179 registered inputs unchanged.
MathJax is pinned to 4.1.3, revision
`fb987178c2d279a99b0db5ea02e751691703955e`.

| Observation | Result |
| --- | --- |
| Public API | 9/9 Source matches |
| Common layout corpus | 75/75 Source matches |
| Public SVG/error corpus | 150/150 matches, continuous plus fresh sessions |
| Actual SVG records | 146; four records are matching host errors |
| Paired Scene/SVG routes | 146 successful pairs, four matching error pairs |
| BBoldx text SVG and glyph-path regression | Accepted |
| Parser, Regexp, LinkedList runners | Pass |
| Core runner | Access violation, exit 3221225477 |
| Host and Output Foundation runners | Fail |
| Package captures | 24/25 complete; mhchem times out after 60 seconds |
| Package comparisons | 16/20 pass |
| Request lifetime and owner-retirement observations | Both time out after 60 seconds |

The four failed comparisons are colortbl, mhchem-package, tex-next-six, and
tex-font-packages. The mhchem comparison lacks a completed capture; it does
not establish a rendering difference. Its partial output contains 123/220
complete records. Earlier Gate197 completed the same observation in 17.556
seconds, so simply increasing the limit would not explain the throughput
change.

The public corpus's `action-toggle` and `action-tooltip` fail in both session
modes because the Lite host lacks `addEventListener`. Matching those errors
does not verify successful Maction SVG. A pinned Source probe with passive
listener registration produces SVG for both formulas. Event registration
belongs to the output host; completed Scene values must not retain Context
closures. Actual event dispatch and rerendering remain separate host work.

The staged Tree batch's focused checks pass 387 Factory, registry, parser,
identity and BaseItem assertions. Native Tree passes 154/155 assertions across
18 tests; its remaining failure is a null `oldChild.parent` read that should
throw after replacing the child and assigning the new parent. The pinned CJS
witness confirms that mutation order. Gate213 contains the correction but has
not yet verified it.

The owner-count probe reports total Context slots, including cleared retired
records: 7,523 after state setup, 15,747 after 32 renders, and 23,971 after 64.
Native products, wrapper and Scene handles are zero after each sample.
These counts alone do not establish live payload retention. Native identity,
Attributes view ownership and retired-slot storage require distinct checks.

Raw records are under ignored `build/math-port/`: `gate-212-inputs-before.json`,
`gate-212-post-evaluation-changes.json`, `diagnostic-exits-212-root.json`,
`checkpoint-comparisons-g212.json`, `package-captures-g212.json`,
`profile75-full-svg-comparison-g212-root.json`,
`profile75-scene-route-audit-g212.json`, and
`bboldx-text-svg-shared-g212.provenance.json`.

The first finish command encountered an observation filename mismatch.
Re-running the existing boundary annotator and comparer with the expected
`-root.json` filename allowed the unchanged-input audit to finish. Its
evaluation exit remains 1. Folia still uses the math QuickJS worker; native
integration has not been accepted.
