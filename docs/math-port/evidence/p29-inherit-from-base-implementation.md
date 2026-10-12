# P29 base `inheritAttributesFrom` implementation

Implemented only `AbstractMmlNode::inheritAttributesFrom(NodeValue)` in
`src/math/core/mml_tree/mml_node.cpp`, mapping pinned `MmlNode.ts:889–898`.

The body enters the existing `TreeCallGuard`, checks the tagged input when
reading `attributes`, and retains the single virtual attributes handle. It
then performs the source-ordered virtual calls: `get("displaystyle")`,
`get("scriptlevel")`, `isSet("mathsize")`, and conditional
`get("mathsize")`. A fresh attribute bag is created for each call; the
conditional mathsize value is wrapped in a fresh `attributeTuple("math", ...)`.
Only after those reads does it call the source node's virtual
`getProperty("texprimestyle")`. The raw prime value is preserved when truthy;
falsey values become false. Finally, the receiver's unqualified
`setInheritedAttributes` facade dispatches through its virtual inheritance
hook without default normalization in this method.

Null/Undefined source-node tags fail at the source `attributes` read through
the tagged `requireNode` boundary. A present node with a null native attributes
handle raises TypeError at the first `get` operation. No values are cached
between calls. No other production body, helper, declaration, fixture, oracle,
expected value, or build file was changed. I did not run generation, tests, or
builds; Root owns replay, mutation checks, regression tests, and acceptance.
