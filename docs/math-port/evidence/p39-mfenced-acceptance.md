# P39 tagged mfenced acceptance

Root accepts four restored methods: setTeXclass, child inheritance, addFakeNodes
and fakeNode. Ownership follows p39-mfenced-contract.md. Algorithms remain in
mfenced.cpp; Root owns tagged indexing/text/string primitives. Fake separators
retain their own fields and do not become real children or output geometry.

Real-stub red evidence builds all 90 modules, exits 1 through Unimplemented and
passes byte guards (9c1f810). Root review corrects the actual getList receiver/
error boundary and captures all three destructured raw values before ordered
string replacements. Nullable text-setter results and mo class/parent writes
retain tagged failures and Source partial effects. No original gold changes.

The unchanged original mfenced oracle reproduces 225 cases/984 assertions/509
records. Root supplements original methods before Native comparison:

| Boundary | Records | JSON SHA-256 |
| --- | ---: | --- |
| concat null/sparse/prototype snapshot, field rebind and nullable text factory/setter | 11 | 17f83db2ba652f0f5f204db407c91b7071c3616e2701af279fa3e380991810a6 |
| exact delimiter string-method failures | 9 | bba7034b0e430c23a53ee3a23e041b9a36fc2eaa2eddf8037d30d6f6105faf91 |
| loop length captured before later child grow/shrink/rebind/null callbacks | 4 | 59c43d7a3a0c9a4720af97682fb4bddcb5785b78d92e9580c97a02479fd8bc2c |

All --check runs reproduce original results. The class-length corpus follows a
surviving wrong implementation: the old growth witness mutated slot 0 before
loop length was captured and could not distinguish later length rereads. The
new original witnesses exercise mutation from slot 1; the fault is now detected.
The separate Native entry fixture contributes four assertions, not Source records.

Final tools/math-port/mml-mfenced-gate.cjs baseline passes 1012 assertions in 229
executed tests. All 21 independent semantic faults compile and then fail with
assertion exit 1. Source/fixture byte guards pass. Coverage includes class chaining,
captured loop length, live slots, fallback update, snapshot versus current fields,
fake/base inheritance order, exact whitespace and UTF-16 padding, null-slot index
consumption, captured push receiver, argument defaults, nullable setter results,
class/parent writes and exact string failures.

Normal `cmd /c build_test.bat` builds and passes the complete core test suite,
exit 0, in ignored build/p39-full-core.log. Its initial run exposed one remaining
Native observer binding: inheritance-boundary-oracle.cjs and its emitted fixture
passed an old vector through std::any, which compiled but the current factory
correctly rejected at runtime. Root changes only that carrier to ChildArray;
the original 350 values remain byte-exact under --check and its filtered 249
tests pass. No production compatibility path or dense-array fallback was added.
The complete core suite passes after that repair. Build products remain ignored.

No explicit MML Unimplemented stubs remain in the current source tree. This is
not whole-source reflection parity or complete TeX-to-SVG acceptance. Complete
NodeUtil/TexError, configuration/stack/parser/packages, fonts/common wrappers,
shared scene/SVG and Folia native integration remain open. Mermaid stays paused.
