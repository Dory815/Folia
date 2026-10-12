# P47 live option filter bridge acceptance

The actual Context option arrays now feed the existing FunctionList through one
borrowed collection interface and one appendDefinitions algorithm. They retain
the captured array identity, live length/indexed getters, tuple slot read order,
virtual add dispatch and canonical FunctionRef identity. Root strengthened the
host binding so it accepts only receiver/argument/result converters and always
invokes that one retained native function body.

Readonly Luna review identified static C++ default-argument binding before
virtual dispatch. Root added an original subclass-default witness. The old API
compiled but failed that assertion (build/p47-virtual-default-red.log): direct
and undefined tuple priorities were 5 instead of the subclass's 17. The corrected
optional-priority API preserves omittedness until the actual callee resolves its
default. Explicit priority 3 remains explicit.

Validation:

- The initial shared-loop stub compiled: 6 passed and 12 failed original records
  (build/p47-option-filter-stub-red.log).
- All 19 pinned original records pass; both the option-filter oracle and existing
  403-record FunctionList oracle reproduce with --check.
- All seven independent faults compile and fail the original assertions. They
  cover length snapshots, tuple default freezing, pre-dispatch defaults, bypassed
  virtual add, reversed tuple reads, premature indexed reads and skipped canonical
  function bodies. Restored baseline passes. p47-option-filter-mutations.json
  locks the actual implementation, declaration and host hashes.
- Readonly Luna re-review found no remaining blocker in the specified profile.
- The normal complete core build/test passes, exit 0, including existing native
  FunctionList constructor/execution regressions (build/p47-full-core.log).

Historical FunctionList mutation anchors were adjusted to its shared loop after
the adapter extraction. The historical full mutation campaign was not rerun;
P47's seven-fault campaign and complete normal regressions are the current gates.

This accepts ordinary arrays, optional numeric priorities and bound native
functions. Custom iterators, arbitrary unbound callables/objects/Symbols and
nonnumeric tuple priorities remain explicit unsupported host boundaries. This
packet supplies a real InputJax prerequisite; it does not accept InputJax, TeX
parsing, mathematical layout, fonts, SVG parity or Folia integration.
