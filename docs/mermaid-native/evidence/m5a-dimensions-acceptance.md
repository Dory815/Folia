# M5a Root shared measurement acceptance

Luna extracted the existing dimension computation into
scene/compile/dimensions.rs. DiagramDimensions is the one six-field f32 type;
render::SvgDimensions re-exports it, and the existing SVG measurement entry
delegates to measure_layout_dimensions. No second dimension algorithm exists.

Root's shared-dimensions-review.cjs compares against committed c3cdf17 render.rs
at SHA-256 026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
It verifies the entire extracted type, methods, ratio helper, constants and
measurement function unchanged except the explicitly authorized names and
visibility. It reconstructs the expected render.rs and checks all surrounding
render source unchanged. Root normalized the agent's mixed line endings to LF.

The actual vendored crate builds with cargo check --locked --offline
--no-default-features. Existing render::tests all pass: 12 executed, zero
failed, 312 other unit tests filtered out. The published crate omitted five
comparison_sources fixtures required by unrelated compiled unit-test modules,
so the initial --lib attempt could not compile. Root restored those unchanged
fixtures from the crate's recorded upstream revision
2f993bd79a55235eb59a34d807852276ba25bea7 into an ignored build copy and ran the
existing renderer tests there. Production source and manifests in that copy
match the reviewed working source; test fixtures and restored artifacts were
not added to the vendored repository. The existing unused as_slice warning
does not originate in this extraction.

Ignored evidence: build/m5a-portable-build.log,
build/m5a-render-unit-restored.log and build/m5a-original-fixtures.json.
This accepts shared measurement extraction only. Converting it to final scene
dimensions/root mapping, complete Layout -> DiagramScene lowering, shared text,
the scene SVG consumer, actual diagram Direct2D acceptance and Folia remain open.
