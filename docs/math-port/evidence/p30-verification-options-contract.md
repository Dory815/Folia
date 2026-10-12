# P30 Root verification options coordinate

This contract fixes the representation boundary identified in
`p30-verification-options-boundary-audit.md`. It does not accept verification
algorithms. The pinned Source has method-local default evaluation and distinct
undefined/null property-read errors, including observable earlier factory writes.

## One options value

Use existing `Property` for every verification/error options argument:
`verifyTree(Property options=Missing{})`, `verifyAttributes(Property options)`,
`verifyChildren(Property options)`, and
`mError(std::string message,Property options,bool shortError=false)`.
The public and all overriding `verifyTree` declarations use the same Missing
call-site default. A C++ virtual default is statically bound, so Source defaults
must be applied inside the actual implementation. Only base `AbstractMmlNode`
and `MmlMath` convert Missing to local `PropertyNull{}` at entry because only
those Source methods have `= null`. A qualified base call normalizes its own
copy; it must not rewrite the derived caller's parameter. Empty, HtmlNode,
mprescripts and none have no local conversion. `mError` has no options default.

Remove `ErrorOptions` and optional-PropertyList verification parameters rather
than retaining overloads, aliases, legacy conversions or a second options path.
An object is the existing retained PropertyList handle. Undefined, null,
booleans, numbers, strings and arrays remain the same Property domain used by
all other mathematical host operations. Use existing `host::get(options,key)`
when Source accesses a property; no synthetic empty object, early validation,
boolean coercion, generic null-error message or cloned options bag is allowed.

## File duties and controlled migration

Root owns replacement signatures in `mml_node.ixx` and the derived declarations
listed in the audit, corresponding definitions/calls and removal of the two
old Html/multiscript conversion helpers. Root also owns Native-only emitter
representation repairs and their regenerated fixtures. Original JS stimuli
and Source records remain immutable; emitter refresh is never gold refresh.
The boundary migration may change parameter types and property access syntax,
but must leave unrelated algorithm bodies and honest stubs intact.

Before delegation, Root must establish a compiling declaration graph, typed
dispatch observers retaining raw options identity, and an original-driven red
gate for the specific verification methods to be assigned. Each subsequent
method packet receives one Source method/file duty, unchanged original records,
and its own real red gate. Normalization, option consumption, error construction
and child traversal stay in their original methods; no centralized wrapper may
normalize all methods before virtual dispatch.

## Acceptance requirements

Boundary acceptance requires all affected observer emitters to reproduce their
unchanged original records; the full core must compile; raw options dispatch
must preserve identity and distinguish undefined/null via every override; and
the previously accepted primitive, inheritance and replacement packets must
still pass. Record all source/default/fault evidence separately from algorithm
acceptance. Complete verifyTree/verifyChildren/mError behavior additionally
requires original callback ordering, exact exceptions, partial factory/tree
effects, and effective compiling mutants. The 12-case original experiment in
the audit is a regression anchor, not full verification coverage.
