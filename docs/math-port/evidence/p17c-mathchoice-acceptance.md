# Root P17c — complete MathChoice source acceptance

Authority: MathJax 4.1.3 ts/core/MmlTree/MmlNodes/mathchoice.ts, SHA256
e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76.
Root read the complete class and froze the file/interface/default/dispatch/
evaluation-order/acceptance contract before Luna implementation. Root owns
declarations, bindings, CMake, registrations, fixtures and acceptance; Luna
owned only the semantic body and report. All five AST members are independently
mapped and checked in p17c-mathchoice-members.json.

The class preserves AbstractMmlBaseNode inheritance, a fresh shallow copy of
base defaults, exact kind MathChoice, arity 4 and notParent=true. Its single
inheritance body owns no defaults and calls no superclass. Display truthiness
selects zero without coercing level; otherwise min/max propagate NaN and clamp
level before adding one. Fractions miss the typed node-array slot, as do NaN
and missing slots. Only a missed/falsy slot allocates an mrow with zero factory
arguments. The current parent is read after that allocation; replacement occurs
before raw inheritance on the captured selected node, ignoring replacement's
return. Observable partial states/errors and factory/parent/child callbacks are
retained. No inert MathChoice wrapper remains in the completed inherited tree.

Actual pinned CJS comparisons: 403 direct selection/argument/state cases and
six real MML tree cases, plus one defaults case. The native suite has 410 cases
and 412 assertions, with 1,081 encoded source values. Selection covers raw
Missing/null/Boolean/number/string/array/object values, signed zero, fractions,
NaN and infinities, failed primitive conversion and truthy-display short
circuiting. All five call arities preserve the original raw attributes/styles.
Complete state observations retain child/parent identities, source fields,
ordered factory/replace/inherit events, exceptions and partial writes.

Malformed child arrays/slots, detached nodes, null/throwing factory results,
factory reparenting/child-array rebinding, replacement exceptions before/after
mutation, replacement array rebinding/null returns and inheritance exceptions
are compared directly against source. The six actual math/mrow/mi/Text nested
trees use inline/block display and three levels, comparing every retained
node's kind, text, inherited attributes, properties and children after normal
propagation. These tests exercise the existing MML algorithms, with selected
MathChoice nodes actually removed, rather than only a recording child seam.
Defaults compare source values and independently verify fresh identity and
isolation from later base-default writes. --check reproduces the fixture.

19 isolated deliberate mutants compile and fail unchanged assertions with
exit 1. They cover all five members, eager defaults, display truthiness,
Missing/NaN/fractional selection, text offset, factory argument count, early
parent validation, stale parent capture, omitted replacement/inheritance,
raw argument cloning/prime changes, inheritance-before-replacement and wrongly
using replacement's return. The final two ran through --only after the first
17; all runs passed their unchanged baselines and byte-checked working sources
and fixtures. Complete results are p17c-mathchoice-mutations.json. Per-mutant
compile/test logs remain in the ignored build directory.

Root independently reviewed the body and four binding members; inventory
checking and the complete core build/test pass (build/p17c-core.log, exit 0).
Local full-source acceptance increases from 46/246 to 47/246. This is MML
semantic acceptance in the declared typed host, not acceptance of the remaining
MmlFactory/MML registry, TeX parser, Common/SVG renderer or Folia replacement.
All full-renderer G0–G7 gates remain closed.
