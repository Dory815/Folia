# P23f MML constructor and child-policy implementation evidence

This packet implements only the constructor/child-policy members assigned in
the frozen P23f contract: `child_at`, `AbstractMmlNode::initializeMml`,
`AbstractMmlNode::setChildren`, and `AbstractMmlNode::appendChild` in
`src/math/core/mml_tree/mml_node.cpp`. It is not acceptance of the MML family,
the whole source file, TeX, layout, SVG, or Folia integration.

## Pinned source mapping

| Native implementation | MathJax 4.1.3 source | Mapping |
|---|---|---|
| `mml_node.cpp:42–46 child_at` | `MmlNode.ts:596–598, 608–610`; pinned CJS `MmlNode.js:299–300, 307–308` | Read child zero as a tagged `NodeValue` and require a node only at the source member dereference. Null array handles fail at index `0`; holes/Undefined/Null fail at the requested `setChildren` or `appendChild` member access. |
| `mml_node.cpp:121–186 initializeMml` | `MmlNode.ts:443–454`; pinned CJS `MmlNode.js:133–145` | Initialize `prevClass`, `prevLevel`, and backing `texclass` to Null before child callbacks. Create/attach an inferred row for negative arity, always call virtual `setChildren`, then resolve live-kind defaults followed by `math` defaults, install Attributes, and apply the supported input properties. The installed array's tagged child-zero value uses a write-receiver check for `parent`, preserving a TypeError naming `setting 'parent'` if construction ever yields a nullish slot. Null/Undefined property input becomes an empty Object.assign result; explicit null children remain a null handle and reach the child iteration error. |
| `mml_node.cpp:228–234 setChildren` | `MmlNode.ts:595–600`; pinned CJS `MmlNode.js:298–301` | For negative arity, dereference child zero and call its virtual `setChildren`; otherwise qualify the generic superclass call, which owns sparse for-of semantics. |
| `mml_node.cpp:236–266 appendChild` | `MmlNode.ts:607–633`; pinned CJS `MmlNode.js:302–336` | For negative arity, delegate to child zero and return the original tagged argument. Otherwise dereference `isInferred` before mutation. Infinite-arity inferred rows flatten the captured original ChildArray through its captured-length/live-Get `forEach`, invoking qualified generic append for each present slot and returning the original child. Finite-arity inferred rows become factory-created `mrow`s, receive the original child array and Attributes handle, copy a snapshot of property names with live value reads, then use generic append. Ordinary children use generic append directly. |

Pinned hashes:

- TypeScript `MmlNode.ts`: `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`
- Installed CJS `MmlNode.js`: `86c5eb572dcf2652c3c52a60b6c720d231e17f600cc9aeef3ace4aeafc81d45`

The actual CJS constructor order was checked in the pinned bundle: field
assignments precede the inferred-row creation; `setChildren` precedes both
class-default lookups; `Attributes` assignment precedes `setList`. The source
constructor and CJS hash are also recorded in
`p23f-constructor-source.json`.

## Native input boundary

`FactoryArgs` is a bounded native host boundary, not an arbitrary JavaScript
argument evaluator. Properties accept the frozen `Property`, `PropertyList`,
null, or omitted/Undefined representations; other `std::any` payloads throw
an explicit TypeError. Children accept the frozen `Children` handle or
null/Undefined representations; other payloads likewise throw. The native
Object.assign adapter is used to copy supported property values, and its
nullish behavior preserves the source's ignored Null/Undefined attributes.
An explicit null children handle is passed to the virtual child path rather
than silently replaced with an empty array.

The negative-arity branches retain tagged ChildArray reads until member
derefencing. The constructor's inferred-row parent assignment performs its
own write-receiver check after installing the array; the `setChildren` and
`appendChild` child-zero paths retain read-receiver checks for their respective
method names. Generic Node remains responsible for array mutation, captured
for-of behavior, parent assignment, and partial-write order. No alternate
child storage or parent mirror was introduced.

No build, test, mutation, generated fixture, or commit was run for this
delivery. Root owns independent compilation, comparison against all frozen
Source constructor cases, mutation checks, and acceptance. The remaining
MML stubs and other Node/MML consumers remain outside this packet.
