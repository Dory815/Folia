# P31 `AbstractMmlNode::replaceChild` implementation

Implemented only `AbstractMmlNode::replaceChild` in
`src/math/core/mml_tree/mml_node.cpp`. The pinned MathJax 4.1.3 source is
`.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts`, lines 639–644, SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`.

The method enters the existing `TreeCallGuard`, reads virtual `arity()` once,
and branches on the raw numeric comparison `arity < 0`. In that branch it
captures the current `childNodes` handle, reports the source null-array read
error, performs tagged `get(0)` (including prototype lookup), requires the
result to be a node for the `replaceChild` property access, and invokes that
node's virtual `replaceChild(newChild, oldChild)` with both original tagged
arguments. It ignores the delegated return and returns the original
`newChild` value.

For all other arities—including zero, positive values, and `NaN`—the method
uses qualified `Generic::replaceChild(newChild, oldChild)`. This retains the
already accepted generic implementation's virtual `childIndex` dispatch,
post-callback current-array lookup, replacement-before-parent-validation
ordering, conditional old-parent detach, partial effects on exceptions, and
return identity.

No other body or file was changed. No build or test was run; root owns the
independent verification and acceptance of this method.
