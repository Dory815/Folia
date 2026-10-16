# Gates186–187: real field deletion and direct Stack storage

This increment makes native MmlStack methods read their backing _nodes
directly, as pinned emitted CJS does. SourceAccess methods continue to use
the current Context descriptors. Explicit Source dispatch through Size,
First and create remains observable. The original source_75 test now passes.

Lite and Scene native fields now implement actual removal callbacks. A
configurable data property can become an accessor; deletion updates its
actual owner instead of writing an undefined data value. SourceView parent
removal also invalidates its cached parent before deleting the Scene handle
field. Its own-property query follows the canonical Lite descriptor. The
typed Lite projection still fronts the backing owner, so accessor reads
do not recurse into their own native field. Additional tests exercise
parent deletion, redefinition, typed reads and direct owner-view hasOwn.
Pinned CJS witnesses are under `tools/math-port/regressions/g186/`.

Gate186 fails compilation because a Scene options::Value parent field is
assigned the separate Property Missing type. Root corrects the value to
Undefined; SourceView's Property cache still uses Missing. Gate186 closes
with 2,102 unchanged inputs and no runtime evaluation.

Gate187 compiles all 59 targets and completes evaluation with all 2,102
inputs unchanged. Manifest SHA-256:
`f6b6d3e5aa0620127a082627afd6334087f587f694815691683c00e18bb4afb1`.

| Check | Actual result |
| --- | --- |
| Independent complete SVG/errors | 150/150 match |
| API / Common layout | 9/9 and 75/75 match |
| Paired native routes | 146 equal successful Scenes and four equal error pairs |
| Scene-only materialization / XML serialization | Zero calls in all 150 records |
| Package captures / checkpoint comparisons | 25/25 and 18/20 pass |
| Host / Parser / Regexp / LinkedList | All pass |
| Core | Seven raw Stack-field failures and one new dispatch-trace fixture difference |
| Output Foundation | Only MML bad-kind-before-children ordering fails |
| Extra Renderer lifetime drivers | Both pass their existing checks; broader owner counts grow |

All eight Scene-parent tests pass (268 assertions). The previous Source75
direct-storage observation passes. The new Source76 observation has matching
values but an additional First dispatch in native compared with its expected
trace. Root requests an exact CJS receiver/operation witness before changing
that expectation; it is not counted as passing in this gate.

The seven raw-field tests are now diagnosed against the actual library:
StackProperty rejects Symbol lookup during Object.prototype.toString;
other failures involve the post-construction parser stack item identity,
getter-only assignment, sparse array and First/Peek expectations, raw _nodes
identity, and two test lookups of PrimeItem in the wrong module. Candidate
fixture corrections must be independently supported by pinned CJS; none is
included in this evaluation. The shared Tree/MML, general plain-array,
Symbol-key, six-method and broader owner-retirement packets remain candidates.

The complete-SVG executable SHA-256 is
`b1f1a954ff2aef45d016d62fc6c6756cb23189e55726f975b87a56368a7338de`;
its comparison report SHA-256 is
`247ac6283e35f0b8e052b3f984d7b7c44245b061ccd96e9b1910c0b27746c242`.
The API report SHA-256 is
`22ce4ade41daf0b24b65c4454b6c34a149e0cfb19abcfc328a2d71fa7b75d6ad`.
Source captures and comparers are unchanged. Evaluation closes with exit 1
and zero post-evaluation input changes. Folia still uses math QuickJS, and
Mermaid remains paused.
