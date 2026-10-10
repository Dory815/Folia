# P11 operator dictionary — root contract revision 1

Root owns declarations, shared Property graph, array/primitive host services,
pristine data generation, locks, oracle, tests, builds, mutations and acceptance.
All six OperatorDictionary.ts exports are required. mo, full registry and
LinebreakVisitor remain separate unfinished dependencies; no substitutes.

math_layout may write ONLY core/mml_tree/operator_dictionary.cpp and
evidence/p11-operators.md, porting OPDEF/getRange completely. math_fonts may
later write ONLY evidence/p11-data-review.md for a read-only data graph audit.
tex_parser may later write ONLY evidence/p11-host-review.md for a read-only
host audit. Root alone changes shared code. Paths are relative to src/math or
docs/math-port. No other edits, builds, commits, test/oracle changes, production
wiring or scope changes. Report interface gaps instead of inventing repairs.

PropertyArray and PropertyList use the SAME PropertyStorage references in
Property. Array tags, slots, holes, length, own keys and shallow spread preserve
identity and missing-versus-own-undefined. Mixed prototype edges reject cycles.
Attribute provenance tuples now use these actual two-slot arrays; no old-record
fallback. Existing inherited __proto__ tuple behavior must remain, including
prototype identity. No copied property-object bridge or alternate semantic tree.

OPDEF defaults apply to Missing ONLY and allocate a fresh four-slot dense array;
never repair null or coerce incoming values. Default class is TEXCLASS.BIN,
default properties is null. Object/array properties preserve their handle.
Root supplies host::get, truthy, number, debugText and UTF16 codePointAt.

getRange reads infix[text], then prefix, then postfix by truthy short circuit.
Each lookup is direct prototype-aware access; missing/null table receivers throw
at the source read. Truthy inherited functions are hits and slot2 is Missing,
not a REL fallback. A hit allocates fresh [0,0,def[2],mo]. The COMPLETE text is
the key before codePointAt(0). On miss compute first UTF16 codepoint once
(empty→Missing/NaN), capture RANGES for its iterator, use live length/slots, read
upper before lower, and stop at first applicable upper bound even when lower
fails. Return actual matching row Property identity, including object rows;
fallback is fresh REL/mo. Stage reads; C++ argument order is not source order.

MO/RANGES/MMLSPACING/OPTABLE reference pristine generated records. Generator
validates pinned CJS and all graph edges/prototypes/entries. It must not import
common output or bake LinebreakVisitor startup writes into initial data. Retain
all 1,252 entries, 37 shared definition arrays, ranges, spacing and props aliases.

This is a native typed graph, not a JS evaluator. Data properties, mixed
prototypes, sparse indices, default bound intrinsics and source coercion are
native services. Arbitrary descriptors, evaluated functions, custom iterators
and global intrinsic mutation are not exposed. Profile audit finds no array
accessor/iterator overrides or named array writes in dictionary consumers.
Nonbound callable conversion cases throw explicit host errors. Generic array
length property writes currently require numeric Properties; resize takes a
validated integer. Allocation limits are host resources. Default nested joins
use explicit frames and preserve cycles, lengths, holes/null/undefined/coercion.

Before dispatch commit independent CJS fixtures and declarations. Compare every
pristine node's tag/keys/fields/identity, table lookup, range/spacing, OPDEF
defaults/null/freshness, inherited names and Unicode range domain. Add live
mutations/malformed records and host/provenance regressions. Require compiling
mutations failing unchanged assertions and full core suite. Startup mutation
remains LinebreakVisitor-owned; this packet cannot accept the whole visitor.
