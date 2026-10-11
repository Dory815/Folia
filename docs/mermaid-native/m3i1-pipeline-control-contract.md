# M3i1 Root freeze — whole-pipeline compile control projection

Root owns architecture, declarations, Source corpus, tests, mutation matrix,
builds and acceptance. Sol may edit only
`third_party/mermaid-rs-renderer/src/scene/prepare/pipeline_control.rs` and
`docs/mermaid-native/evidence/m3i1-pipeline-control-implementation.md`.
No types, APIs, manifests, tests, fixtures, source observers, commits, builds,
tests, additional delegation, stage sampling or backend/Folia integration.

This packet projects the control decisions of original tiny-skia 0.12.0
`RasterPipelineBuilder::compile` (pipeline/mod.rs 395–480) to one portable
typed plan. Both consumers will receive this same precision/order/tail decision.
It does not port executable stage bodies or claim complete compiler/pixel parity.
No rendering-function acceptance count advances here. Sampling/gamma/blitter,
radial resources, complete shared scenes and SVG/D2D/Folia remain required.

Original full-byte SHA256s:

* pipeline/mod.rs `fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4`
* pipeline/highp.rs `81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416`
* pipeline/lowp.rs `3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88`

`pipeline_control_types.rs` freezes all 79 original Stage ordinals/names and
the Stage/Tail/Return operation variants. No geometry-only precision flag,
alternative stage list, or per-stage mixed precision is allowed. Public
`pipeline_stage_is_lowp` reflects every original lowp STAGES null_fn entry.
It must inspect all original table branches, including non-gradient stages.

`compile_pipeline<C: Default>` consumes its context once, with no Clone bound.
Its ordered input is the complete final Source stage list; shader-only lists
are insufficient for a consumer's final precision decision. Empty input returns
High, width eight, empty full/tail vectors and C::default(), discarding supplied
context before eligibility/force-HQ logic. Nonempty chooses High when force-HQ
or any stage lacks lowp implementation; otherwise Low. Width is eight/sixteen.
Full operations retain input order and repetitions, then exactly one Return.
Tail operations start from that same sequence and replace precisely the five
Source function substitutions: LoadDestination, Store, LoadDestinationU8,
StoreU8 and SourceOverRgba, in either precision. No other operation changes.
Nonempty context moves unchanged, including raw signed zero/NaN bits.

Original ArrayVec capacity is 32: pushing 33 stages panics; compiling 32 stages
panics when appending Return; up to 31 compiles. Match that control failure at
this isolated boundary; do not truncate or return a fabricated usable plan.
The eventual public scene/backend error boundary remains a separate duty.

Root observer executes the original builder push/set-force/compile methods.
Append-only exports label the actual compiled full and tail function pointers
against original functions. The observer reads raw context after original
compile; it neither recomputes selection nor links the native Mermaid crate.
All original Rust files in its ignored copy are byte-guarded. The fixture has
12,872 cases, all 79 single stages, every ordered stage pair, force on/off,
empty/context-reset, lengths 1–33 and representative final shader/paint suffixes.
Twelve outcomes are actual caught Source capacity panics, not sampling failures.
13 context fields observe exact signed-zero/NaN/infinite marker preservation.
Fixture SHA256:
`669acea555dfdc725c4ee09118ffeed0a36bb44b23a7388e90dca23f85566bbb`.
`pipeline-control-oracle.cjs --check` regenerates independently.

Source execution: rustc 1.96.1 (31fca3adb283cc9dfd56b49cdee9a96eb9c96ffd),
x86_64-pc-windows-msvc, unoptimized debug, original default std/simd/png-format
features, no RUSTFLAGS/CARGO_ENCODED_RUSTFLAGS. rustc cfg reports cmpxchg16b,
fxsr, sse, sse2, sse3; no AVX/AVX2/SSE4.1. Stage arithmetic remains unobserved
by this control packet; this profile is not permission to use scalar sampling.

Root's initial Source build failed on an observer-only nonexistent u32x8
to_array method. Root corrected the append-only observer to original bytemuck
cast; this adapter failure was not parity acceptance. Corrected Source ran and
generated the fixture before implementation authorization.

Native stubs built successfully; both tests failed at their named M3i1 stubs,
exit 101. Raw `build/m3i1-pipeline-control-red.log` SHA256
`4185c85169263d5cdb5e10debe744427fd96f8536e5ad8711e93f39fcfbc9b55`.
Final acceptance requires unchanged Source observations, native complete parity,
effective compiling mutants detected by assertion failures (not crashes),
appropriate native matrix and independent Root diff/source review.

Frozen mutation families: missing final-stage eligibility, force ignored,
empty selects Low/keeps context/appends Return, wrong lane width, full reordered/
deduplicated, lost terminal, each of five tail substitutions missing or applied
to full, unsupported/supported table changes, lost nonempty context and capacity
truncation/wrong limit. Agent completion passes no gate.
