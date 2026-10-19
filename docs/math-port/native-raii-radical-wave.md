# Shared radical placement contract

Luna owns canonical `output/common/wrappers/msqrt.ixx/.cpp` and
`mroot.ixx/.cpp`, partitions `folia.math.output.common:msqrt` and `:mroot`,
namespace `folia::math::output::common`. The earlier standalone radical
placement allocation is withdrawn. Current code implements the numerical
portion only; full native wrapper orchestration remains unimplemented.
These files contain the one numerical
implementation of CommonMsqrt/CommonMroot and their SVG placement equations.
No parser, MML, Scene, font repository or SVG writer dependency is allowed.

Inputs own the base outer BBox, an optional degree outer BBox (present for
mroot), named font parameters rule_thickness/surd_height/x_height, displaystyle
and wrapper BBox scale. Preserve scale/rscale/L/R distinctions. Return an
owned `RadicalStretchRequest` containing these inputs, required surd height,
the two requested stretch dimensions and Source rootWidth. Do not fabricate
the selected surd size or metrics. Common font stretching supplies the actual
surd BBox and Source size index (negative means assembled); finishing the
request returns a completed RadicalLayout.

The completed result contains its measured BBox, base/surd origins, optional
degree origin, overbar rectangle, and useful named Source intermediate values
for independent verification (p, q, measured height, placed height, degree
offset/height/padding). Child scene creation and percentage-width scheduling
remain wrapper responsibilities. Do not generate SVG elements or serialized
transforms in this service.

Preserve an easily missed Source distinction: CommonMsqrt.computeBBox uses
H=base.h+q+rule_thickness; SvgMsqrt.toSVG places with
H=base.h+q+surd_height*wrapper_bbox_scale. They are distinct measurement and
painting quantities in the same shared layout result, not interchangeable
constants. Both consumers use the completed result; neither reruns these
equations. Likewise Common's temporary `new BBox(baseOuter)` copies w/h/d but
resets scale/rscale/L/R and other defaults, while placement reads the original
base outer box. A native BBox copy constructor does not reproduce this step.

For mroot preserve getRootDimens/rootHeight, negative-size coefficients,
degree rscale, rootBBox combine position versus placed degree dx*rscale, and
the separate measured/placed H arguments. Negative depths and signed widths
must not be silently normalized. Ordinary sqrt has no degree contribution.

Acceptance requires actual pinned Common/SVG methods executed on paired input
fixtures covering display/inline, sqrt/root, assembled/fixed surds, unequal
rule/surd thickness, wrapper scales, base/degree scale/rscale/L/R, short/tall
and signed metric cases. Compare every returned field with Source, using
identical inputs and exact operation order. Generated native UT assertions
must exercise complete fields, including request ownership after the original
input is changed. Hash the actual TS and CJS methods. Compile/test execution
and canonical promotion belong to Root; write only the assigned ignored draft
packet until reviewed. This component does not establish wrapper integration.
