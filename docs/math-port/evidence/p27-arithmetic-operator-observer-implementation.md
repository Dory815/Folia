# P27 arithmetic/operator observer emitter repair

This report records native C++ emitter adaptations in the three assigned
oracle scripts. It does not claim fixture generation, compilation, semantic
acceptance, or completion of the six-observer packet.

In `mo-order-oracle.cjs`, the native `children()` helper now constructs the
current `ChildArray<MmlNode>` from an `initializer_list<NodeValue>` and stores
each tagged value unchanged. This is required to represent a missing child
separately from explicit Null. The native `missing-under-embellished-over`
case now puts `NodeValue(Missing{})` in slot 1; the separate null-base case
remains explicit Null. The `Probe::Parent`
override now returns `NodeValue`, matching the current virtual interface.
`Snapshot::isEmbellished` uses the raw parent handle and indexed ChildArray
`set` for slots 1 and 2, corresponding to the Source's direct
`this.parent.childNodes[index]` writes. The generated malformed-accent setups
assign raw parent links through `setParent(NodeValue(...))`. `coreParent()` was
left as a raw `MmlNode*`, as declared by the current Mo interface.

In `arithmetic-oracle.cjs`, the native child helper now creates tagged
ChildArray entries. The `Record::setTeXclass` and proxy `getPrevClass` overrides
use tagged `NodeValue` arguments/results, preserving the tagged child result
and Null/Undefined distinction. The proxy and free hook accept
`m::AttributeList`; no conversion to a copied attribute record was added. The
setup parent chain writes in Source assignment order (r2, r1, r0). The
inferred-child observer uses indexed `get` plus checked node access instead of
vector indexing. The generated TeX-class call stores a `NodeValue`, passes a
tagged previous node, and compares the observed result through
`nodeOrNull()`. The frozen 1,440-row raw inheritance matrix, its value and
attribute pools, and the JavaScript calculations were not changed.

In `arithmetic-order-oracle.cjs`, the native child helper, `getPrevClass`,
`addInheritedAttributes`, hook signatures, and `Record::setTeXclass` now match
the tagged node and attribute interfaces. The snapshot helper retains its
postorder behavior: after reading the node fields and inherited attributes, it
captures the child-array handle and performs a live-length indexed Get loop;
nullish values are checked as a node before recursion rather than skipped.

All changes were confined to native C++ strings and expressions in the three
emitters. I did not change the JavaScript Source prefix, Source calculations,
stimuli, expected values, matrix rows, fixture files, or build inputs. I did
not run the generators, `--check`, builds, or tests, as the packet assigns
those checks to Root. The frozen profiles remain: mo-order, 50 cases/538
assertions; arithmetic, 7 cases/13,501 assertions with 1,440 matrix rows; and
arithmetic-order, 22 cases/56 assertions. After Root's review, I made the
narrow initializer-list and explicit-Undefined correction above. I did not run
the fixture generator or compile gate; Root review and the compile-only gate
remain pending.

Root's follow-up review also identified the source initializer state for the
arithmetic emitted result: the JavaScript local starts as Undefined before
`setTeXclass` is invoked. The native emitted local must therefore be initialized
with `NodeValue(Missing{})`, not Null, so a throwing callback leaves the same
tagged state. My attempted one-line oracle update was blocked by the mapped-file
write error. Root subsequently applied that exact Native-only initializer
correction with an equal-size in-place write and verified its readback.
