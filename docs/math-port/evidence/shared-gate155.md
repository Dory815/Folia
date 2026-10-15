# Gate155: compilation succeeds; callback argument conversion regresses

All 55 targets compile. All 2,040 captured build inputs retain their hashes
through evaluation. Core, Host, Parser, Regexp and LinkedList processes are
terminal, as are the 25 package captures, API and complete SVG capture.
The evaluation is closed with exit code 1.

The new shared FunctionList owner makes native callbacks cross the Context
boundary. Its primitive-only argument conversion rejects actual
ParserConfigurationRef and TeX pointer arguments from ParserConfiguration
initialization/configuration. The public API and complete SVG entry points
exit with `TypeError: FunctionList argument has no Context Value binding`.
No full SVG comparison can be made from these unsuccessful native captures.
Gate150's 150/150 result applies to Gate150's recorded code and binaries only.

Core exits with Windows code 3221225477. Root independently reproduces this
using `*math_port_function_sync_missing_null_next*` with an unchanged
FoliaTests executable. The test deliberately places a null record in the
list's next slot. PrioritizedListItemHandle::contextBacked dereferences that
null record before the existing invocation TypeError path can run. A null
check is the scoped correction; the test and Source slot behavior stay intact.

Host and Parser exit 1; Regexp and LinkedList exit 0. Seventeen package
captures exit nonzero, and seventeen of the twenty comparison commands fail.
Several comparison commands lack a successful native input; these failures
are not evidence of specific SVG or layout mismatches. API and full SVG
executables retain their before/after hashes despite their failing exits.

Evidence remains in the ignored build directory: diagnostic-exits-155-root,
package-captures-g155, checkpoint-comparisons-g155, the focused null-next
provenance, API provenance, full SVG binary audit, and the input hash audits.
No executable or directory copies were made. The renderer is not accepted;
Folia still uses the math QuickJS path, and Mermaid remains paused.
