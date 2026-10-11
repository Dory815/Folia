# P23l Packet A: script observer native bindings

This packet updates only the native C++ emitted by `tools/math-port/script-oracle.cjs`
and the regenerated `tests/math/reference_script.cpp`. The source-side MathJax
execution and recorded observations remain the authority.

## Native binding changes

The emitted `children` helper now constructs `tree::ChildArray<MmlNode>` from
`initializer_list<NodeValue>`, and the emitted `at` helper returns the tagged
`NodeValue` obtained from `childNodes->get(index)`. This keeps child identity,
explicit Null, Undefined, and missing-slot state in the source value model; it
does not collapse reads to raw pointers or conflate a missing array with an
empty one.

The two native observer overrides that model source indexed assignment now use
`childNodes->set(index, nullptr)`: `Changing::sub` writes slots 1 and 2, while
`Changing::over` writes slot 0. The ChildArray setter records explicit Null and
the original shared handle remains observable. These are the native equivalents
of the unchanged JavaScript assignments in the oracle.

The generated equality expressions still test source `childNodes[i] === null`
through `at(...) == nullptr`; the helper now preserves the tagged result through
that comparison. `ACCENTS` deliberately remains its separate
`shared_ptr<vector<Property>>` service because it is not child storage.

## Frozen observations and verification boundary

Regenerated from the original locked oracle: 1,816 profile records, 1,683
reference assertions across 17 original cases, plus the 6 existing native guard
assertions, for 1,689 assertions across 18 generated cases. The source lock,
JavaScript stimuli, expected observations, assertion/helper categories, and
case names were not edited.

The only executed command was `node tools/math-port/script-oracle.cjs`, which
reported `{ "assertions": 1689, "cases": 18, "referenceAssertions": 1683,
"referenceCases": 17, "mode": "generate" }`. No native build or test was run;
independent fixture review, compilation, and repository gate remain with root.

Root's first compile review rejected two remaining dense-vector expressions in
the captured-array mutation case (`(*original)[2] = late`, generated at lines
1719 and 1739). Both native emitter instances now call
`original->set(2, m::NodeValue(late))`, preserving mutation through the captured
ChildArray handle after `n->childNodes` is rebound. The original JavaScript
mutation and case remain unchanged. The oracle was run again and reported the
same 1,689 assertions/18 cases and 1,683 reference assertions/17 cases. No
build or test was run after this correction; root will recompile.

## File fingerprints

Before this packet, `tools/math-port/script-oracle.cjs` had SHA-256
`33f07ddc6b25e14f1978c0a9ee947eac16c08c869c8e48f445fed574aede6828`, and
`tests/math/reference_script.cpp` had SHA-256
`3cffa89f317c880725821d2485df24cf850f8b0de6832efdbafd1f8251b38716`.
The frozen P10 source lock is
`de99f49b39caeb4138143875b29028464132f3d2937210d54831ea2bbcce9b7d`.
