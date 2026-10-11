# P20c numeric reductions implementation evidence

This implementation maps the two exported functions in pinned MathJax 4.1.3
`ts/util/numeric.ts`, SHA-256
`a2004a9888f5c85f3cf81126e43d04dfe66b64c55e3b5f7d97366631809d7c15`.
The source file was read directly from the pinned checkout. This ledger does not
claim differential acceptance; root owns the frozen actual-CJS fixtures and
native review.

| Source member | Source lines | Native definition | Preserved behavior |
|---|---:|---|---|
| `sum(A)` | 28–30 | `numeric::sum(std::span<const double>)` | Starts with positive zero and adds each dense binary64 element in input order using ordinary double addition, matching `reduce((a,b) => a+b, 0)`. |
| `max(A)` | 36–38 | `numeric::max(std::span<const double>)` | Starts with positive zero and visits the entire span in order. When either accumulator or current value is NaN, the accumulator becomes NaN; otherwise it is replaced only when the current value is greater. Empty and all-negative inputs therefore return positive zero; infinities and signed zero follow the Source fold. |

The span is borrowed and read-only. The implementation does not reorder or
parallelize either reduction, narrow values, use compensated summation, or add
finite-value guards. `std::max` is not used because its NaN behavior does not
match JavaScript `Math.max`.

## Caller boundary

The frozen caller audit identifies `output/common/Wrappers/mtable.ts` as the
only importer in the enabled 246-file closure (SHA-256
`a1e060baab84d7b842c811c560ab73f32ca91614e3a5b02f7f1712db73b0100c`). Its
numeric callers reduce table extents, row cell counts, line-spacing arrays,
and widths after source-side number conversion (`mtable.ts:1210–1223`); H/D
arrays initialize with zero at lines 786–787. Those callers provide dense
numeric sequences through the native span contract. Parsing table widths and
preserving that typed producer responsibility are outside this packet.

The source-only CJS reference was frozen by root at 145 cases and 290 exact
numeric observations. No build or test was run here; root owns fixture replay,
mutation checks, full core validation, and acceptance.
