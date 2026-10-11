# Root acceptance: generic Gradient writes the sole builder

Root reviewed the single delivered push_gradient_stages body against pinned
tiny-skia 0.12.0 gradient.rs. The previous GradientProgram/GradientStage and
independent context/stage owner are removed. Pattern and Gradient now write the
same PipelineBuilder; consuming compile moves its sole context. Generic Gradient
does not initialize radial parameters or mask. Actual wrapper dispatch is a
separate remaining responsibility, not an implicit success in this packet.

## Original and partial-state comparisons

All **20,644** original complete-state records match, with **zero stubs**. This
includes 6,656 capacity-prefix cases, all persistent context groups/force flag,
warnings, stage order and original true/false/panic/color-rejected outcomes.
The actual red was 20,564 stub executions / 20,052 mismatches; rejected colors
and coincident capacity panic snapshots were not individually red records.
The unchanged original fixture reproduces SHA
`a21784340fb84f0c8504832a038fd1e255078017bfe356fcd09bca243d6bd749`.

The prior **13,988 gradient programs**, **1,296 color expansions** and **8,010
power evaluations** pass with their original gold. Their native observer now
calls the sole builder API; its explicit radial parameter writes remain the
same original observer stimulus. There is no old-signature native fallback.

## Mutation and full native matrix

Root rebased the original numeric faults onto this API, replacing obsolete
second-context mutations with inactive-field corruption faults, and added exact
two-stop/general write-order and explicit stage mapping faults. All **39** final
semantic mutants compile and fail assertions. Crashes and build failures do not
count; all working source/fixtures remain byte-identical during the isolated run.
The old gradient-program mutation command now invokes this canonical builder
gate, retaining the historical spec as input for the unchanged numeric faults.
Result: build/mermaid-gradient-builder-mutations/results.json, SHA
`306aa67af1e4a6d544666e32530fe97d898bbbd2ac051bee31b92e704f579d14`.
Baseline log SHA:
`f6d1371fa94abe71deaceadc5d3d298b8d77e41d041c446118ff896347726e8e`.

Final locked/offline native matrix: **22 targets / 65 tests**, including **11
actual Direct2D WARP tests**. Root's first matrix invocation set the wrong font
environment name and failed one font test; that run was rejected. The complete
rerun set FOLIA_SCENE_TEST_FONT to C:/Windows/Fonts/arial.ttf and passed all tests.
Log: build/m3i6c-final-native-matrix-with-font.log, SHA
`34f55b429e23e13dd1f228414674180c127dbbccbfcc9d6c4c9349be25558e78`.

This restores the historical **34/70** rendering tally on the shared builder;
it does not add whole-rendering acceptance. Original LinearGradient/RadialGradient
wrapper setup/dispatch, full shared executor, all three blitter paths, shared
scene paint/text and final SVG/Direct2D/Folia acceptance remain open. Folia is
not connected before the complete backend gate.
