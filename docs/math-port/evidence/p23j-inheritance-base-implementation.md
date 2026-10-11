# P23j-A base inheritance and token-text implementation

This packet implements four existing method bodies from the pinned MathJax
4.1.3 `MmlNode.ts` source. It is implementation evidence, not acceptance. Root
owns the frozen source oracle, builds, mutations, regression checks, and final
acceptance. No source declarations or other method bodies were changed.

Pinned source: `.reference/MathJax-src/ts/core/MmlTree/MmlNode.ts`, SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`.
The emitted CJS used by the frozen profile is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNode.js`,
SHA-256 `86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`.

## Method mapping

| Source method | Native body | Source behavior represented |
|---|---|---|
| `AbstractMmlNode.setInheritedAttributes` (TS 765–834; CJS 416–486) | `mml_node.cpp`, `AbstractMmlNode::inheritAttributes` | Runs the frozen Missing-only argument defaults; captures defaults once and snapshots own input keys; checks own default/always-inherit membership, unpacks the current tuple, applies no-inherit filtering and writes through the current Attributes handle; clones/deletes the inherited record only at a stop-inherit key; distinguishes Missing from Null for explicit displaystyle/scriptlevel; writes raw truthy prime; normalizes finite arity with sparse slicing or virtual append/factory calls; applies linebreak indent additions; ends in virtual child-hook dispatch. |
| `AbstractMmlNode.setChildInheritedAttributes` (TS 846–854; CJS 488–510) | `mml_node.cpp`, `AbstractMmlNode::setChildInheritedAttributes` | Captures the child-array handle at iterator creation, observes its live size and indexed Get values, and dispatches every slot without filtering. An Undefined/Null slot fails at the `setInheritedAttributes` member call. |
| `AbstractMmlTokenNode.getText` (TS 1055–1067; CJS 712–733) | `mml_branches.cpp`, `AbstractMmlTokenNode::getText` | Captures one child-array handle, iterates live indexed slots, calls virtual TextNode `getText`, otherwise checks the optional `TextContentBinding` and invokes a present callback in order. Null/Undefined fail at the original `in` operation; an absent binding is skipped and a present non-callable binding throws. Results concatenate as UTF-16. |
| `AbstractMmlTokenNode.setChildInheritedAttributes` (TS 1072–1085; CJS 735–751) | `mml_branches.cpp`, `AbstractMmlTokenNode::setChildInheritedAttributes` | Uses the same captured-array/live-slot traversal but dispatches only values that dynamically identify as `AbstractMmlNode`; TextNode, XMLNode, other empty nodes, and nullish slots are skipped. |

The base entry points retain `TreeCallGuard<MmlNode>`. Attribute-list key
enumeration, shallow Object.assign-style copies, tuple unwrapping, own-key
membership, inherited attribute storage, sparse slice, and virtual factory/tree
operations use the already frozen native services. The stop-inherit operation
rebinds to a fresh shallow record before deleting the key, leaving the caller's
record handle unchanged. Arity `1` is treated as a minimum of one child; other
finite nonnegative arities use the source's exact-length predicate. Growth checks
the current array after each virtual append so callbacks may alter later loop
iterations.

## Native representation boundary

RTTI provides the source `instanceof` checks for TextNode and AbstractMmlNode.
The optional `TextContentBinding` is the accepted typed host for a non-TextNode
`textContent` property: a disengaged optional means absent, an engaged empty
function means present but non-callable, and an engaged callback executes at
the source traversal point. This does not model arbitrary JavaScript property
descriptors or accessors. The callback result is typed UTF-16 text; arbitrary
non-string JS coercion remains outside this binding.

The child loop retains the ChildArray handle acquired at loop start; it does not
retarget if `childNodes` is rebound during a callback. Array length and slots
remain live on that captured handle, including sparse/prototype Get semantics.
The implementation has not been built or tested by this packet, and no
acceptance or full MathJax/MML/Folia equivalence is claimed here.
