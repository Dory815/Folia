# Gate150: complete SVG parity with explicit runtime boundaries

All 55 targets compile with 2,031 captured input entries unchanged through
evaluation. Core, Host, Parser, Regexp and LinkedList tests all pass. The
raw-edge fixture correction uses null-prototype objects only where the test
requires a complete graph; a separate regression verifies that default
intrinsic prototypes retain their actual opaque-owner pins. Empty optional
results are guarded after their assertions. Production tracing is unchanged.
The actual-session processor metadata test now also passes in complete Core.

The complete public Renderer SVG/error comparison matches 150/150: 75 through
one continuous renderer and 75 through separate renderer-owned runtimes.
Root independently recaptured the fresh Source group in 75 clean Node
processes using the pinned MathJax 4.1.3 profile and all 299 locked dependency
files. The original continuous Source records remain byte-identical. Native
Gate149 records and executable are reused after checking the captured probe
source against its build manifest and the executable's before/after hashes.
The comparer checks explicit Source and native runtime ownership boundaries,
all inputs, full SVG bytes, errors and completed Scene presence. Its synthetic
positive, incorrect-boundary and one-SVG-difference checks also pass; those
synthetic checks do not establish native parity.

Gate149's four fresh-document differences remain preserved. Root reproduced
each in both isolated and reused Source processes. The upstream FontData
class owns shared dynamic-file records, and loadDynamicFileSync calls setup
only before that record has a promise. Creating later SVG output instances
in the same module realm can therefore skip their per-instance font setup.
A fresh native Renderer owns its own Context/module graph; a clean Source
module realm is the corresponding comparison boundary. No native font code
or expected SVG was changed to resolve these four differences.

Thirty-one unchanged native binary observations retain their recorded
Gate149 results, including failures. Foundation passes; layout observations
match 75/75 with 38 registrations, API is 9/9, complete texttip SVG is exact,
and all 25 captures pass. The existing package comparisons still have three
failures: texhtml-xml, tex-next-six and tex-font-packages. Fonts remain 13/16
and next-six 18/20 at live FindTeX properties; two XML differences remain.

Evidence is under build/math-port: diagnostic-exits-150-root.json,
observation-reuse-g150.json, profile75-full-svg-source-clean-g150-root.json,
profile75-full-svg-native-boundary-g150-root.json,
profile75-full-svg-comparison-g150-root.json,
profile75-full-svg-comparer-check-g150-root.log and the empty post-evaluation
change report. All processes are terminal. EvaluationExitCode remains 1
because the three known comparison failures remain. No executable or whole
directory copies were created. Reclamation, remaining live owner semantics,
cancellation and the actual Folia build still require acceptance; Folia is
unchanged and Mermaid remains paused.
