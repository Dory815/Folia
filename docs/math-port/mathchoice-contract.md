# Root P17c — complete MathChoice semantic source

Authority: complete pinned ts/core/MmlTree/MmlNodes/mathchoice.ts, all five
members. Root read the entire source; verify its scope hash before changes.
The class remains AbstractMmlBaseNode-derived, kind MathChoice, arity 4,
notParent true, with fresh shallow-spread base defaults (root bindings).
One semantic method replaces/removes this node during inheritance; finished
Common/SVG layout does not retain a MathChoice wrapper.

Root owns mathchoice.ixx, bindings/defaults, CMake, factory registration,
actual-CJS oracle, tests, mutations and acceptance. math_layout Luna owns only
src/math/core/mml_tree/mml_nodes/mathchoice.cpp and
docs/math-port/evidence/p17c-mathchoice-method.md; no other changes, builds,
tests, commits or delegation. Apply inheritance-dispatch-contract.md first.

Implement MathChoice::inheritAttributes(attrs,display,level,prime) only.
There are NO own defaults and no superclass call. Preserve original arguments.
selection = truthy(display)?0:Math.max(0,Math.min(ToNumber(level),2))+1.
JS min/max propagate NaN; fractions are not truncated. Preserve evaluation
order, including numeric conversion before reading the child array. For a
fractional/NaN/missing/null child slot, call current factory.create("mrow",{})
with zero factory args. Never approximate the index by integer truncation.
Then access the CURRENT parent and replaceChild(selected,this), ignoring its
return; invoke the CAPTURED selected child with the original raw four args.
Replacement precedes child inheritance, and errors leave that partial state.
Detached node failure occurs only after selection/fallback allocation. A null
childNodes array fails during indexed access; a null factory result fails only
where the source later uses it (after parent replacement). Retain native typed
factory/parent/child interfaces and structured TypeError; do not normalize a
missing level to zero, prevalidate parent, auto-attach or retain an inert Choice.

Root acceptance covers all five members, raw args/selection, valid nested real
MML and direct source behavior, callback/rebinding/factory exceptions/order,
base-pointer omissions, exact partial replacement state and completed removal.
Compiling omissions/coercion/default/ordering mutations must fail unchanged
assertions. Full core test/build must pass. No G0–G7 or Folia switch follows
from this individual source class.
