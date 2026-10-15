# Gate169: current full SVG parity and shared Scene route

All 57 targets compile with 2,075 captured inputs unchanged. The XML Scene
importer now enumerates children only for element nodes. Text and comment
leaves retain their existing semantic content and no longer ask LiteAdaptor
for an unsupported children field.

Fresh public Renderer conversion matches all 150 complete SVG/error records
against the clean pinned MathJax capture: 75 cases with a continuous owner and
75 cases with fresh owners. API comparison passes 9/9. The public executable
SHA-256 is 0a87a57352b2ac8f3eaaa3bfafaa17b16f144f5e3e7d9116878ea5345ce3034d.
The matching report is profile75-full-svg-comparison-g169-root.json.

The paired NativeRenderSession probe completes 146 successful pairs with
identical complete frozen Scene fields/resources, plus four equal error
pairs. All 150 Scene-only records have zero Serializer::materialize calls
and zero LiteAdaptor::serializeXML calls. The probe executable SHA-256 is
7202500d5ea95e4673e539057bda52c5d28a86fa04b7628ae53b6c4c73511d02.
Before/after executable hashes and all captured input hashes remain unchanged.
This proves the two native output routes consume equal layouts and that the
Scene-only route avoids the SVG consumer; Source geometry needs its separate
observations and unit tests.

All 25 package captures exit 0. Parser, Regexp and LinkedList suites pass.
Core, Host and Foundation still fail. Remaining Core failures are the XML
helper fixture's parent/text/comment observations, Common field descriptor
replacement, RegExp clear ownership/NaN equality and direct factory-call
semantics. The corrected nested Lite measurement and Promise chaining tests
pass. Foundation also retains the filter record deletion/replacement failure.

Sixteen of twenty checkpoint comparisons pass. Common layout is 74/75:
TeXHTML's paint observation has six native records versus five Source records.
XML's isolated placement fixture lacks the creator service normally provided
by SvgOutputJax::produceScene. Next-six/font observations still differ in
callback metadata and stack own fields. These remain required work; no full
renderer acceptance or Folia integration is claimed.

All evaluation processes are terminal. Post-evaluation input audit is unchanged
and overall evaluation exits 1. Evidence is under the existing build/math-port
directory, including the full SVG comparison, profile75-scene-route-audit-g169,
package capture records, diagnostic exits and checkpoint comparisons. Folia
continues to use math QuickJS; Mermaid remains paused.
