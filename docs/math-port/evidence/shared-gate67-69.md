# Actual native document execution, Gates67–69

Gate67 exits 1 with all 1,147 captured input hashes unchanged. The Core/HTML
document and SVG assembly compile, while HTMLMathItem UTF-16 restoration and
diagnostic namespace/Boost.UT assertion issues still prevent linking.

Gates68 and 69 each build all 21 targets and exit 0, with all 1,147 captured
input hashes unchanged. They are successful builds, not successful renderer
acceptance.

## Document probe and whole-profile executions

| Gate | Probe executable SHA256 | PID | Actual exit | Observation |
| --- | --- | ---: | ---: | --- |
| 68 | `4dc733f9527f4b2e41bcb84420523920154eabcf8b773e5dd2bbbeaa36045080` | 52208 | 1 | `x+y` stops at duplicate own-field binding before layout |
| 69 | `aefde9b492d0b50a1f84fbdceb12355601ee089c3113dd46f66e214ad49bcdf2` | 41272 | 1 | Actual compile succeeds; Scene dispatch rejects an unregistered MML kind |

Gate68 probe runs 15:37:08.787–15:37:10.022 UTC. Gate69 runs
15:45:05.734–15:45:06.829 UTC on 2026-10-13. The Gate69 repair binds actual
HTMLMathItem native fields without creating conflicting Context properties.
The document assembly also takes the real MmlFactory shared owner instead of
an alias owning the entire service graph. The earlier alias created
Graph → Parser/TeX → Factory(alias Graph) → Graph. Gate69 verifies that Context,
graph and host all expire after the failed conversion is released. It produces
no completed Scene, so successful Scene lifetime acceptance remains pending.

The diagnostic runner wraps the actual MathItem prototype compile method in
its Context and snapshots the real receiver.root immediately after successful
compile. It does not insert the temporary item into the document math list or
construct a substitute tree. Gate69 emits actual non-null MML for all 75
inputs. The TextNode observer still omits its own text getter and is being
corrected; the snapshots therefore do not establish whole-MML parity.

| Gate | Whole-profile executable SHA256 | PID | Actual exit | SVG results | Whole records matched |
| --- | --- | ---: | ---: | ---: | ---: |
| 68 | `54eb33576580dc7f9a0916e474f750033dcf35555cd3be4aefae9813c8166f17` | 31152 | 0 | 0 | 0/75 |
| 69 | `50d3397f599cb676cad2d2bf3ccda21d2e45fd3d62006c719fcf0f7237353fbf` | 53524 | 0 | 0 | 0/75 |

The driver exit indicates it emitted all observations; each of the 75
conversion records contains an actual error. Its selected package profile is
still base. All 38 enabled packages and all 75 complete Source observations
remain mandatory. The exact pinned Source result remains 73 SVG outputs plus
two Lite event-handler TypeErrors.

## Executed shared tests and font provider

Gate68 Foundation exits 1. Its eight other packets pass: Map 69/10,
native service lifetime 25/2, Lite projection 28/4, immutable Scene 16/2,
Context numeric coercion 24/6, reduce/text/plus 45/10, RenderActions 13/6,
XML provider lifetime 19/2 (assertions/tests). The FontData options packet has
two failures among six tests. The trailing array-hole expectation is wrong:
Source Options.insert copies enumerable keys into a fresh array, so a length-3
source containing only index 1 yields destination length 2. The other failure
requires Source deletion and replacement in the actual native field owner.

The fresh full Gate68 core also exits 1, with exactly one failed assertion/test
in `failed_async_dynamic_provider_runs_source_setup_placeholder`.
The remaining 170 suites pass, totaling 345,634 passed assertions and 5,321
passed tests including the passing part of the failed suite. Core executable
SHA256 is `f788db494ad6b14e8fac2564068759d02f4978e68a2ebeac7580a2f0732aca00`;
PID 31856 runs 15:40:58.429–15:44:38.739 UTC. Host, Parser, Regexp and LinkedList
all exit 0 (458/196/10, 864/56/7, 18,518/41/3, 27/7/1).

The actual Gate69 NewCM module provider matches every field of the pinned
Source trace: 40 modules, 80 ordered events and 24,646 glyphs, including actual
js/cjs cache identity witnesses. Executable SHA256 is
`96eb31df7fbce3a59f98d3c3972c69d7cc2c96ff5f16f5e64f8cd1a7d269104a`;
PID 26208 runs 15:46:39.391–15:46:41.073 UTC and exits 0. Root independently
reruns its strict comparator successfully. This validates that module-provider
packet, not complete FontData behavior, rendering or Folia integration.

Pending owner work includes raw FontData field storage and extension merges,
inferredMrow Scene registration and all remaining Source painting, actual
full-profile package assembly, complete 75-case output comparison, and Folia
integration replacing math QuickJS. The six new action/bbox/braket/cancel/
enclose/color packages remain unregistered candidates. Mermaid remains paused.
