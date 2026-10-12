# P36 table, multiscript and arithmetic dependency acceptance

Root accepts fifteen restored semantic methods: ten table/row/cell/multiscript
methods and five bounded fraction/radical dependency methods. Their algorithms
remain in the respective upstream node classes. Root's table/multiscript hosts
implement tagged indexed access, member dereference, factory invocation and
splice only. Table factory results now retain Null until the Source member access;
zero factory arguments and `{}` plus a shared one-child array remain distinct.
The contract, its bounded extensions, and four agent reports identify ownership.

Root's original real-stub red baseline compiles and fails assertions through
actual Unimplemented bodies, with production/fixture guards passing. The first
green attempt exposed the remaining radical/fraction consumers in the existing
nested table and arithmetic tests. Those attempts were rejected; their dependency
methods were restored under explicit Root scope extensions before final acceptance.

Root's method review also corrected row iteration child capture before virtual
kind/alignment callbacks and exact Null/Undefined raw option read errors in row
and multiscript verification. The independent original witnesses below check the
affected order and graph effects. No source expected values were rewritten.

## Original evidence

Pinned original MathJax-src 4.1.3 generates the unchanged table 4700/135,
multiscript 1304/226 and arithmetic 13501/7 corpora. All reproduce with --check.
Root created four supplemental original observations before their native runs:

| Observer | Records | JSON SHA-256 |
| --- | ---: | --- |
| inheritance repair, factory/replace/append failures and intermediate graphs | 64 | d72eeb6d8e6be1e36acfc95b0112463dfaf9e620dfc0ca038c844697728781e3 |
| cell literal slot 0, inherited slots and exact errors | 14 | 96789fae20955d8a46a61b7832b0bc081f1b6702b675e9315c1bca4abe1cb576 |
| raw verification options and prior repair mutations | 18 | 0ef495fb29b2d43aa16740a02284e3cd8b7b7348dfadb94f6630030bb6b8acbf |
| row child capture before a virtual kind callback changes the slot | 6 | e5fbaabbee6f2c10b5bd75d9f79bbf264f6d534d217c2c271184757cb67d1324 |

All four additional --check comparisons pass. Expected callback errors are
compared to the original error and partial graph, rather than skipped. Source
repair tests also compare created-node order, captured array and parent links.
The separate Native entry fixture contributes two checks for real implementation
entry; it does not add two original Source observations.

## Final checks

The final isolated all-90-module gate baseline passes 19609 assertions in 373
executed tests. All 39 independent wrong implementations compile and then fail
assertions with exit 1. Full final source/fixture guards pass. Faults cover script
base/prime/level/parity/duplicate/fix/parent/base-traversal behavior; table repair,
group boundaries/index decrement/cell wrapping/parent reassignment; display,
cramped and alignment values; iteration child capture; previous-class/Null child
dispatch; exact literal slot errors; and radical prime/index-level behavior.
Early test-tool anchor mistakes did not compile a mutant and were not counted.

Final ignored results.json SHA-256:
43b61f9588b236746648e93346f307dd78d056682402af18aafd32917a818b59.

The normal `cmd /c build_test.bat "*math_port_table*" 2>&1` builds and links the
actual FoliaTests target and passes 4764 assertions in 136 executed tests. The
same executable passes multiscript 1304/226, arithmetic 13501/7 plus its separate
56/22 arithmetic utility tests, and the three remaining new witnesses 38/3.
Skipped registered tests are excluded from every executed count.

This packet accepts these semantic methods only. Remaining operator, under/over,
action and choice consumers still include stubs. TeXParser/Stack/packages,
mathematical fonts, common/scene layout, SVG output and Folia integration remain
unfinished; Folia still uses its math QuickJS path. Mermaid remains paused.
