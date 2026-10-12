# Root token walk acceptance

Accepted exactly AbstractMmlTokenNode::walkTree, using the pre-implementation
P28 contract, original observations and actual frozen stub baseline. Luna
changed only this body and its implementation report; Root reviewed the diff
and independently reran the original oracle and native gates.

All 780 original complete records pass, including 224 original errors. Source
JSON SHA256 is unchanged:
646bba252a86677fdab47a598d1cd75a828ad9c3e2f275b741898b6a238c6e71.
The four separate Native depth/data-identity/recovery assertions also pass:
784 assertions in two tests against the actual 90-module graph.

Root detects eleven compiling faults: visit-only, capture before visiting,
fixed initial length, re-read rebound array, visit Text nodes, return child data,
skip inherited slots, skip receiver visit, swallow null-array error, skip every
other slot, and omit the shared depth guard. Each fails tests with exit 1;
working implementation/fixture bytes remain unchanged during the isolated run.
The existing inheritance regression gate passes 1,765 assertions in 17 tests
and its effective prime-type fault. The normal repository runner filtered with
`cmd /c build_test.bat "*math_port_token_walk*" 2>&1` rebuilds/links and passes
the same nonzero 784 assertions in two tests; other families are skipped.

Ignored evidence SHA256:

| Evidence | SHA256 |
|---|---|
| build/math-port-mml-token-walk/results.json | 7c7bdc6986942d8007d01ff4feb1d2da24c59eead033454c4bd74345642236d2 |
| build/p28-token-walk-core.log | 25866d39f569614389d0e8f87b62cb88e143cdd1faa7e33978216984f7fa0021 |

The previous full core runtime remains failed on unaccepted MML consumers.
No whole-file, full TeX/parser/layout, SVG/Direct2D or Folia gate is accepted
by this single method. Historical 54/246 is unchanged. The read-only companion
verification consumer audit is a frontier description, not an implementation
contract; Root rejected and corrected its initial Boolean interpretation of
prime || false before further use.
