# Root M2d local acceptance — edge-label background geometry

Root read pristine render.rs lines 1770–1966 before freezing the public API,
file ownership and acceptance contract. The unchanged source SHA256 is
026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4.
Sol implemented only edge_label_geometry.rs and its method report. Root
independently compared all ten free-function bodies and LabelRect::from_center;
they match exactly after public visibility adaptation. Only the frozen public
types/fields and PartialEq derive change declarations.

This shared compilation helper determines label-background visibility and
distances using the source's f32 geometry. It performs no diagram layout,
wrapping, shaping, paint resolution or SVG/XML construction. It will feed the
one scene compiler; neither backend repeats these placement decisions.

An independent reference crate executes the extracted complete unchanged source
block, never the native module. Reference function hashes are in the fixture.
Its dependency versions, sources and checksums are checked against the existing
text-reference lock, including packages with multiple simultaneous versions.
An initial offline lock regeneration selected newer cached packages; root
restored the established pins and reproduced identical observations with
--locked/--offline before acceptance. Folia/vendor dependency locks are unchanged.

6,650 source observations cover all 23 diagram kinds and Center/Start/End,
threshold values and adjacent f32 values, varying Sequence heights/clamps,
empty/single/multiple polylines, crossings/tangencies/collinearity, corners,
interior/exterior points, degenerate segments, negative/zero rectangles,
signed zero/NaN/infinities/overflow and deterministic random geometry.
Each helper is tested directly, including from_center. Results compare exact
f32 bits, treating NaN as a category, and exact Boolean outcomes. No distance
tolerance or approximation substitutes for the source. --check reproduces the
whole fixture, and the native complete-source test passes.

22 isolated mutants compile and fail unchanged assertions with Cargo exit 101,
without crash/build failures counted as detection. Mutants cover padding,
nonnegative size, center positioning, short inputs, all diagram threshold
branches, polyline candidates/near-zero return, rectangle-corner candidates,
distance axes, degenerate/projection handling, rectangle edges/inclusivity,
collinear equality, orientation and epsilon-expanded segment bounds. The first
bound-box mutant survived: root added negative epsilon and adjacent-value
source cases, then reran all 22 successfully. Original working source/fixture
bytes were checked unchanged by the runner. Results are in
m2d-edge-label-geometry-mutations.json; ignored logs retain individual builds
and assertion failures.

The relevant native matrix passes 32 tests, including all 10 actual Direct2D
WARP tests, existing scene/path/edge/node/text tests and the new geometry test.
WARP's Arial bytes match SHA256
b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a.
The broader upstream library-test build remains unavailable from the registry
archive: 13 include_str sites refer to comparison fixtures excluded from that
archive. Root's explicit native matrix and pristine helper oracle do not depend
on those files. This is not reported as a passing upstream library suite.

Local complete render-function acceptance increases from 17/70 to 27/70; the
struct helper is additionally accepted. Full diagrams, shared text preparation,
complete SVG/Direct2D backends and Folia wiring remain unaccepted. No production
Mermaid drawing route was changed by this packet.
