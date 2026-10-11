# P23a Root freeze — sole sparse child-array binding

Root owns architecture, declarations, corpus, tests, builds, mutation cases and
acceptance. Luna math_layout may edit only ChildArray implementation bodies and
private arithmetic helpers in `src/math/core/tree/child_array.ixx`, plus
`docs/math-port/evidence/p23a-child-array-implementation.md`. ChildValue, public
signatures, sole slots_/prototype_ fields, dependencies, tests, fixtures,
manifests and ownership may not change. No builds, tests, commits or delegation.

This is the required storage prerequisite for the full 31-consumer migration in
p23-child-storage-design.md, not a replacement parser or an accepted partial
migration. FactoryChildren still has its old alias during this frozen red test;
Root must replace that single alias and all consumers together before the
sparse-node gate. No runtime selector or mirrored active child store is allowed.

ChildValue is Root-defined Undefined, Null or non-null borrowed Product identity.
Missing defaults to Undefined; nullptr becomes Null. No implicit pointer cast.
nodeOrNull is explicit truthiness/RTTI projection; dereference uses requireNode
and retains the distinct undefined/null TypeError text. No strong node ownership
or shared-pointer node aliases may be introduced by ChildArray.

One vector<optional<Value>> stores both length and own numeric presence. Empty
optional is a hole; engaged Undefined is a present undefined property. Numeric
prototype values use the same ChildArray graph. get/has consult the prototype
even when index is beyond the receiver's own length; hasOwn does not. set grows
with holes and writes one own value. erase removes own presence without shrinking
length. resize preserves existing slots and grows holes or truncates own slots.
push_back appends a present value and returns length; pop_back performs indexed
Get before deletion/shrink and returns Undefined for empty arrays.

slice uses Source relative start/end clamping in the frozen signed-int domain,
captures receiver length, performs HasProperty then Get and makes a fresh plain
array. Inherited values become own values; absent properties remain holes. It
does not copy the prototype graph. splice preserves Source move direction and
HasProperty/Get/Delete behavior, including inherited slots and holes: captured
initial length, relative start, omitted count removes rest, explicit count is
clamped, removed segment is a fresh plain sparse array, item insertion is dense,
then receiver length updates. Do not densify all slots or rebuild via get alone.
indexOf captures length, normalizes its signed fromIndex, skips absent properties
and compares exact ChildValue tags/identity; present Undefined can be found.

forEach captures length once, checks live HasProperty before each Get and calls
the callback with exact current value/index/receiver. Append beyond captured
length is not visited; deleted absent slots are skipped; inherited slots count;
shrink/fill changes remain observable. Retain a shared self handle over callbacks
when shared-owned so receiver rebinding cannot destroy the in-flight array.
setPrototype rejects a cycle before mutation; nullptr removes it. No second cache,
deferred node deletion, iterator materialization or alternate dense storage.

The binding domain is the numeric-index ordinary child arrays used by the source
tree, with bound data prototypes/callbacks and signed relative indices; generic
JS exotic descriptors/species/non-index named properties are not bound here.
This native typed domain does not waive any source behavior required by Folia's
enabled TeX packages. Root's later migration/oracles must cover the actual call
domain rather than infer full generic JS-array compatibility from this packet.

Root actual Node 24.13.0 Array corpus: 8,256 cases covering four slot states,
numeric prototype data, get beyond length, growing set, deletion, resize, push,
pop, slice/splice starts/ends/counts, indexOf and callback mutations.
Source SHA256:
`9533e7b450232aa81dcf06b119093bc67a062a18d5def5b081b4a0d7ca7378c0`.
child-array-oracle.cjs --check independently replays it; freeze-child-array.cjs
--check confirms native input/expectation generation. Native code supplies no
expected value. Three native tests additionally check exact tags/identity/errors,
prototype-cycle rejection, alias/rebind and callback receiver lifetime.

Root fixed initial declaration-import and observer compile errors before the
real red test: option_types needed explicit export; test argument shadowing and
Boost.UT tag-name ambiguity needed repair. These failed builds are not red tests
or passing evidence. The corrected core build completed and the two Array tests
failed through their real Unimplemented stubs; ChildValue checks executed.
Raw evidence is build/p23-child-array-real-red.log. Sol/Luna must not alter Source
gold or observer code to make a proposed implementation pass.

Root mutation families: hole/present Undefined/Null conflation, lost prototype
lookup beyond length, hole growth/deletion/shrink, wrong pop ordering, slice
densification/prototype copying, splice movement/removal/insertion/count/length,
indexOf skipping or matching holes, forEach live length or stale presence/value,
receiver lifetime loss and prototype-cycle acceptance. Mutants must compile and
fail meaningful assertions; crashes/compiler failures do not count as detection.
Acceptance requires Source/native parity, effective mutations, root Source/code
review and appropriate core build. This host packet alone advances no complete
MathJax Source-file count and passes no sparse-node/parser/Common/SVG/Folia gate.
