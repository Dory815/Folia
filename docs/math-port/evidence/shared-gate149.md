# Gate149: initial mapping generation and complete SVG comparison

All 55 targets compile with 2,021 captured inputs unchanged through
evaluation. Gate147 and Gate148 failed on new test compilation errors;
Gate149 includes their corrected test declarations. The production changes
add non-observing raw Context edge tracing and generate BaseMappings from a
fresh pinned MathJax process into the ignored binary directory. The older
post-corpus runtime oracle remains unchanged: its shared token attribute
mutations are valid runtime observations, not initial mapping data.

The new public Renderer comparison uses the locked 75 inputs twice: once
through one continuous renderer and once with a new renderer per case.
Complete SVG/error observations match 146/150. Continuous rendering matches
75/75; fresh sessions match 71/75. Fresh-session differences are
extpfeil-arrows and textcomp-symbols (SVG bytes), plus verb-literal and
unknown-text-families (Source RangeError versus native SVG). These remain
under investigation, including the Source harness's module-level state;
the comparison is not accepted. The native executable's before/after hash
is unchanged. Layout observations remain 75/75 with 38 registrations;
layout parity alone is not complete SVG parity.

All 25 package captures pass. API is 9/9, complete texttip SVG is exact,
and 17 of the existing 20 comparisons pass. Fonts remain 13/16 and next-six
18/20, but their first differences advance past the incorrect initial
stretchy attributes to missing live jax.findTeX properties. The two XML
namespace differences remain.

Parser, Regexp, LinkedList and Foundation tests pass. Core and Host both
display an MSVC assertion for operator->() on an empty optional. Root read
and recorded the runtime dialog, then stopped only these two waiting test
processes so the harness could close with failure. Their recorded exit
codes are 4294967295; neither suite passed. The new raw-edge tests and the
underlying trace completeness are being investigated. No reclamation or
bounded-memory claim follows from the new tracing API.

All evaluation processes are terminal and the 2,021 frozen input entries
are unchanged. Evidence is under build/math-port: diagnostic-exits-149-root.json,
g149-core-host-assertion.txt, package-captures-g149.json,
checkpoint-comparisons-g149.json, profile75-full-svg-comparison-g149.json,
profile75-full-svg-binary-audit-g149.json and the empty post-evaluation change
report. EvaluationExitCode remains 1. No executable or directory copies
were created. The renderer is not accepted; Folia remains unchanged and
Mermaid remains paused.
