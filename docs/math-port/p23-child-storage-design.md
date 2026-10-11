# P23 Root decision — sparse children and native identity

This is Root's architecture decision after the two closure audits. It is not
implementation acceptance. All 31 existing child consumers still use the dense
vector and must be replaced together before the sparse-child gate can pass.
The 54/246 local Source-file acceptance count does not advance here.

## Ownership and one authoritative child array

Keep the existing FactoryServices adoption boundary: a factory/session owns
every complete or partially initialized Product through unique_ptr. Node identity
is its stable Product address within that session. Parent and child edges borrow
that identity; they do not own the node. Node operations and callbacks execute
while the factory/session remains alive. The final backend-independent scene
must own its glyph/path/layout data and contain no borrowed MML node pointers.
No no-op-deleter shared_ptr, aliasing shared_ptr pretending a borrowed node is
owned, parent/child strong-reference cycle, or arena-self-ownership is allowed.

FactoryChildren becomes a shared handle to one typed sparse ChildArray<Product>.
The handle owns the array storage, permitting Source aliases and capture/rebind
semantics; it does not extend Product lifetime. Every present value is exactly
one ChildValue<Product> tagged as Undefined, Null or Node(non-null Product*).
An absent indexed property is an empty optional slot. Length is the vector's
length; there is no separate dense vector or mirrored presence/value store.
Hole and present Undefined both read Undefined but have different presence.
Null is explicit and never inferred from an uninitialized hole. Construction
from nullptr normalizes to explicit Null; callers cannot construct a Node tag
holding nullptr. Strict equality compares tags and then node identity.

This typed Source-array adaptation is the sole childNodes representation, not a
generic JS object interpreter. Option PropertyArray remains the representation
of option arrays. Do not inject an owned NativeObjectRef into Property just to
smuggle a factory-borrowed node into it. TeX NodeUtil binds its typed MML values
directly. A later genuinely generic node-property consumer must get an explicit
ownership/operation binding, rather than a fake owned reference.

## Files and responsibilities

* `src/math/core/tree/child_array.ixx`: tagged borrowed ChildValue and sole sparse
  ChildArray storage/API. No node algorithms, attributes, parser or renderer.
  Numeric inherited slots, when bound, use the same typed array prototype graph;
  no parallel generic property mirror. Prototype cycles must reject before write.
* `factory_types.ixx`: the one FactoryChildren alias. Factory adoption and node
  ownership stay here; no sparse-array algorithm is added to the factory facade.
* `node_types.ixx`, `node.ixx`: port original AbstractNode and EmptyNode operations
  against tagged values, preserving Source's write-before-nullish-dereference
  order, partial state on errors, identity comparisons and captured/live lengths.
* `node_factory.ixx`: normalize exactly the new typed child handle in FactoryArgs;
  wrong std::any shape/null array handle stays an explicit boundary error. No
  dense-vector overload or old/new dispatch path remains.
* `wrapper_types.ixx`, `wrapper.ixx`: preserve missing child-list property versus
  present null handle and captured handle/live iterator behavior.
* MML node/base/helper files listed in the closure audit: translate each original
  child read, truthiness check, dereference, direct write, splice and rebind at its
  own Source responsibility. Inferred rows and finite arity repairs keep their
  original dispatch/order; no generic forgiving child helper masks errors.
* TeX NodeUtil: its direct setChild grows an array and sets parent only for truthy
  child values. copyChildren reads every index and writes every target index,
  deliberately converting holes to present Undefined. It must not call the
  semantically different AbstractNode appendChild/setChildren operations.

The exact 31-file consumer inventory and 19 existing test-file inventory are in
`evidence/p23-sparse-identity-closure-audit.md`. No agent may redesign these
responsibilities or migrate just a convenient subset while claiming acceptance.

## Frozen behavior requirements for the upcoming code contract

The ChildArray API must provide indexed Get, own/inherited presence, indexed Set
with hole-producing growth, Delete without length shrink, length resize, push,
pop, slice, splice and indexOf with Source hole/prototype/identity behavior.
Slice preserves absent properties; splice copies/moves presence and inherited
values in Source order; indexOf skips absent properties and includes present
Undefined. Bounds and negative start/end conversion are specified by the actual
Source callsite numeric domain, not accidental C++ signed/unsigned conversion.
Node algorithms choose Get/truthiness/dereference explicitly; there is no implicit
ChildValue-to-pointer conversion that collapses Undefined and Null.

An array handle captured before node.childNodes rebinding continues to refer to
the old array. Source for-of reads Undefined for a hole and checks the iterator's
live array length; Source forEach captures length and skips absent properties.
Neither is replaced globally by a C++ range loop with a different mutation rule.
Source join captures length, reads indexed values in order, uses empty text for
nullish child values and retains the existing cycle guard. Methods that fail
after indexed write/append must leave exactly the Source partial array and parent
state, including the distinct error source for Undefined versus Null.

Root must write the concrete declarations, actual pinned Node/NodeUtil/MML oracle
and tests, compile the red stubs, then allocate disjoint implementation files.
Luna is not authorized to invent that API or its expected values. The current
document fixes ownership/storage/responsibilities; it does not substitute for
the concrete declarations or authorize implementation on its own.

## Acceptance and separate regex boundary

Root's corpus must include superscript-first msubsup [base,hole,sup], subsequent
subscript writes, explicit nulls in enabled packages, copyChildren's hole to
Undefined transition, aliases/rebinds, callbacks that append/delete/resize/rebind,
partial append/set/replace failures, copied/inferred rows and arity repairs.
Compare length, own presence, tagged indexed value, exact node identity, parents,
errors and callback order against actual pinned Source. Existing dense behavior
corpora stay unchanged in expected values; their native test helpers migrate to
the new sole representation. Mutations must distinguish all four states and
detect lost live reads, order, identity, partial writes and presence movement.
Finish with the complete core tests. No parser/Common/SVG/Folia gate is passed
by a standalone sparse-storage test or a partially migrated consumer graph.

The RegExp audit is a different ownership graph: RegExp is a genuinely owned
identity-bearing value, must flow through canonical Property/config storage,
and must preserve lastIndex, flags, coercion and actual match/test/exec/replace/
split protocols. Adding a node slot does not solve that graph; P22b's controlled
test service does not accept the engine. Source-reachable boolean/string option
replacements must retain their String.match coercions. Known candidate engine
mismatches remain required repairs, with no QuickJS fallback or weakened corpus.
