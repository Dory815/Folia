# P20c root local acceptance — complete util/numeric.ts

Root accepts both pinned MathJax 4.1.3 numeric.ts functions under
../numeric-reductions-contract.md. Complete local Source acceptance becomes
**53/246**. Common table layout, the complete TeX parser, SVG output and Folia
integration remain open.

Root froze the borrowed dense binary64 span boundary, declarations, actual-CJS
oracle and exact input bits before Luna implementation. The only Source caller
in the enabled closure is Common/Wrappers/mtable.ts; its numeric producer and
string-width conversion responsibilities remain required in that future port.
This explicit native array adaptation does not implement arbitrary JavaScript
accessors or prototype mutation.

Source review verifies the input-ordered sum fold from positive zero, ordinary
binary64 addition, and the complete max fold from positive zero. Root required
removing an early NaN return so max retains full ordered visitation. Empty and
all-negative maximums return positive zero; NaN propagates. No reordering,
compensated sum, float narrowing or finite-value filtering is introduced.

The independent oracle executes hash-pinned actual CJS and records 145 cases /
290 observations. Native tests pass all 290 assertions in one test, comparing
every finite/infinite/signed-zero result by exact binary64 bits and NaN by its
classification. Inputs cover empty arrays, zeros, negatives, cancellation,
overflow, infinity, subnormal values, NaNs and seeded binary64 sequences.
`numeric-reductions-oracle.cjs --check` reproduces the committed artifacts.

All seven isolated mutants compile and fail assertions with exit 1: sum's
negative-zero seed, reverse order and float narrowing; max's negative-infinity
seed, negative-zero seed, discarded NaN and minimum operation. Baseline test
count and working-tree byte guards pass; compile failures and crashes cannot
count as detections. Results: p20c-numeric-reductions-mutations.json.

The two-member ledger is checked against fresh TypeScript 5.9.3 AST inventory,
including whole-file and method hashes. Final `cmd /c build_test.bat 2>&1`
completed with exit 0; ignored log: build/p20c-final-full-core.log. Focused and
mutation logs are build/p20c-numeric-focused.log and
build/p20c-numeric-mutations.log. No application or platform path changed.
