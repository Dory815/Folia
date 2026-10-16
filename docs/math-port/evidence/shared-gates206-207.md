# Shared MML runtime evaluation, Gates206–207

Both builds compile all 61 registered targets in the existing build/core
directory. Both full evaluations complete with 2,157 unchanged manifest
entries. All actual runner handles have terminated, and
`finish-shared-evaluation.cjs` records evaluationExitCode=1 for both gates.
The goal and Folia integration remain incomplete.

| Check | Gate206 | Gate207 |
| --- | --- | --- |
| Complete SVG/errors against pinned MathJax | 0/150 | 0/150 |
| Common mathematical layout | 0/75 | 0/75 |
| Promoted BBoldx text SVG/glyph regression | 0/6 | 0/6 |
| Checkpoint comparisons | 4/20 | 4/20 |
| Core | 0xC0000005 | 0xC0000005 |
| Host / Parser / Foundation | fail | fail |
| Regexp / LinkedList | pass | pass |
| Three real Renderer lifetime drivers | fail during warm render | fail during warm render |

The complete-SVG capture processes exit normally, but their captures contain
errors rather than successful SVGs. Their binary audits passing means the
process completed with the expected unchanged executable; the separate SVG
comparison reports reject the output. Similarly, both paired Scene/SVG
captures contain 150 equal error pairs and zero successful Scene pairs.
This is no evidence of mathematical layout or rendering acceptance.

## Argument bridge correction

Gate206 rendering fails with TypeError, `Node properties must be a
PropertyList`. mmlFactoryContextArguments wrapped an existing record-valued
Property inside std::any, while the native Tree constructor consumes a
PropertyList inside std::any. The correction unwraps only a genuine record
view and preserves its shared storage; null and primitive values are retained
without defaulting them to an empty object. The added native test checks
writes in both directions against that same owner.

Gate207 rendering reaches the next failure: TypeError, `Native MML argument
has no Context Value representation`. The bound/native constructor
initialization path and its operation-time argument projections remain under
investigation. This is a runtime regression introduced by the shared MML
packet, not an accepted restriction of the requested renderer.

## Core process diagnosis

The full Core runs end abnormally with 0xC0000005. The last captured output is
among the direct mfenced throw cases, but the buffered log does not establish
the crashing expression. Narrow Gate206 runs of case 207, its throw group,
and the mfenced group terminate with ordinary assertion failures and do not
reproduce that process failure. The diagnostic checks the unchanged Gate206
executable before and after those runs. No root cause for the full-run
exception is claimed.

## Authoritative local records

For N=206 and 207, inspect:

- `build/math-font-package-gate-N.log` and the corresponding input manifests;
- `build/math-port/diagnostic-exits-N-root.json`;
- `build/math-port/checkpoint-comparisons-gN.json`;
- `build/math-port/profile75-full-svg-comparison-gN-root.json`;
- `build/math-port/profile75-scene-route-audit-gN.json`;
- `build/math-port/bboldx-text-svg-shared-gN.provenance.json`;
- the three `renderer-*-gN.log.provenance.json` records and their stderr files.

No comparison inputs, SVG bytes, goldens, or source captures are changed to
accept the failing native results. The freeze now also waits for the full
SVG, paired Scene and promoted BBoldx process records before releasing build
inputs. Folia still uses its existing QuickJS math worker.
