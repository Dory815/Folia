# Gate175: factory fixtures and measured request-owner retention

The shared `build/core` build completes all 58 targets on 2026-10-16 local.
The 2,082 captured inputs remain byte-identical throughout compilation and
the complete evaluation. No additional build tree or copied libraries are used.
The manifest SHA-256 is
`f9263d8e063bae90dd6529e57564a9d427e0c1c7b9ad80ce6628a5bdb101e6b9`.

The integrated increment makes the native direct-call callback mandatory,
provides it in the lifetime fixture, and tests a retained Map adapter instead
of querying a replaced Source descriptor after its provider has been released.
Core now passes both corrected factory fixture files (73 and 52 assertions).
Host, Parser, Regexp and LinkedList suites pass. Core still fails three parent
or callback tests; Foundation still fails its seven previously diagnosed tests.
The Scene/clone corrections and MML factory/Stack candidates are not integrated.

Complete public SVG/error comparison against the pinned independent Source
capture passes 150/150; API passes 9/9 and Common layout passes 75/75. The public
SVG executable SHA-256 is
`9147a818469fc2151f9b0a3542da1565f8bde92b21412441f82ba742fe5545cb`.
The comparison report SHA-256 is
`e77d6014e5b86e023307f94f5f964131d590aa3e3dc4dd4020ce8be1e6934938`.

The paired native routes retain 146 equal complete successful Frozen Scenes
and four equal error pairs. All 150 Scene-only records have zero SVG
materialize and XML serialization calls. The executable SHA-256 is
`ce045aed709fd38750a27c40ec12c755859f618d3e570ff2762e25485830d97e`;
the output SHA-256 is
`71c3785510d230d4ae74a5fe86e753c60dc499631bed3b5abe71ab26a04efb30`.
Route equality does not replace the independent Source SVG comparison.
All 25 package captures pass; 18/20 checkpoint comparisons pass. The next-six
and font groups still expose MML factory and Stack field differences.

The new diagnostic reads actual owning containers while one Renderer performs
warm-up, macro/tag setup and 64 additional renders of `x`:

| Owner count | Warm | After setup | After 32 | After 64 |
|---|---:|---:|---:|---:|
| Context object slots, cumulative | 4,273 | 5,325 | 11,245 | 17,165 |
| MML factory products | 13 | 35 | 163 | 291 |
| Parser canonical registry entries | 173 | 321 | 1,121 | 1,921 |
| Wrapper identities | 12 | 33 | 161 | 289 |
| Distinct retained LayoutJobs | 2 | 7 | 39 | 71 |
| Lite records | 32 | 67 | 259 | 451 |
| Lite node identities | 22 | 57 | 249 | 441 |

Context Symbol slots remain eight and Scene handle count remains zero in this
path. Context slot totals are allocations, not live-object measurements.
The numeric accumulation is observed, not an estimate from process memory.
The diagnostic exits 2 because its combined macro/tag continuity check fails;
which assertion fails and whether its expectations match the exact Source
profile remain under investigation. It must not be reported as a passing
persistent-state or lifetime test. The executable SHA-256 is
`eb6d65f5e4c0cb6440e421bf78027c55123ed42c8a3a674334228a6e71d519c4`;
its preflight records unchanged binary and inputs. No automatic retirement
implementation has been applied, and no boundedness claim is made.

The complete shared evaluation closes with exit 1 and zero changed inputs.
The extra owner-count diagnostic failure is recorded separately above; it is
not included in the existing checkpoint aggregate. Folia still uses math
QuickJS and Mermaid remains paused. Renderer acceptance is incomplete.
