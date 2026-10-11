# P23n MathChoice observer binding update

The native observer emitter now uses `tree::ChildArray<MmlNode>` with tagged
`m::NodeValue` entries for child construction and rebinding. The `Parent`
observer override accepts tagged replacement arguments and returns a tagged
`NodeValue`; its explicit null result remains tagged Null. Native parent changes
and reads use `setParent` and `getParent`. The generated factory's `create`
interface continues to return a raw `MmlNode*`.

The observer's array projections use indexed `size`/`has`/`get` operations.
The `ids` projection preserves holes and maps present Null/Undefined child tags
to the same null-like identifier output as the original JS helper. The real
tree projection visits present slots and explicitly requires a node before
recursion. Child-array append calls remain pushes. The raw `notParent()` value
continues to be recorded as a `Property`, without boolean conversion. Source
JavaScript stimuli, overrides, calls, expected values and XML/property-array
services were left unchanged.

The authorized oracle run regenerated `tests/math/reference_mathchoice.cpp`
and reported 410 cases and 1,081 source-value records, matching the frozen
profile. The frozen profile contains 412 assertions and 409 Source call
observations. No native build or test was run; this is an observer binding
repair, not semantic acceptance of MathChoice or its remaining consumers.
