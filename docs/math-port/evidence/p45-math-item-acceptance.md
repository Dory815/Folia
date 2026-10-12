# P45 core MathItem acceptance

Root accepts core/MathItem.ts's constructor, protoItem, STATE/newState and the
AbstractMathItem lifecycle within the frozen typed native engine profile. This
is a parser prerequisite; complete TeX input, layout, SVG and Folia integration
remain unimplemented.

## Evidence

- 50f2b78 freezes the sole core owner, actual MathItem fields, native engine edges,
  implementation stubs, original observer and 90 original Source records. Frozen
  stubs compile and fail all 90 assertions, zero passed
  (build/p45-math-item-stub-red.log).
- Two Luna agents implement only math_item_state.cpp and math_item.cpp. Root
  corrects one attempted abstract input-service copy to a borrowed reference
  before building. A third Luna independently reviews declarations/host/gold and
  then both stable bodies against the pinned TS and locked CJS.
- Root adds two original observations: rerender resolves the current action
  receiver after virtual state callbacks; typeset stores the successful output
  before a throwing state setter. All 92 original records pass
  (build/p45-math-item-baseline.log). The original oracle reproduces with --check.
- Coverage includes Missing-only defaults, explicit Null, distinct fresh locations
  and data, supplied location identity, strict-null escaped display, fresh metrics,
  state thresholds and NaN/infinity, virtual removal's current state/constants and
  exception prefix, record aliases, reset/clear virtual dispatch, live actions,
  compile and typeset gates/completion constants/return assignment, output receiver
  selection order, nullable services, inherited and own-Undefined state keys, and
  protoItem's actual nested n-offset records.
- All twenty independently injected compiling semantic faults are detected.
  Baseline and restored implementation pass. See p45-math-item-mutations.json and
  tools/math-port/math-item-mutation-test.cjs; agents do not write during this gate.
- Full build_test.bat passes, exit 0 (build/p45-full-core.log), including prior
  MML/factory/stack/input tests and all editor/core regressions. Generated build
  products remain ignored.

Final SHA-256 hashes, unchanged after fault restoration:

- math_item_state.cpp:
  66b947eee517b4fb3c61773e37ba86ae8ede9566b7cb7888011124f25d87bae5
- math_item.cpp:
  228922c462138c6851c63250411cfbc46a68953624e385b27c5acbcc32201cb4
- math_item_host.ixx:
  241a429b401d496b37197afb1b186a65d67f83a3c9e83957f5e0252e8ddab302

## Boundary and next work

MML uses the existing NodeValue graph; typeset output retains its native owner;
locations retain their complete supplied object. Typed borrowed document/jax/
render-action interfaces carry real receivers; they do not provide a substitute
MathDocument or renderer. Arbitrary JS descriptors/functions and concrete browser
DOM insertion/discovery remain outside this base-class host contract.

The target now has 108 module interfaces, a structural count only. The actual
parser-family classes follow p45-parser-module-ownership.md. Before their fields
are frozen, Root must resolve canonical option graphs: accepted util/Options uses
Context/Value while the existing MML/handler Property graph uses separate typed
storage. Session/configuration options must share authoritative objects and reuse
the accepted Source option algorithms without copied option dictionaries.
Complete base/AMS/enabled packages, native font/common layout, SVG parity and
Folia integration remain release gates. Mermaid stays paused and its pending
working-tree files are preserved.
