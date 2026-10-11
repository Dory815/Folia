# P23i NodeUtil child-array methods

This packet implements only `getChildAt`, `setChild`, and `copyChildren` from
MathJax 4.1.3 `ts/input/tex/NodeUtil.ts` (SHA256
`bd7e048c49cf042cb5878c6a73d049ae49e7d6437fb4ba975651dae5a5125cc2`). The
frozen native declarations are in `src/math/input/tex/node_util.ixx`; this
implementation does not claim whole-file NodeUtil acceptance.

`NodeUtil::getChildAt` maps to source lines 191–193. It reads the supplied
node's current `childNodes` handle and performs one indexed `ChildArray::get`.
The returned tagged value retains Node, Null, and Undefined distinctions;
holes and out-of-range positions produce Undefined, while prototype-backed
indices use the array's inherited lookup. A nullish node fails while reading
`childNodes`; a null child-array handle fails while reading the decimal index.

`NodeUtil::setChild` maps to source lines 202–208. It resolves the node and
captures its current child-array handle once, performs indexed Set first, then
tests the tagged child for node truthiness. Only a node child receives the
virtual `setParent(node)` call. Null and Undefined values are still stored as
own slots and do not trigger parent writes. A sparse gap remains sparse; the
method does not append, clone, rebind the target array, clear a prior parent,
or invoke inherited-attribute logic. A nullish node fails before array access;
a null target array reports the current numeric index as a setting error.

`NodeUtil::copyChildren` maps to source lines 216–220. It captures the source
child-array handle once, validates its length before examining the destination,
and uses a live-length indexed loop over that captured handle. Each iteration
performs Get (including holes and inherited indices) and invokes
`this->setChild(newNode, index, value)` virtually. Thus source-array rebinding
does not change the traversed handle, while edits to that captured array can
change later Gets or the loop bound. Each virtual set resolves the destination's
then-current child-array handle. Empty input returns without inspecting a
nullish destination. Exceptions preserve earlier target writes and parent
updates.

The native adaptation uses the shared `ChildValue<MmlNode>` and
`ChildArray<MmlNode>` storage, preserving handles and tagged slot state. A
`TreeCallGuard<MmlNode>` bounds each public entry and virtual dispatch. The
array API accepts the frozen nonnegative integral `size_t` index domain; it
does not model arbitrary JavaScript property keys or foreign node objects.

No build, tests, or acceptance replay were run in this packet. Root owns the
independent source comparison, semantic mutations, and acceptance decision.
