# P7 generic Node evidence

This packet implements the generic `AbstractNode` and `AbstractEmptyNode`
behavior from pinned MathJax 4.1.3 `ts/core/Tree/Node.ts` in
`src/math/core/tree/node.ixx`. The source SHA256 is
`532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0`.
MathJax attribution is retained in the module and the Apache-2.0 license is
provided by `src/math/LICENSE-MathJax.txt`. Root's revision-3
`docs/math-port/tree-contracts.md`, `node_types.ixx`, and
`tests/math/reference_tree.cpp` define this packet's native contract.

## Runtime member ledger

| Pinned source member | Native implementation and behavior |
| --- | --- |
| `AbstractNode.parent` (`Node.ts:166`) | Stored once in the root-owned `Node<Product>` interface; initialized to null. |
| `AbstractNode.properties` (`:171`) | Protected fresh `PropertyList` per object. Its live shared handle is returned by `getAllProperties`. |
| `AbstractNode.childNodes` (`:176`) | Stored once in `Node<Product>` as a replaceable shared child-array handle, initially fresh and empty. |
| `AbstractNode.factory` / constructor (`:186-197`) | Factory reference is stored by `Base`. C++ construction initializes storage only; public `initializeNode` runs source-equivalent property and child phases after complete allocation and adoption, preserving virtual dispatch. |
| `kind` getter (`:202-204`) | Returns `"unknown"`; derived types can override it. |
| `setProperty` (`:209-211`) | Calls `PropertyList::set`, retaining inherited lookup and `__proto__` setter behavior of the accepted host property object. |
| `getProperty` (`:216-220`) | Uses prototype-aware `PropertyList::get`; missing values and inherited built-ins remain distinguishable. |
| `getPropertyNames` (`:223-226`) | Returns own enumerable string keys using the host's ECMAScript integer-key ordering. |
| `getAllProperties` (`:230-233`) | Returns a handle alias to the same mutable property storage. |
| `removeProperty` (`:237-242`) | Erases own names in supplied order; inherited properties become observable again. |
| `isKind` (`:246-248`) | Calls the current factory's registered native instance predicate; it does not compare kind strings. |
| `setChildren` (`:253-258`) | Replaces the child handle before checking/iterating the supplied handle, then invokes virtual `appendChild` over a live-length loop. It does not detach previous parents. Null input fails after replacement; null members are pushed before append fails. |
| `appendChild` (`:263-267`) | Pushes first, then writes the child's parent; returns the supplied pointer. Null throws after the null slot is appended. |
| `replaceChild` (`:272-285`) | Finds the first identity match. If present, replaces the slot, sets the new parent's link, then clears the old link only when it still points here. Returns `newChild` whether matched or not. Null failures occur at the corresponding dereference after preceding mutations. |
| `removeChild` (`:288-297`) | Removes the first identity match and then clears its parent unconditionally. Returns the supplied pointer; a matched null slot is erased before the null-parent failure. |
| `childIndex` (`:300-305`) | Returns the first identity index, including a null slot, or empty when absent. A null child-array handle throws. |
| `copy` (`:308-317`) | Ignores generic `keepIds`, creates the current virtual kind through the current factory, verifies the returned product has this `AbstractNode<Product,Base>` binding, replaces properties with a fresh shallow `Object.assign` result, then iterates the captured source child handle live-length, skipping nulls and virtual-copying/appending each child. A null source array uses the source's empty fallback. |
| `findNodes` (`:322-330`) | Calls virtual preorder `walkTree`, includes self, asks each node's virtual `isKind`, and returns identity-preserving pointers. |
| `walkTree` (`:335-343`) | Guards recursion, calls the visitor on self, then captures the current child handle and reads its live length while traversing. Null children are skipped; replacing the field from a callback does not replace the captured array. Returns the original `std::any` data handle/value. |
| `toString` (`:350-353`) | Guards native recursion, evaluates virtual `kind()` first, then captures the child-array handle and its length once for `Array.join(',')`. A `TreeJoinGuard` tracks active child-array identities; re-entering the same array returns `kind()` plus `()` without converting children. Each saved index reads the captured array's current value; null or removed/out-of-range entries are empty strings and separators remain. A null array throws after `kind()` runs. |
| `AbstractEmptyNode.setChildren` (`:373`) | No-op; does not replace existing child storage. |
| `AbstractEmptyNode.appendChild` (`:378-381`) | No-op returning the input pointer, including null. |
| `AbstractEmptyNode.replaceChild` (`:385-388`) | No-op returning `oldChild`. |
| `AbstractEmptyNode.childIndex` (`:392-395`) | Always returns empty. |
| `AbstractEmptyNode.walkTree` (`:401-408`) | Guards the call, visits only self, returns unchanged data, and ignores child storage. |
| `AbstractEmptyNode.toString` (`:411-414`) | Guards the call and returns virtual kind only. |
| native `initializeNode(args)` | Split constructor phase required by C++: empty/absent `std::any` arguments use fresh defaults; properties are enumerated and sent through virtual `setProperty` before children are inspected; nonempty children are sent through virtual `setChildren`; explicit invalid/null values throw TypeError at the corresponding source phase. Extra arguments are ignored. |

## Host and dispatch details

`AbstractNode<Product,Base>` derives the supplied `Base`, defaulting to
`Node<Product>`, and uses its single factory, parent, and child-array storage.
The explicit constructor accepts only the factory and does not call virtual
source operations. The factory callback must allocate the complete concrete
type, adopt it, and then call `initializeNode(args)`; this preserves the
JavaScript constructor's most-derived `setProperty`/`setChildren` dispatch
without relying on C++ virtual dispatch during base construction. The same
template supports the normal and empty MML inheritance branches without
introducing another tree representation.

Property initialization snapshots the source own-key list (`Object.keys`) and
performs each value read and virtual setter call in that order. The live
`PropertyList` remains aliased after construction. Copy starts with fresh
property storage and calls `PropertyList::assign`, which follows the pinned
production profile's transpiled behavior: the TS spread expression becomes
`Object.assign` in both pinned CJS (`__assign`) and MJS, and Folia's `require`
profile resolves CJS. Thus `__proto__` assignment can change the destination
prototype or ignore a scalar instead of creating an own data property. Values
remain shallow aliases. This mapping follows the resolved runtime profile and
revision-3 contract; it does not generalize to other transpilers.

All recursive generic copy, walk, and stringification paths, including Empty
walk/stringification, use `TreeCallGuard<Product>` with RAII unwinding. This is
the frozen 512-call host resource bound, not a syntax limit. Nulls, duplicate
children, parent disagreement, and cycles remain observable rather than being
normalized. Child-array handles may be replaced; traversal/copy loops retain
the array object captured at loop start and inspect its live length. For
stringification, the child kind is evaluated before the child array and join
length are captured. `TreeJoinGuard<Product>` suppresses recursion by active
array identity, matching `Array.join`; cycles through nodes do not necessarily
throw. This is separate from `TreeCallGuard<Product>`, which bounds native
recursive calls for stack safety. The join length stays fixed while each index
reads the captured array's current contents, so removals leave empty
comma-separated slots and later appends do not extend the result.

This packet covers only generic `Node.ts`. MML semantic classes and the wrapper
hierarchy are separate packets; no MML methods, wrappers, parser, or rendering
behavior are claimed here. The pinned source's interface comments say
`replaceChild` returns the old child, but the executable body returns
`newChild`, which the implementation preserves. No contracts, shared
interfaces, tests, oracle, build files, or other implementation units were
changed. Root owns builds, fixture comparison, recursion-bound verification,
and acceptance; this evidence does not claim final acceptance.
