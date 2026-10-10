# P11 shared Property/array host review

Read-only review of root's shared-storage changes. I changed no source,
contract, test, oracle, build, or scope files and ran no builds.

## Storage and MML integration

`src/math/core/tree/property.ixx` stores both `PropertyList` and `PropertyArray`
as `shared_ptr<PropertyStorage>` in the same `Property::Value` alternative.
The `array` tag separates plain records from arrays; `arrayValues` plus
`arrayPresent` preserve holes separately from present `Missing` values. Array
handles and records created by `PropertyList::record` share the same storage,
so nested values and prototype edges retain identity. `PropertyList::object`
continues to reject arrays while `record` accepts either kind. Numeric array
slots, `length`, own-key enumeration, and shallow spread are represented in
that graph.

`PropertyArray::spread()` materializes a missing slot as a present `Missing`
value. That matches array spread's iteration of a hole as `undefined`; callers
must not reuse it to emulate `slice`, which preserves holes. `setOwn("length")`
accepts only numeric `Property` values, and `erase("length")` is a no-op. The
operator contract explicitly calls out the numeric-write adaptation and host
allocation limits; it does not claim arbitrary descriptors or strict-mode
delete behavior.

The MML bridge now matches the upstream `AttributeList` tuple shape more
closely. `MmlNode.ts:39` defines each attribute value as `[string, Property]`;
`mml_bindings.cpp` constructs a dense two-slot `PropertyArray`, and
`unpackAttribute` reads slots 0 and 1 from that same array handle. The old
record-shaped tuple fallback is gone. This preserves the value graph and makes
holes/identity behavior explicit. `mml_node.cpp` builds those tuples during
`addInheritedAttributes` and `inheritAttributesFrom`; this path should remain
covered alongside the already-frozen MML host tests.

## Primitive conversion and array joining

`mml_node_host.ixx` now recognizes arrays in `record`, exposes indexed and
length reads through `get`, and implements `arrayString` with explicit frames.
The join traversal captures each frame's length once, reads slots in order,
emits commas, maps holes/missing/null to empty fields, recursively joins nested
arrays, and suppresses recursive cycles. This matches the default
`Array.prototype.toString` → `join()` path without native-stack recursion.
`host::number` then applies the existing numeric-string conversion to that
primitive string. UTF-16 and numeric helper boundaries remain intact.

The nested-array fast path checks that both `toString` and `join` resolve to
the default array intrinsics. Non-callable overrides fall back to
`[object Array]`; other callable conversions produce an explicit host error.
That is consistent with the documented typed boundary: evaluated functions,
arbitrary getters/descriptors, custom iterators, and global intrinsic mutation
are not represented. The profile audit in `operator-contracts.md` reports no
such overrides in dictionary consumers.

## Concrete consistency findings for root review

1. `PropertyList::has` in `property.ixx` walks only `entries`, then the ordinary
   builtin table. It does not recognize an array's own `length`, present
   numeric slots, or the implicit array `constructor`/`toString`/`join` values.
   Thus `PropertyList::record(array).has("length")` and `.has("0")` can disagree
   with `hasOwn` and `get`. It also fails for those array intrinsics when an
   array appears in another record's prototype chain. This may be outside the
   current operator call path, but the shared record API should not claim full
   prototype-aware `has` semantics until addressed or explicitly scoped.

2. The `BuiltinProperty::arrayPrototype` sentinel is not a shared storage
   handle. `get(Property(array), "__proto__")` returns that sentinel for a
   default array; `host::get(BuiltinProperty::arrayPrototype, "__proto__")`
   currently delegates to a fresh default array and returns the same sentinel.
   In JavaScript, `Array.prototype.__proto__` is `Object.prototype`. Also,
   `PropertyList::set("__proto__", Property(BuiltinProperty::arrayPrototype))`
   ignores the value because the setter accepts shared-storage objects,
   `BuiltinProperty::prototype`, or null, but not `arrayPrototype`. If this
   sentinel is intended to identify the actual Array prototype, its identity,
   parent, and setter behavior need a consistent representation. Current
   dictionary data uses ordinary shared records/arrays, so this is a host
   prototype edge case rather than a mismatch in the generated operator graph.

3. `host::arrayString` implements the default comma-separated conversion used
   by `String(array)`. It does not expose a separator-taking `Array.join`
   operation. Upstream array consumers also call `join('')` to assemble regex
   fragments in `MmlMo` and `join(', ')` when formatting diagnostics in
   `MmlNode`. Those callsites are currently outside this host bridge; their
   future native ports need a separator-aware helper or an explicit typed
   construction, rather than assuming `arrayString` covers every source
   `.join(separator)` call.

The first two findings are concrete gaps in the shared host's general
prototype-aware API. The third is an integration requirement for source
callers, not a defect in default `String(array)` coercion. Root owns any source
corrections and independent acceptance; this review does not accept the host.
