# P28 token-node traversal implementation

Implemented only `AbstractMmlTokenNode::walkTree` in
`src/math/core/mml_tree/mml_branches.cpp`, corresponding to pinned
`MmlNode.ts:1090–1097` (`func(this, data)`, then captured-array `for...of`,
`instanceof AbstractMmlNode` dispatch, and return of the original `data`).

The implementation invokes the receiver visitor before reading `childNodes`,
then captures that array handle. It iterates the captured ChildArray with a live
length check and indexed `get`, so prototype-backed slots are observed and
array growth/deletion/rebinding follows the original for-of receiver rules.
Only a non-null child dynamically castable to `AbstractMmlNode` receives its
virtual `walkTree` call; Text nodes and nullish slots are skipped. The child
return value is ignored, and the input `std::any` is returned unchanged in
value/contained identity. A null child-array handle raises the frozen TypeError
message `object null is not iterable (cannot read property Symbol(Symbol.iterator))`.

`TreeCallGuard<MmlNode>` supplies the documented native recursion budget and
unwinds on visitor, child, or null-array exceptions. The guard is a native host
boundary, not an additional MathJax source condition.

No other source body, helper, declaration, fixture, oracle, expected value, or
build file was changed. I did not run the oracle, tests, or builds; Root owns
the frozen 780-observation comparison, the additional guard assertions,
mutation checks, and acceptance. This report does not claim those gates passed.
