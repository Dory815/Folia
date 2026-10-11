# Root frozen Layout/Base TeX-class propagation packet

Luna may replace only AbstractMmlLayoutNode::setTeXclass(NodeValue) and
AbstractMmlBaseNode::setTeXclass(NodeValue) in mml_branches.cpp, and write
p25-container-class-implementation.md. No new helper or signature, other body,
declaration, storage, oracle/test/gold, pins/scope, build/Folia, build/test runs,
commit or delegation. Keep existing child_zero/required_child_zero services,
NodeValue and sparse ChildArray as the sole native boundaries. Root accepts.

Pinned MmlNode.ts SHA-256:
cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656.
Original methods at 1153-1156 and 1199-1221 retain their class responsibility.

Layout reads child slot zero and calls its virtual setTeXclass(previous).
Only after the call completes assign the returned tag to previous. Then re-read
the CURRENT child array/slot zero, call virtual updateTeXclass with that value,
and return the original child result. Neither normalize the return nor retain
the old child across callbacks. Preserve read/set/update failure barriers.

Base virtually calls getPrevClass(previous), then sets texClass to ORD through
the virtual setter, then captures current base slot zero. Initialize result to
Null. For a node, short-circuit virtual this.isEmbellished or base.isKind("mi").
If true, save base.setTeXclass(previous), then virtually call this.core() and
this.updateTeXclass with its original tagged return. Else call base.setTeXclass
(Null), ignore its return, test captured base.isKind("TeXAtom") and if true
copy its virtual texClass via the current node's virtual setter. After these
callbacks re-read CURRENT child array and slice(1), then for-of that distinct
snapshot, invoking setTeXclass(Null) only for node values. Retain holes and
prototype values; a callback shrinking/rebinding the original array must not
stop the snapshot loop. Return result when it is a node, otherwise this. The
shared native recursion guard is retained, not a reason to bypass virtual calls.

Root freezes 6,480 original CJS calls (3,888 errors), SHA-256
7ac242e586d6672e30e09e404a90b28930c6513d08da10a94bfa030b91b61a10.
Controlled virtual previous/update/core/leaf services isolate these TWO real
original class bodies and expose dispatch/order, tags, partial setter effects,
base rebind/null-array, tail shrinking/rebind, holes/prototypes and throw sites.
The already accepted base previous/update algorithms are separate regressions;
this controlled service profile does not claim a complete real parser/layout.

Root full 90-module graph compiled and actually executed the two stubs as red.
An initial observer constructor used an unregistered kind alias and failed
before the tested method; Root rejected it and fixed only its native alias.
Initial std::string-table compiles were deliberately stopped and static table
fields became string_view with unchanged Source inputs/expected values/counts.
Only the corrected compiling/runtime stub red is accepted before delegation.
Root requires unchanged source checks, all 6,480 assertions, callback/partial
effect mutants, relevant inherited/base regressions and repository build.
