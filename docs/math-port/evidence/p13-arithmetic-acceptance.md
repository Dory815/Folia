# P13 root review — fraction and radical semantic nodes

Status: locally accepted by root after complete source review, regression and
34 compiling/assertion-failing isolated mutants. This is three semantic-node
acceptances; it does not pass a renderer or production gate.

Root froze the three interfaces, constant/default bindings, operation helpers,
source hashes, member inventory, initial actual-CJS oracle and exclusive file
assignments in 8a5e337 before dispatch. Agents implemented exactly two methods
in one CPP each and wrote one report each. They could not change interfaces,
oracles, CMake, data, scope, builds or acceptance. Root separately reviewed all
three pinned TypeScript sources and corresponding native bodies.

Each class remains in the MathJax MML hierarchy. Mfrac owns fraction semantic
inheritance and separate child TeX-class traversal. Msqrt owns its inferred-row
inheritance, and Mroot owns separate radicand/index inheritance. These files
contain no fraction/root geometry, TeX recognition or backend operations.
Root-generated files own six added default fields, ten constant accessors and
the two radical field phases. There are 21 AST members, including 16 executable
members; all have individual reviewed mappings in p13-arithmetic-members.json.

## Root corrections and independent evidence

Root's initial own-spread contract was wrong for the locked runtime profile.
The pinned CJS compiler lowers `{...attributes}` to `__assign({}, attributes)`
using Object.assign. The existing MmlNode helper correctly used that operation.
A new actual-CJS virtual-helper trace inspected keys/prototype before the base
helper could mask the difference and failed the initial arithmeticOwnCopy
implementation. Root revised the contract and renamed/bound arithmeticCopy
accordingly. No shared MmlNode behavior change was needed. This adaptation
matches the compiled distribution; it is not a claim of uncompiled TS spread
semantics. The source and full compiled package hashes are checked by every
oracle through arithmetic-reference.cjs and the established pinned reference.

Root's generated fixture initially had a Boost.UT namespace name collision and
an exception-path result declaration error. Both were corrected without
changing source observations. An iteration mutant subsequently exposed an
unsafe test vector access when the native result had fewer children. The
fixture now bounds that observation and reports an assertion failure. The
initial crash/timeout was rejected as acceptance evidence. Compaction stores
each observed raw value once and scalar row indices; it keeps all 1,440 source
rows and their observed fields. Each native row reconstructs fresh input
objects, so native input mutation cannot modify an expected value.

Final fixtures reproduce exactly under --check:

| Fixture | Assertions | Cases | Source observations |
| --- | ---: | ---: | --- |
| reference_arithmetic.cpp | 13,501 | 7 | All defaults/getters; 1,440 raw display/level/prime rows; alignment/indent fields; shallow aliasing and prototype behavior; captured-array/live iteration; rebinding; malformed children and partial state |
| reference_arithmetic_order.cpp | 56 | 22 | Constructor phases; virtual attribute receiver replacement; helper sequencing/input identity; direct base-helper copying; complete outer inheritance including empty/explicit states; nested real fraction/root trees |

Root reviewed numeric fraction increment versus string-capable root `+ 2`,
the denominator's source `denalign` spelling, unchanged raw protected-hook
inputs, construction of both fraction records before child callbacks, shared
radical attribute handles, receiver-before-argument evaluation, and previous
state transfer before required-child access. Source quirks are retained.

The full core regression passes in build/p13-core-final.log with the final
native implementation and the initial 52-assertion order fixture. The final
expanded and compacted fixtures pass the focused core build in
build/p13-arithmetic-final2.log (13,557 assertions / 29 cases). No production
implementation changed between those checks. The isolated final baseline also
passes both unchanged final fixtures.

## Mutation acceptance

The root harness arithmetic-mutation-test.cjs operates solely on copies in
build/math-port-arithmetic-mutations. It requires every candidate to compile
and fail unchanged assertions with exit code 1; crashes and build failures
never count. It also checks exact working-source and fixture bytes.

An earlier run stopped on an invalid incremental-link COFF archive result;
root rebuilt the affected ignored archive/object and disabled incremental
executable linking in the isolated harness. That failed run was not accepted.
The complete final run passes the unchanged baseline, compiles all 34 mutants
and detects each with assertion exit code 1. Working sources and fixtures are
byte-equal. Results are build/math-port-arithmetic-mutations/results.json;
the final run log is build/p13-mutations-final2.log.

The detections cover conditional/numeric increment versus concatenation,
previous class state, captured/live array iteration, arity truncation, child
previous arguments and return values, fresh copy identity, source denominator
key, display/prime rules, indentation fields, attribute receiver capture,
radical input aliasing, index addition, child receiver timing, field phases,
defaults and compiled shallow-copy semantics. No compile error or crash counts
toward those 34 detections.

These witnesses validate three complete semantic source files over their
accepted dependencies. Full MML registration, remaining semantic kinds, TeX
packages, font services, Common measurement, scene placement, SVG parity and
Folia integration are still required. Full G0–G7 remains closed.
