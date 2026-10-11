# M3f root local acceptance — shared radial stage projection

Root accepts only the responsibility in ../m3f-radial-stages-contract.md.
The existing RadialKind produces p0/p1 and ordered radial stages before and
after color computation, preserving the final vector mask's insertion point.
Radial-only lowp eligibility stays distinct from whole-pipeline precision.
Mermaid function acceptance remains 34/70; complete backends/Folia remain open.

Root froze interface, types, independent actual-Source observations and native
test before Sol dispatch in fa23361. The native test compiled and failed at
the implementation stub before dispatch. Sol edited only radial_stages.rs
and its report. Root reviewed every Source radial/Focal branch, float operation,
inclusive Scalar threshold, separate sign/swapped conditions, stage ordering
and post-color masks. No nonfinite filtering or D2D brush approximation appears.

The Source observer uses a separate pristine tiny-skia 0.12.0 copy with appended
read-only observers. It supplies controlled Source GradientType values and an
identity two-stop wrapper, runs the original push_stages, reads actual context
and emitted stages, and reads the actual lowp function table. All Rust Source
bodies are byte-guarded. Native code is never linked into this oracle. The
initial observer compile failed for an unqualified vec macro; root fixed only
the observer to alloc::vec before generating the frozen corpus. This failed
build is not counted as acceptance.

Final --check reproduces **3,266 Source cases** with SHA256
71cc0ab0cbab59eaee266a1261c8b5e2bee1aa109a6152cd3b5a0bb70e831e23.
Native comparison passes **16,331 assertions**: parameters are bit-exact for
finite/infinite/signed-zero values and categorical for NaN; both sequences and
eligibility match exactly. Raw-kind boundary inputs supplement all M3e conical
outputs. The test does not replace Source gradient stop/paint or pixel fixtures.

All **20** independent semantic mutants compile and fail assertions with exit
101. They exercise zero-radius order, radius max/difference/bias, Strip square,
Focal reciprocal, inclusive thresholds, formula precedence, swapped/sign
selection, Nan/degenerate masks, compensation/unswap, before/after placement and
both wrong lowp flags. Crashes, failed compilation and changed Source fixtures
cannot pass. Working source and fixtures are byte-guarded.

Root's final twelve-target locked/offline Rust matrix with direct2d passes
**39 tests**, including all **11 actual WARP tests**. The existing fixed font is
Arial SHA256 b3658eadae55e682b5f69eb64c439c1ecc8f196c0bb8d4756d145d13bc86476a.
Logs: build/m3f-radial-stages-native.log, m3f-radial-stages-mutations.log and
m3f-mermaid-matrix.log. The unrestricted upstream lib target's existing missing
13 package-excluded fixtures remains a separate limitation; it was not claimed
as passing. The first red attempt omitted Cargo's explicit test registration;
root registered the target and then confirmed actual compilation/stub failure.

Four Source methods are mapped in m3f-radial-stages-members.json. This is a
projection of push_stages' radial responsibility, not acceptance of its still
unported Gradient::push_stages wrapper, mask runtime, sampling kernels or entire
file. Geometry M3e remains separately accepted. Shared stops/color expansion,
full pipeline and precision, actual SVG/D2D radial pixels, text/COLR resources,
completed diagram compilation and app resource/integration behavior remain
required. No production Folia or WinUI route changed.
