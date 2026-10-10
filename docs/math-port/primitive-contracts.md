# Root-frozen primitive contracts, revision 1

These packets are the first implemented prerequisites, not acceptance of the
unreviewed dependency closure or the full port. The full G0 gate still blocks
broader implementation and production replacement. Root owns this contract,
shared types, fixtures, comparator and build integration. Agents may not change
them. P4/P5 numeric prerequisites are separately frozen in
`numeric-contracts.md`; the independently accepted first packets below keep
their original interfaces. Options is postponed until root freezes its recursive value/identity,
descriptor, callback and symbol-key contracts; it must not use scalar Property
as a substitute for its upstream `any` values.

## Shared storage (root)

`core/tree/property.ixx`, module `folia.math.core.tree.property`, namespace
`folia::math`: `Property` contains Missing, bool, double, string, an opaque
Object.prototype identity or a PropertyList storage identity. The last two are
observable through prototype-name queries; ordinary Node.ts values are scalar.
JS numbers use double. Missing is not false/zero/
empty. `PropertyList` is a shared mutable dictionary handle with own entries and
a prototype handle. Copying a handle aliases it; creating a new list creates a
new object. Keys enumerate own names in JS Object.keys order (uint32 indices
except 4294967295 first, ascending, then insertion order). Deletion and insertion
retain JS ordering behavior. `set` models JS scalar assignment, `setOwn` models
own data property creation, `assign` models Object.assign. Built-in functions
and the __proto__ getter are opaque native identities, never executable JS.
Prototype getter values retain actual dictionary identities. Object-valued
__proto__ setters update prototype handles and reject cyclic chains; the
Object.prototype identity resets that chain. Attribute-layer maps remain live, so mutating
the supplied globals or getAll* handles affects subsequent lookups.

`util/constants.ixx`, module `folia.math.util.constants`: BIGDIMEN=1000000,
from lengths.ts. No other lengths.ts behavior is claimed implemented.

## P1 Attributes

Source SHA256: c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a.
Own only `src/math/core/mml_tree/attributes.ixx` and
`docs/math-port/evidence/p1-attributes.md`. Export module
`folia.math.core.mml_tree.attributes`, import std and root Property module.
Export `INHERIT` as string_view `_inherit_`, and `Attributes` in folia::math.
Constructor `(const PropertyList& defaults, const PropertyList& global)`;
global is shared; defaults own entries are copied to a fresh layer over global.
Build inherited over defaults and explicit over inherited. Do not flatten.

Port every source method under its original camelCase name. `set`, `setList`,
`unset`, `setInherited` return void; lookups return Property. Queries return
bool; name lists vector<string>; `getList`/`hasOneOf` accept
`const vector<string>&`. `getAllAttributes`, `getAllInherited`, `getAllDefaults`,
`getAllGlobals` return PropertyList shared handles. Read methods are const.
get resolves INHERIT through globals once; getExplicit/getInherited/getDefault
do not resolve INHERIT. hasDefault uses prototype presence, not resolved value.
Missing explicit values remain own properties. There is no upstream clone
method; do not invent one. Native object copying aliases handles.

## P2 BBox

Source SHA256: a4232fac0fce35a15ad05a4dbb4af0999bf04f90ec3f0fe0c594b22c5da70b5b.
Own only `src/math/util/bbox.ixx` and `docs/math-port/evidence/p2-bbox.md`.
Module `folia.math.util.bbox`, namespace folia::math. Import std and constants.
`BBoxData { optional<double> w,h,d; }`. `BBox()` and explicit BBox(BBoxData)
preserve omitted fields. Public doubles w,h,d,scale,rscale,L,R,ic,oc,sk,dx and
string pwidth. Public fullWidth="100%"; boxSides records name/index/dimension.
All fields and source methods retained. JS static empty and instance empty
cannot overload in C++: static `empty()` becomes `makeEmpty()`, instance
`empty()` retains its name and returns BBox&. Static zero returns BBox. clean,
rescale, combine(const BBox&,double x=0,double y=0), append(const BBox&),
updateFrom(const BBox&) return void; copy() const returns BBox by value.
Use IEEE double, identical expression order and comparisons, including NaN
width's JS `||0` construction, infinities, negative zero and sentinel equality.
Do not replace conditional comparisons with std::max (NaN differs). Methods
modify only the fields written by upstream; preserve self-aliasing semantics.

## P3a PrioritizedList (Options excluded from this packet)

Source SHA256: a700459f984ba1a5f18bf0e7dc183c242961b94c33587a9aa81bbaacf97180c8.
Own only `src/math/util/prioritized_list.ixx` and
`docs/math-port/evidence/p3-prioritized-list.md`. Module
`folia.math.util.prioritized_list`, import std, namespace folia::math.
Templates `PrioritizedListItem<T> { double priority; T item; }` and
`PrioritizedList<T>`; DEFAULTPRIORITY=5 is mutable static double.
The static property is shared across all T specializations, matching the erased
TypeScript generic class. Use non-template backing storage plus a reference.
add(T item,double priority=DEFAULTPRIORITY) returns T; remove(const T&) returns
PrioritizedList&. For scalar T use JS strict equality (NaN does not equal
itself); registries use shared_ptr/handle T and identity equality, never callback
body equivalence. Same-priority insertions go after existing ties. Removal scans
from the end and removes one last matching item. begin/end support range loops;
iterator stores list identity plus integer cursor, so list mutation during
iteration matches JS's dynamic items.length and items[i++] semantics. Do not use
invalidatable vector iterators. Items are mutable references; the iterator must
retain the yielded record until increment, including removal of that entry
during the body. Repeated dereferences at one cursor retain its identity.
IEEE NaN/Infinity
priorities preserve source `<` behavior.

## Frozen acceptance

Root generates reference operation logs and C++ checks from pinned npm MathJax
4.1.3 using `node tools/math-port/primitive-oracle.cjs`. Generated fixtures are
root-owned. Agents cannot edit that script, fixtures, contracts or build files,
cannot add heuristics or stubs, cannot run shared builds, cannot touch production.
Root compiles and runs `cmd /c build_test.bat "*math_port*" 2>&1`, reviews every
source method and the diff, and separately verifies comparator mutation checks.
Completion reports must enumerate all methods, explicit native adaptations and
uncovered behavior. Green examples alone do not accept a packet.
