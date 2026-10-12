# P36 row and cell implementation

Implemented the five assigned method bodies from pinned MathJax-src 4.1.3
(fb987178c2d279a99b0db5ea02e751691703955e), using the tagged child values,
shared sparse arrays, and Root-owned table host bindings.

| Source method | Source range | Native method | Native range |
| --- | --- | --- | --- |
| `mtr.ts` `setChildInheritedAttributes` | 76–110 | `mtr.cpp` `MmlMtr::setChildInheritedAttributes` | 12–71 |
| `mtr.ts` `verifyChildren` | 113–133 | `mtr.cpp` `MmlMtr::verifyChildren` | 73–96 |
| `mtr.ts` `setTeXclass` | 137–144 | `mtr.cpp` `MmlMtr::setTeXclass` | 98–106 |
| `mtd.ts` `verifyChildren` | 84–96 | `mtd.cpp` `MmlMtd::verifyChildren` | 12–22 |
| `mtd.ts` `setTeXclass` | 99–103 | `mtd.cpp` `MmlMtd::setTeXclass` | 24–30 |

Row inheritance repairs each non-cell through factory creation, virtual
replacement, and append, then computes alignment from current attributes. Its
two loops capture their respective arrays and observe live length. Alignment
tuple receivers are captured before shift/fallback reads; vertical alignment
keeps the row kind and raw shifted/fallback value. Row verification checks the
parent through a second parent read, repairs non-cells in source order, and
calls the base verifier after the loop. Row TeX class assignment calls each
child with null and returns the row. Cell verification preserves its early
invalid-parent return. Cell TeX class assignment dispatches only slot zero.

No host boundary was missing for these bodies. No build or tests were run, as
Root owns verification and acceptance. No unresolved implementation issue is
known here.
