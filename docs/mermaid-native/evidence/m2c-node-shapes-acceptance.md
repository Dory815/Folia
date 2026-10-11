# Root M2c complete node-shape helper local acceptance

Status: complete source shape_svg responsibility locally accepted. M2a/M2b/M2c
now cover 7 of 70 inventoried rendering functions. Full shared diagram/text
compilation, SVG export, Direct2D/Folia release gates remain open.

Root read all source branches, froze m2c-node-shapes-contract.md before Sol
implementation, then reviewed the complete submitted node_shapes.rs. No
algorithm correction was needed. Root clarified non-area line effective fill
and corrected the documented source end line; render.rs remains byte-unchanged.
No production caller or Folia drawing route was switched.

## Independent source oracle

node-shapes-oracle.cjs verifies pristine render.rs SHA256
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4,
extracts exactly shape_svg, and compiles/executes that unchanged source function
with the real pristine NodeShape/NodeLayout/Theme/LayoutConfig. It verifies the
source enum contains the expected complete 18 variants. Dependencies are pinned
to Folia's current lock; the reference harness imports no vendor scene compiler.
Shared root input construction initializes source public data types only.

The corpus has **508 observations: 480 valid outputs and 28 invalid source
outputs**. Each retains original SVG and independently parsed geometry/style.
It covers every branch/default shape, state-start/state-end/ordinary ids, empty/
Unicode whitespace/visible labels, theme/override paint roles, zero/decimal/
large dimensions, zero/signed-zero/decimal stroke widths and odd dashes,
DoubleCircle threshold and painter/style boundaries, cylinder clamps/order,
subroutine dash boundaries, Mindmap individually quantized relative quadratic
operands/corner/divider behavior and nonfinite/negative emitted geometry/paint.

SVG parsing belongs solely to the oracle. Production node_shapes.rs creates
typed geometry directly and formats/parses only individual decimal scalars.
Expected path commands preserve source relative arithmetic and quadratics;
tests compare exact emitted decimal f64 bits, ordered geometries, paint roles,
caps/joins/miter, dash lists/widths, opacities and completed group invariants.
Each valid result also passes SceneBuilder.finish. Invalid source outputs must
return a diagnostic before publishing a shape. Additional tests cover invalid
dash/corner/divider numbers and resource references.

The first root oracle incorrectly treated an unparseable polygon point list as
empty because PointsParser stops silently. Root added independent strict
NumberListParser validation before observing points; both NaN/Infinity cases
are now invalid source output. No native behavior or source expectation was
changed to accommodate the implementation.

## Mutation and native verification

Root isolated node-shape-mutations.cjs baseline compiles and passes. All **20
mutants compile and exit 101 from assertion failures**, with no crash counted
as a detection. Mutants alter radii, state paints, inner-circle inclusion/
threshold/dash/width, cylinder order/body clamp, subroutine dash, polygon slant,
Mindmap divider position/opacity/curve/Close, default shape, caps, f32 default
display and two-decimal precision. Working source/fixture/manifests are checked
byte-unchanged after the run. The committed mutation record contains results.

Root final validation:

- node tools/mermaid-native/node-shapes-oracle.cjs --check: exact 508-row
  regeneration success.
- Source helper parity and invalid-resource/number tests: 2 passed.
- Final feature-on relevant matrix: 29 passed, 0 failed/ignored — 13 immutable
  scene tests, 2 typed-path tests, 2 explicit-symbol tests, 2 node-shape tests,
  and 10 actual Direct2D WARP tests.
- Cargo runs use --locked --offline --no-default-features, and the Windows
  matrix adds --features direct2d; the Arial test font is retained and pinned.
- git diff --check: success.

Ignored evidence logs: build/mermaid-m2c-focused2.log,
build/mermaid-m2c-oracle-check.log, build/mermaid-m2c-mutations.log,
build/mermaid-m2c-final.log and build/mermaid-node-shape-mutations logs.
This matrix revalidates the native consumer's primitive behavior; it does not
claim full diagram pixel parity, complete colored/variable text, device-loss/
worker/cache integration, new SVG export, or Folia acceptance.
