# Gate177: request wrapper retirement and Scene child ownership

The shared `build/core` build completes all 59 targets on 2026-10-16 local.
All 2,087 captured inputs remain unchanged through compilation and the complete
evaluation. Gate176 failed compilation at the new test's `Key(u16string_view)`
construction; Gate177 uses an owned `std::u16string` there. No extra build tree
or copied library is used. Gate177's manifest SHA-256 is
`a1a76505d403a029459359ae633a9fe6a07d888af5c562de5456eeead15c4f11`.

Private Renderer requests now retire their completed wrapper identities and
LayoutJob after copying the result, with cleanup on layout errors and
cancellation. Generic NativeRenderSession does not automatically retire them.
Scene children use the current ordinary Context array as their actual owner,
including holes and replacement, rather than a restricted indexed view.
These changes compile; their full acceptance is still incomplete.

Public SVG/error comparison passes 150/150 against the independent pinned
Source capture; API passes 9/9 and Common layout passes 75/75. Paired native
routes retain 146 equal successful complete FrozenScenes and four equal error
pairs; all 150 Scene-only records have zero SVG materialization and XML
serialization calls. All 25 package captures succeed and 18/20 checkpoint
comparisons pass. The next-six/font comparisons still expose MML factory and
StackItem field differences. This evidence covers the locked profile, not all
MathJax features.

Host, Parser, Regexp and LinkedList suites pass. Core has five failing tests
and Foundation six. Direct diagnostics locate a discarded TargetScope in a
test, an incorrect expectation that an escaped ordinary array stops being
readable, and real Lite parent descriptor/projection differences. Those
corrections are pending a later build; these failures are not accepted.

The corrected owner-count diagnostic passes, using explicit A/B tags and
glyph comparisons for macro continuity. The prior Gate175 fixture assumed
automatic numbering despite the profile's default tags mode and compared SVG
strings whose `data-latex` differs. Source witnesses establish the corrected
expectations. Actual Gate177 counts are:

| Owner count | Warm | After setup | After 32 | After 64 |
|---|---:|---:|---:|---:|
| Context object slots, cumulative | 4,273 | 5,977 | 11,897 | 17,817 |
| MML factory products | 13 | 67 | 195 | 323 |
| Parser canonical registry entries | 173 | 409 | 1,209 | 2,009 |
| Wrapper identities | 0 | 0 | 0 | 0 |
| Distinct retained LayoutJobs | 0 | 0 | 0 | 0 |
| Lite records | 32 | 97 | 289 | 481 |
| Lite node identities | 22 | 87 | 279 | 471 |

Context Symbol slots remain eight and Scene handle count remains zero. The
saved result remains readable after Renderer destruction. Context slot totals
are allocations, not live-object counts; MML, parser and Lite owners still
accumulate. No overall bounded-memory claim is made.

The broader registered request-lifetime diagnostic exits 1, including on an
exact repeat with unchanged executable. A separately rewritten diagnostic
passes, but changes how checks and recovery calls execute and cannot replace
the original result. Minimal instrumentation of the original test is the next
diagnostic step; the cause is not yet established.

Executable/report SHA-256 values:

- Public SVG executable: `8e2460a4eefe92cf7670e5f7d6dec3bf47197504335a8e693f83faf4e834bb84`.
- SVG comparison: `82c07cdccc20f75971aa6f6def25a23774e7268dbfe7df0d234ed6d9852cc5d8`.
- Scene executable: `0e504d5c9e046a36a002d36dcbe6061b2920a78544668d591fcbce179d2f5373`.
- Scene capture: `71c3785510d230d4ae74a5fe86e753c60dc499631bed3b5abe71ab26a04efb30`.
- Owner-count executable: `f9903ff1c26d76c2dbea18b25c215d07edd82c192c422a00125afddb47abc545`.
- Registered lifetime executable: `a1db65650c511f69cd84af8e411987eb3440b90d28c3346b01cf4186fcac1cae`.

The complete evaluation, including both extra owner diagnostics, closes with
exit 1 and zero post-evaluation input changes. Folia still uses math QuickJS;
Mermaid remains paused. Renderer acceptance is incomplete.
