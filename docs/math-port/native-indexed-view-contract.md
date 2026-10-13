# Native indexed views for the existing Context host

Root-frozen interface contract for the OutputJax/FunctionList binding packet.
Implementation and acceptance are outstanding. This extends the one Context
host; it does not introduce a second JS engine or a copied array model.

## Ownership and interface

`options::NativeIndexedView` is an owner-preserving protocol for an actual
native array storage object. It exposes length, own indexed descriptors,
indexed Define/Delete, length Define, and ordered own index keys. Descriptor
absence represents a hole; a present Undefined descriptor is a distinct value.
The provider retains its actual storage owner. No borrowed pointer may be
wrapped in a shared_ptr with a no-op deleter.

`Context::nativeArray(shared_ptr<NativeIndexedView>)` creates one actual Array
object with the normal Array prototype and a provider handle. Context stores
non-index named/Symbol properties normally; it does not store another indexed
element map or authoritative length for this object. The binding registry
memoizes views by native storage identity, and record views by record identity.

Context's Has/Get/Set/Define/Delete/descriptor/ownKeys operations delegate
indexed keys and length to the provider at the same Source operation boundary.
Normal prototype lookup, receivers, descriptor validation, strict errors,
own-key ordering and named properties retain the ordinary Context behavior.
The general host must not contain FunctionList-specific branches. Array
intrinsics must use these same operations rather than read a parallel internal
Record array. Existing ordinary arrays continue to use their current owner.

The protocol lives in the Context declarations/storage boundary. FunctionList
conversion and canonical identity registries live in the new
`util/function_list_bindings` unit. OutputJax consumes that binding; it does
not implement or serialize a second prioritized list.

## FunctionList's native owner

Expose the current `PrioritizedList<FunctionValue>::StorageRef` through its
existing owner. A FunctionList `items` getter returns the memoized view of
that actual storage. Rebinding to another registry-backed storage changes the
current owner; retained old views keep their old owner. Native iteration and
array-view reads observe the same captured storage, live length and record
identity. Do not redirect old views to whichever storage a list now owns.

`item` and `priority` fields bind to the actual retained item record. Native
add/remove, indexed replacement, priority changes and Source iteration must
observe one state. Metadata caches retain identities, not copied field values.

Dense typed FunctionList storage cannot currently represent every JS array
state. Holes, explicit undefined/null/arbitrary records, nonstandard indexed
descriptors, growth that creates holes and foreign-array replacement must
either gain a real canonical representation or throw an explicit uncovered
boundary before mutation. Do not convert them into dense callable records.
Those boundaries prevent full generic PrioritizedList/array acceptance; they
must not be advertised as complete Source behavior. The enabled pipeline and
frozen dense profile still need successful actual comparisons.

## Acceptance and change ownership

The assigned OutputJax owner may implement the protocol in option_types,
option_storage and option_object/array operations, coordinating shared host
ownership with the concat owner. Stop at a stable compile checkpoint and give
Root the exact file list. Root owns module registration and shared builds.

Independent observations must establish repeated view identity, native/view
mutation visibility, ordered keys and descriptors, record identity after
reorder/remove, old-array retention after rebind, and sync/async captured-array
iteration. Ordinary-array host regressions must still pass. Unsupported states
remain explicitly counted as uncovered; no comparator normalization may hide
them. OutputJax's original fourteen rows must call the real Context graph and
cannot substitute typed fixtures for operations that the Source performs on
ordinary objects.
