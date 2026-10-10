# P6 native generic factory contract, root revision 2

This packet ports Factory.ts, NodeFactory.ts and WrapperFactory.ts; Node.ts and
Wrapper.ts implementations are not authorized by this packet. Every class,
field and executable method in these three sources is retained. The pinned
commit is fb987178c2d279a99b0db5ea02e751691703955e. Root verifies source hashes
in factory-oracle.cjs; local dependency-slice review does not pass full G0.

## Frozen native services and lifetime

`factory_types.ixx` is root-owned. FactoryClass is an identity-bearing native
constructor record, with a native creator, real instance predicate and opaque
native static metadata. Metadata is a typed C++ service object supplied by the
later family contract, not a stringified constructor or alternate AST. Class
aliases share the same record. nodeIsKind invokes the registered class predicate
(native RTTI/dynamic_cast for compiled classes); string equality is forbidden.
Replacing a registration changes subsequent construction and kind tests, not
existing product identities. Missing/invalid class RHS raises TypeError.

The factory owns products in unique_ptr storage; parent/child/wrapper references
will borrow from the owning session. Creators use services.adopt before any
virtual initialization, so abrupt initialization cannot leave dangling parent
links to an object destroyed during constructor unwinding. No shared_ptr cycle
between factory and products is permitted. Metadata/callbacks capture borrowed
services or acyclic data. Nodes and wrappers must outlive neither their factory
nor their source tree. Generic services have no Windows/backend dependencies.

C++ virtual calls during base construction cannot implement JS's derived
dispatch. Later Node/MML packets must allocate a complete native object, adopt
it, then perform the mapped source initialization using dynamic virtual methods.
They cannot run setChildren through C++ base-constructor dispatch. This packet
does not invent node behavior to fill that prerequisite.

## Exact exports

`factory.ixx`: export-import factory_types, namespace folia::math::tree,
template<class Product> class AbstractFactory : public FactoryServices<Product>.
Public inline static FactoryClassMap<Product> defaultNodes; mutable defaultKind
string initially "unknown". Constructor:

```
explicit AbstractFactory(std::optional<FactoryClassMap<Product>> nodes=std::nullopt,
                         const FactoryClassMap<Product>* classDefaults=nullptr);
```

An engaged empty map stays empty; nullopt uses injected derived-class defaults,
or base defaultNodes. Injected defaults adapt this.constructor.defaultNodes in
the JS constructor (C++ base constructor cannot discover derived statics).
Defaults are read at construction, not cached globally. Base defaults are
empty per typed product family; arbitrary JS mutation of an erased generic
class across incompatible native product families is outside the raw-TeX API.

create(kind,args={}) overrides the service. setNodeClass/getNodeClass/delete,
nodeIsKind/getKinds/adopt exactly match frozen signatures. Protected
`Product* createDirect(std::string_view,FactoryArgs)` dispatches without default
fallback for NodeFactory. Product callbacks preserve argument values, order and
native handle identities; spans may not escape the synchronous callback.
Source nodeMap is Map order: replacement keeps position, deletion/reinsertion
moves to the end. Initial constructor map is Object.keys order: canonical
uint32 indices except4294967295, then string insertion order; duplicate native
input entries emulate repeated own-property writes, last value/first position.
No unordered_map iteration or lexical kind sorting substitutes for either order.

Creators capture the class record used at setNodeClass. Generic create uses
the requested creator when present, otherwise the current defaultKind creator.
If neither is callable, TypeError. NodeFactory uses direct requested dispatch
even when a default creator exists. getNodeClass missing returns an empty ref.
Deleting a class removes its own creator. Do not erase retained class data from
existing products or silently use kind text for identity.

Revision 2 freezes the independent live objects explicitly. Protected
`FactoryClassMap<Product> nodeMap_` stores only class registrations. Protected
`FactoryCreatorView<Product> node_` is allocated once per factory; public
`creatorView() const` returns that same handle, enabling later MML/Wrappers
getters. Its entries are insertion-ordered own properties, with null refs
representing own undefined. Caller changes to either object do not change the
other. setNodeClass creates a fresh FactoryCreator record whose call captures
the current class ref and borrowed factory, then writes the Map and creator
object separately. Saved creators retain their class after replacement/deletion.
Do not resolve a creator by reading the current class Map at invocation time.

