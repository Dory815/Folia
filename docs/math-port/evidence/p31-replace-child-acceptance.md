# P31 bounded replacement acceptance

Root independently reviewed and accepts only the restored
`AbstractMmlNode::replaceChild` body. Its Source mapping and limits are in
`p31-replace-child-contract.md`; only that production method changed.

The unchanged pinned original oracle reproduces 1,504 complete records,
including 542 Source exceptions. JSON SHA-256 remains
`b5713fb6a6b66477f07d8b93cd7ec1fb04ed22a15bddf4091a934a1daa8ac1b3`.
Root's real pre-delegation stub gate compiled and failed 1,504 assertions without
crashing. After implementation, the isolated C++23 gate passes 1,508 assertions
in two tests: the same 1,504 Source records plus four separate Native call-budget
and unwind/recovery assertions. Source expected records never changed.

All thirteen independent compiling mutants exit through assertion failures:
no-op, zero/NaN entering the inferred branch, wrong first index, lost prototype
lookup, swapped arguments, returned delegated result, bypassed child virtual,
premature new-child validation, repeated arity, array captured before arity,
discarded generic result, and missing Native call-budget protection. The first
version of the virtual-bypass mutant did not compile because its receiver was
typed MmlNode; Root corrected that mutant's explicit cast. That compilation
failure was not counted as detection. The final complete rerun compiles and
detects all thirteen, with final production/fixture guards passing.

Normal `cmd /c build_test.bat "*math_port_replace_child*" 2>&1` compiles/links
the repository core and passes the same 1,508 assertions in two executed tests.
Other registered tests are filtered/skipped, not counted as passed assertions.
This is not a full core runtime pass; the known full mathematical suite still
has unaccepted methods and failures.

Ignored evidence SHA-256:

| Evidence | SHA-256 |
| --- | --- |
| `build/math-port-mml-replace-child/red-result.json` | `e271b92d888ce62449ecaef6f9cf2e867cd8388cf8b31fce1e6f7178f9299550` |
| `build/math-port-mml-replace-child/results.json` | `22f46fafb0651f7b019fbf4f77939ed3e52d6506acadf90a669667031e89863f` |
| `build/p31-replace-child-core.log` | `24dba6d5ecbe016d3b33914e4530ee369dccff7b94a8dfc3b6c5d78a66f8bd41` |

TeX parser stack, remaining MML algorithms, shared layout, SVG/Direct2D output
and Folia remain open. Historical whole-file tally 54/246 does not advance from
this single restored method. P30 Root's options coordinate contract must precede
verification method implementation.
