# P15a root acceptance — multiscript semantic nodes

Status: locally accepted for the complete mmultiscripts.ts source over its
accepted dependencies. Full renderer and production gates remain closed.

Root read the complete pinned source and reviewed the prerequisite audit,
froze the architecture/ownership in multiscript-contracts.md, and committed
the three-class interface, primitive host operations, generated constants,
13-member inventory and independent actual-CJS oracle in 6e1cba3 before
implementation dispatch. One agent could write only mmultiscripts.cpp and
its method report. Two other agents performed read-only contract/acceptance
reviews, followed by an independent oracle review. Root owns all interfaces,
test observations, build/mutation work, corrections and acceptance.

MmlMmultiscripts remains a MmlMsubsup subclass; MmlMprescripts and MmlNone
remain AbstractMmlNode subclasses. The packet provides semantic inheritance,
placeholder normalization and verification. It includes no TeX recognition,
script measurement, placement or rendering. Root generated three distinct
defaults copies and six accessors; the agent implemented four methods. Every
one of the 13 source members has an individual reviewed mapping in
p15a-multiscript-members.json.

## Root review and corrections

Root reviewed original base inheritance arguments, current-property live
array access, first-marker handling, insertion into the post-factory array,
positive splice clamping and parent-write order, script-only counter updates,
raw prime OR/level addition, separate virtual append, post-append receiver
access and original-prime inheritance. Verification retains captured raw fix,
duplicate diagnostics/options/flags, returning/throwing errors and superclass
dispatch last. Marker/None verification calls the superclass before checking
the current direct parent, even with null options. Real partial mError state
and the parent-merror early return remain observable.

Root required two corrections in the submitted CPP: bind/check the final
child receiver before computing its level argument, and explicitly sequence
child.kind before owner.kind in the duplicate-marker message. C++ string
addition alone does not guarantee the JavaScript operand evaluation order.
The authorized agent corrected only its CPP/report; root supplied the tests.

The oracle review identified missing error-options identity, in-loop child
callbacks and kind-getter order observations. Root added all three. The
ErrorOptions host wrapper now exposes value(), returning its existing nullable
PropertyList handle. Virtual callbacks can observe identity and mutate the
actual argument; no snapshot or parallel options state is introduced. The
existing MML error algorithm remains unchanged. Source and native callbacks
both mutate the passed options, rather than merely a saved caller alias.

Initial generated tests had a Boost.UT events namespace collision and two
incorrect native API names. Root corrected generator naming, Exception.name
and getAllProperties without changing source expectations. Those failed builds
are rejected evidence in build/p15a-initial.log and p15a-focused2.log.

## Independent checks

The initial source oracle has 1,286 assertions/222 cases. The expanded final
fixture reference_multiscript.cpp passes 1,304 assertions/226 cases, retaining
all 480 raw inheritance rows and 1,170 source-value graph records. Both the
binding generator and source oracle reproduce their outputs under --check.
They validate pinned source/package equality and the previously established
whole compiled-package hashes before executing the actual CJS methods.

Observations cover complete defaults/accessors/identities, marker positions,
pre/post parity, inserted versus appended None callbacks/parents, raw values,
attribute/options aliases, base/script/marker/factory/append and verification
array mutations, null/empty/factory-return failures, captured fix, message
getter order, superclass/return/throw sequencing, real error trees, direct
inferred-row parents, null options, partial error-node creation and full outer
inheritance. Native comparisons retain ordered own keys, array identity where
relevant, kind/length, NaN and signed zero. Fresh reconstructed inputs cannot
mutate expected source records.

Focused native build: build/p15a-focused3.log. Complete core regression with
the same final sources and fixture: build/p15a-core-final.log. Both pass.

## Mutation acceptance

multiscript-mutation-test.cjs builds an unchanged isolated baseline and 34
copied mutants. Every mutant compiles and fails unchanged assertions with
exit code 1; no crash/build failure is counted. The final byte check confirms
all working source and fixture files are unchanged. Results are in
build/math-port-multiscript-mutations/results.json and the final run log is
build/p15a-mutations.log.

Detections cover original base inputs, live iteration/array capture, first
marker normalization, insertion handle/parent/skip effects, inserted versus
appended inheritance, raw prime/level behavior and parity, captured/reread fix,
duplicate flags/options/getter order, superclass calls, nullable marker/None
verification, direct versus logical parent, defaults identity/arity, exact
factory argument shapes, null option coercion and error-option handle cloning.

Local source acceptance is 33/246. TeXAtom's shared Mo operation, semantics,
HTML adaptor, remaining MML nodes/factory, TeX packages, font services, Common
measurement, shared scene, SVG equivalence and Folia integration remain
required. This local semantic acceptance does not pass G0–G7.
