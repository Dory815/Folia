# P23a ChildArray implementation evidence

This report describes the implementation in `src/math/core/tree/child_array.ixx`
under `docs/math-port/p23a-child-array-contract.md`. The pinned Node Array
source SHA-256 is
`9533e7b450232aa81dcf06b119093bc67a062a18d5def5b081b4a0d7ca7378c0`.
The frozen oracle has 8,256 cases; its expected values were not used to derive
implementation behavior. This report is an implementation ledger, not a claim
of parity or acceptance.

## Storage and method mapping

The implementation retains the frozen sole storage pair:
`std::vector<std::optional<Value>> slots_` and `Handle prototype_`. An empty
optional is an absent own index; an engaged `ChildValue(Missing{})` is a
present Undefined; Null and borrowed Product pointers retain their distinct
ChildValue tags and identity. ChildArray owns no Product and introduces no
node aliasing or parallel dense storage.

| Frozen ChildArray member | Implementation behavior |
|---|---|
| initializer-list constructor | Appends each supplied ChildValue as an engaged slot, including explicit Undefined. |
| `size`, `empty` | Report own array length only; prototype properties do not affect length. |
| `hasOwn(i)` | True only for an engaged slot within own length. |
| `has(i)` | Checks own presence, then the prototype chain, including indices beyond own length. |
| `get(i)` | Returns own value when present, otherwise inherited value, otherwise Undefined; does not truncate prototype lookup at own length. |
| `set(i,v)` | Grows with holes, then writes one engaged own slot; rejects an index that cannot be represented by the native vector's length domain before adding one. |
| `erase(i)` | Clears own presence for an in-range index without changing length; inherited lookup remains visible. Out-of-range deletion is a no-op. |
| `resize(n)` | `vector<optional<Value>>::resize`: preserves the common prefix, grows with holes, and truncates removed own slots. |
| `push_back(v)` | Appends one engaged value and returns the resulting length. |
| `pop_back()` | Empty returns Undefined. Otherwise performs indexed Get (including prototype lookup for a hole) before removing the final own slot and reducing length. |
| `slice(start,end)` | Captures own length once, clamps signed relative indices, creates a fresh plain array at the resulting length, and for each source index performs HasProperty then Get. Inherited values become own output values; holes remain holes. Prototype is not copied. |
| `splice(start,count,added)` | Captures original length and normalized start, clamps optional removal count, creates a fresh sparse removed array, shifts source values using HasProperty/Get and deletes target own slots for absent sources, inserts added values densely, then sets final length. Move direction is left for shrinking and right for growing. |
| `indexOf(value,fromIndex)` | Captures own length, normalizes signed start, skips absent properties, includes inherited numeric properties in-range, and compares ChildValue tags and Product pointer identity. |
| `forEach(callback)` | Validates a callable binding, retains `weak_from_this().lock()` over callback execution when shared-owned, captures initial length, and performs live HasProperty/Get before each callback with the same array receiver. Mutations remain visible; appends past initial length are not visited. |
| `setPrototype(handle)` | Walks the candidate prototype chain and rejects a chain containing this array before assigning. Null removes the prototype. |
| `prototype()` | Returns the current shared prototype handle. |

Private arithmetic helpers stay inside ChildArray: `relativeIndex` avoids
negating `INT64_MIN` and handles lengths wider than the signed argument;
`checkedLength` guards vector index/length arithmetic. Splice checks final
length by subtraction before addition, so the bound check itself cannot wrap.

## Boundaries and verification status

The module implements the frozen numeric-index typed domain. It does not bind
generic JavaScript descriptors, named properties, species, custom iterators,
or arbitrary callback objects. `Each` remains the root-declared native callback
shape. Prototype chains can only be formed through `setPrototype`, which
prevents cycles before mutation.

Only `src/math/core/tree/child_array.ixx` and this evidence file were edited for
this packet. Per dispatch, no build, test, mutation, or commit was run. Root
owns compilation, independent source review, oracle parity, mutation checks,
and acceptance; this delivery does not advance the complete tree migration or
other MathJax gates by itself.
