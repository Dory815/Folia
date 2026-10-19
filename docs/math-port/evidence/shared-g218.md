# Gate218 focused evaluation

All 61 targets compiled in `build/core` with at most four concurrent compile
jobs. The 2,183 registered input entries remained unchanged through the
focused evaluation. Every invoked test/capture process exited 0, with stable
executable hashes. Package comparisons were run separately and still fail;
the focused driver exit 0 is not a complete acceptance result.

| Observation | Actual result |
| --- | --- |
| Native Promise / ParserRuntime queue ownership | 25 assertions pass |
| Pinned FunctionList reference tests | 432 assertions pass |
| FunctionList Context tests, including void result | 80 assertions pass |
| Maction passive host / Scene metadata and lifetime | 100 assertions pass |
| Property-copy ordering | 7 assertions pass |
| TextMacros callbacks and preprocessor | 55 assertions pass |
| ParseOptions.clear | 22 assertions pass |
| Complete Host executable | 2,821 assertions pass |
| Six mfenced null-text tests | All 36 assertions pass |
| Attributes constructor ordering | 19 assertions pass |
| Next-six package comparison | 18/20 match; not accepted |
| Font-package comparison | 13/16 match; not accepted |

The TextNode constructor now initializes through its base field-assignment
path without invoking a derived `setText` override. The six previously failing
mfenced call sequences pass without weakening their expected observations.
This does not locate or resolve the earlier full Core crash.

TextMacros fallback now performs one dynamic Source property/call algorithm
for compatible records and canonical native identities. It preserves method
receivers, ignores the delegated parse return value, and uses the existing
Abstract Addition operation for character accumulation and macro names.
FunctionList projects native void/monostate results to Source undefined while
preserving false short-circuit behavior.

Maction's host stores callbacks in one Context-attached owner, records only
event-name metadata in frozen Scenes, and checks provider lifetime before
accessing the Lite module. Focused tests include request retirement, callback
reachability, own-method precedence, scope restoration, and metadata surviving
Renderer destruction. Exact native listener positions and SVG byte parity
require the subsequent profile observation; these tests alone do not prove
them.

The next-six `textcomp-basic` difference is now the missing `mathItem` key in
the observed packageData jax's `_parseOptions`. Further observation also shows
uncompiled values (`root: null`, `texParser: null`) there, so adding a synthetic
key is not a justified fix. `textcomp-symbols` now reaches a different error,
`TypeError: Invalid ParseMethod receiver`. Font-package text cases retain
packageData differences. Both comparison counts remain unaccepted.

Artifacts are `build/math-font-package-gate-218.log` and, under
`build/math-port`, `focused-evaluation-g218.json`, the per-step logs, package
capture/provenance files, `next-six-focused-comparison-g218.json`, and the
registered input audit files. All results are from the shared working tree;
the complete Core, layout, SVG, and request-lifetime gates were not rerun by
this focused driver. Folia still uses its math QuickJS path.
