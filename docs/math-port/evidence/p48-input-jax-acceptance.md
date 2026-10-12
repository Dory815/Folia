# P48 actual core InputJax acceptance

The native owner implements the pinned AbstractInputJax constructor and all eight
concrete methods/getters. Its compile operation remains abstract, inherited from
the same MathItem compile service. The two independent Luna body assignments
changed only input_jax_constructor.cpp and input_jax.cpp. Root froze the complete
declarations, class/allocation host, Source observations and responsibilities
before body delegation (a7078fe).

Source order and identity are preserved: capture actual instance.constructor once
for CLASS.OPTIONS, merge into one fresh Context object, publish options only after
both merges, and construct the actual pre/post FunctionLists in Source order.
The name getter freshly resolves the current constructor.NAME. Generic adaptor
and actual MmlFactory setters retain provided identity. Empty findMath returns a
fresh array. Filter execution creates one mutable shared record, passes it as the
sole argument to the existing FunctionList, ignores its boolean, and returns the
current data only on successful completion; exceptions propagate unchanged.

Validation:

- The complete declaration/host/stub graph compiled. All initial 29 original
  records failed assertions through explicit unimplemented methods, without a
  runtime crash (build/p48-input-jax-stub-red.log).
- The final 30 original records pass (build/p48-input-jax-baseline.log). Source
  includes original AbstractInputJax statics inherited by a real subclass,
  constructor capture with distinct old/new marker values, live metadata, initial
  field phases, actual option merges and callback aliases, getter exceptions
  during merge, real filters, setter identities, fresh results, shared records,
  mutable data, strict-false stopping, heterogeneous data, and raw throw identity.
- Root corrected one native stimulus to execute filters in a separate statement
  before observing data.value; C++ concatenation operand order is unspecified.
  The original expected result and production body were unchanged.
- Readonly Luna body review found no concrete semantic blocker. Its inherited
  metadata coverage concern was closed by the additional original witness; the
  eventual TeX factory's derived Source-field order remains an explicit gate.
- All 15 independent faults compile and fail original assertions. The campaign
  covers constructor re-reading, shared instance options, skipped merge, early
  publication, shared pre/post lists, stale names, processStrings, setters, reset,
  reused result arrays, copied filter records, stale returned data, swallowed
  exceptions and wrong original static NAME. Restored baseline passes.
- The normal full core build/test passes, exit 0 (build/p48-full-core.log), with
  the accepted P47 and prior math/core regressions. The oracle reproduces with
  --check against locked cloned TS and installed TS/CJS hashes.

p48-input-jax-mutations.json locks constructor, method and class/allocation-host
hashes. This accepts the generic base within the P47 ordinary option-array,
numeric-priority, bound-native-callback profile and P46 single-runtime lifetime.
The Context and binding/module owners must outlive retained native instances;
class registration occurs once per runtime, rather than once per instance.

The base stores an opaque complete generic adaptor without claiming DOM methods.
It does not implement TeX or its finder/configuration/ParseOptions, actual package
commands, fonts/common layout/SVG, or Folia integration. The full original goal
remains active, and Mermaid remains paused.
