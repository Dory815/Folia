# Ordinary Array host implementation

This implementation adds the native Array.prototype callables used by the
enabled TeX source: slice, reverse, includes, push, pop, join, fill, and find.
Existing accepted map, forEach, indexOf, and filter bodies remain untouched.
The methods are registered on the existing ordinary Array prototype, so
callers resolve and invoke the actual current property through Context::get
and Context::call.

The Node oracle at tools/math-port/parser-array-host-oracle.cjs records 69
observations against Node 24.13.0. The generated source record is
parser-array-host-source.json. Its nativeCoverage ledger maps every source
observation to a candidate case in parser-array-host-reference.cpp. Sixty
cases have direct native matches; nine are explicitly marked as unsupported
object conversion or custom species boundaries. The observations cover
metadata, holes, inherited values, callback arity and receiver, live element
reads, constructor and argument coercion order, sequential mutations, and
TypeErrors/RangeErrors.

Source callers include Configuration.ts:361 and :439 for packages slice,
reverse, and parser includes; MapHandler.ts:79, :164, and :166 for reverse,
push, and join; StackItem.ts:148, :155, :193, and :560 for node-array push,
pop, slice, and join; and Tags.ts:330, :345, :347, and :619 for history
push, pop, and find. Other enabled package callers use the same ordinary
methods for row, list, and option arrays. These remain normal dynamic method
calls.

reverse follows the per-pair Has/Get order and the three source Set/Delete
branches. includes reads holes with Get and compares with SameValueZero.
push snapshots only its incoming native span, then Sets each value in order
and Sets the final length; pop performs Get, Delete, then Set length.
join captures length, converts its separator before reading elements, and
maps nullish elements to empty strings. fill converts start then end after
capturing length and writes each index in order. find validates its callback
after length capture, Gets every slot including holes, and calls with three
arguments and the supplied receiver. slice captures length and converts
start/end before reading constructor; it preserves holes and value identity
for the default native species.

Numeric conversions use the canonical Context::toNumber bridge backed by the
existing object host conversion. The actual Symbol.species identity and the
native Array species getter are used when slice reads constructor species.
Custom species construction and object-to-primitive conversion remain
explicit unsupported boundaries. The host resolves the actual
Symbol.species identity and native Array species getter; primitive join values
and symbols use corresponding native behavior.

Root compiled the production implementation and linked the candidates into
`FoliaMathHostTests`, which passed together with the iterator, previously
accepted map/iteration, and live Context Property suites: 239 assertions in
149 tests. The source record's nine unsupported rows remain outside matching
behavior acceptance; their tests verify the explicit boundary only.

The shared fault run compiled, detected, and restored all 33 method-scoped
faults across Array/Object/iterator hosts. Its result is saved in
`parser-host-native-faults.json`; the original fixtures and source hashes were
unchanged after restoration. This accepts the bounded host observations, not
the complete TeX parser, math layout, SVG output, or Folia integration.
