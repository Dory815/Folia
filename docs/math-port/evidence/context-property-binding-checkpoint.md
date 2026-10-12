# Live Context record to Property checkpoint

This Root-owned host adaptation preserves one graph at the typed parser/MML
boundary. `ContextPropertyBindings::from(Value)` returns a canonical view of an
existing Context record or array. Bound Property storage has no field entries
or array slots of its own. Enumeration, Get, assignment, definition, deletion,
and prototype access reach the original Context. Nested and cyclic references
retain identity; converting a bound view back returns the original handle.
Canonical functions retain the registered FunctionRef and Context callable.

The actual production adaptation and relevant MML graph compiled into
`build/core/folia_math.lib`. A focused executable linked the existing Boost.UT
runner and actual library, using the compiled
`tests/math/context_property_bindings.cpp` object. It passed **35 assertions in
5 tests**, terminal exit 0, recorded in `build/context-property-check.log`.
The tests exercise alias/cycle identity, live two-way writes, inherited getter
and setter receivers, key order and nonenumerability, array holes and live
length, and an actual NodeUtil/MmlFactory definition with getter side effects
and a shared nested attribute value.

This focused result does not certify the current entire parser target. The
combined build compiled TeX and its parser peers, then exposed the missing
BaseItems definition at link. Adding the real BaseItems implementation exposed
compile errors still being repaired by its owner. No complete TeX to MML,
SVG, or Folia acceptance is claimed here.

Native-created Property records currently require a reverse live binding before
they can be exposed as Context objects. Symbols, object coercion, and mixed
native/Context prototype receivers remain explicit unsupported host boundaries.
They are dependencies to implement, not copied dictionaries or successful
Source parity observations. The parser runtime owns the binding service and
borrows its longer-lived Context and callable service.
