# P38 maction implementation report

Pinned Source: MathJax 4.1.3 `core/MmlTree/MmlNodes/maction.ts`, SHA-256
`fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895`.

Only the six assigned stubs in `maction_selection.cpp` and
`maction_behavior.cpp` were implemented. No build or tests were run.

## Source correspondence

- `selected`: reads the current attributes receiver and its raw `selection`,
  then reads the current child-array length before ToNumber. It applies
  NaN-preserving Math.min/Math.max and subtracts one, then resolves the
  numeric slot against a fresh child-array read. Null/undefined slots and
  fractional or NaN indices follow the factory fallback. The fallback calls
  the current factory for `mrow` with no arguments and returns its nullable
  result without attaching or caching it.
- `core` and `coreMO`: each captures one virtual `selected()` result and calls
  the corresponding virtual method on that same tagged result.
- `setTeXclass`: strictly compares the current `actiontype` with lowercase
  `tooltip`. On a match it reads slot 1 for the condition and rereads slot 1
  for the virtual `setTeXclass(Null)` call. It then captures one virtual
  selection, dispatches the incoming previous node, calls virtual
  `updateTeXclass` with the same selected node, and returns the child callback
  result.
- `nextToggleSelection`: converts the raw selection through ToString then
  `parseInt`, adds one and applies NaN-preserving Math.max with one. It reads
  current child length, wraps only on strict greater-than, then reacquires
  attributes for the final double-valued write.
- `setChildInheritedAttributes`: reads actiontype once and applies the
  lowercase-string operation. Non-tooltip calls the qualified base method
  with the original arguments. Tooltip mode reads slot 0 and optionally
  dispatches inherited attributes with the raw arguments. After that callback
  it reads the current slot 1 and optionally dispatches with the same
  attributes handle and literal `false, 1, false`.

## Bindings and limits

The methods use Root-provided `numericChild`, `mathMin`, and `mathMax`, plus
existing `number`, `string`, `integer`, and tagged `NodeValue` APIs. Indexed
lookups preserve missing/null/node values; they do not truncate fractional or
NaN indices. Null arrays fail at the corresponding `length` or numeric-slot
access. No helper or interface was added. Root owns independent compilation,
oracle, and acceptance review; this report makes no parity claim.
