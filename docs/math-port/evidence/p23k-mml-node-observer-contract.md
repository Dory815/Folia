# Root frozen stale native observer migration

The full-core gate previously stopped compiling at reference_mml_node.cpp. The
production math graph already compiles in the independent 90-module/all-unit
packets; this older test emitter still uses dense vectors, raw parent fields and
raw-pointer virtual signatures. Do not restore those APIs in production.

## Frozen original evidence

Root executed the unchanged pinned actual original oracle with --check: **1,084
assertions / 43 cases**. The initial native fixture SHA is
`a34004baa7ead53796eee175c2e4015aa025af70562487e5273d1c1dac8eac25`;
the initial oracle SHA is
`3a37c9d07a91930c4d1991aada3d762840ddf4855d32f7c7b6e3d2c04b2b9b5c`.
Root's separate Source profile records **1,051 helper observations** from actual
original calls plus 42 original case names. Seven native budget checks and the
remaining direct emitted checks remain separately reviewable in the fixture.
Profile SHA:
`05b8ce98c2e6fa93edac57aac32deba966404e39e240fac042ff55cbf3841a03`.
No native implementation feeds this profile. It must reproduce unchanged after
the observer migration. The older compile failure is evidence of a stale binding,
not a new algorithm red and not an accepted complete MML implementation.

## Exact file duties and authorization

Luna may modify only native C++ emitter strings/declarations/call expressions in
tools/math-port/mml-node-oracle.cjs and regenerate its sole output
tests/math/reference_mml_node.cpp. Write p23k-mml-node-observer-implementation.md.
Do not alter JavaScript original classes, stimulus operations, Source calls,
expected values, case names, helper semantics, assertions/counts, Source pins or
any other tests/golds/tools/build files. No src changes, compatibility API, source
special cases, tests removed/skipped/disabled, builds/native tests, commits or
delegation. Running this one original oracle to regenerate its native observer
is authorized; Root owns independent profile/reproduction/build gates.

## Native binding rules

Use the established sole ChildArray/NodeValue representation. Test-side children
constructors write tagged values into ChildArray; indexed queries use get and
size with explicit node/tag projection at the observer boundary. Live array
stimuli use existing push/set/erase/resize/rebind operations, never vector [] or
begin/end. Keep holes, Undefined and Null distinct wherever Source observes them.

Replace raw parent reads/writes with getParent/setParent(NodeValue), preserving
exact original strict comparison tags rather than silently conflating Null and
Undefined. notParent returns Property. TeX-class/core/replace/append methods and
callbacks use the current NodeValue signatures. A returned node is required or
projected only where the original operation uses its node fields; exact strict
identity comparisons use tagged identity. Use auto for tagged returns, not auto*.
Do not add global implicit conversions or a second parent/child store.

## Root acceptance

Require unchanged Source profile, unchanged 1,084/43 count, reproducible generated
fixture, a review showing only native binding changes and preserved direct
expected literals, then the repository core build/test command. Any genuine
runtime failure in still-unported methods remains visible; do not make the
observer declare them passing. Passing this packet only repairs the gate. It does
not increase the 54/246 whole-file tally or accept TeX/Common/fonts/SVG/Folia.
