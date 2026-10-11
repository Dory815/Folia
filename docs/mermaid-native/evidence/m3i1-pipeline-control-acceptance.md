# M3i1 Root local acceptance — whole-pipeline compile control projection

Root accepts the portable control projection frozen in
../m3i1-pipeline-control-contract.md. It retains all 79 Source stage identities,
the complete lowp eligibility table, whole-program precision/lane width, ordered
full/tail operations and terminal, empty/default-context semantics, nonempty
context movement, and isolated capacity success/panic outcomes.

This is a compile-control projection, **not an executable stage compiler or
sampling/backend acceptance**. The slice API cannot observe the earlier moment
when Source's builder.push rejects its 33rd stage, nor does it reproduce ArrayVec
panic text; both are explicitly outside this projection. Source sampling/SIMD/
color-space/blitter, scene resources and complete SVG/D2D/Folia remain required.
Mermaid rendering-function acceptance remains **34/70**.

Root froze declarations, scope, observer and real compiled stub-red tests in
a2b6623 before Sol implementation. Sol owned only pipeline_control.rs and its
implementation report. Root owns tests/golds, mutation matrix and acceptance.
The original compiler body remains byte-identical in an append-only Source copy;
observers label actual compiled full/tail function pointers and read raw context
after original compile. No native renderer output produces expected values.

Independent Source has **12,872 cases**, including every ordered pair of the 79
stages, force-HQ on/off, empty input, lengths 1–33, shader/paint suffixes and
13 signed-zero/NaN/infinite/raw context observations. Twelve results are actual
caught Source capacity panics. Source fixture SHA256:
`669acea555dfdc725c4ee09118ffeed0a36bb44b23a7388e90dca23f85566bbb`.
`pipeline-control-oracle.cjs --check` independently regenerates unchanged bytes.
Native parity is **64,552 assertions in two tests** (64,315 + 237).

All **27** isolated semantic mutants compile and exit 101 through failed
assertions without process crashes: empty precision/width/context/terminal,
force and whole-list eligibility, unsupported/supported table entries, lane
widths, reordered/deduplicated full lists, terminal, nonempty context, each tail
substitution/only-first/full-corruption and capacity limit/truncation. Production
source and all expected fixture bytes remain unchanged.
Results: m3i1-pipeline-control-mutations.json.

Root reviewed original compile branches and both complete STAGES tables: all
36 null_fn and 43 executable lowp slots match; XYToRadius is eligible, every
other conical/gamma exclusion remains High. Empty takes High/default before
force/eligibility. Both precision paths substitute exactly the five original
tail pointers; order/repetitions survive and nonempty context has no Clone bound.
Native implementation LF SHA256:
`b97bc1abdf7fe452ccfe88ef8122e3a48aa74f7bebb422aa3ee904fb45db02c0`.

The explicit final **15-target/45-test native matrix** passes, including all
**11 actual WARP tests** with the previously pinned Arial bytes. Raw
`build/m3i1-native-matrix.log` SHA256:
`9b41e1435ddafca957573e16e4a4819358cb84fc9d6822fc7a8859ca05df2f13`.
The unrestricted library's previously recorded package-excluded fixture failures
are not claimed passed. No backend/Folia wiring changed, no SVG transit or CPU
raster fallback was added, and no complete renderer/release gate is passed.
