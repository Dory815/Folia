# P14 `MmlMtr` method-port evidence

Upstream is pinned MathJax 4.1.3
`.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mtr.ts`, SHA-256
`ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f`.
The MathJax Consortium copyright and Apache-2.0 notice are retained in the
implementation; the complete license is in `src/math/LICENSE-MathJax.txt`.
This report covers only the three root-assigned executable methods; Mtr and
Mlabeledtr getters, defaults, declarations and class bindings are root-owned.

| Native member | Upstream member and lines | Port mapping |
| --- | --- | --- |
| `MmlMtr::setChildInheritedAttributes` | `setChildInheritedAttributes`, 76–106 | Captures the first child array and repairs non-mtd children through factory create → virtual replace → returned-node append. Reads/splits columnalign then data-break-align; label arity reads raw parent side and prepends it/top. Calls virtual `addInheritedAttributes` with source-ordered properties. Captures the then-current child array separately, captures each column tuple receiver before shift/fallback, evaluates kind before break-align shift/fallback, mutates the shared column tuple, creates each vertical tuple, and passes the shared attributes handle plus raw display/level/prime to each child. |
| `MmlMtr::verifyChildren` | `verifyChildren`, 113–132 | On a present invalid parent, reads kind, calls virtual `mError(..., short=true)`, and returns. Otherwise it captures the child array, repairs each non-mtd child in create → replace → append order, then checks `fixMtables` truthiness and optionally errors on the original child. Ends with qualified `AbstractMmlNode::verifyChildren(options)`. |
| `MmlMtr::setTeXclass` | `setTeXclass`, 137–143 | Calls inherited `getPrevClass` first, captures the resulting child array, dispatches each child with null previous, ignores child returns, and returns this row. |

## Evaluation order, mutation, and aliases

Each source `for...of this.childNodes` captures that array object once and
observes its live length and current slots. The first inheritance loop and
second inheritance loop capture at distinct points; the second loop therefore
sees a child-array replacement performed by an override of
`addInheritedAttributes`. The verification loop captures once, while the
qualified base verification performs its own later capture. `setTeXclass`
captures only after `getPrevClass` returns. The native implementation uses a
shared `Children` handle plus an index/size loop for each capture, preserving
array identity and live changes.

Alignment inputs are read from separately reacquired virtual attribute handles
and passed raw to the root-frozen string-only `tableSplit`. It does not coerce
non-string Properties. For Mlabeledtr (`arity() == 1`), the method reads the
current parent and its attributes, obtains raw `side`, then prepends that raw
Property and literal `top`. The output `AttributeList` is the virtual
`addInheritedAttributes` result. It is passed by handle to every cell; the
inherited `columnalign` tuple remains shared and is mutated in place.

For each child, `columnalign` is captured as the indexed assignment receiver
before `calign.shift()`. A falsey shifted value then reads slot 1 from the
current `attributes.columnalign`, while the write targets the captured
receiver. For vertical alignment, the virtual `kind()` is evaluated before
`balign.shift()` and the fallback read from
`attributes['data-break-align'][1]`; a fresh tuple `[kind, alignment]` replaces
`data-vertical-align` for that iteration. The source uses JavaScript truthiness
for both shift fallbacks, so empty strings and other falsey raw label-side
values fall back to the existing tuple slots.

The invalid-parent verification branch does no child repair or base traversal.
The repair branch calls the factory before virtual `replaceChild`, then calls
`appendChild` on the value returned by `replaceChild`. It tests
`options.fixMtables` only after those graph mutations. When falsey, the error
is dispatched on the original child after it has moved into the new cell. The
final base verification runs only after the whole repair loop completes.

All three methods enter `TreeCallGuard<MmlNode>`. Root-frozen
`table_host.ixx` provides checked table/child access, zero-argument cell
creation, raw string-only split, shift, strict indexed tuple writes, and
virtual attribute acquisition. The row methods add no array, attribute,
factory, or tree policy beyond the pinned source. No layout or geometry behavior
is included.

Root owns oracle fixtures, builds, mutations, and final acceptance. This report
maps source behavior and does not claim packet acceptance.

Root review correction: the output loop now captures a raw tableSlot value.
Column/vertical tuple updates precede tableNode validation of the inheritance
receiver. The original early null check failed source partial-state witnesses
for row and labeled row; root corrected it under contract revision 2.
