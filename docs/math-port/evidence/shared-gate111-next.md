# Shared build after Gate110

The latest accepted compile remains Gate110 until a later complete build
and runtime capture finish. No new package has been production-registered.

Gate111 selected 46 targets and captured 1,901 inputs. It terminated with
exit 1; all inputs remained unchanged. The errors were explicit registry
arguments missing from Physics/Mathtools helper parameters, plus TexHtml and
HTMLDomStrings calls that did not match the current native APIs. Their Luna
owners corrected the responsible callers and module imports.

Gate112 selected the same 46 targets and captured 1,904 inputs. It terminated
with exit 1 and zero changed inputs. The production packet compiled, but
seven observer/test translation units failed: stale NodeFactory conversion
arguments, incomplete explicit registry fixture wiring, incorrect Property
namespace/UTF-16 conversions, ambiguous empty MML arguments, and Context
calls that passed a Value where an ObjectRef was required. Those owners are
correcting the actual fixtures and calls without changing Source outputs.

The text-layout identity failure has been localized to
`TextParser::copyLists`. `ParseOptions::addNode` creates ordinary Context
arrays containing canonical MML node identities. The previous copyLists
incorrectly resolved the array itself as a native ChildArray. Its candidate
correction uses the existing Context iterable protocol and resolves yielded
nodes individually, preserving the Object.entries snapshot, partial effects,
and clearing lists only after successful completion. Focused draft checks
exercise ordinary arrays, live iterator methods and propagated exceptions.
These changes require a new successful shared compile and official captures.

The 358-module dependency audit reports no missing imports, cycles, duplicate
interfaces or Scene imports of backend-specific modules. That audit does not
establish behavioral equivalence. Luna owns the implementation and test
packets; the bounded Sol callable-boundary review is complete.
