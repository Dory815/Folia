# P29 empty-node inheritance implementation

`AbstractMmlEmptyNode::inheritAttributesFrom(NodeValue)` in `src/math/core/mml_tree/mml_empty.cpp` now has an empty body, matching the pinned upstream override. MathJax 4.1.3 `MmlNode.ts` (SHA-256 `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`) defines `AbstractMmlEmptyNode.inheritAttributesFrom(_node: MmlNode) {}` at lines 1410–1410; the corresponding CJS file (SHA-256 `86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`) emits the unconditional no-op at lines 1000–1002.

The native override does not inspect the tagged argument, receiver attributes/properties, or invoke inherited-attribute setters. It adds no guard, validation, or exception path; nullish and otherwise invalid observer inputs are ignored just as upstream specifies. This is only the empty-node override; the base `AbstractMmlNode::inheritAttributesFrom` implementation remains a separate method and responsibility.

No build or tests were run, per packet instructions. Root owns parity and acceptance.
