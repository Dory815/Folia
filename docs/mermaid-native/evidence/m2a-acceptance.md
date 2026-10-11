# Root M2a local path acceptance

The complete four source presentation path functions are locally accepted as
typed shared geometry. The remaining renderer functions and full SVG/native
backends remain unaccepted. No Folia dependency or rendering route was changed.

Root froze exact signatures/files/source arithmetic/quantization/error policy in
m2a-path-contract.md before constrained Sol implementation. A reported ambiguity
was resolved explicitly: predicate-only overflow preserves source behavior when
emitted geometry stays finite; nonfinite inputs/emitted coordinates diagnose.
Root reviewed every branch/formula against the pinned source, not only tests.

Independent source harness extracts unchanged points_to_path,
rounded_polyline_path, basis_curve_path and dedupe_points from the checksum-
verified pristine render.rs, verifies exact function set and source hash, then
executes those actual functions. The modified vendor is not an oracle dependency.
Root fixture has 164 cases, three source path outputs each, raw dedupe results,
64 fixed-seed random routes, source epsilon/trim/degenerate/U-turn branches,
empty/single/two-point cases, quantization and opposite f32 extremes. Source
decimal coordinate lexemes are retained as strings to avoid JSON numeric parser
precision or signed-zero loss; native comparison uses exact f64 bit patterns.
Seven source outputs have invalid numerical path data and instead require the
explicit typed InvalidNumber diagnostic. No invalid-source path counts as a
successful native draw.

Final root test build: build/mermaid-m2a-focused.log, two tests passed with all
source cases/coordinates, plus nonfinite inputs/radius and signed-zero checks.
--check reproduces source fixtures. All 12 mutants compile and fail Rust
assertions (exit 101, test result FAILED), with no crash accepted. They exercise
dedupe point/epsilon, degeneracy/trim/curve endpoint/radius, initial/interior/final
basis weights, quantization and radius validation. Final isolated results:
build/mermaid-path-mutations/results.json; build/mermaid-m2a-mutations2.log.
Byte checks confirm every copied working implementation and expected fixture
unchanged. The first harness run's Windows separator map-key exception is
rejected evidence; root corrected the harness key before the final full run.

SVG baseline independently reproduces 470 observations/all 23 families with
Folia dependency versions, source hashes and Windows font fingerprints in
build/mermaid-baseline-check.log. This is an initial baseline, not complete
option/geometry/pixel or Direct2D acceptance. M1 + M2a do not establish a full
compile_scene implementation, SVG scene serializer, text shaper, native device
rendering or production readiness. M3's real Direct2D consumer is authorized
under a separate frozen contract while remaining M2 extraction continues.
