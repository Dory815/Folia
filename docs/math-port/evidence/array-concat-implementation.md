# `Array.prototype.concat` host implementation

The native host installs the canonical `concat` function on the real
`Array.prototype` record and exposes `Symbol.isConcatSpreadable` on `Symbol`.
ConfigMacros continues to resolve and call the method through ordinary
property lookup. The algorithm captures the argument span, boxes the receiver,
performs ArraySpeciesCreate before spreadability checks, and handles the
receiver and arguments in order. Spread items use `ToLength`, `HasProperty`,
and `Get` for each index; non-spread items become own data properties. Values
retain their input identity, and holes remain holes unless inherited
properties supply a value.

The original Node v24.13.0 display oracle has 24 rows. An additive
`array-concat-structured-oracle.cjs` emits tagged slots, own keys, identity
booleans, and traces so its serializer does not require `String(object)`.
Native unit coverage directly checks metadata, one-level spreading, identity,
holes and inherited values, spreadability overrides, species ordering, generic
object receivers, null receivers, getter abrupt completion, and
CreateDataProperty behavior.

Six companion rows remain outside the implemented host boundary: primitive
receiver `toString`/`valueOf`, custom species allocation, and object-to-primitive
conversion for a spread length. Primitive receiver probes actually execute
the source method and observe those operations; their native payload/prototype
checks do not count as parity. The custom-species and object-length rows also
execute the source method. `array-concat-row-map.json` records these gaps and
the current status. A native dump and strict row-by-row comparison remain
pending, so this packet is not yet accepted as complete parity.
