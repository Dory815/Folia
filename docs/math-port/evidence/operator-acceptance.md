# Root audit of P11 operator dictionary

Status: local root acceptance passed under the explicit typed host scope.
This packet covers all six OperatorDictionary.ts executable/data exports:
OPDEF, MO, RANGES, getRange, MMLSPACING and OPTABLE. The AST ledger contains
two functions and four initialized globals, with member hashes and native
targets; source/runtime locks are in p11-source-lock.json. It accepts neither
MmlMo nor LinebreakVisitor nor the complete MathJax renderer.

Root froze declarations, host graph, source locks, pristine generator and
9,840 CJS assertions in f027581 before dispatch. math_layout could implement
ONLY operator_dictionary.cpp and its evidence; the other Luna agents performed
read-only data/host audits. Root owns architecture, all shared code, fixture
extensions, builds, mutation runs, source review and acceptance. Implementation,
host corrections and final fixed fixtures are separate reviewable commits.

## Source/data review

The pristine graph has 129 shared object/array nodes, 27 named MO definitions,
50 range rows, eight spacing rows, and 1,252 operator keys referencing 37
distinct definition arrays. All own keys, scalar values, array lengths/tags,
standard prototype identities and alias edges compare against pinned CJS.
Neither the 33 postfix-fence startup writes nor the U+2061 definition clone
from LinebreakVisitor is baked into this graph; the future visitor owns them.

OPDEF applies defaults to Missing only, keeps raw values and reference identity,
and allocates a fresh dense four-slot array. getRange preserves full-text keys,
ordered prototype-aware truthy form lookup, inherited-function hits, UTF16
codePointAt, upper-before-lower comparison, immediate gap break, captured live
range handles and actual matching row identity. Nullish receivers fail at the
corresponding source read. No numeric/string repair or approximate lookup exists.

The shared host uses one PropertyStorage graph for objects AND arrays. Attribute
provenance now has actual two-slot arrays with no old-record fallback. Sparse
slots distinguish holes from present Missing; shallow spread, own keys, length,
mixed prototype edges and array coercion retain their scoped source behavior.
Root corrected the review's missing has slots/intrinsics and inconsistent Array
prototype parent/setter findings. The host's nontrivial TLS traversal state lives
in one implementation unit after MSVC imported-inline TLS produced a linker
failure. Iterative default joins cover deep nesting and recursive cycles.

## Verification

Commands: operator-data.cjs --check, operator-oracle.cjs --check,
cmd /c build_test.bat 2>&1, and operator-mutation-test.cjs. All generated output
and mutation copies/results remain in ignored build directories.

The final immutable fixture contains 13,578 CJS-backed differential/native-host
assertions in nine cases. It checks the complete pristine graph and every table
key, every pristine range boundary, all 1,114,112 codepoint values (including
unpaired surrogate code units) against the range algorithm with empty form
tables, missing/null/raw OPDEF parameters, table priority and short circuit,
live named-definition aliases, malformed receivers/rows, sparse arrays,
prototype assignment, coercion, cycles and 2,048 nested array levels.
The full-domain test does not claim pristine operator lookup across that domain;
all pristine keys and boundary lookups are independent checks.

The initial extended alias fixture failed because root restored source table
handles but omitted the corresponding native restore before the alias phase.
Root fixed only that setup; pinned upstream expectations were unchanged. The
final complete core suite passes, including the previously accepted MML,
leaf and script fixtures after the shared array/tuple changes.

All 21 deliberately incorrect implementations compile and exit 1 through
unchanged assertion failures: default/null/coercion/identity changes; form
priority, inherited lookup, truthiness and eager reads; first-unit Unicode,
exclusive endpoints, gap continuation, copied rows and fallback class; baked
startup writes, broken definition aliases, hole/spread semantics and Array
prototype parent. Each result records the focused immutable case pattern after
the complete immutable baseline passes. Working implementation and fixture
bytes are unchanged. Ignored logs/results live in build/math-port-operator-mutations.

## Explicit boundary and remaining work

Property/array services are compiled native bindings, not a JS VM. Arbitrary
descriptors, evaluated functions, custom iterators and global intrinsic mutation
are not exposed. Array length writes require validated numeric Properties;
allocation/ownership are native host resources. Known default nested joins use
comma coercion; future source join(separator) callers need their own reviewed
binding. Unbound evaluated conversions fail explicitly. See operator-contracts.md.
Malformed native-host tests check exception presence and short-circuit behavior;
they do not establish a general JS engine diagnostic-message contract. Real
TeX diagnostics remain an independent full-pipeline G1/G7 obligation.

The full MML registry, MmlMo, remaining node families, TeX parser, fonts, Common
layout, shared scene, SVG, Direct2D verification and Folia replacement remain
unfinished. Full G0–G7 remain closed. This packet does not replace QuickJS.
25 of 246 inventoried sources have local acceptance; the remaining 221 source
entries and unreviewed external dependencies still block full G0.
