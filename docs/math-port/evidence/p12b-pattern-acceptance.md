# P12b root acceptance — five default Mo pattern services

Status: locally accepted native dependency service. MmlMo remains unfinished;
this does not accept its whole source file, any renderer gate or production use.

## Root control and source correspondence

Commit 184a1bb freezes interfaces, source data, initial actual-CJS observations
and exact file ownership before agent dispatch. Root owns mo-pattern-contracts,
MoPatternSpec, immutable generated data, source locks, expected values, build,
mutation copies and acceptance. One agent implements only the two frozen
methods; two other agents write disjoint read-only review reports. None can
modify the contract, data, expectations, scope or production.

The source is MathJax 4.1.3, mo.ts SHA-256
2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605;
resolved CJS hash is
7c68f0570d82f585eb6ca7e8bbca119b96a02666e0d4b268a413bf4035ff0b2c.
mo-pattern-reference.cjs checks clone/package TS equality, package/Node
versions, exact resolved module identity and the entire 424-file compiled
package tree hash in p12b-source-lock.json. It loads the pristine actual MmlMo
class, verifies absent flags and the anchored class/quantifier expression
structure, and derives sorted inclusive UTF-16 intervals from its actual regex.

| Source field | Intervals | Length | Native correspondence |
| --- | --- | --- | --- |
| pseudoScripts | 13 | one or more | spec 0 plus generic complete-unit matching |
| primes | 3 | one or more | spec 1 plus generic complete-unit matching |
| opPattern | 2 | two or more | spec 2 plus generic complete-unit matching |
| mathaccents | 18 | exactly one | spec 3 plus generic complete-unit matching |
| mathaccentsWithWidth | 4 | exactly one | spec 4 plus generic complete-unit matching |

Source strings, flags, intervals and repetition bounds remain distinct metadata.
The C++ implementation checks size bounds and every char16_t unit. It neither
trims/folds input, decodes Unicode scalars, relaxes anchors, interprets source
text nor substitutes heuristic categories. makeMoPattern validates before
returning an independent closure capturing only the enum value. Invalid native
enums use the shared typed TypeError service rather than native undefined access.
Root reviewed the entire two-method implementation and generated data.

## Independent verification

Final oracle: 1,136 assertions/16 cases, 2,342,369 actual String.match(RegExp)
queries, and thirty complete 65,536-unit-domain masks. Masks cover singleton,
double/triple repeats and allowed prefixes/suffixes/middle placement. Ordered
allowed-pair products, mixed text, all line terminators, NUL, ASCII/non-ASCII
letters, surrogate pairs/lone surrogates, long strings and callback lifetime
have independent source observations. Regex metadata, callback creation,
independence and invalid native enums have separate checks. Expectations never
come from C++ output.

The read-only fixture review identified absent direct range-metadata assertions;
root added them and multiletter whitespace witnesses in b6c6180. The reports
retain the initial 971-assertion snapshot as historical review evidence. The
final three --check commands reproduce the extended fixture and data exactly.

The complete `cmd /c build_test.bat 2>&1` suite passed, including all 1,136
pattern assertions, the existing MML/leaf/script/operator differential suites
and editor/core tests. Log: build/p12b-core.log. No platform or application
behavior changed; no partial factory was registered.

All 22 isolated mutants compiled and exited 1 through assertion failures:
missing/broken length bounds, excluded interval endpoints, intersection instead
of union, first/last-only checks, accepted invalid middle units, byte truncation,
relaxed leading/final LF, NUL truncation, wrong kind, deferred creation
validation, wrong/inverted callbacks, repaired invalid enums and altered
source data. No compiler error/crash counts as detection. Working source and
fixture copies remained byte-equal. Results:
build/math-port-mo-pattern-mutations/results.json; log:
build/p12b-pattern-mutations.log.

## Scope and remaining work

This is a compiled binding of the five default no-flags UTF-16 languages,
not general JS RegExp construction, descriptors, mutable globals, flags,
lastIndex or evaluation. Later MmlMo statics must retain actual-constructor
callback lookup and replacement through a separately frozen typed service.

Five of Mo's 32 source members now have reviewed prerequisite mappings; the
remaining 27 members have no acceptance. Complete Mo static aliases/defaults,
getter side effects, class/form adjustment, parent/accent traversal and prime
remapping remain. Full MmlFactory, TeX, fonts, common layout, shared scenes,
SVG, Direct2D integration and Folia/QuickJS replacement also remain. Source
inventory acceptance stays 25/246 and full G0–G7 stay closed.
