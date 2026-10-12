# P36 MmlMtable methods

Implemented only these three bodies in `src/math/core/mml_tree/mml_nodes/mtable.cpp`:

- `MmlMtable::setChildInheritedAttributes`, from pinned `mtable.ts` lines 134–164.
- `MmlMtable::verifyChildren`, from lines 167–204.
- `MmlMtable::setTeXclass`, from lines 209–214.

The pinned source is MathJax 4.1.3, SHA-256
`8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda`.

Inheritance captures the initial child array for the first live-length
iteration. Each non-row is replaced through virtual `replaceChild` using the
zero-argument `mtr` factory call, then the tagged replacement result is
dereferenced for virtual append. It computes display from explicit-or-default
displaystyle with Source truthiness, reads columnalign and break alignment
through separate virtual attributes accesses, and dispatches virtual
`addInheritedAttributes`. It then reads raw cramped and rowalign values. The
second loop captures the then-current child array, reads each slot before the
body, captures the rowalign tuple receiver before evaluating the live shift or
fallback, writes tuple index 1, and calls the child's inherited-attribute
dispatch with raw level and Boolean-coerced cramped.

Verification preserves the indexed loop's repeated reads of the current
`childNodes` for length and slot access. It captures the factory once, groups
consecutive non-row children, decrements the index after removal, and preserves
the tagged result of virtual replacement. Cell wrapping uses the table host's
distinct `mtd` factory overload with an empty property record and one-child
array. When `fixMtables` is falsey, it performs parent removal and reassignment,
optional empty-cell append, child `mError`, then indexes the last cell from a
captured row array using the separately read current length. It finishes with
the qualified `AbstractMmlNode::verifyChildren(options)` call.

TeX-class assignment calls `getPrevClass(previous)`, captures the current
children handle for live-length iteration, calls each child's virtual
`setTeXclass(Null)`, ignores child results, and returns this node.

All three methods use `TreeCallGuard`. No other method, helper, declaration,
fixture, or oracle was changed. No build or test was run; root owns independent
review and acceptance. No additional host interface requirement was found
within these three method bodies.
