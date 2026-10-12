# P38 action, choice and layout-node responsibility contract

Root freezes this contract before implementation. This is not acceptance.
Reference remains MathJax 4.1.3, fb987178c2d279a99b0db5ea02e751691703955e.
Mermaid remains paused and is outside this packet.

## Ownership

- Root owns typed bindings, declarations, original observations, gates and review.
- `mml_verify_nodes` owns six existing stubs: selected/core/coreMO in
  maction_selection.cpp, and setTeXclass/nextToggleSelection/
  setChildInheritedAttributes in maction_behavior.cpp.
- `math_layout` owns only MathChoice::inheritAttributes in mathchoice.cpp.
- `mml_row_mi` owns only setTeXclass in menclose.cpp and mpadded.cpp.

Keep behavior in these upstream classes and files. Agents may not edit helpers,
interfaces, constructor metadata, other implemented bodies, tests or gold. Root
releases implementation explicitly after all guarded runners finish and primitive
bindings are ready. Agents do not build/test. Use LF and import std.

## Required semantics

Selection uses raw Property ToNumber and Math.min/max with NaN propagation.
Fractional or NaN selection must never be truncated to an integer child slot.
The native tagged ChildArray represents indexed node slots and numeric-slot
prototypes; it does not claim arbitrary JavaScript named-property reflection.
Root supplies its explicit numeric lookup binding, which preserves Undefined,
Null and node identity and reports numeric null-array accesses at the dereference.

Maction selected captures its attributes receiver, reads selection, reads length,
then converts selection and rereads the current indexed array. Only a falsey slot
creates a zero-argument mrow. core/coreMO call virtual selected once and dispatch
the requested member on that result. Tooltip class assignment uses the strict
lowercase actiontype, reads slot 1 for the condition and rereads it for the call,
classes it with Null, then captures selected, classes it, updates from that same
selected node and returns its callback result. Toggle uses parseInt(ToString(raw))
plus one, max with one, live length, strict greater-than reset and fresh attributes
for the final set. Tooltip inheritance uses lowercase string-method semantics,
optional slot 0 then fresh slot 1, with original raw arguments and literal
false/1/false for slot 1. The non-tooltip path calls the qualified base hook.

MathChoice has no Source default arguments and does not call the base inheritance
method. Preserve Missing/raw argument handles. Select display style by truthiness
or clamp numeric level then add one; falsey slot creates mrow. Read raw parent
after selection/factory effects; replace the choice, ignore replace return, then
inherit on the captured child. Preserve factory-null and partial replacement effects.

Menclose classes slot 0 with incoming previous, rereads live slot 0 for virtual
updateTeXclass, returns the first child callback result. Mpadded falsey vbox calls
qualified AbstractMmlLayoutNode::setTeXclass. Its truthy branch calls virtual
getPrevClass, assigns ORD via the Source virtual class setter, reads current slot
0, classes it with Null and returns self. No implicit update is added.

Every semantic entry uses TreeCallGuard. Preserve Source callback ordering,
receiver capture, raw/logical parents, tagged failures and earlier partial effects.
An API gap is reported to Root; agents do not invent a simplified substitute.

## Independent acceptance

The unchanged pinned Source oracles reproduce before implementation:
maction 934 cases / 2318 Source value records; MathChoice 410 / 1081;
simple MML 106 cases / 7667 expected assertions / 296 Source value records.
These counts are corpus inventories, not passing Native results.

Root first proves real Unimplemented failures, then reviews full bodies and runs
the unchanged Native fixtures over the actual module graph. Supplement gaps from
the pinned original before Native comparison. Each bounded semantic method needs
independent effective compiling faults; surviving faults, crashes or compilation
failure reject acceptance. Run the normal FoliaTests build and affected regressions.
This packet does not accept a complete parser, layout, SVG or Folia integration.

Root's first implementation baseline passes all 9023 assertions in 1451 tests.
Full method review also identifies the action_host lowerString error projection:
Root supplements eight actual original string-method observations before Native
comparison and corrects Null/Undefined/non-callable errors at that existing
primitive boundary. This bounded extension adds no MML semantic algorithm to a
helper and does not alter the old Source gold.
