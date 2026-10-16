# Shared MML compilation, Gates199–205

These are failed compilation attempts, not native test evaluations. Their
build processes have terminated and Root closed each gate using its actual
exit code, 1. The before/after input checks report zero changes: Gate199 and
all subsequent gates in this record each freeze 2,157 manifest entries.
Gate199/200 introduce the shared MML/Tree packet and the promoted BBoldx
regression target; the planned graph contains 61 targets. A planned target
list is not evidence that those targets compiled.

The authoritative local records are `build/math-font-package-gate-N.log`,
`build/math-port/gate-N-inputs-before.json` and
`build/math-port/gate-N-input-changes.json`, for each gate N from 199 to 205.
No runtime test or SVG parity result from Gate198 is attributed to this new
working-tree implementation.

## Corrections and remaining compilation work

Gate199 exposes the child-array alias declaration order and a generic
Product conversion that unconditionally requires `toString`. Root moves the
alias before use and requires the conversion before calling it.

Gates200–203 expose dependent-base lookup, duplicated operation fields,
unqualified Context and ChildValue types, Value/ObjectRef confusion,
UTF-16/string overload mistakes, const PropertyList mutation and a childIndex
callback with the wrong arity. Root corrects these interfaces. The pinned
emitted `core/Tree/Node.js` returns the original data from walkTree; the
native raw visitor entry now returns that original data after traversal.

Gate204 reaches the deeper MML and Common implementations. Root corrects
PropertyArray and ChildArray API use, accesses raw properties through the
existing AbstractNode storage owner, and updates derived-node factory calls
to `factoryOwner()`. The Object.assign helper now copies enumerable string
and Symbol keys, so Lite, Stack, AMS and test call sites use the one renamed
interface. The focused test preserves the target `__proto__` Set behavior
and now asserts the enumerable Symbol is copied. A direct Node witness for
that behavior passes; this does not replace the pending native test.

Gate205 reports a CommonFactoryLiveFields signature accepting only the
derived CommonWrapperFactory although exported base constructors also call
it with AbstractFactory<CommonWrapper>. Root generalizes the field-view
owner type to that actual shared base, without a cast or alternate owner.
This correction has not yet been compiled in this record. The MML focused
test additionally contains Value/ObjectRef/optional and temporary-reference
errors; those remain under correction. Gate205 is not accepted.

## Candidate review boundaries

Independent Luna review finds that the empty-node constructor candidate
drops properties/children arguments, Array-method candidates change
observable Get order, and the native addLatex prefix conversion uses a
different primitive hint. These packets are being corrected and are not
part of the accepted implementation. A later empty-constructor overlay also
reverses two Root compilation fixes; Root rejects that overlay pending a
properly rebased incremental patch.

Folia still uses QuickJS for math. The integration review is preparation
only, and finds pending queued-callback ownership and cancellation cleanup
issues. Mermaid remains paused. No completion or whole-renderer memory
bound is claimed.
