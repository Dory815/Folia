# Root frozen NodeUtil child-array packet

Root adds the existing manifest's proposed input/tex/node_util.ixx module and a
separate implementation unit. NodeUtil is a stateless typed receiver; its three
virtual methods preserve the original utility object's this.setChild dispatch.
It is a consumer of the one MmlNode/ChildArray/NodeValue graph, not a tree owner,
parser, layout engine or replacement generic append path. Remaining NodeUtil.ts
functions are unported and this packet does not accept that whole source file.

Luna may edit only the three existing frozen stub bodies in
src/math/input/tex/node_util.cpp, adding their parameter names with unchanged
types. Write p23i-node-util-child-implementation.md. No declarations, helper,
other production file, Source/gold, native test, generator, manifest, build,
commit or further delegation is authorized. Root owns independent acceptance.

## Exact behavior and responsibilities

getChildAt(NodeValue node,size_t position) const resolves the input's childNodes
member, then performs current indexed Get(position). Return raw NodeValue:
hole/out-of-range Undefined remains distinct from Null and inherited slots use
ChildArray::get. Null input arrays fail reading the actual numeric index. A
Null/Undefined node fails first reading childNodes.

setChild(NodeValue node,size_t position,NodeValue child) resolves and captures
the current target child array once, then indexed Set(position,child). Grow gaps
are holes; a supplied Undefined/Null is an own slot. Only truthy node children
receive a virtual setParent(node), after the array write. Do not call appendChild,
replace a target array, clone a node, clear an old parent, infer arity or perform
inheritance. Null target arrays fail at assignment with the current numeric
setting index; Null/Undefined node receivers fail reading childNodes first.

copyChildren(NodeValue oldNode,NodeValue newNode) captures oldNode.childNodes
once, iterates that captured array's live length, and performs indexed Get for
every index including holes. It calls this->setChild(newNode,index,value)
virtually in Source order; each call resolves the target's current array. Do not
snapshot length, skip absent/falsey slots, follow a rebound source array,
prevalidate the new node, clone or replace either array. A Null source array fails
reading length. Empty source succeeds without inspecting even a Null new node.
Callbacks may grow/truncate/delete/inherit slots, rebind either node's array,
or throw after already completed writes; retain those partial effects.

Use existing shared tagged node values and native Error representation. Recursive
callback dispatch can use the existing TreeCallGuard; no extra recursion service,
pointer normalization, dense compatibility store or hidden source of truth.
The finite native indexed domain is nonnegative integral size_t within allocation
limits. Arbitrary JavaScript string/nonintegral property keys and foreign node
objects are not certified by this API.

## Original observations and actual red

Root's 163 observations execute the pinned original CJS NodeUtil getChildAt,
setChild and copyChildren functions on actual registered MML mi nodes. Receiver
hooks invoke the qualified original setChild; parent hooks preserve the original
parent value before applying a controlled callback. Neither substitutes the
target algorithms. Captured/current source and target identities, own/has/Get
slots, lengths, node parent identities, errors, raw results and call traces are
observed. Source SHA:
`660c46a99de4f6cfa2f2ea31838fca3658b38cbdd75c04401c87cca29814d705`.
Set cases use an initially empty target and varied untouched source shapes;
copy cases additionally cover existing target shapes. Additional targeted cases
may be needed if independent semantic mutants reveal a blind spot.

The complete 90-module/all-unit graph compiles and **163/163 actual callable
assertions fail through the real frozen stubs**, without crashes:
build/math-port-node-util-child-red/baseline-test.log, SHA
`291f7bb5e2be2594409038f0acbf994951a21a82c9c318bb0925de7895dd389c`.
Source and native generators reproduce unchanged before delegation. Require full
original comparison, effective compiling semantic mutations and relevant MML
boundary regression before acceptance. Source299's four superscript-first
consumers also depend on inheritance and are not yet accepted by this packet.
The full-core stale-observer build failure and complete TeX/Common/SVG/Folia gates
remain open; whole-file acceptance stays 54/246.
