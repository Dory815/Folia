# P36: restore the remaining table and multiscript consumers

Owner: Root. This packet restores the ten explicit tagged-tree stubs; it does
not create a parser, output renderer, or Folia integration. The pinned source
is MathJax-src 4.1.3 at fb987178c2d279a99b0db5ea02e751691703955e.

## File ownership

- Root owns `table_host.ixx`, `multiscript_host.ixx`, independent observers,
  mutation gates, acceptance, and module/public interface changes.
- `math_layout` owns exactly the three stub bodies in `mml_nodes/mtable.cpp`.
  The existing `inheritAttributes` method is outside this assignment.
- `mml_row_mi` owns exactly the three stub bodies in `mml_nodes/mtr.cpp` and
  two stub bodies in `mml_nodes/mtd.cpp`.
- `mml_verify_nodes` owns exactly the two stub bodies in
  `mml_nodes/mmultiscripts.cpp`. The accepted marker verification bodies must
  remain unchanged.

Each implementation is a direct method translation from its upstream node
file. Keep inheritance, tree verification/repair, and TeX class assignment in
their owning node classes. Helpers implement indexed Get, member dereference,
factory invocation and array splice only; no helper owns a semantic algorithm.
Use existing tagged NodeValue, shared sparse ChildArray and Property objects.
Do not introduce another tree, dense-vector path or compatibility fallback.

Root corrects the old tableCreate declaration to return NodeValue, preserving a
null factory result through virtual replace/append until Source dereferences it.
Its no-child overload passes zero factory arguments; its child overload passes
the distinct `{}` and shared one-child array arguments. Indexed slot helpers
return NodeValue; member helpers require it with the exact member name. Array
access helpers accept the access member (iteration versus length) so a null array
does not raise an unrelated generic helper error. Agents may directly read arrays
and use existing requireNode at the exact Source access point.

## Observable requirements

Read the complete assigned source methods before coding. Preserve evaluation
order, virtual calls, callback-visible state, exact argument identity, factory
argument count, and intermediate graph mutations. Capture arrays only where
Source captures the array iterator; for-of observes its live length. Indexed
loops re-read `this.childNodes` for length and slots at their source points.
Rebinding an array does not rebind a captured iterator. Preserve inherited slots,
Null versus Undefined, and exact member errors at the access that triggers them.
Keep TreeCallGuard at each semantic method entry.

Multiscript inheritance first dispatches base slot 0, then walks a live indexed
array starting at 1. `level + 1` uses host addition, not forced numeric addition.
Preserve raw `prime || parity` values. None creation may return null until the
source dereferences it; splice keeps array identity; inserted parent assignment
and final append are separate operations. Verification captures raw
`fixMmultiscripts` once and delegates to base verification only after the source
checks and virtual error calls.

Table inheritance repairs each non-row child through factory, virtual replace,
and append before attribute evaluation. Row inheritance similarly repairs cells.
Alignment tuple assignments retain receiver capture and RHS callback order;
alignment exhaustion uses source truthiness. Labeled rows use virtual arity and
current parent attributes. Do not conflate break alignment and vertical alignment.
Table display uses its exact explicit-or-default expression, and table cramped
is Boolean-coerced. Row display/level/prime remain raw.

Table verification groups consecutive non-rows, including removal/index decrement,
cell creation, parent reassignment, and actual mError placement. `fixMtables` is
read at each Source location. Row/cell parent checks re-read parent before isKind;
invalid-parent error returns before base traversal. Table/row TeXclass traverses
children with Null previous; cell dispatches only literal slot 0, including an
error for a missing slot. All three return their own node after getPrevClass.

## Acceptance

Root freezes Source observations before implementation. Existing unchanged gold:
`reference_table.cpp` (135 tests / 4700 assertions) and
`reference_multiscript.cpp` (226 tests / 1304 assertions). Source generators must
remain byte consistent; implementation agents may not edit any expected data,
test generator, fixture, CMake list, interface, or unrelated file.

Root supplements the known observer gaps: inheritance repair of non-row/non-cell
children, factory/replace/append failure points and partial effects, and literal
slot-0 cell class errors. Native baseline must compile and pass through the real
methods. Independent compiling faults must be detected by assertions, with no
crash or non-compiling fault accepted as evidence. Root reviews each body against
its full Source method and runs relevant regressions before accepting.

Agents do not build or run tests. They report source method ranges, changed body
inventory, unresolved host needs, and a line-by-line semantic correspondence.
They must stop if the frozen API cannot represent required behavior. No production
or fixture edits may begin while Root's prior consumer gate is live. Root dispatches
implementation only after that gate is terminal.

Root's real-stub red baseline compiled and exited 1 through assertions: table
126 of 135 tests failed (149 failed assertions of 174 executed), multiscript
137 of 226 tests failed (357 of 824 executed), and both Native missing-method
entry assertions failed with Unimplemented. Those executed counts are the
truncated red paths, not the green Source corpus's expected 6004 assertions.
The red runner's working source/fixture guards passed. No implementation was
accepted from compilation or from merely having a Source fixture.
