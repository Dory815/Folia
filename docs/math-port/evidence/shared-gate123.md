# Gate123 actual result

Gate122 ended with namespace errors in the new AMSCD UT: `FunctionRef` and
`Property` belong to `folia::math`, not `folia::math::tex`. Gate123 corrects
those qualifications. All 51 targets compile with all 1,944 captured inputs
unchanged. Core, Host, Parser, Foundation, Regexp and LinkedList pass. All
capture handles and complete comparisons are terminal; the comparison runner
audits the frozen inputs before and after its work.

The full renderer matches 63/75 complete Source records, up from 60/75.
Units and AMSCD registration increases actual packages to 31/38 and reduces
configuration warnings to seven. Public API remains 9/9. Prior passing
comparisons remain passing: Physics, Colortbl, Mathtools, original/expanded
Bussproofs and exact utility/module sessions, Tag/SetOptions/defaults,
TexHtml/XML, parser projections, Attributes, Mhchem, Map sessions and host/layout.
Reports and executable/raw-output provenance are under `build/math-port`,
including `checkpoint-comparisons-g123.json`.

Next-six remains 18/20 and fonts 13/16. Both now reach the same missing real
MML node field projection: an inferred row exposes only attributes and parent,
while Source exposes its factory, properties, children and mathematical fields.
The MathItem `_state`/extra-document fix and font observer WeakSet traversal
correction have passed beyond their previous differences.

Texttip's exact field collision is `jax` on the canonical CommonWrapperFactory.
`State::bind` calls `NativeIdentityBindings::instance`, which returns the
existing identity, then incorrectly installs another native data field.
The wrapper's new factory getter reveals that repeat-projection path. The
saved fix reuses the original identity and owner, preserving its actual
constructor/prototype and mutable jax value; it is not accepted until rebuilt.

`compare-texttip-svg.cjs` now compares the complete SVG string against the
pinned Source texttip observation and validates native capture provenance.
Gate123 correctly fails it because the process exits before producing SVG.
It does not claim adaptor trace parity. Complete renderer and repeated-render
lifetime acceptance still block Folia integration. Mermaid remains paused.
