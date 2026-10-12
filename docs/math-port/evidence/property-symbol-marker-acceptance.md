# Canonical Symbol-valued marker boundary

This bounded change retains an existing `options::SymbolRef` when a Context
value crosses the native Property bridge. It is required by Begingroup's
actual marker values. It does not implement Symbol property keys, complete
TokenMap Context reflection, or Begingroup package semantics.

`property-symbol-oracle.cjs --check` loads the pinned original TokenMap,
validates its cloned TS and installed CJS hashes, and observes marker
identity, distinct same-description symbols, truthiness, actual CharacterMap
storage/removal, and implicit string/number coercion errors. Its locked output
is `property-symbol-source.json`.

The native bridge validates the owning Context in both directions and keeps
the original handle. Two actual native tests exercise the same boundary plus
foreign-context rejection. `build/math-symbol-marker-run-8.log` records 11
assertions in two executed tests, exit 0. The integrated host gate
`build/math-host-run-8.log` passes 337 assertions in 165 tests. The RegExp gate
also passes 18,518 assertions in 41 tests.

These results do not accept the current full parser/font/package integration.
That integration still has failing candidates and has not completed a new
full core gate. No semantic mutation run is claimed for this change.
