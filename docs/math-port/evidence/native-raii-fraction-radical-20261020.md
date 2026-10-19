# Shared fraction and radical numerical layout

## Test representation correction

The earlier 4,241/3,954-line generated C++ suites have been replaced by compact
table-driven runners and separate Source observation tables. Re-running the
tracked witnesses reproduces all 288/192 case records exactly (deep comparison
with the earlier observations). The focused native run passes both tests:
14,208 fraction assertions and 11,424 radical assertions. Fraction now also
checks the staged request after mutating its original input. All 480 inputs and
the original output comparisons remain covered; no native equations generate
expected values. Log: `build/math-port/layout-table-focused.log`.

Reproduce from the repository root, using the pinned local source and bundles:

```powershell
node tools/math-port/layout-source-fraction.cjs --output build/math-port/fraction-source.json
node tools/math-port/layout-source-radical.cjs --output build/math-port/radical-source.json
node tools/math-port/layout-source-table.cjs --family fraction --input build/math-port/fraction-source.json --output tests/math/fixtures/fraction_source_pairs.txt --check
node tools/math-port/layout-source-table.cjs --family radical --input build/math-port/radical-source.json --output tests/math/fixtures/radical_source_pairs.txt --check
build/core/FoliaMathTests.exe "*all_generated_source*"
```

Omit `--check` only when intentionally regenerating the tables. Their first
non-comment line names every column; subsequent rows use those field names,
quoted strings and `~` for missing/null observations. Source TS/CJS hashes are
checked before accepting witness output. The test-local reader is not part of
the native math module.

These remain numerical component tests. Canonical Common wrapper orchestration
and actual layout-to-Scene integration are still incomplete. The additional
`placement/` family introduced in b030b874 has been removed. The numerical
implementations now replace canonical Common `wrappers/mfrac`, `msqrt`, and
`mroot` files, exported through the existing Common primary module/partitions.
Degree formulas live only in mroot and are called from the shared radical
implementation; old Scene fraction/root pairs and registrations were deleted.
The post-consolidation build, both paired tests, and the full 101-test / 30,564
assertion native component suite returned 0. Logs:
`build/math-port/common-family-consolidation-{build,focused,full}.log`.

## Earlier component observations

The native placement services own inputs and results and depend only on BBox
and std. They combine the pinned Common measurement and SVG placement
equations once; neither backend owns a second fraction/root algorithm.
Stretching is dependency-ordered: the request supplies required delimiter
dimensions, then completion consumes the actual stretched glyph box.

Fraction acceptance uses 288 paired pinned Source inputs in one native test,
with **10,944 assertions**. It covers ruled/atop/bevel, style, alignments,
delimiter padding, unequal scale/rscale, L/R and signed metrics. Percentage
content width and child wrap widths preserve their different Source arithmetic
order; the first draft incorrectly conflated them and was corrected before
native acceptance.

Radical acceptance uses 192 paired pinned Source inputs in one native test,
with **11,424 assertions**. It covers sqrt/root, assembled/fixed surds,
different rule/surd thickness, wrapper scale and signed/unequal child metrics.
The result preserves the Source distinction between measured height using
rule_thickness and placed height using surd_height*wrapper_scale, as well as
the temporary BBox's reset scale/L/R fields. Requests retain independent owned
inputs after caller mutation.

Both services compiled and their focused native runs returned 0. Ignored logs
are build/math-port/native-raii-component-17.log and component-18.log. Witnesses
in the reviewed candidate packets execute actual Common/SVG methods with
pinned TS/CJS hashes. These are component results, not full wrapper, parser,
font stretching, 150-equation SVG or Folia integration acceptance. Full math
evaluation remains stopped pending the diagnosed font initializer stack fix.
