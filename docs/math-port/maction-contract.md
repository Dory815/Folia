# Root P18 — complete action node and MML registry

Root read complete pinned maction.ts (12 members), MML.ts (one ordered registry)
and MmlFactory.ts (two members), plus existing factory/class-binding/attribute/
inheritance contracts and all required host operations before delegation.
These remain MML/core responsibilities; they perform no TeX parsing or layout.
Root owns interfaces, bindings, host/data primitives, tests/oracles/mutations,
CMake/scope/review/acceptance. No agent may edit those or claim acceptance.

## P18a string host prerequisite

js_case.ixx exports folia::math::js_case::lowerCase(u16string_view)->u16string.
Root implements general default Unicode lowercase on UTF16, retaining lone
surrogates, supplementary mappings, expansions and contextual Greek final
sigma. Pin Node 24.13.0 / Unicode 16.0 / ICU 77.1 data and verify against actual
String.toLowerCase over the full domain and context/property boundaries. This
is a compiled native string primitive, never a runtime JS interpreter. Do not
replace it with ASCII-only matching against tooltip or locale-dependent C APIs.
action_host.ixx lowerString(Property) accepts actual strings only, decodes via
existing lossless host::text, calls this primitive, and raises TypeError for
non-string receivers. Global parseInt uses existing integer(host::string(raw)),
which has String-hint primitive coercion; do not use direct methodString.

## P18b Luna method ownership

math_layout owns only mml_nodes/maction_selection.cpp and
evidence/p18b-maction-selection-methods.md: selected, isEmbellished, isSpacelike,
core, coreMO. selected()const is virtual. Read current attributes/selection first;
read childNodes.length BEFORE numeric conversion; Math min/max propagate NaN;
fractions are not truncated. Then read CURRENT childNodes for the computed slot.
Missing/falsy slots call current factory.create("mrow",{}) with zero factory
args, and may return null. No caching/attachment/repair. All four projections
call selected exactly once and dispatch to that captured child; null receivers
fail only at the projected property/method access. Keep native bool-returning
MML predicates and borrowed node-pointer services; no new erased JS layer.

tex_parser owns only mml_nodes/maction_behavior.cpp and
evidence/p18b-maction-behavior-methods.md: verifyAttributes, setTeXclass,
nextToggleSelection, setChildInheritedAttributes. Exact protected signatures
are in root maction.ixx. verifyAttributes calls qualified base first, then strict
actiontype != "toggle", live hasExplicit and unset, respecting short circuit.
setTeXclass tests strict lowercase literal "tooltip" (NO lowercasing here),
then the current second slot; dispatches second with null before selected(),
then selected.setTeXclass(previous), updateTeXclass(captured selected), return
the child result. All array/attributes reads remain at their own source sites.
nextToggleSelection coerces source parseInt with String hint, adds 1, then max
1 with NaN propagation; reads current child length even for NaN; wraps only on
selection > length; rereads attributes before setting the raw double.
Child inheritance DOES call lowerString on raw actiontype. Non-tooltip invokes
qualified base hook with four unchanged raw parameters, then returns. Tooltip
reads current childNodes[0], optional-nullish dispatch with original arguments,
then rereads current childNodes[1], optional-nullish dispatch with attrs,false,
1,false. Null arrays throw; null/missing slots skip. First callbacks can replace
the array or mutate attrs; do not snapshot both children. Add only existing
TreeCallGuard where native recursion protection requires it. Root owns fresh
source defaults (base spread, actiontype toggle, selection 1), kind and arity.

## P18c Luna registry ownership

mml_registry (Luna, reassigned from the planned math_fonts packet) owns only mml.cpp, mml_factory.cpp and
evidence/p18c-mml-registry-methods.md. mml.ixx/MmlFactory declarations are frozen.
MML() returns one function-local const shared_ptr to a mutable ordered
FactoryClassMap. Seed all 43 source registry entries in exact source order using
actual compiled classBinding<T>(); no omitted node kind or test-only creator.
Use exact keys from pinned CJS/prototype getters, including case. Every entry
retains its constructor identity across factory instances; creators themselves
are per factory. Import actual corresponding per-source modules, no new tree.
Source MML const binding prohibits rebinding but contents remain live. Native
MmlFactory::defaultNodes is a separately rebindable shared handle, initially
aliasing the registry, preserving source alias/break-alias behavior.
MmlFactory ctor adapts inherited this.constructor.defaultNodes via the existing
P6 injected-default signature. Explicit nodes (including empty) bypass defaults;
nullopt uses injected classDefaults if supplied, otherwise current defaultNodes.
Null defaultNodes throws TypeError only when actually selected, never silently
uses AbstractFactory::defaultNodes. Pass selection into the existing Base ctor;
do not duplicate factory seeding, constructors or dispatch. MML()const getter
returns existing creatorView(), the same live per-factory object on every read.
Keep existing session ownership and nodeMap/creator object distinction.

## Root acceptance

Verify all source hashes/AST members and unchanged existing expectations. Action
oracle executes actual pinned CJS, exercises all 12 members, raw values/NaN/
fractions/string coercion, direct and real nested MML, live getters/factory/
selected/child callbacks, strict case versus lowercased tooltip, nullable slots
and exact exceptions/partial state. Registry oracle compares all 43 entries,
order/class identity, actual construction/nodeIsKind/defaults/subclass behavior,
creator aliases/mutations, live source registry changes, default-handle rebinding,
injected defaults and explicit empty maps. Include complete MML/HTML primitives
and all source class field/default/inheritance behavior where this registry
exposes it. Compile deliberate body/member/ordering/identity/coercion omissions
and require unchanged assertion failures; full core build/test before acceptance.
These packets cannot pass full TeX/Common/SVG or Folia gates by themselves.
