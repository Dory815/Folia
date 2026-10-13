# Shared native checkpoint: Gates 34 and 35

Gate34 completed with exit 0. All 814 frozen build inputs retained their hashes.
The combined target build includes the core runner and 20 math diagnostics.
The ordinary core runner completed with exit 0: 345,517 assertions, 5,291 tests,
168 suites. These results apply to the frozen Gate34 worktree, not later edits.

Root's fresh executable observations compare every emitted record with the
independently frozen original corpora:

| Corpus | Whole records matched |
| --- | --- |
| Base | 37/37 |
| Frontend | 37/37 |
| Boldsymbol/Configmacros | 16/16 |
| AMS/Newcommand | 24/24 |
| AMS boundary | 36/36 |

The actual Equation method now follows Source's omitted/undefined display=true
default and retains explicitly supplied false/null values. This fixes the three
remaining tag-reference/display observations without input-specific exceptions.
Boldsymbol/Configmacros tests complete with exit 0, 69 assertions in seven tests.

ParserTests completes with exit 1. Two tests still throw an options Exception:
`base_item_source_class_reflection_uses_live_context_identities` and
`null_item_constructor_call_initializes_the_supplied_receiver`. The old capture
does not identify the failing operation. An expanded diagnostic is pending a
fresh link. Full class/constructor acceptance is therefore still open.

Ignored logs and provenance are `build/math-font-package-gate-34.log`,
`build/math-core-gate-34.log`, `build/math-port/core-run-state.json`,
`build/math-port/parser-checkpoint-34-root-fresh/summary.json` and
`build/math-port/diagnostic-exits-34-root.json`. The checkpoint summary records
the executable and original corpus hashes and actual exit codes.

Gate35 registers RuntimeModules, LinkedList, MathList, MathLocation and two
dedicated LinkedList diagnostics. It exits 1 with all 823 frozen hashes unchanged.
MathList calls a missing property_host comparison function, and the native dump
passes the wrong Property null type to MathItem. Those two compile failures block
the library link; no old executable is attributed to Gate35. Repairs have been
returned to the owner and require a fresh build and original-record comparison.

The native indexed-array descriptor regression tests likewise await that fresh
link. Unregistered CoreDOM/Lite/Common files do not inherit this acceptance.
There is no completed native SVG renderer or Folia integration at either gate.

## Gate36 repair validation

Gate36 fixes the two actual interface errors and completes the full target build
with exit 0. All 823 frozen input hashes remain unchanged. The fresh five parser
corpora again match all 150 original records. LinkedList's complete 13-row JSON
matches the original, and its lifetime/module tests pass 27 assertions/seven
tests. Bold tests pass 69 assertions/seven tests and Begingroup passes 43/four.
The focused native indexed-array provider/descriptor tests pass 17 assertions
in four tests, with actual exit 0.

ParserTests still exits 1 with the same two failures; the newly linked diagnostic
identifies their actual phases: global-field/factory identity fails with
`Unimplemented: StackItem Property value has no canonical Context identity`,
and NullItem bound-field initialization fails with
`TypeError: Native data field already has an own property`. The owner is fixing
the actual projection and supplied-receiver initialization boundaries.

The ordinary Gate36 core run completes with actual exit 0: 345,534 assertions,
5,295 tests and 169 suites. This includes the four new indexed-array tests.
Logs/provenance are `build/math-core-gate-36.log`,
`build/math-font-package-gate-36.log`,
`build/math-port/parser-checkpoint-36-root-fresh/summary.json`,
`build/math-port/diagnostic-exits-36-root.json` and
`build/math-port/native-indexed-36-root.log`.
