# Canonical reduce, live textContent and XML ownership: Gates56–58

Gate56 exits 1 with all 1,050 input hashes unchanged. The Common XML wrapper
enters the actual compile closure, and CommonOutputJax owns the single XML
bridge over its retained DOM adaptor. SVG inherits that bridge. Remaining
errors are UTF-16 Property conversion in the new wrapper and an ambiguous
factory overload in Root's new test; both are corrected after release.

Gate57 enrolls the canonical Array.prototype.reduce and Context-backed Lite
textContent body, then exits 1 with all 1,055 hashes unchanged. Missing
namespace qualification and an incorrect erase argument affect the reduce
packet. Root's UTF-16 assertion also instantiates Boost.UT's unsupported
char16_t printer. The fixes retain the actual algorithms/assertions and use
the repository's module-backed Boost.UT runner.

Gate58 builds all 13 targets, exit 0. Actual build session 31182 terminates
successfully; all 1,055 captured inputs remain unchanged through termination.
Root executes the freshly linked Foundation runner, PID 67288, from 10:50:16
through 10:50:17 UTC on 2026-10-13. Its actual exit is 0 and executable SHA256 is
`f9dedbd5893324369fce6a447627765bc6663de6b869f0ae12586792d7f1e728`.

| Focused Foundation packet | Assertions | Tests | Result |
| --- | ---: | ---: | --- |
| Context Map | 69 | 10 | Pass |
| Lite Context projection and retained-module lifetime | 28 | 4 | Pass |
| Immutable Scene and SVG precision/order | 16 | 2 | Pass |
| Canonical reduce and dynamic textContent | 29 | 7 | Pass |
| Shared XML DOM and real provider lifetime | 19 | 2 | Pass |
| Total | 161 | 25 | Pass |

The reduce method is installed by the existing Context Array intrinsic
initializer. There is no optional reduce service or duplicate array storage.
The Context Lite method reads the real children/reduce/kind/value properties
and recursive receiver method. Tests cover live indices, captured length,
inherited initial accumulator, callback validation order, replacement reduce,
recursive lookup and a text receiver with assigned children.

The XML tests use actual LiteModule/adaptor construction and the real MmlFactory
XMLNode. They verify canonical node identity through Context projection,
cross-adaptor mutation, independent cloning in the same store, serialization
after external bridge handles are released, and eventual bridge destruction.
They do not accept Common XML layout or SVG XML painting.

Fresh Host and Parser runners also exit 0: Host has 458 assertions/196 tests;
Parser has 864 assertions/56 tests. Focused core queries `*array*` and `*font*`
exit 0 with 17,067 and 22 assertions, respectively. The forked runner reports
registered test counts even for filtered-out tests, so those counts are not
reported as executed coverage. Full core's last complete execution remains
Gate49, not Gate58.

All five fresh parser corpora match 150/150 complete original records: Base37,
Frontend37, Bold/ConfigMacros16, AMS24 and AMS Boundary36. Ignored records are
`build/math-port/focused-diagnostic-exits-58-root.json` and the parser
checkpoint outputs for `gate58-root`.

Root re-executes the original Array reduce and Lite textContent witnesses.
Their output hashes are, respectively,
`3e5136d5418980c03b76bcf8db2980e45e514e020ef9d8014ecc327c3fe9d4a7`
and `5b5c09b23bc37a8e895433cabc73903c4cae757b653b24aa12f056fe64f99597`.
These are focused witnesses, not the complete mathematical output corpus.

Complete dynamic textContent coercion is still open: its local string helper
must be replaced by shared primitive/binary-plus semantics. In particular, a
custom reduce callback can supply a numeric accumulator, which Source `+`
preserves as numeric when both operands are numeric. FontData live fields and
canonical sparse arrays, the real output bootstrap, remaining wrapper kinds
and painting, all 75 profile output cases and Folia integration remain required.
The passing Foundation result is bounded substrate evidence only.
