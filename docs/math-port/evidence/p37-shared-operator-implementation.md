# P37 shared operator implementation report

This report records the implementation correspondence only. No build or test
was run here; Root owns review and acceptance.

Pinned MathJax source is 4.1.3, commit
`fb987178c2d279a99b0db5ea02e751691703955e`.

| Native member | Pinned source | Implemented behavior |
| --- | --- | --- |
| `adjustOperatorTeXclass` in `operator_class.cpp` | `MmlNodes/mo.ts:347–410`, SHA-256 `2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605` | One guarded shared implementation for both concrete callers. It reads receiver `texClass` then `prevClass`, returns the original tagged previous value for strict `NONE`, uses raw `autoOP` truthiness, preserves virtual class/previous-class/previous-level writes and their source order, applies leading/trailing BIN rules, and performs the source parent walk and terminal child identity comparison. Parent/indexed-child values retain `NodeValue` null versus missing tags. |
| `MmlTeXAtom::setTeXclass` in `tex_atom.cpp` | `MmlNodes/TeXAtom.ts:87–91`, SHA-256 `bad114224392b94852456a4636b85ad90a5a2c96b63e6c7911d155db299f7028` | Enters the shared tree-call guard, requires child slot zero and invokes its virtual `setTeXclass(Null)`, ignores that return, then virtually calls `adjustTeXclass` with the original tagged argument and returns the result. |
| `MmlTeXAtom::adjustTeXclass` in `tex_atom.cpp` | `MmlNodes/TeXAtom.ts:97–105` | Delegates to `adjustOperatorTeXclass(*this, previous)` without another guard, matching the final prototype assignment that replaces the class-body identity method with `MmlMo.prototype.adjustTeXclass`. |

The shared adjustment owns the only `TreeCallGuard`, as specified in the P15b/P15c
contract. The branch that repairs a previous BIN class writes this node's
`prevClass` before invoking the previous node's `setTexClass`. The operator
walk uses the raw parent link, re-reads `parent.parent` for the loop condition
and loop-body assignment, and captures the terminal indexed array before
reading the current array length. A zero-length terminal array produces the
source's missing indexed value rather than underflowing a native index.

Root-frozen `mo_host` now supplies tagged member and indexed-child operations.
This implementation uses `moRequired` for member dereferences and preserves
the indexed result as `NodeValue`; it does not change that host layer.

The existing source-derived observer fixtures include the shared adjustment
matrix and callback traces in `reference_mo.cpp` and `reference_mo_order.cpp`,
plus TeXAtom invocation in `reference_atom_semantics.cpp`. Their presence is
not a passing result: no native gate was run for this delivery.
