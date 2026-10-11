# P23h1 Root frozen predicate restoration

This is a four-body packet: AbstractMmlNode::isEmpty in mml_node.cpp,
AbstractMmlTokenNode::isEmpty in mml_branches.cpp, MmlMrow::isSpacelike and
MmlMrow::isEmbellished in mml_nodes/mrow.cpp. No other body or declaration is
authorized. Core/coreMO/notParent and Return32 binding remain a subsequent packet.
Current complete-core failure and historical 54/246 tally remain unchanged.

## Architecture, identity and implementation duties

The actual MmlFactory owns products and selects registered concrete classes.
One ChildArray carries hole/Undefined/Null/prototype/Get semantics. No dense
compatibility, semantic shortcut, second child/parent track or replacement test
node algorithm is permitted. All four predicates stay in their original class
responsibilities and use existing TreeCallGuard for recursive native bounds.

Each original for-of captures the child array once, tests live length at every
iteration, and gets each yielded value including holes and prototype values.
It must not use forEach, snapshot length, omit prototypes, or re-read a rebound
this.childNodes during iteration. Root initially froze an incorrect generic
iterator message; actual original gold and the independent delivered comparison
show `object null is not iterable (cannot read property Symbol(Symbol.iterator))`.
The corrected bodies preserve that TypeError. Gold is unchanged. Child reads/calls
are virtual and ordered as in Source.

Base isEmpty skips falsey Undefined/Null; present nodes call isEmpty and stop
immediately when false. Token isEmpty does not use a child's isEmpty predicate:
every yield must be an actual TextNode family and its virtual getText must be
empty. Hole/Undefined/Null/non-Text nodes return false without a member call.
Use checked native RTTI preserving TextNode subclasses, then virtual getText;
do not call token getText, which has a separate unimplemented concatenation duty.

Mrow isSpacelike unconditionally dereferences each yield to call isSpacelike,
preserving missing/Null member errors, and stops on false. Mrow isEmbellished
skips falsey yields but advances its source index for them. For a present yield,
read isEmbellished first: a second embellished child immediately returns false;
the first sets core_ to that iteration index. Only a non-embellished child reads
isSpacelike and may stop false. Do not clear core_ at entry: partial writes and
its persistence after a later false invocation are observable original behavior.
An empty/all-spacelike row returns false embellished, true spacelike and true
empty where its actual children permit it. Keep the optional core_ identity.

Luna may edit only these four existing bodies and write
p23h1-predicate-implementation.md. No helper/declaration/other body, test, Source
gold, graph, property/child storage, build, commit or agent delegation may change.
Map original TS/CJS functions and list adaptations/limits; Root owns acceptance.

## Original evidence and actual executable red

The 75 Source299 rows use actual mi seeds a/b/c (all empty). These only establish
negative/falsey/short-circuit cases; b must not be mistaken for mo. Their complete
actual native registry graph compiled and all 75 callable assertions failed from
the actual stubs: build/math-port-mml-predicate-red/baseline-test.log.

Root added 111 independent actual registry probes, gold SHA
`8bc90abd87281a43e77484e90bb0df045f3a896367769dd372b3cc36ce2d6147`.
These use actual mo, mspace, empty/nonempty/emoji TextNode and mi-with-TextNode
instances, positive/negative shapes, holes/Null/Undefined/prototypes, captured
live array callbacks, second-invocation core-cache persistence and Null arrays.
Getter/method hooks call the original registered child's getter/method first,
then apply the source mutation; native hooks likewise call the qualified original
class implementation before the callback. The row observer only reads protected
core_ and does not replace its algorithm. All node identities, sparse array
own/has/Get state, parent state, captured original array, result/error/core and
callback trace are compared. These exact original probes reproduce via
mml-predicate-probe-oracle.cjs --check; immutable native bindings via both
freeze-mml-predicate*.cjs --check.

The same full 89-module / all-unit graph compiles for the supplemental suite.
All 111 actual callable assertions fail from the stubs without crashes:
build/math-port-mml-predicate-probes-red/baseline-test.log. Both native fixtures
are frozen before implementation. Root requires all 186 comparisons and effective
compiling semantic mutants covering each skip/dereference/RTTI branch, callback
order, captured/live iteration and core-cache side effect, plus relevant prior
copy/constructor/append/factory regressions. No whole-file or Folia acceptance
can be inferred from this packet.
