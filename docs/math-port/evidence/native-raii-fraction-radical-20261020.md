# Shared fraction and radical numerical layout

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
