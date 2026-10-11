# M3g root acceptance — shared stop preparation only

Root locally accepts six exact Source stop/color methods under
`m3g-gradient-stops-contract.md`. Complete Gradient::push_stages, color-space
expansion, sampling/precision/masks, radial scene resources, SVG/Direct2D
consumption and Folia integration remain required. Mermaid rendering-function
count stays 34/70; no full release gate passes here.

Root froze the single authoritative f32 stops/flags/mode/transforms boundary
and actual Source corpus in d5d7795 before Sol implementation in 7104cef.
Root reviewed every constructor, loop, numeric branch and field assignment
against pinned gradient.rs and color.rs. The six-member source/target map is
m3g-gradient-stops-members.json; the ledger tool checks declarations, source
identities and frozen fixture. Only those methods are accepted; whole
gradient/color Source files and other color methods remain open. Two initial
descriptive Color line ranges were corrected to the actual pinned positions,
without behavior/interface changes. No backend recomputes these shared decisions.

The independent Source observer appends only read-only observers/exports;
every .rs file equals original plus exact append before/after execution.
It links no Mermaid native crate, and pins Source gradient/color/scalar/float
helpers. Constructor/color acceptance, initial/each-opacity stops, both flags,
spread and both transforms come only from actual Source execution.

The initial **3,050** Source cases were frozen before dispatch. Root added
**192** exact quarter-step inclusive-threshold neighbors while verifying the
entire original prefix unchanged. This exposes `<` replacing `<=` without
changing the reference uniform step. Final **3,242 cases / 382,002 native
assertions** pass by exact f32 bits, including signed zero and nonfinite
transform metadata; 3,222 cases have valid colors and 20 reject invalid color
components. Final fixture SHA256:
db620fb7db4ddfdde7cf47357ed8f9375be0f63d52c8576265be4058e3f6bd77.
`gradient-stops-oracle.cjs --check` independently reproduces it.

All **22** compiling semantic mutants fail actual assertions: color acceptance,
channels/opaque threshold, distinct clamp semantics, overwritten alpha, dummy
omissions/colors, opaque scans, loop start, uniform reference/inclusive
threshold, monotonic bound/previous position, skipped opacity/stale opaque flag,
and discarded mode/transforms. Failed builds, crashes and placeholder panics
cannot count. Source/native expectations remain byte-guarded. Results:
m3g-gradient-stops-mutations.json.

The final explicit thirteen-target locked/offline no-default-features/direct2d
matrix passes **40 tests**, including all **11 actual WARP tests**. The fixed
Arial font is C:/Windows/Fonts/arial.ttf, SHA256
b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a.
Root's first matrix omitted FOLIA_SCENE_TEST_FONT and failed one WARP test with
NotPresent. After supplying/hash-checking the pinned font, root reran the
complete matrix; the failed attempt remains recorded and is not a pass.
The known unrestricted lib target's 13 package-excluded missing fixtures remain
separate and unpassed. No Folia integration is authorized by this packet.

Ignored logs: build/m3g-gradient-stops-{red,parity,mutations}.log and
build/m3g-native-matrix{,-fixed-font}.log, plus named Source/mutation build
directories. The pre-implementation red baseline really compiled and failed.
Source/consumer boundaries contain no SVG parsing or Windows API dependency.
