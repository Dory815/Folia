# P15b/P15c root acceptance — TeXAtom and semantics

Status: local source acceptance only. Full renderer and production gates remain closed.

Root read the complete pinned sources, froze interfaces, generated bindings,
shared operation, source inventory and actual-CJS expectations in 4e59bcb before
dispatch. Agents could write only the two assigned CPP/report pairs in
atom-semantics-contracts.md. Root owns architecture, host adaptations, fixtures,
builds, corrections, mutation checks and acceptance. Agent self-reports are not
acceptance. Implementations are in 2ee346f, root corrections in 0456c33 and
mutation harnesses in 68a8a77.

TeXAtom retains AbstractMmlBaseNode inheritance, inferred row, ORD field phase,
virtual constructor getter/property write, child-first null dispatch and virtual
adjustment. Root mechanically extracted the already accepted Mo class operation
into operator_class.cpp. Both receivers call that one operation without an extra
guard. The native API does not expose JS function-object reflection; effective
call/effect/return behavior is preserved. The overwritten source stub and final
prototype assignment are both mapped, rather than accepting the stub.

Semantics retains minimum-one arity and logical-parent behavior. AnnotationXML's
inheritance hook is empty. Annotation owns fresh defaults and replaces its
property record after the superclass phase. No measurement, parsing, placement
or backend logic belongs to this packet. All 18 class members and the separate
module assignment are root-reviewed in p15bc-members.json; the ledger check
verifies their pinned AST hashes and completeness.

## Root correction and independent observations

The independent review found missing constructor callback traces, two defaults
identity witnesses and a real inferred-parent class-adjustment witness. Root
expanded the oracle accordingly. Actual CJS writes prevClass, then prevLevel,
then the raw texclass field after its generic superclass and before children or
attributes. Native initialization now follows that order and routes the public
previous-state writes through virtual bindings. Pre-phase storage is Missing;
throwing setter witnesses compare the exact partially initialized state.

This exposed one old fixture setup discrepancy: source class-state fresh()
explicitly reset current/previous/level, while native setup relied on a member
default for level. Root made native setup perform those same explicit resets;
source expectations were unchanged. Mo's main fixture now registers real
TeXAtom, replacing its absent-class descriptor. Narrow kind-query fixtures
remain explicitly scoped. Existing Mo source expectations are unchanged.

atom-semantics-oracle.cjs executes actual pinned CJS methods and independently
generates 6,888 assertions / 74 cases / 924 raw shared-operation rows / 157
source-value records. It covers both receivers, raw states, call/return identity,
virtual dispatch, callback mutations/errors, fresh defaults, property replacement,
inherited attributes, actual parent chains, null/malformed children and partial
construction. Four assertions additionally check the native shared guard budget;
they are explicitly a host-binding check, not a CJS recursion measurement.
Bindings and all affected source oracles reproduce their committed output with
--check. No native result generates expected data.

## Validation

Focused final fixture: build/p15bc-expanded.log. Complete final core build/test:
build/p15bc-core-final2.log, exit 0 with all suites passing. The earlier
p15bc-core-final.log has the fixture setup failure and is rejected evidence.
Earlier generated test compile errors and the isolated Mo missing-module build
are also rejected; fixes changed generator/API/dependency wiring, not source
expectations. See build/p15bc-initial.log, p15bc-focused2.log,
p15bc-focused3.log and p15bc-mo-mutations.log.

All 26 packet mutants compile and fail unchanged assertions with exit 1, including
child ordering/return, virtual dispatch, overwritten stub, duplicate guards,
constructor field/getter/property actions, defaults identity, logical parent,
annotation replacement/empty hook, constructor callback order/partial state and
direct-parent traversal. The 33 existing Mo mutants and 20 class-state mutants
also compile and fail assertions. No build failure or crash counts as detection.
All harnesses confirm working sources/fixtures are byte-unchanged. Results:
build/math-port-atom-semantics-mutations/results.json,
build/math-port-mo-mutations/results.json,
build/math-port-class-state-mutations/results.json. Final logs:
build/p15bc-packet-mutations.log, build/p15bc-mo-mutations2.log,
build/p15bc-class-mutations2.log.

Local source acceptance is 35/246. The remaining 211 sources, HTML host boundary,
MML factory, TeX packages, fonts, Common measurement, shared scene, SVG parity
and Folia integration remain required. Full G0–G7 are closed.
