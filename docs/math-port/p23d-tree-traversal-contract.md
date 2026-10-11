# Root P23d sparse generic traversal contract

This is a continuation of P23c, not whole Node/MML acceptance. Root migrated
only the emitted C++ observer binding in tree-oracle.cjs: original Node/Wrapper
JS computations and their 15 cases/140 assertions are unchanged. The two
native resource tests retain ten assertions. Three native entry-point smoke
assertions explicitly exposed Unimplemented in the real red run. The closed
C++23 graph compiled; it exited 1 through failed tests (18 tests, 99 executed
assertions while stubs terminate some bodies). A compile failure was rejected
as evidence, and exception::what() alone was insufficient to identify stubs;
the added entry-point probes catch and print the actual exception name.

## Fixed interfaces and responsibilities

Only AbstractNode::walkTree/toString in core/tree/node.ixx and
AbstractWrapper::walkTree in core/tree/wrapper.ixx are authorized. No signatures,
parent/child storage, ownership, Empty behavior, other six methods, factories,
tests, tools, manifests, or old/dense compatibility paths may change.

MathJax 4.1.3 pin fb987178c2d279a99b0db5ea02e751691703955e is authoritative.
Node.ts walkTree lines 334–342 invokes self first, then captures current array
and iterates live length/index Get. Falsey U/N/hole Get is skipped; virtual
walkTree dispatch remains intact. A null captured handle throws after self visit.
Wrapper.ts preserves absent childNodes versus engaged null and present empty;
self callback may assign/rebind these before the presence test and capture.
Present array iteration likewise uses live length, exact Get, falsey skip.

Node.ts toString evaluates kind before childNodes/join. Capture the current
array after kind, then the original length once for Array.join. Get each index
live; missing/Undefined/Null converts to empty string, Node invokes virtual
toString. Commas still represent every original slot after shrink/deletion.
TreeJoinGuard suppresses joining an already-active array identity, including
mutual shared child arrays, and unwinds on exception. Do not replace this with
a visited-node set or short-circuit calling a child's toString.

Existing TreeCallGuard applies to recursive generic calls and Wrapper walk.
It must allow 512 nested calls and reject 513, unwind on exception and allow
later unrelated calls. It is a native stack adaptation, not Source syntax limit.

## Acceptance held by Root

Root owns builds, all expected values and mutations. Real red evidence is
build/math-port-sparse-tree-red/baseline-{build,test}.log plus red-result.json.
After delivery Root must review source-order, run unchanged golds (153 asserts
in 18 tests), add sparse traversal original observations and semantic mutants,
then migrate the MML/NodeUtil consumers before any full source-file/Core claim.
This packet does not authorize Folia integration.
