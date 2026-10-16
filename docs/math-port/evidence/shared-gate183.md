# Gates181–183: restored initialization, remaining child-array boundary

Gate181 tests the actual generic Factory bootstrap before Common constructor
registration. Compilation fails because a local `parser` variable shadows a
member function; Root renames the local `parserOwner`. All 2,096 captured
inputs remain unchanged. No runtime acceptance is claimed.

Gate182 compiles all 59 targets with 2,096 unchanged inputs. Public Renderer
initialization proceeds, but every full-SVG case reports `Unimplemented:
Typed intrinsic Property has no canonical Context binding`. The independent
SVG comparison is 0/150. The paired route audit has zero successful Scene
pairs and 150 equal error pairs, so its equality flag is not rendering
acceptance. Both extra Renderer diagnostics now catch exceptions and exit 1
without an uncaught-exception window. Package captures are 17/25 and checkpoint
comparisons 4/20; Core, Host, Parser and Foundation fail, while Regexp and
LinkedList pass. Evaluation closes with exit 1 and no changed inputs.

Root runs an actual marker diagnostic against Gate182 IFCs and library. Its
six initial/simple/macro/tag/reference paths reproduce the same intrinsic
binding error. The new Stack property record prototype returns a typed
default sentinel through the generic Property converter instead of the
current Context's actual Object.prototype. Gate183 maps that sentinel at
the Stack record boundary to the real intrinsic object. It also corrects
generic Factory test expectations against pinned emitted CJS: distinct
receivers have distinct owners; a factory nodeMap stores the class while
node[kind] stores a creator. Released execution leases follow the existing
TypeError contract. Source witnesses are retained under
`tools/math-port/regressions/g183/`.

Gate183 compiles all 59 targets and completes evaluation with all 2,098 inputs
unchanged. Input manifest SHA-256:
`779935acef184246e73c27b25e5ac190afd1fb8faf9a7575e5760add27f6d324`.

| Check | Actual result |
| --- | --- |
| Core | Fails: 18 Stack raw-field tests, two Scene-parent tests and one Stack observation |
| Host | Pass |
| Parser | Fails: one test with three array inputs |
| Regexp / LinkedList | Both pass |
| Output Foundation | Fails: five tests |
| Package process captures | 25/25 complete successfully |
| Checkpoint comparisons | 12/20 pass |
| Independent complete SVG/error comparison | 132/150 match |
| Public API | 8/9 match |
| Common layout | 66/75 match |
| Paired native routes | 128 equal successful Scenes plus 22 equal error pairs |
| Renderer request lifetime | Pass |
| Renderer owner-count diagnostic | Passes its wrapper/job and retained-result checks; other counts grow |

All 18 full-SVG differences report `TypeError: Native NodeFactory children
slot is not a child array`, in both a continuous session and fresh sessions.
They cover matrix, AMS align/tag, AMSCD, braket, cases, colortbl, empheq,
display linebreaking and mismatched-environment inputs. The API difference
is aligned math. The paired Scene audit observes zero materialization/XML
serialization calls in all 150 Scene-only records. Its successful equality
flag does not validate the 18 additional errors against MathJax.

Failed checkpoint comparisons are whole75, api9, physics, colortbl, mathtools,
tag-setoptions, tex-next-six and tex-font-packages. A capture exit of zero
means the process produced observations, not that its observations match
Source. Source captures and comparers are unchanged.

Actual owner observations still grow over 64 renders: MML products 67 to
323, parser registry entries 561 to 2,673, Lite records 97 to 481 and Lite
identities 87 to 471 after persistent state setup. Wrapper handles, retained
layout jobs and Scene handles remain zero. Context slots are cumulative;
this is not a bounded-memory result. The broader owner-retirement candidate
has not been integrated or tested.

After closure, Root compiles the staged Common repeat-binding diagnostic
against Gate183 IFCs and library without rebuilding or copying modules. All
14 assertions in two tests pass. The formal fixture calls registerConstructor
before bindFactoryMethods, whereas the actual service graph initializes the
dependencies first. The corrected diagnostic also verifies repeated binding,
derived prototype/constructor identity and the live jax field. The formal
test correction remains pending; the Gate183 failure is preserved.

Relevant local artifacts are `build/math-port/shared-build-state.json`,
`diagnostic-exits-183-root.json`, `checkpoint-comparisons-g183.json`,
`profile75-full-svg-comparison-g183-root.json`,
`profile75-scene-route-audit-g183.json`, both Renderer diagnostic provenance
files and `common-repeat-g183-run.log` beneath `build/math-port/`.
Evaluation closes with exit 1 and zero post-evaluation input changes.
Folia integration is still deferred and Mermaid remains paused.
