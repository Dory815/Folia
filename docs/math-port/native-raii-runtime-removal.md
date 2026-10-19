# Generic object runtime removal

The first removal batch deletes these withdrawn utility modules/units:

- `option_types.ixx`, `option_storage.hpp`, `option_object.cpp`;
- `option_array.cpp`, `option_array_iteration.cpp`, `option_iterator.cpp`;
- `option_functions.ixx`, `options.ixx`;
- `context_property_bindings.ixx/.cpp`, `native_identities.ixx`;
- `runtime_modules.ixx`.

These files implemented generic JS records, properties/descriptors, prototype
and invocation behavior, array/iterator objects, identity projection and Context
service ownership. They are replaced by explicit domain types, native handler
registration, request owners and immutable resources. No retirement utility or
generic object runtime replacement is retained.

The typed active `folia_math` build and import graph no longer used this batch
before deletion. Native foundation and core tests passed against that graph.
Historical package/wrapper/binding sources that still import these removed
modules are inactive implementation references, pending their own replacement
or removal. They are not a selectable build or working fallback. Mathematical
package options and handler behavior still require their typed implementation;
deleting the runtime does not count those features as complete.

Earlier local changes to the six modified runtime files were preserved as an
ignored patch and hashes under `build/math-port/withdrawn-context-local*` before
removal. They were not accepted as fixes. Prior committed versions remain in
Git history. Unrelated application changes were left untouched.
