# P14 root acceptance — table semantic family

Status: locally accepted for the three complete semantic source files after
root review, full core regression and 39 compiling/assertion-failing mutants.
This does not accept table geometry, a renderer or Folia integration.

Root froze the architecture, interfaces, primitive operations, source hashes,
member inventory, initial actual-CJS observations and exclusive assignments
in bf17c75. Agents could implement only mtable.cpp, mtr.cpp or mtd.cpp and their
individual method report. Root owns declarations, generated bindings, all
tests, build changes, mutations and acceptance. The family has 26 inventoried
members, including 21 executable members: nine agent methods, twelve generated
accessors, three defaults records and two ordered table instance fields.
All members have reviewed mappings in p14-table-members.json.

Mtable owns semantic row inheritance and repair, Mtr/Mlabeledtr own cell
inheritance and repair, and Mtd retains its inferred-row semantics. They use
the accepted uniform tree and MML class hierarchy. Measurement, column sizing,
labels, placement and rendering belong to subsequent Common/scene packets.

## Root review and corrections

Root reviewed all three pinned sources and native bodies, including captured
array/live-slot iteration versus table verification's live property reads,
consecutive non-row grouping, removal-index decrement, direct parent writes,
factory argument shape, error receiver/short flag, superclass calls, shared
alignment records, exhausted-list fallback and raw inheritance inputs.

Root found that the initial table and row output loops checked a null child
at iterator yield. The pinned CJS first updates the shared alignment tuples,
then fails at the child's inheritance-method access. Three new actual-CJS
partial-state witnesses failed the initial native implementation. Revision 2
adds tableSlot to yield the raw pointer and delays the check until dispatch.
Commit 6f62270 fixes both methods and their contracts. This is a source-order
correction, not a formula-specific branch.

The initial deeply nested generated Property expressions exhausted the MSVC
compiler heap. Root emitted 428 flat, deduplicated source-value records and
reconstructs fresh input objects per case. No source rows or observed fields
were removed. The rejected compile is recorded in build/p14-table-initial.log.
Final comparison includes ordered own keys, array kind/length, raw values,
NaN, signed zero and graph identity where relevant. Callback-result indexing
is bounded so behavioral mutations fail assertions rather than crash.

## Independent checks

Both table-bindings-data.cjs --check and table-oracle.cjs --check reproduce
the checked-in files under the pinned TS/CJS source and compiled-package hashes.
The final reference_table.cpp fixture passes 4,700 assertions in 135 cases,
including 240 raw inheritance rows and 160 explicit/default/cramped rules.
Evidence includes all defaults/accessors/class identities, superclass and
derived field phases, alignment aliases and exhaustion, label-parent side,
malformed split receivers, partial failures, array growth/replacement, TeX
class traversal, invalid-parent early returns, virtual verification calls,
actual tree repair with full/short errors, and nested real fraction/root cells.
The focused log is build/p14-table-final.log. Full core regression passes with
the same final sources and fixture in build/p14-core-final.log.

table-mutation-test.cjs builds an unchanged isolated baseline, then 39 copied
mutants in build/math-port-table-mutations. All 39 compile and fail unchanged
assertions with exit code 1. No crash or build failure counts as detection.
The harness confirms exact working-source and fixture bytes after the run.
Results: build/math-port-table-mutations/results.json; final run log:
build/p14-mutations.log.

Detected changes cover indent iteration and receiver order, explicit removal,
display/default and cramped rules, alignment exhaustion/identity and null
receiver timing, row grouping/index repair, fixMtables/error branches and
parent links, previous class state and return values, label alignment/arity,
constructor record replacement and ORD phase, cell linebreak rules, and
non-string split coercion.

Local source acceptance is 32/246. Complete MML registration, remaining
semantic nodes, TeX parsing/packages, fonts, Common measurement, shared scene,
SVG equivalence and Folia integration are still required. Full G0–G7 is closed.
