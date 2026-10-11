# P23h2 Root frozen raw-return and child-zero responsibilities

Luna may restore only seven existing bodies: child_zero and required_child_zero,
AbstractMmlLayoutNode::coreMO and AbstractMmlBaseNode::coreMO in mml_branches.cpp;
MmlMrow::core/coreMO in mml_nodes/mrow.cpp; MmlMstyle::notParent in mml_nodes/mstyle.cpp.
Write p23h2-return-implementation.md. No other helper/body, declaration, test,
fixture/gold, storage/graph, build, commit or delegation is authorized.

## Source architecture and exact typed behavior

Raw core returns remain NodeValue; notParent remains Property. Hole/Undefined and
Null are distinct. ChildArray::get performs the single authoritative own/prototype
indexed Get. The factory owns actual registered products. No dense vector adapter,
null-pointer normalization, additional parent/child track or fallback is permitted.
Retain existing TreeCallGuard for recursive virtual calls.

child_zero preserves raw Get(0), returning Missing for absent/out-of-range/hole
and explicit Null for Null. A Null array fails at its actual indexed read,
`Cannot read properties of null (reading '0')`, not at the later member.
required_child_zero does the same current indexed Get, then requireNode(member).
It serves the already present layout isSpacelike/isEmbellished and base isEmbellished
bodies; those wrappers must not be rewritten. Undefined and Null errors use the
requested member. The unmodified layout/base core wrappers use raw child_zero.

Layout/base coreMO obtains child zero and calls that actual node's virtual coreMO,
returning its raw NodeValue unchanged, including Undefined/Null. Never substitute
the child itself or normalize the virtual result to a pointer.

Mrow core/coreMO first call this->isEmbellished() virtually. If false, or if the
current optional core_ is absent, return this without touching the child array.
Otherwise Get the current array at the current core_ index *after* the getter.
Do not capture an array or index before the getter, confuse index zero with absent,
or cache/reset the getter's side effects. Null-array errors identify the actual
index (including 999); present out-of-range indices yield Missing. core returns
that raw Get result; coreMO requires it at member coreMO and invokes the selected
node's virtual coreMO, retaining any returned NodeValue.

Mstyle notParent is Source `this.childNodes[0] &&
this.childNodes[0].childNodes.length === 1`. Preserve the first falsey raw Missing /
Null result as Property, rather than boolean false. For truthy input perform the
second current Get(0), then inspect that child's array length. Null top array fails
reading 0; Null inner array fails reading length. The containing mstyle array's
own length is irrelevant. Keep both Gets in Source order; do not infer a new
boolean-only API or redefine the node's structural responsibility.

## Original observations and actual red

Original Return32 SHA
`b9e7aefa6c3d0910acd775acb2a63652e1c20287d314f431b9de33dc71ab7ebf`
remains unchanged. Its actual full registry/class callable stubs compiled and
failed 32/32 assertions. Root supplements **155** actual original cases, SHA
`285bc035320086997717fef0e47d8fabb64176fdb385d3d0b7f95f763e70e0bf`:
normal layout/base/raw getters; empty/Null arrays; prototypes; positive mstyle
first-child lengths 0/1/2 independent of containing length; Null inner length;
normal mrow core shapes; virtual embellished getter forcing absent/index 999,
rebinding to Null, returning false with a cached index; child coreMO callbacks
rebinding the target array; and raw virtual coreMO results Missing/Null/another node.

The supplemental fixture registers actual mrow/inferredMrow/mo subclasses for
read-only cache observation and virtual hooks. Getter/child methods invoke the
qualified original implementation before applying the same Source callback.
No observer substitutes the target algorithm. All factory-created node identities,
parent/sparse own/has/Get/current/captured arrays, cache side effects and traces
are compared, including automatically constructed inferred rows.

The complete 89-module / all-unit graph compiles and all **155/155** actual
supplemental callable assertions fail through the frozen stubs, without crashes:
build/math-port-mml-return-probes-red/baseline-test.log and red-result.json.
Original/native generators reproduce these immutable cases before delegation.
Root requires all 187 comparisons, discriminating compiling mutations and existing
predicate/copy/constructor/append/factory checks as relevant before acceptance.

This covers native typed arrays/nodes/index/text domains. Arbitrary JavaScript
slot/property accessors, nonscalar cache fields and foreign property-shaped node
products are not certified; the source operation order is nevertheless retained.
Source299 binding count remains 245/299 and whole-file count 54/246. Full core,
remaining MML, TeX, mathematical layout, SVG and Folia are still mandatory gates.
