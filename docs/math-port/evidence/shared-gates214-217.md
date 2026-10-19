# Gates214–217 focused evaluation

Gate217 compiled all 61 targets in the shared `build/core` tree with at most
four compile jobs. Its 2,181 registered input entries remained unchanged
through the focused evaluation. This is a working-tree result, not an isolated
build of the intervening commits. The focused evaluation finished with exit 1;
it does not replace the last full evaluation at Gate213.

| Focused observation | Gate217 actual result |
| --- | --- |
| Property-copy getter ordering | Exit 0; 7 assertions pass |
| TextMacros preprocessor record projection | Exit 0; 16 assertions pass |
| ParseOptions.clear generic receiver and abrupt completion | Exit 0; 22 assertions pass |
| Complete Host executable | Exit 0; 2,821 assertions pass |
| Attributes constructor ordering | Exit 0; 19 assertions pass |
| Six mfenced null-text tests | Exit 1; 30/36 assertions pass; no crash reproduced |
| Next-six package capture / comparison | Capture exit 0; 18/20 cases match |
| Font-package capture / comparison | Capture exit 0; 13/16 cases match |

The two next-six differences are separate: `textcomp-basic` reports a native
FunctionList result without a Context primitive binding, while
`textcomp-symbols` reports an invalid TextMacros fallback parser. Font-package
text cases also reach that fallback boundary. Successful capture processes
therefore do not establish package parity. The font-package comparator's
process exit alone is insufficient; its comparison result remains unaccepted.

Each of the six mfenced failures records one extra leading empty-text setter
call; the subsequent fence-character setter and node creation arguments match.
The isolated tests complete without reproducing the full Core executable's
Gate213 access violation. That crash location remains unproven.

Gates214 and 215 failed compilation of new test expressions and performed no
native evaluation. Gate216 compiled successfully with 2,180 unchanged entries,
but its two TextMacros compile assertions and complete Host executable failed
because the shared ParseOptions prototype lacked `clear`. Gate217 contains the
Source-ordered generic method and verifies both direct foreign receivers and
the native caller's delegation to the same method. The Gate216 Host failure
must not be reported as a pass.

Provenance is retained under `build/math-port`: `focused-evaluation-g217.json`,
the per-observation stdout/stderr and package capture provenance manifests,
`gate-217-inputs-before.json`, `gate-217-focused-input-changes.json`, and
`next-six-focused-comparison-g217.json`. The build log is
`build/math-font-package-gate-217.log`. Folia's math QuickJS path remains in use;
complete native layout/SVG and lifetime acceptance is still pending.

After Gate217, the working tree receives further FunctionList void-result,
TextNode construction and TextMacros callback changes. Those changes require
a new build and evaluation and are not covered by the results above.
