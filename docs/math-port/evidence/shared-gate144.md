# Gate144: new test fixtures corrected; complete Core passes

This checkpoint changes two tests without changing the production library.
The Bboldx callback metadata fixture now initializes its actual BasePackage
dependency, matching the native render graph. The ordinary-formula Scene
assertion checks mathematical italic U+1D465, as emitted by pinned Source SVG,
instead of ASCII x. Neither change modifies rendering to satisfy the test.

All 53 targets build with 1,995 captured inputs unchanged during evaluation.
Complete Core, Host, Parser, Regexp and LinkedList pass. The complete Core
run ends at 2026-10-15T17:05:14Z. Both stable callback/configuration CJS
witnesses also execute successfully.

The production inputs are byte-identical to Gate143. The input difference is
limited to those two test files and removal of EOF whitespace from the
callback Source witness; reconstructing its old trailing line endings matches
the frozen Gate143 hash. The 29 binaries used by the Gate143 package, layout,
API, Foundation and document checks still match their recorded hashes.
Therefore those results are reused, including their failures, rather than
rerunning unchanged binaries. No executable copies are made.

The complete layout profile remains 75/75 with 38/38 actual registrations;
API is 9/9 and complete texttip SVG is exact. StackOwners still has its MML
Push assertion failure, fonts remain 13/16, next-six remains 18/20, and two
standalone XML differences remain. A new pinned CJS witness indicates that
StartItem consumes a pushed MML node without adding a second stack entry;
its proposed fixture correction has not been applied or verified here.
The renderer is not accepted and Folia remains unchanged.

Evidence is in build/math-port: diagnostic-exits-144-root.json,
observation-reuse-g144.json (29 executable hashes and exact changed inputs),
the two Source witness outputs, Gate144 input/audit manifests and empty
post-evaluation change report. The final state keeps evaluationExitCode 1
because the inherited failing checks remain open. Mermaid remains paused.
