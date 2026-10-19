# Native C ABI probe: gate 271

The opaque C bridge forwards UTF-16 code units to the native Renderer and owns
the returned SVG/error bytes independently of the Renderer. Cancellation uses
an opaque stop source. Render calls reject the wrong thread; destruction remains
the caller's creating-thread responsibility. No Windows API enters the math
implementation.

The Stage A bridge and probe compile. Gate 271's probe exits 0, covering invalid
arguments, UTF-16 input, retained result bytes, wrong-thread rejection,
pre-cancellation, overlapping stop requests, deadline cancellation and a fresh
session after cancellation. All 2,223 frozen inputs remain unchanged. This is
bridge-only acceptance, not complete-tree or Folia integration acceptance.

The pinned MathJax 4.1.3 profile includes `noundefined`: `\notacommand` becomes
red `mtext`, whereas `\frac{` produces `merror` with "Missing close brace".
The corrected probe checks both outcomes. Root independently reran the Source
witness, which validates 299 locked dependencies. The reproducible witness and
captured Source records are in
`tools/math-port/regressions/g271/native-bridge-errors/`.

Gate 270 compiled but failed four Host assertions: the new MML birth observer
read a Source-visible factory getter, and an mtable ownership test omitted its
persistent class-default projection from the baseline. Both remain separate
pending corrections at this checkpoint. The ParseOptions real-owner cases pass
39 assertions, the new Lite callback scope cases pass, and Foundation passes
116 tests / 1,334 assertions. No complete Core/profile rerun was accepted for
that tree. Across 192 renders, live owner-bearing growth falls to 18,816, but
remains unresolved; this measurement does not override the Host regression.

Ignored provenance: `build/math-port/g271-native-bridge.txt.provenance.json`
and `build/math-port/final-evaluation-g271.json`. Gate 270's failures and
observations are retained in `build/math-port/final-evaluation-g270.json`.
Real platform raster tests and the application worker switch remain pending;
Folia still uses QuickJS for math. Mermaid remains paused.
