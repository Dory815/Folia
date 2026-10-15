# Gate145: sequential public Renderer SVG matches Source exactly

All 54 targets build with 2,000 captured inputs unchanged through evaluation.
The new public probe creates one actual folia.math Renderer and renders an
unary proof, x+1, and a binary proof in that order. It does not create separate
parser/layout services or reset the session between cases. A pinned MathJax
4.1.3 full-profile CJS document runs the same sequence and options.

All three complete SVG strings match byte for byte. Both sides report no
exception and no serialized MML error; the native results also contain
completed Scene nodes. The exact SVG SHA-256 values are:

- unary: `f511be3c8ac046d343d990c7d276cd933777575c8fc8506a8287153eb7a4becf`
- ordinary: `56940c3cd2a0cf645713a1777f4d0f73edc2ee55a31b77b11b9edcdaa70d14ec`
- binary: `ba573cf438da6b7d620fed47ff1ba593f9d9f3388b4cdb518cd2ec9160740bc1`

The StackOwners probe also passes. Its prior MML Push assertion assumed a
second stack entry, but pinned CJS demonstrates that the existing StartItem
consumes the MML node into First and the same stack array keeps length one.
The corrected native check verifies that canonical root identity, array slot
and length. Other replacement/alias/module-scope assertions remain intact.

The production library inputs are unchanged from Gate144. CMake adds only
the public probe; the only changed existing test input is StackOwners. The
33 binaries used by the prior diagnostics and other comparisons still match
their provenance hashes. Those results are reused: Core/Host/Parser/Regexp/
LinkedList and Foundation pass, full layout matches 75/75 with 38 registered
packages, API matches 9/9 and texttip's complete SVG is exact. Fonts remain
13/16, next-six remains 18/20, and two standalone XML differences remain.

Evidence is in build/math-port: observation-reuse-g145.json, both fresh native
captures and provenance, fresh public/Stack CJS observations, the complete
SVG comparison report, Gate145 input manifests and empty post-evaluation
change report. The comparator utility's positive and mismatch paths were
checked separately; synthetic inputs are not used as native parity evidence.
No executable or directory copies are made. EvaluationExitCode stays 1 for
the remaining XML/package failures. The renderer is not accepted, Folia is
unchanged, and Mermaid remains paused.
