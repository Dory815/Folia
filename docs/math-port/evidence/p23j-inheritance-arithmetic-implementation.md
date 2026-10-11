# P23j inheritance arithmetic, packet B

This packet implements only the four bodies authorized by the frozen P23j
contract. Root owns source comparison, mutation testing, and acceptance; no
build or test was run by this implementation.

`MmlMfrac::setChildInheritedAttributes` ports
`ts/core/MmlTree/MmlNodes/mfrac.ts` lines 96–141 (source SHA
`4ded5fc9922d462d02818523d676c30299d08cfe7794067bd4e20950c8fccfc7`). It
keeps the `TreeCallGuard`, applies the source condition with short-circuiting,
and uses numeric conversion plus one for the source `level++`. It reads
`numalign` then `denomalign` from the current attributes receiver, makes two
separate shallow copies with `arithmeticCopy`, and calls
`addInheritedAttributes` with the corresponding independent additions objects.
The numerator is dispatched through a Get from the current child array at
index 0; after that callback, the denominator is re-read from the then-current
array at index 1. Each child gets `display=false`; the numerator gets the
original prime value and the denominator gets `true`. Failures preserve work
already completed before the failing Get/member call.

`MmlMsubsup::setChildInheritedAttributes` ports
`ts/core/MmlTree/MmlNodes/msubsup.ts` lines 84–107 (source SHA
`37fd01fa0c44ceab5b4ed30ba3d95ab43ffb8aed55db27c7290d6d786131f6e7`). It
captures one child-array handle. Slot 0 receives the original four arguments.
Then slot 1 is resolved as the call receiver before computing `level + 1` with
JavaScript addition semantics; the virtual `sub()` is read only when `prime`
is falsey, and a truthy prime is forwarded unchanged. After that child returns,
slot 2 is read from the captured array. Null/Undefined returns early; a node
captures the receiver before the separately computed `level + 1` and the
short-circuited virtual `sub()` comparison. This preserves string
concatenation for script levels and differs intentionally from mfrac's numeric
increment.

`host::arithmeticChild` in `arithmetic_host.ixx` now performs current-array
indexed Get followed by the requested member's tagged `requireNode`. A null
array reports its numeric index at the Get boundary; nullish slots report the
requested member. `host::arithmeticAttributes` retains the current virtual
attributes lookup and now reports a null receiver at the source `.get` member.
These helpers are shared only where the source reads the current child array;
msubsup uses its captured array directly.

Root's first independent build caught a return-type mismatch in
`arithmeticChild`: `requireNode` returns `MmlNode*`, while the frozen helper
signature returns `MmlNode&`. The expression now dereferences that validated
pointer. Root's second independent build caught the same pointer/reference
mismatch in the subscript and superscript local references; both now
dereference their validated pointers. These fixes were made without rerunning
a build or tests.

The native adaptation uses the established `Property`, `ChildValue`, and
`ChildArray` handles, so Null/Undefined and sparse slots remain distinct. The
four modified bodies are `src/math/core/mml_tree/mml_nodes/mfrac.cpp`,
`src/math/core/mml_tree/mml_nodes/msubsup.cpp`, and the two helper bodies in
`src/math/core/mml_tree/arithmetic_host.ixx`. No declarations or other method
bodies were changed.
