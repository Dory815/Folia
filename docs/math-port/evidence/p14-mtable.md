# P14 MmlMtable implementation report

Pinned source: MathJax 4.1.3
`.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mtable.ts`, SHA-256
`8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda`.
Only `src/math/core/mml_tree/mml_nodes/mtable.cpp` and this report were edited
for this dispatch. Root owns source review, tests, build, mutation testing, and
acceptance. No build or test was run here.

| Native member | Pinned source | Implemented source behavior |
| --- | --- | --- |
| `setInheritedAttributes` | `mtable.ts:106–125` | Uses a live index into the mutable `indentAttributes` vector, copying each yielded name before callbacks. Tests the incoming property by JS truthiness. For a truthy tuple, captures the current virtual attributes handle before the second incoming tuple lookup and index-1 read, then writes the raw value. Checks explicit presence afterward, re-reads the receiver for `unset`, and invokes the qualified superclass method only after the loop. |
| `setChildInheritedAttributes` | `mtable.ts:134–160` | Captures the first child array for for-of-style repair with live length/slots, wrapping each non-`mtr` child in a factory-created `mtr`. Computes display with explicit/default short-circuit and boolean coercion, constructs the table additions in source field order, and calls the virtual inherited-attribute helper. Reads raw `data-cramped`, splits only a string `rowalign`, captures a fresh child-array handle, shifts each row alignment, mutates the shared rowalign tuple at index 1, and dispatches each child with the raw level and boolean cramped prime value. |
| `verifyChildren` | `mtable.ts:167–204` | Captures the factory once and uses a signed numeric index over the live `childNodes` property. Existing `mtr` rows reset grouping. Non-row children are grouped by removal/index decrement or wrapped by `replaceChild`; `mtd` children move directly and other children receive a new `mtd`. The `fixMtables` false branch performs the source parent removal/direct reassignment, optional extra cell creation, child error creation, and append to the last cell in source order. Calls qualified base verification last. |
| `setTeXclass` | `mtable.ts:209–215` | Calls `getPrevClass` first, then captures the current child array for live for-of iteration, sends null previous to each required child, and returns the table. |

The implementation uses root-frozen `table_host` helpers for required nodes,
captured child arrays, strict string splitting, mutable shifts, tuple writes,
and zero/two-argument factory calls. `arithmeticAttributes` preserves virtual
attribute receiver evaluation; `TreeCallGuard` protects all four recursive or
callback-bearing entries. `AbstractMmlNode` superclass calls are qualified.

Call ordering preserved in this implementation includes capturing the
`setInherited` attributes receiver before reading its tuple argument; building
the table additions in order (`columnalign`, `rowalign`, `data-break-align`);
capturing the rowalign assignment target before shifting and falling back;
capturing each append receiver before factory-created arguments; and reading
the error child receiver before evaluating the message's virtual `kind`.
`verifyChildren` re-reads the table's child-array field for the loop condition
and indexed child, while each for-of-style loop keeps its own captured array
handle and observes that handle's live length and slots.

The root-frozen fixture is `tests/math/reference_table.cpp`; its generator and
actual-CJS observations are root-owned. This report does not claim that those
fixtures pass, that the implementation is accepted, or that table-family
integration is complete.
