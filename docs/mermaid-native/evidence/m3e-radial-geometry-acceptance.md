# M3e root local acceptance — shared radial geometry projection

Root accepts the frozen portable geometry responsibility in
../m3e-radial-geometry-contract.md. It precedes both consumers and preserves
the two centers, two radii, Radial/Strip/Focal distinctions, exact local
matrix and thin-ring replacement instruction. This is not a completed radial
paint, SVG backend or Direct2D gradient rendering acceptance. Mermaid rendering
function acceptance remains **34/70**; Folia integration remains closed.

Root froze the API, types and 4,284 actual-Source observations before Sol
implementation. Sol owned only radial_geometry.rs and its mapping report.
Root independently reviewed Source branch order, inclusive thresholds,
point-distance overflow behavior, transform calls, concentric/Strip/Focal
construction, Focal swap, on-circle scales and failure results. The locked
portable tiny-skia-path 0.12.0 helpers preserve Source precision and operation
order; no geometry equation was approximated for a D2D brush. Root assigns
unused is_well_behaved/is_natively_focal predicates to their later stage
responsibility and removed speculative copies from this module.

The oracle calls unchanged actual tiny-skia 0.12.0 RadialGradient::new in an
ignored Source copy. Its appended read-only accessors observe private fields
and the resulting thin-ring stop sequence, without changing constructor or
matrix bodies. It never links the native preparation module. Source hashes
and the shim-only modifications are checked before execution.

Root coverage review identified the absent Focal on-circle input and added
672 independent actual-Source observations including threshold neighbors.
All initial 4,284 records are asserted unchanged before extension. Final
**4,956 cases / 23,173 native assertions** pass: tags and flags match exactly;
finite/infinite/signed-zero f32 values and all matrices compare bit-exactly;
NaN compares by category. --check reproduces the final committed fixture.
Cases cover empty/single/multiple stops, negative/nonfinite radii, threshold
boundaries, thin rings, all spreads, singular/nonfinite/affine transforms,
both concentric radius directions, Strip, Focal swap and on-circle branches.

All **21** effective independent mutants compile and fail assertions with
exit 101. They cover precheck ordering, missing stops, singular transforms,
threshold inclusivity, thin-ring spread/flag, simple center/radius, concentric
scale, Strip scale, Focal point/swap/flag/radius/scales and matrix basis/order.
Working files and Source observations are byte-guarded; crashes and failed
compilation cannot pass. Results: m3e-radial-geometry-mutations.json.
The preliminary removal of the finite-distance check survived: later mapping
also rejects those nonfinite inputs in this projection. Root retained the
original Source check and excluded that observationally redundant mutant
from effective detections; the preliminary run is not acceptance evidence.

Root's final explicit eleven-target Rust matrix uses --locked --offline,
--no-default-features --features direct2d and the hash-verified
FOLIA_SCENE_TEST_FONT=C:/Windows/Fonts/arial.ttf. **38 tests pass**, including
all **11 actual WARP tests** and the final radial projection. The new dependency
is already in Cargo.lock; only the vendor package's direct dependency list
changes. Log: build/m3e-mermaid-matrix-final.log. Mutation log:
build/m3e-radial-mutations-final.log. Source log:
build/m3e-radial-source-expanded.log.

The unrestricted upstream lib test target still lacks 13 package-excluded
comparison fixtures. The first native matrix attempt omitted the fixed-font
environment and failed one test; neither run is counted as passing. Root then
verified the existing Arial hash and ran the complete explicit matrix with
the required environment. No expectation or renderer behavior was changed.

All eight selected geometry methods are Source-hashed and mapped by the root
ledger. The remaining Source stage methods, Gradient::new/stops, shared text
and COLR preparation, completed scene compilation, GPU gradient sampling,
backend pixel parity, device/application integration and all supported diagram
families remain required. No WinUI or Folia production path changed here.
