# P15b/P15c root contracts — TeXAtom and semantics

Root has read both complete pinned sources and reviewed the P15 prerequisite
audits and P15b shared-operation review. Source hashes are frozen by
atom-semantics-reference.cjs, which also verifies that the effective upstream
TeXAtom method is exactly Mo.prototype.adjustTeXclass. The class AST has eight
TeXAtom members and ten semantics members. TeXAtom's final module-level
prototype assignment is a separate required behavior mapping.

## Architecture and ownership

operator_class is an MML semantic operation depending on MmlNode, with no
dependency on concrete Mo or TeXAtom. Root extracts the accepted Mo algorithm
mechanically, preserving virtual state calls and parent reads in source order.
Its body owns exactly one TreeCallGuard. Both class adjustTeXclass entry points
delegate to it without a second guard. This preserves the upstream shared
algorithm and receiver semantics. The typed native API does not expose JS
prototype function objects or their reflection/identity; this is an explicit
host adaptation, not a claim that distinct C++ member-function pointers equal.
The source assignment's effective call/effect/return behavior is required.

MmlTeXAtom derives from AbstractMmlBaseNode; it is not a Mo subtype. Arity -1
retains the inferred-row structure. notParent=true affects logical Parent
traversal. Its field phase initializes inherited texclass to ORD after base
initialization. The subsequent constructor-body action calls virtual texClass
then setProperty('texClass', value). Do not substitute literal ORD for that
getter or move any action to generic initialization: a derived virtual getter
may return a raw value distinct from the stored field. Its static defaults are a fresh
copy, not a base alias.

MmlSemantics derives from AbstractMmlBaseNode, with minimum-one arity and
notParent=true. MmlAnnotationXML derives from AbstractMmlNode; its inheritance
hook is genuinely empty upstream. MmlAnnotation derives from AnnotationXML,
owns a fresh defaults copy, and replaces its properties with {isChars:true}
after the superclass phase. Do not merge the old record or propagate inherited
attributes to annotation children. Geometry and output-wrapper decisions
remain in Common/scene/output, including rendering only the primary semantic
child. HTML adaptor behavior is a separate prerequisite; these nodes do not
parse or measure HTML.

Root owns both interfaces, generated bindings/phases, shared operator operation,
source/member mappings, actual-CJS observations, tests, CMake, mutations and
acceptance. Agents may write only these exact CPP/report pairs after freeze:

| Agent | Only implementation | Only report |
| --- | --- | --- |
| tex_parser | src/math/core/mml_tree/mml_nodes/tex_atom.cpp | docs/math-port/evidence/p15b-tex-atom.md |
| math_fonts | src/math/core/mml_tree/mml_nodes/semantics.cpp | docs/math-port/evidence/p15c-semantics.md |

No builds/tests/commits/delegation, interface/shared-file changes, altered
expectations, full factory registration, parsing, geometry or backend work by
agents. Report missing requirements to root; do not invent a fallback.

## Method obligations

TeXAtom.setTeXclass uses TreeCallGuard, accesses required child zero and calls
its virtual setTeXclass(null) unconditionally, discards that return, then calls
the receiver's virtual adjustTeXclass(original previous) and returns its result.
It does not call the base setTeXclass or getPrevClass first. Validate a null
child/array at required access rather than returning early. Use the existing
arithmeticChild primitive for this typed dense-array binding.

TeXAtom.adjustTeXclass is exactly the thin call
adjustOperatorTeXclass(*this, previous), without a separate guard or duplicated
algorithm. The class-body identity stub is overwritten by the final upstream
prototype assignment and is not the effective runtime behavior. Map that
initial member and final assignment explicitly rather than leaving the stub.

AnnotationXML.setChildInheritedAttributes has an empty body and ignores all
arguments. No superclass call, traversal, normalization, error or guard is
added to this source-empty hook. The CPP contains only that method definition.
All other semantics behavior is inherited or root-generated.

## Root acceptance

Obtain expected values from actual pinned CJS methods, never copied formulas.
Verify complete defaults/getters/identities, inferred-row and semantic trees,
superclass/derived constructor phases and virtual getter/property order,
virtual getter result and old-record identity, logical Parent, raw inherited
values and annotation non-propagation, TeXAtom child-first/null argument and
virtual adjustment result, malformed child failures, and shared adjustment
through both receiver kinds over all class branches/raw states/autoOP/parents.
Retest Mo source fixtures with a real TeXAtom class registration and all 33
existing compiling Mo mutants, moving adjustment mutations to the new body.
Add packet-specific compiling mutants and run full core regression. No
source acceptance or renderer gate passes solely because a library builds.

The initial independent oracle has 6,828 assertions / 63 cases, including
924 raw shared-operation rows and 152 source-value records. Source observations
include both actual receiver kinds, constructor virtual-getter behavior,
malformed dispatch and annotation hook states, outer trees and logical parents.
Root may extend missing witnesses; agents cannot change or reduce expectations.

Root's final oracle has 6,888 assertions / 74 cases, retaining all 924 raw
rows and adding fresh-default identities, throwing constructor callbacks,
real inferred/semantic parent-chain adjustments and the explicit native guard
budget check. Base initialization now writes prevClass, prevLevel, raw texclass
in that source order after the generic superclass. Storage remains Missing
before those phases, and throwing callbacks expose the actual partial state.
The complete core regression and 26 packet/33 Mo/20 class-state compiling
mutation checks pass. Local acceptance is recorded only in
evidence/p15bc-atom-semantics-acceptance.md; full renderer gates remain closed.
