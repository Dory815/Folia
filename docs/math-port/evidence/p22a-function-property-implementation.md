# P22a implementation record: shared callable values and array slice

This report maps the two assigned implementations to the frozen P22a
contract. It does not claim root acceptance or test/build verification.

## `FunctionValue` identity and conversions

`src/math/util/function_value.cpp` implements the declarations in
`src/math/util/function_types.ixx:10–12,23–26`:

| Member | Implemented branch | Contract correspondence |
| --- | --- | --- |
| `functions::reference` | Direct `FunctionRef` returns that handle; a `Property` arm containing `FunctionRef` returns the same handle; all other Property values return an empty handle. | Resolves either binding to the one shared function object, without copying/cloning its `FunctionBody`. |
| `functions::fromProperty` | A function-valued Property becomes the direct typed `FunctionRef` arm; every other Property is returned as the raw Property arm, retaining its existing value/handle. | Typed/raw binding conversion with identity preserved. |
| `functions::toProperty` | A raw Property arm is returned unchanged; a direct FunctionRef is passed to the guarded `Property(FunctionRef)` constructor. | Preserves raw values and validates direct raw-callable conversion. A null direct handle is rejected by that constructor. |
| `operator==(FunctionValue, FunctionValue)` | If either side is a callable arm, equality requires both sides to be callable and their FunctionRefs pointer-equal. Otherwise, two Property arms use existing Property equality. | Cross-container equality is strict callable identity; different callbacks with identical bodies are unequal; ordinary raw Property equality is unchanged. Two direct null FunctionRefs compare equal as before. |

No function label, body comparison, callable wrapper, or callback copy was
introduced. The `FunctionRef` value stored inside `Property` is the same
`shared_ptr<Function>` that backs the direct `FunctionValue` arm.

The raw Property constructor guard is declared in
`src/math/core/tree/property.ixx` and is root-owned. It throws `TypeError` on
a null FunctionRef. Therefore a null Property function arm cannot be created
through that constructor; a direct null FunctionRef remains representable in
the typed arm and is not callable. The Property `value` field itself is
public; mutation that bypasses the constructor guard is outside this packet's
frozen input protocol.

## `PropertyArray::slice`

`src/math/core/tree/property_array.cpp` implements the declared
`PropertyArray::slice(std::size_t first = 0)` at
`src/math/core/tree/property.ixx:257–261`:

1. Capture the receiver length once.
2. Set the source start to `min(first, length)` and allocate a fresh result
   with length `length - start`, initially all holes.
3. For each source index in that captured range, query prototype-aware
   `PropertyList::has` first. If present, query `get` and write that value to
   the corresponding result slot.
4. Return the new array handle.

This distinguishes a hole from a present `Missing`/undefined value: an absent
source index leaves the destination slot absent, while own or inherited
`Missing` is written as a present destination slot. Inherited numeric values
are copied, including callable Property values; the Property copy retains
FunctionRef identity. Source records, nested arrays, and functions retain
their shared identities. The receiver's storage remains aliased only through
its original handle; the result owns fresh array storage.

The bound source caller is `TokenMap.ts:273–301` (`MacroMap`): a configured
array value reads `value[0]` as the parse method, then calls `value.slice(1)`
for macro arguments. This slice binding accepts a native nonnegative `size_t`
start; it does not implement general JavaScript index coercion, species,
proxies, or user-defined getters/iterators, as specified by the frozen
contract. The related `FunctionList.ts:25–61` has direct function and
`[function, priority]` entries; its typed tuple API is separate from this
MacroMap slice path.

## Validation boundary

Only the assigned implementation units and this evidence file were changed.
No build or test command was run, per dispatch. Root owns fixture comparison,
builds, mutation checks, and acceptance. The implementation is ready for that
independent review; this record does not claim acceptance.
