# P17c — MathChoice inheritance method

This file maps the single implemented semantic method to the pinned MathJax
4.1.3 source. It does not assert root acceptance of the whole MathChoice class
or of the broader math port.

Pinned source: `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mathchoice.ts`,
SHA-256 `e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76`.
The source method is `MathChoice.setInheritedAttributes`, lines 74–84.

| Source operation | Native mapping | Behavior preserved |
|---|---|---|
| `display ? 0 : Math.max(0, Math.min(level, 2)) + 1` (line 80) | `MathChoice::inheritAttributes`, `mathchoice.cpp` | Tests `display` truthiness before converting `level`; uses host number conversion; explicitly propagates NaN through min/max; keeps fractional levels fractional, so they do not become integer child indices. |
| `this.childNodes[selection] || this.factory.create('mrow')` (line 81) | Typed child-vector lookup, then `factory.create("mrow", {})` | Null child-array errors at indexed access. Missing, null, fractional, NaN, infinite, out-of-range, and null slots use the current factory. The native factory's empty argument span represents the source call with no factory arguments. |
| `this.parent.replaceChild(child, this)` (line 82) | Reads `this->parent` after selection/fallback and calls `replaceChild` | Detached-parent failure follows fallback allocation. The replacement return value is ignored. Replacement side effects/errors occur before selected-child inheritance. |
| `child.setInheritedAttributes(attributes, display, level, prime)` (line 83) | Calls the captured child through the raw-argument facade with all four original `Property` values | No defaults or normalization in MathChoice; the child is retained across parent replacement; a failure leaves the preceding replacement state observable. |

Native adaptation: the source method name remains `setInheritedAttributes`,
while the frozen C++ virtual body is named `inheritAttributes`; the facade
dispatches to that body without inserting defaults. `AttributeList` is the raw
`Property` carrier in the frozen inheritance-dispatch contract. A shared
`TreeCallGuard` bounds recursive native dispatch; this is a host resource guard,
not a change to selection policy.

Root owns the class declaration, defaults and kind/arity/notParent bindings,
factory registration, source oracle, expected fixtures, mutations, builds, and
acceptance. No fixture or build was run for this implementation packet.
