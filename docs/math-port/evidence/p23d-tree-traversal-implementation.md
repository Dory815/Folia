# P23d generic traversal implementation evidence

This packet implements only generic `AbstractNode::walkTree` and
`AbstractNode::toString` in `src/math/core/tree/node.ixx`, plus
`AbstractWrapper::walkTree` in `src/math/core/tree/wrapper.ixx`. It does not
claim full Node/MML migration or Source-equivalent acceptance.

## Source mapping

| Native implementation | Pinned MathJax source | Preserved behavior |
|---|---|---|
| `node.ixx:181–197 AbstractNode::walkTree` | `Node.ts:335–343` | Enter the shared tree-call budget; invoke the visitor on self first; only then capture `childNodes`; iterate the captured array with live length and indexed Get; recurse through virtual `walkTree` for Node values; skip hole/Undefined/Null values; return the original `std::any` data. A null captured array throws after the self callback. |
| `node.ixx:199–221 AbstractNode::toString` | `Node.ts:350–352` | Enter the shared call budget, read `kind` first, capture the current child array, enter the array-identity join guard, and capture its length once. Each original slot contributes a comma; live Get supplies its current value; Node values dispatch virtual `toString`; hole/Undefined/Null contribute an empty segment. Re-entering an already-active array returns `kind()` formatting with an empty join, matching `Array.join` recursion suppression. |
| `wrapper.ixx:32–50 AbstractWrapper::walkTree` | `Wrapper.ts:159–169` | Enter the shared call budget and visit self first. Afterward inspect the optional property state: absent returns immediately; present-null throws; present array is captured and iterated with live length/Get, recursively dispatching Node values and skipping nullish values. Return the same data. |

Pinned source SHA-256:

- `Node.ts`: `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0`
- `Wrapper.ts`: `8729a810aabd6eb3ad22021464f2f27d86116fae9edc0f818527f82e517bf7d9`

The implementation uses `TreeCallGuard<Product>` for the specified native
512-call resource boundary and `TreeJoinGuard<Product>` for active array
identity. The join guard is scoped so exception unwinding removes its active
identity. Traversal does not use a visited-node cache: shared nodes are visited
per occurrence, and mutations to the captured array remain observable.

## Native boundary and validation status

`ChildValue` can represent only a node pointer, Undefined, or Null. For this
tree packet, join maps the latter two to empty text and traversal skips them,
matching the pinned paths. Wrapper absence is separately represented by
`optional<FactoryChildren>`; an engaged null handle throws after the visitor.
Native null-array TypeErrors are explicit, while their host wording is an
adaptation of JavaScript's runtime-dependent non-iterable/join diagnostics.

No builds, tests, fixture generation, mutations, or commits were run for this
delivery. Root owns the closed red/green evidence and acceptance. Other Node
methods, Empty overrides, MML overrides, and remaining consumers are outside
this packet.