Own null refs mask inherited names and trigger generic fallback; direct create
throws. A non-null creator with an empty call is truthy but non-callable and
throws without fallback. Registering __proto__ updates prototypeCreator and
prototypeAssigned only when no own __proto__ entry exists; an explicit own
entry is replaced normally. Deleting a class also deletes its own creator entry,
without resetting the prototype. Native view mutation uses the typed entries;
arbitrary JS property descriptors/prototype chains are outside this bridge.
Creators and views borrow the factory lifetime and must not escape its session.
Factory copy and move construction/assignment are explicitly deleted: moving
an owning session would invalidate captured service addresses. This is a native
lifetime adaptation; JS object identities never move when references are copied.
Only initial input maps undergo Object.keys ordering; nodeMap_ retains Map
order. No implementation may merge the two objects even when common callers
write them together.

JS's creator object has Object.prototype. Preserve the inherited __proto__
setter behavior: registering __proto__ replaces its prototype with that creator;
deleting the map entry does not undo that prototype; direct/generic create can
still call the retained prototype creator. Explicit own creator entries win.
Other inherited builtin names can produce non-FactoryNode JS values. Native
typed product APIs reject such invalid products with TypeError, rather than
invent a node or use the default fallback. This is an explicit typing adaptation
for a raw-TeX service, not universal JS Factory API equivalence. E9 must verify
required caller kinds; any required non-node outcome invalidates this adaptation.
Unregistered __proto__ is non-callable and throws, never default-fallback.
When __proto__ registration makes the prototype a captured arrow function,
inherited call/apply/bind are truthy invalid typed creators and caller/arguments
are throwing accessors. All five must raise TypeError, without default fallback.
The arrow's length=0 and name="" remain falsy and use generic fallback; own null
entries mask those inherited values. Arbitrary replacement prototype graphs
remain outside this typed service.

`node_factory.ixx`: export-import factory; template<class Product>
AbstractNodeFactory derives AbstractFactory<Product>. Override raw create(kind,
FactoryArgs) with no default argument: normalize the first two arguments
(absent/empty std::any => fresh PropertyList / fresh FactoryChildren<Product>),
discard extras, then createDirect. Also expose the typed overload
create(kind,PropertyList properties={},FactoryChildren<Product> children={}):
a null children handle here means an omitted argument/new empty array, while
raw std::any(nullptr) retains an explicit null for its constructor to handle.
PropertyList and child-array handles forward without cloning; constructors own
the property-copy/parenting behavior. Source defaults occur at each call.

`wrapper_factory.ixx`: export-import factory; template<class Product,class Node>
AbstractWrapperFactory derives AbstractFactory<Product>. Inherit constructors.
wrap(Node*,FactoryArgs args={}) reads the node's virtual kind, prepends exactly
the same native node pointer, forwards every extra arg and calls generic create.
Null node => TypeError before dispatch. No second tree or automatic wrappers.

## Ownership and acceptance

math_layout owns ONLY factory.ixx and evidence/p6-factory.md.
tex_parser owns ONLY node_factory.ixx, wrapper_factory.ixx and
evidence/p6-factory-bridges.md. math_fonts owns ONLY the read-only
evidence/e9-factory-profile-usage.md audit. Root owns all other files, types,
fixtures/oracle, scope, builds and acceptance. Agents report missing prerequisites;
they cannot alter contracts, add files, build, commit, implement nodes, suppress
exceptions, weaken expectations or wire production. Module private helpers must
not be inside export namespace anonymous namespaces. Use import std;.

Root generates tests/math/reference_factory.cpp from actual pinned JS before
dispatch, including null-vs-empty/default registries, order/replacement/deletion,
class alias/inheritance/rebinding, creator args/defaults/fallback, metadata
identity, __proto__ retention and wrapper arguments. Root reviews every source
member, builds, runs frozen tests, and detects deliberate compiling mutations.
Incomplete Node/MML/Wrapper semantics are not accepted by this prerequisite.
