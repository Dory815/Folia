# Gate167: current native build and runtime checkpoint

All 57 shared targets compile. The build manifest contains 2,075 entries and
the post-build audit records zero changes. Gates163-166 failed compilation;
the final corrections fix Common factory descriptor dereferences, the live
Wrappers getter's missing State capture, six Context creator callbacks still
using the old ObjectRef signature, and exact container comparisons whose
Boost.UT failure printer cannot stream options::Value. Equality assertions
remain exact, computed as bool before being passed to the test framework.

The actual public API executable exits 1 with
`TextMacros CONFIG arguments are invalid`. Complete SVG and layout capture
report the same initialization failure. There is no current full SVG parity
result. The native document probe exits 0, whereas Host, Parser and Foundation
report failures; Regexp and LinkedList exit 0. Core also exits 1. All 25 package
capture processes finish: 20 succeed and five fail (parser-fields, next-six,
font packages, parser projection and texttip). The final evaluation audit
confirms all 2,075 inputs remain unchanged and closes evaluation with exit 1.

Source inputs remained frozen throughout evaluation. A later passing build is
required before the new Scene-only route, live geometry/paint, cancellation,
factory fields and filter record changes can be accepted. The last complete
passing public SVG comparison is Gate158, which predates these changes.
Folia still uses its math QuickJS path, and Mermaid remains paused.

Evidence is stored in the existing ignored build directory:
`math-font-package-gate-167.log`, `gate-167-inputs-before.json`,
`gate-167-input-changes.json`, `math-api-native-g167.json.provenance.json`,
`diagnostic-exits-167-root.json` and the per-target runtime provenance.
No build directory or executable copies were created for this checkpoint.
