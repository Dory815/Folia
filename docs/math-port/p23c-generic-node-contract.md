# P23c Root concrete freeze — generic Node migration

This is an in-progress replacement of the sole active child/parent binding, not
an accepted full graph. FactoryChildren now names ChildArray; Node has only
private ChildValue parent_ (initial Null) with virtual getParent/setParent.
TreeJoinGuard tracks the same ChildArray identity. There is no pointer-vector
alias, raw parent mirror, implicit value-to-pointer conversion or old overload.
The full MML consumers and dense observers must still migrate before core-wide
acceptance. Temporary compile failures outside this closed tree graph are not
claimed as passing builds or red evidence.

Root owns declarations in factory_types/node_types/node_factory, the Source
corpus, generated observers, builds, mutations, manifests and acceptance.
Luna math_layout may implement **only** six frozen AbstractNode bodies in
src/math/core/tree/node.ixx: setChildren, appendChild, replaceChild, removeChild,
childIndex, copy; private narrow helpers are allowed if needed. No declaration,
dependency, fields, ownership, other bodies, tests, golds or tools may change.
The remaining walkTree/toString stubs have not yet received their new concrete
red observer and are not authorized in this dispatch. No build/test/commit or
delegation by Luna. Report only in evidence/p23c-generic-node-implementation.md.

All six bodies map to original core/Tree/Node.ts:253–317 (pin fb987178c2d279a99b0db5ea02e751691703955e).
setChildren rebinds receiver to a fresh ChildArray before checking/iterating
the supplied handle; for-of captures input handle but reads its live length and
every indexed Get, including a hole's Undefined; calls virtual appendChild.
append captures current receiver array, pushes tagged input, then dereferences
input for **writing** parent and calls setParent(this), finally returns same
tagged input. The array mutation remains if the write receiver is nullish.

replace calls virtual childIndex first. If found, it writes into the receiver's
then-current array, sets newChild parent, then reads oldChild parent and clears
it iff identical to this; a self-replacement clears parent by this same order.
Do not cache receiver array before childIndex or inspect old parent early.
Missing replacement match returns newChild unchanged, including Null/Undefined.
remove calls virtual childIndex, splices the then-current array, then sets the
input child's parent to Null. A found nullish input is removed before throwing.
Not-found input is returned unchanged. childIndex uses ChildArray.indexOf:
skip holes, distinguish present Undefined/Null, compare exact node identity,
and include inherited numeric slots.

copy keeps existing TreeCallGuard and ignores extra keepIds for generic Node.
Creates through current factory/kind, copies properties with the established
CJS objectAssign lowering (retain PropertyList identity/prototype semantics),
captures source child handle after creation/properties, then iterates live
length. Truthy Node values call virtual copy and destination append; nullish
values/hole reads are skipped. No copied array snapshots, parent adoption or
shared node ownership is introduced. Factory owns products through unique_ptr.

Every Source member read uses requireNode(member); a nullish assignment receiver
uses requireNodeForWrite(member). Parent access always uses getParent/setParent.
Generic empty branch's tagged identity returns remain unchanged. Handle-boundary
null errors must remain explicit; these typed handles do not claim a generic
JS descriptor/species/Proxy implementation.

Root's freeze-sparse-node.cjs selects all 64 generic Node cases from the existing
Source111, including all 27 actual TypeErrors, and generates fingerprints for
length/own/has/tagged value, registered node identity, all parents, exact errors
and method return. It does not port the other 47 NodeUtil cases by test helper.
The diagnostic captured property written by the original JS observer is not
part of this fingerprint; this packet certifies no generic node-valued Property
graph. NodeUtil and complete MML Source299/return32 acceptance remain required.

Root repaired an initial stub-generator syntax error (default argument braces
mistaken for the walk body) before the real red. That failed compilation is not
red evidence. sparse-node-red.cjs then compiled the actual declarations/observer
and ran the real Unimplemented bodies, exiting 1 through failed assertions.
Raw evidence is build/math-port-sparse-node-red/{baseline-build.log,baseline-test.log,red-result.json}.
No implementation expected value supplied this red corpus.

Before full acceptance Root must compile and run remaining walk/join/wrapper
and unchanged dense behavior through migrated observers, all Source111 and MML
cases, semantic mutants, and the complete core graph. No whole Source-file tally
or parser/Common/SVG/Direct2D/Folia gate advances from this partial closed graph.
