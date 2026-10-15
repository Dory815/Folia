# Gate134: complete tooltip record and passing test suites

All 51 targets build with 1,968 captured inputs unchanged. Complete Core,
Host, Parser, Foundation, Regexp and LinkedList pass. Public SVG/error API
remains 9/9 and the complete 1,315-character texttip SVG remains exact.

CommonOutputJax now has one processMathObserved boundary: call the actual
virtual renderer, then notify the existing observer with the actual active
LayoutJob after success. Both ordinary typeset and the Context SVG method
use it. The nested math-tooltip render consequently retains its wrapper
snapshot before the later Source event-handler error. The complete tooltip
record now matches, including root, wrappers, calls, paints and error.
The focused tests distinguish mathtip's error from texttip's successful title.

Full rendering matches 74/75. The only remaining record is Bussproofs;
37/38 packages are registered and its missing-package warning remains.
This is not renderer acceptance. Its Stack and parser owners remain separate
pending integration work.

Standalone font/next-six probes now construct the actual AbstractMathItem
owner and a standalone TeX Jax without creating a MathDocument or installing
an adaptor. The item projection follows the real owner fields and measured
CJS constructor/prototype. No Source expected output or serializer tolerance
changes. Counts remain fonts 13/16 and next-six 18/20, but their first
difference advances to configuration.configMethod.items missing from the
native PrioritizedList projection. Parser/Stack fields remain required too.

Supplemental font-family checks are now UTs, preserving their assertions
and restoring both locked probe envelopes. TexHtml's six records and both
DOMStrings records pass. Two XML records still differ at foreignObject's
own xmlns attribute. The actual current SVG-helper/Scene owner packet is
under review; parent-based namespace inference is not an accepted fix.
All other previously passing package and owner comparisons pass.

All evaluation processes are terminal and comparer input audits pass.
Ignored evidence includes gate-134-inputs-before.json,
gate-134-input-changes.json, diagnostic-exits-134-root.json,
package-captures-g134.json and checkpoint-comparisons-g134.json.
The full native record hash is
`795255bf0b7f754b17b9a49ae7a107873925825a6572e8b03a8a09ac34b23881`;
the full Source hash remains
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.

Factory fields, XML helper semantics, lifecycle and cancellation still need
implementation acceptance. Folia retains its current math QuickJS worker;
Mermaid remains paused.
