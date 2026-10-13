# Shared parser/output checkpoint 27

Root ran one existing CMake/Ninja cache on 2026-10-13. The math library,
20 diagnostic targets and ordinary FoliaTests all freshly linked, build exit 0.
All 814 registered/generated input hashes were unchanged during compilation.
Owners were released immediately after linking and continued development;
later edits do not inherit this snapshot's verification.

Gate25 failed before linking because of new OutputJax/binding observer/test
compile errors and a remaining ambiguous ParseUtil diagnostic call. Gate26
fixed those compilation errors and linked all targets with unchanged inputs.
Its ordinary core runner passed 345,517 assertions in 5,291 tests/168 suites.
OutputJax Gate26 matched 13/14 original rows and passed 19/20 focused tests;
the remaining observer read the list wrapper instead of `items`, and the
inherited-setter test had supplied its setter as the getter argument.
Those fixture corrections and constructor metadata fixes were built by Gate27.

## Independently executed Gate27 results

| Check | Result |
| --- | --- |
| OutputJax strict complete-record Source comparison | 14/14 match, exit 0 |
| OutputJax/native indexed-view focused suite | 98 assertions in 21 tests pass, exit 0 |
| BaseMethods exact MML tree and parser-state comparison | 37/37 match, 103 comparisons, exit 0 |
| Ordinary FoliaTests | Exit 0; 345,517 assertions in 5,291 tests, 168 suites |
| TextMacros frontend | Exit 1; actual configuration count 0, prefilter count 0, null inner MML factory |
| Begingroup suite | Exit 1; 33/35 assertions pass, 3/4 tests pass |
| AMS empty-arrow observation | Still fails before returning a result; root contains sparse munderover nodes while observed node-list count is 0 |

Root independently re-executed the pinned original OutputJax oracle as part of
the strict comparator. Six corrupted JSON outputs (hash, omission, order,
base name, extra field, error kind) were rejected; the unmodified real output
passed again. These are comparator sensitivity checks, not compiled production
fault tests or full OutputJax acceptance.

Base's last mismatch was a real coordinate error in HFill: native stored stack
depth, whereas Source stores the current ArrayItem node count. The owner fixed
that producer; no special-case row attributes were injected.

Begingroup's fresh diagnostic identifies the failing observation as
`NodeUtil.getText(inferredMrow)`. An actual original CJS call throws the same
TypeError, `node.getText is not a function`. The former test expectation of
recursive wrapper text was invalid. The owner is correcting the test to verify
the original error and each actual child-token text. That correction is after
Gate27 and uncompiled; no production recursive-getText fix was made.

TextMacros' configuration list being empty is direct executable evidence.
The owner subsequently found that native addPackage appended handlers instead
of registering the package in the canonical configurations priority list.
Source registers first, then the constructor appends once and config registers
filters by iterating that same list. Its correction is after Gate27 and requires
a fresh build. It is not accepted based on source inspection.

## Scope

OutputJax's bounded base lifecycle/filter profile now has a compiled parity
checkpoint. Arbitrary Context constructor invocation/inherited assignment,
async filters, foreign storage/reflection and native MathItem/MathDocument/
DOMAdaptor Context identities remain incomplete. These 14 observations do not
accept CommonOutputJax, LiteAdaptor, wrappers, the shared scene or SVG output.
Folia still uses the existing QuickJS math path. Mermaid remains paused.
