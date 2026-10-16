# Gates184–185: restore the complete SVG comparison

This increment repairs the native allocation representation used by
MmlStack.Clear: the new ordinary Context array now exposes a fresh canonical
ChildArray owner through bindNodeArray. Source still sees a new array, its
old array remains independently usable, and later NodeFactory children
consumption uses that exact live owner. The focused test checks both
identities, ordinary array operations, mutation and repeated Clear. The
pinned emitted-CJS witness is retained at
`tools/math-port/regressions/g184/stack-clear-source.cjs`.

This fixes the allocation path; it does not establish that arbitrary plain
Context arrays are accepted by NodeFactory. The general live-view adapter
remains a separate candidate depending on the shared Tree/MML array owner.

Two fixture corrections follow actual production initialization. The Common
repeat-binding fixture obtains its initialized binder from NativeRenderSession.
The 18 direct Stack-field fixtures attach the session's actual MML factory to
their TeX InputJax before TexParser construction, as MathDocument does. Root's
staged diagnostics against Gate183 verify both missing-dependency causes.
Existing identity, descriptor, alias and lifecycle assertions remain intact.

Gate184 compiles the production library and other test targets but fails
the new test TU: Boost.UT eq() does not accept the scoped ChildKind enum.
Root uses the equivalent boolean equality assertion. Gate184 closes with
2,100 unchanged inputs and no runtime evaluation. Its failure is preserved.

Gate185 compiles all 59 targets. All 2,100 inputs remain unchanged through
the complete evaluation. Manifest SHA-256:
`55cda88c9dccfab614f627d15f7133f54c95291b629b964889ced9217f32f3df`.

| Check | Actual result |
| --- | --- |
| Complete SVG/error Source comparison | 150/150 match |
| Public API | 9/9 match |
| Common layout observations | 75/75 match |
| Paired native routes | 146 equal successful Scenes and four equal error pairs |
| Scene-only materialization / XML serialization | Zero calls in all 150 records |
| Package process captures | 25/25 complete successfully |
| Checkpoint comparisons | 18/20 pass |
| Host / Parser / Regexp / LinkedList | All pass |
| Core | Fails: seven raw Stack-field tests, two Scene-parent tests, one Stack-storage observation |
| Output Foundation | Fails: two Scene-parent tests and MML bad-kind ordering |
| Renderer request lifetime / owner-count drivers | Both exit zero; broader owners still accumulate |

The two failed comparisons are tex-next-six and tex-font-packages. Seven of
the 18 raw Stack-field tests now fail actual assertions or narrower exceptions
after the missing MML factory is supplied; the remaining 11 pass. These are
not waived by the passing renderer corpus. The Core direct-storage observation
correctly detects a virtual nodes getter being called where emitted Source
reads _nodes directly. Its independently reviewed correction is applied only
after Gate185 evaluation and is not evidence from this build.

MML products grow from 67 after persistent state setup to 323 after 64 renders;
parser registry entries grow from 561 to 2,673, Lite records from 97 to 481,
and Lite identities from 87 to 471. Wrapper handles, retained layout jobs and
Scene handles stay zero. Both additional drivers verify macro/tag/reference
continuity and retained results; they do not prove complete memory bounds.

The full-SVG executable SHA-256 is
`7a48a65fd8761701210e49a1291898202100dd08c3356d8a138e4133057344a7`.
The independently serialized SVG comparison report SHA-256 is
`0ea6c15b14d97b1b8fc627609a6415e5d2ed88a4d453401956c038e7dae81da5`;
the API comparison report SHA-256 is
`22ce4ade41daf0b24b65c4454b6c34a149e0cfb19abcfc328a2d71fa7b75d6ad`.
The paired-route executable SHA-256 is
`96e11667e87960f56fc27b77496ead9330fa01ecd339e036df8b121f2a84400a`.
Each executable is hashed before and after its run. Source captures and
comparers remain unchanged. Evaluation closes with exit 1 and zero changed
inputs; public SVG/API results are separately recorded as passing.

The tests run in the existing build/core directory. Executable linking uses
/DEBUG:NONE and /INCREMENTAL:NO to avoid new final PDB/ILK caches; compiler
module debug information is retained. Available D: space remains about
102 GB. Folia still uses its QuickJS math worker, and Mermaid remains paused.
