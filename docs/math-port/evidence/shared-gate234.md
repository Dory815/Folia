# Shared gate 234: MML owner fixtures

Build: 61 targets, exit 0, at most four compile jobs. All 2,196 registered
inputs remained unchanged through evaluation.

All 17 tests in `mml_context_allocator_ownership.cpp` passed independently in
`FoliaTests`, including the four corrected fixtures:

- The Source constructor writes `factory`; the fixture now explicitly reads
  the accessor before checking that its getter ran.
- Repeating Source initialization replaces the outer `childNodes` array while
  preserving the same canonical receiver and native product.
- Constructor validity is checked using its Source prototype and canonical
  payload instead of host-internal object classification. The temporary binding
  lease ends before testing release of the last RuntimeModules facade.
- Class metadata fallback is tested with a descriptor acquired while the
  Context projection is alive, avoiding a lookup through an expired accessor.

The pinned CJS witness and draft rationale are available in the ignored
`tools/math-port/drafts/g232-owner-fixture-corrections/` packet. JavaScript class
constructors do not permit literal `.call(existingReceiver)`; same-receiver
retry identity is checked by the native fixture rather than claimed by that
witness.

The complete `FoliaMathHostTests` run terminated normally with exit 1. It has
three assertion failures in two other MML tests:
`bound_native_initialization_converts_lexical_factory_and_constructor_arguments_once`
and `mml_native_operations_use_current_accessor_backed_properties_and_children`.
These remain open; the suite is not accepted. Full output and executable/input
provenance are in `build/math-port/host-g234.txt.*`.
