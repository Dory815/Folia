# P32 base verification methods

Implemented only `AbstractMmlNode::verifyTree` and
`AbstractMmlNode::verifyChildren` in `src/math/core/mml_tree/mml_node.cpp`.
The source is pinned MathJax 4.1.3
`.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts`, SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`;
the corresponding pinned CJS SHA-256 is
`86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`.

`verifyTree` uses its source-local default: only Missing becomes a local Null,
and exact Null returns before callbacks. It calls virtual `verifyAttributes`
with the retained options value, reads virtual `arity()` once, then reads
`checkArity` through the property host. The wrong-count branch preserves the
Source comparisons: nonnegative arity excluding positive Infinity; arity one
requires at least one child, while every other checked arity requires exact
length equality. The current child-array handle is read only when the matching
length expression is reached. A mismatch calls virtual `mError` with the
original message/options and `short=true`; its result is ignored, and virtual
`verifyChildren` still follows if the callback returns normally.

`verifyChildren` captures the current children handle once and iterates its
live length. Each step performs tagged indexed Get, including prototype values
and holes, then invokes the resulting node's virtual `verifyTree` with the
unchanged options. Nullish slots fail at the `verifyTree` property access;
child-array replacement during a callback does not retarget the captured
array. Both methods use the existing `TreeCallGuard`.

No other implementation body, declaration, fixture, or oracle was changed.
No build or test was run; root owns independent verification and acceptance.
