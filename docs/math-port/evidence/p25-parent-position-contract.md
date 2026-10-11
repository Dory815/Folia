# Root frozen Parent and childPosition implementation packet

Luna may replace only AbstractMmlNode::Parent() const and childPosition() bodies
in src/math/core/mml_tree/mml_node.cpp and write p25-parent-position-implementation.md.
No other bodies, declarations, helpers, storage, build wiring, tests/oracles,
gold, pins, scope, Folia, builds, tests, commits or delegation. Root owns acceptance.

Preserve pinned MmlNode.ts responsibility (575-580 and 671-688), SHA-256
cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656.
The existing getParent/Parent/notParent virtual services, NodeValue tags and
ChildArray already provide the required boundary; do not introduce a pointer
compatibility path, parent cache or common simplification of both algorithms.

Parent reads this.getParent once. While the result is a node and its virtual
raw notParent is truthy, virtually call that node's Parent and continue from
its tagged result. Preserve Null versus Undefined, callback exceptions and
getter order. Return the tagged parent unchanged when iteration stops. Keep
the existing shared TreeCallGuard resource boundary for recursive native calls.

childPosition reads this.getParent once; while a node's virtual notParent is
truthy, save that exact node as child, then virtually read its raw getParent.
This method does not call logical Parent or childIndex. If no child was saved,
the searched identity is this. If the final parent is a node, capture its
current child-array receiver and perform for-of semantics: Get every index
(holes become Undefined, inherited numeric values are visible), live length,
strict identity, first match returns its index. Null array raises the original
TypeError `parent.childNodes is not iterable`. Missing parent/match returns
null via optional<size_t>. No precomputed dense index or hole skipping.

Root actual original oracle --check freezes 10,800 calls, 1,836 source errors,
SHA-256 18bd877b63702d5ce65b44019265bb2d55dda120e1e214221386f392d74277b0.
It checks exact result/tag, error name/message, changed raw parent and virtual
read trace for false/truthy/raw notParent, logical overrides, callbacks that
move raw parent or rebind child arrays, sparse/prototype/duplicate slots and
error short circuit. This profile covers ordinary sparse numeric properties;
numeric accessor-property callbacks and cyclic raw parent chains are not added
as a new host contract by this packet.

Root compiled the full actual 90-module graph and verified real red through
both named Unimplemented stubs before delegation. The initial std::string
table compile was deliberately stopped after verifying its exact process/path;
Root switched only static native table fields to string_view. Source profile,
all rows and expected values stayed unchanged. No stopped compile is red proof.
Root requires unchanged Source checks, all 10,800 assertions, effective compiling
semantic faults, relevant base/inheritance regressions and the repository build.
This accepts two methods, not the whole MmlNode file or math engine/backend.
