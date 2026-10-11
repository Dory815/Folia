# Root acceptance: borrowed Pattern producer and common builder

Root reviewed the six delivered bodies against pinned tiny-skia 0.12.0. The
builder owns one 32-entry ArrayVec and one PipelineContext; consuming compile
moves that context into the previously accepted compiler. Pattern stores a
borrowed pixmap and parameters, without a separate context or pixel clone.
Production changes are limited to push/push_transform and the four Pattern
methods specified in m3i6b-pattern-producer-contract.md. No Folia integration.

## Original comparisons and ownership

The unchanged original 13,284 records reproduce SHA
`e7f086362d8d8544d0b20ac9face8b2fbc630b70fc3e7a869e0188419e2744c5`.
The initial actual red executed 13,284 constructor stubs, preventing downstream
methods from running; it was not six separate method reds. Native output now
matches all original parameters, stages, persistent context, borrow identity,
warnings and true/false/panic outcomes with zero stub calls.

A capacity-order mutation initially survived because saved opacity and initial
coverage were both 0.25. Root rejected that run and independently generated 648
additional original cases with saved opacity 0.5. This post-delivery supplement
does not replace the frozen gold or count as pre-delivery red. Original outcomes
are 128 true and 520 panic. Its fixture SHA is
`7eab0ddb89f08e435f1ad0352336f57edce7d2825e3ebce704eacaa0da36618d`.
Native complete-record comparisons total **13,932**, all matching.

The new builder also passes the unchanged **12,872** original compiler records
(including 12 expected capacity panics), SHA
`669acea555dfdc725c4ee09118ffeed0a36bb44b23a7388e90dca23f85566bbb`.
A separate native ownership witness checks that consuming compile retains all
three gradient allocation pointers. This witness establishes move wiring rather
than a new numeric Source gold.

## Mutation and matrix gates

Root's runner isolates the real crate and immutable fixtures. All **37** final
semantic mutants compile and fail assertions, covering each delivered method,
capacity write order, transform guards/composition, opacity semantics, quality,
limits/sampler, stage order, gamma dispatch, force flag and consuming ownership.
Crashes or compile failures do not count. The first clone mutant failed the
ownership assertion but concurrent panic-hook suppression hid its diagnostic;
Root rejected that run and serialized the test invocation. The final run uses
`--test-threads=1` and requires assertion diagnostics for every mutant.
Working production and fixtures remained byte-identical throughout.

Mutation result: build/mermaid-pattern-producer-mutations/results.json, SHA
`1fe6b75ae8c4a99da2689efebb2a390ea2fd8e789a8ae395cadd483c59668e65`.
The final locked/offline native matrix passes **21 targets / 64 tests**, including
**11 actual Direct2D WARP tests**, with Arial from the Windows font directory.
Matrix log: build/m3i6b-final-native-matrix.log, SHA
`1ba6744fbd6aaa61a159c42b4b8ac39209b0df6004df05df151639552b72cc15`.

## Remaining scope

This packet accepts the selected producer and builder, not whole rendering.
Whole-rendering tally remains **34/70**. Gradient must write into this same
builder; the complete shared executor, three blitter paths, scene paint/text,
final SVG/Direct2D comparison and Folia integration remain open. SVG and Direct2D
must consume the same native scene without an SVG transit path.
