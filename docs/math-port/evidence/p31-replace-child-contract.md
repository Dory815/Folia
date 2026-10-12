# P31 Root replaceChild contract

This packet contains only `AbstractMmlNode::replaceChild` from pinned MathJax
4.1.3 `core/MmlTree/MmlNode.ts`, lines 639–644, SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`.
Existing generic Tree replacement and tagged child array/value services retain
their separate responsibilities. No verification/options types or methods,
parser, output backend, registry, or Folia changes belong to this packet.

Root freezes the actual original records and C++ observer before implementation,
then compiles/runs the real existing stub to establish an assertion-failing red
gate. Luna receives write access only to this one method body in
`src/math/core/mml_tree/mml_node.cpp`, plus a bounded implementation note.
No expected records, emitter, declarations, helpers or other bodies may change.

The method reads virtual arity once. If negative it performs indexed Get on the
current child array at zero (including prototype lookup), invokes that child's
virtual replacement with unchanged new/old tagged arguments, ignores the child
result, and returns the original new argument. Otherwise it calls qualified
generic Tree replacement. That existing method calls virtual childIndex and
observes the current array after that callback, writes the replacement before
new-parent validation, then reads/detaches the old parent only if it still equals
this receiver. Callback exceptions and prior mutations remain observable.

The independent original observer covers negative infinity/negative/zero/
positive/infinite/NaN arities, empty/sparse/prototype/null arrays, nullish/new/old
node identities, duplicates, arity/index/child callbacks that throw or rebind
arrays, ignored child results, and parent setter failures/parent read failures.
It compares returned tagged identity, error name/message, every array's holes
and indexed values, all retained parents and ordered callback traces. The Native
observer uses the same single Children/NodeValue coordinates and fork Boost.UT
module. It never derives expected results from Native behavior.

Root independently reviews the body, reruns unchanged Source records, requires
the green gate and effective compiling mutation detection for wrong branching,
wrong dispatch, argument order, return identity, eager reads and omitted partial
effects, and runs the narrow normal core test. This accepts one method only;
complete math module and Folia acceptance remain open. P30's options-domain audit
remains required before implementing verification over a Property-based domain.

Root froze 1,504 actual original records, including 542 original exceptions.
Source JSON SHA-256 is
`b5713fb6a6b66477f07d8b93cd7ec1fb04ed22a15bddf4091a934a1daa8ac1b3`.
The isolated C++23 gate compiled all 90 math interfaces and executed the existing
real replacement stub; the test exited 1 through assertion failures, with no
crash. The runner's final source/fixture guard passed. Logs and red result are
under ignored `build/math-port-mml-replace-child/`. Only after this evidence is
Luna authorized to implement the one frozen method body.
