# Root remaining observer compile acceptance

Root accepts the six Native observer emitter adaptations only. No production
algorithm changed and no runtime or rendering acceptance follows from this.

The frozen original Source prefixes, complete original replays and profiles,
three value tables, all raw rule/matrix tables and arithmetic attribute/value
pools are unchanged. Durable checks are in remaining-observer-source-check.cjs.
The profiles remain mo-order 50 cases/538 assertions; arithmetic 7/13,501 with
1,440 rows; arithmetic-order 22/56; table 135/4,700 with 240 rows and 160 rules;
atom-semantics 74/6,888 with 924 rows; multiscript 226/1,304 with 480 rows.

Root rejected two tag mismatches in the operator/arithmetic emitter: an original
Undefined child had become Null, and an uninitialized result started as Null.
Both now explicitly preserve Missing. Root applied the result correction with
an equal-size in-place write after a mapped-section error blocked the agent's
write; readback matched exactly. The first independent compile also found old
atom observer array indexing; Luna repaired the two emitter expressions, Root
regenerated the fixture, then reran both checks and compilation.

The fresh isolated compile gate builds all six actual fixture translation units
against the current 90 production module interfaces. Its final byte guards pass;
semantic_tests_executed is false. Evidence SHA256:

| Ignored evidence | SHA256 |
|---|---|
| build/math-port-remaining-observer-compile/compile-result.json | 6374f8e620a6fb8a3dd54c4136db25eb5dcbb63bd8eeb52d5c0e88ec0c0f1279 |
| build/math-port-remaining-observer-compile/baseline-build.log | 7aa15ea6bcd9e9cafebf5bb283e4c1ac4b7733a3d4b267ee26fa028fc7390121 |

Root then ran the normal `cmd /c build_test.bat 2>&1`. All repository test
translation units compile and FoliaTests links; the previous six stale fixture
compile failures are closed. The tests actually execute and the command exits
1 with mathematical exceptions/assertion failures, including the six adapted
families and pre-existing unaccepted MML consumers. The Boost.UT exception
summaries also contain unsigned passed-counter underflow; those numbers must
not be treated as passed assertions. The current executable SHA256 is
`55ab62429a4c8dd208a12ffcd8731659fff0dc65d83d289df913ee6155f141e5`.
The normal command's output is in the task tool transcript; no complete saved
core runtime log is claimed here.

Reproduce with `node tools/math-port/remaining-observer-source-check.cjs`,
`node tools/math-port/remaining-observer-compile.cjs`, and the normal core runner.
Missing sparse consumer bodies, original callback/failure semantics, full TeX,
wrappers/layout, SVG/Direct2D and Folia remain required. Historical 54/246 does
not advance from this observer packet.
