# P22b Root local acceptance — complete TokenMap class family

Root accepts all 22 executable members and the initialized Map field of pinned
MathJax 4.1.3 `ts/input/tex/TokenMap.ts`, under the explicitly frozen native host
profile in ../p22b-token-map-contract.md. Local complete Source acceptance becomes
**54/246**. The controlled RegExpContext service proves map dispatch only; no
regex engine, complete configured TeX parser, Common layout, SVG renderer or
Folia switch is accepted. Full G0–G7 and parallel Mermaid gates remain open.

Root froze declarations, construction/registration/storage/identity protocols,
actual Source corpus and compiled failing stubs in commit 2d10234 before Luna
implementation. Luna owned only token_map.cpp and its member mapping report.
Root owns Source observation, fixtures, tests, mutations, build and acceptance.

The original inheritance and per-source file responsibility remain intact:
AbstractTokenMap, RegExpMap, AbstractParseMap, CharacterMap/DelimiterMap and
MacroMap/CommandMap/EnvironmentMap. One final shared allocation precedes Source
initialization; registration observes final virtual type and Missing derived
fields, retaining the same partially initialized owner on later exceptions.
The Map field initializes after registration and uses one ordered SameValueZero
store. Callable, Token/Macro, arguments and receiver identities reuse the
authoritative native Property/callable graph; no second parser/object model.

Root source review checked every constructor/accessor/method/field, including
single virtual name read, private base parser assignment, presence independent
of map value, +0 key canonicalization, replacement/reinsertion order, delayed
Character keys/value reads and bounded pair iteration, eager Macro entries,
prototype method lookup, canonical functions and sparse slice. Dispatch stages
all virtual reads explicitly, retains two Macro lookups, uses the first object's
live token/Args and second method, preserves RegExp live double-test behavior,
and applies parseResult only to undefined. Command restores currentCS solely on
normal return. Environment captures general parser first, invokes with its exact
owner and passes method and original Args as two unspread slots. Delimiter uses
the original base parse and default-hint backslash concatenation.

Independent actual Node 24.13.0/CJS observations remain **208 base cases, four
regex sequences and 220 flags in twelve groups**, Source fixture SHA256
`470dba782f01758d0027a1c50bb59b52368ea367d58da8cd9880046f9c738f05`.
All three original CJS hashes and generated test bytes are verified by
`token-map-oracle.cjs --check`. Controlled abstract obligations/accessors/regex
service do not replace any algorithm under test or accept that service's engine.
Native parity is **870 assertions in 13 tests**.

All **41** isolated semantic mutants compile and exit 1 through failed tests.
They cover result collapse, registration/name/parser timing, derived-field order,
lookup/receiver/atom identities, regex repeated calls, Map equality/presence/
zero/order/removal, Character enumeration/bounded reads/attrs, Macro entries/
function-map defaults/name resolution/slice/two lookups, delimiter/command
prefixes and exception restoration, and Environment setter/receiver/method slot/
spread/Args alias. Working implementations and fixture bytes are checked unchanged.
Results: p22b-token-map-mutations.json.

The initial environment-spread mutant exposed an unsafe Root test observer:
it dereferenced a missing array after a malformed argument shape. That crash
rejected the run and is not a detection. Root guarded observed receiver/rest/
array shapes, retaining every Source expected value and the same 870 assertions,
then reran all 41 mutants successfully. Production implementation was unchanged.

TokenLookup now exports MacroRef; Root mechanically added macro.ixx/.cpp to the
old isolated MapHandler module graph. Original MapHandler fixtures/algorithms
remain unchanged: **198 assertions/22 tests and all 29 mutants pass**. The
existing callable/receiver matrix also passes **79 assertions and 12 mutants**.
These regression runs do not reopen or expand the accepted regex/JS host profile.

Complete `cmd /c build_test.bat 2>&1` finishes exit 0:
**87 suites, 3,940 tests, 161,915 assertions**. Raw
`build/p22b-token-map-final-core.log` SHA256:
`4920ca38cf2cdcebbd97841c5ccf0926fd07639af1a8e83bc15e1f0c9ee125a0`.
No editor/platform or Folia rendering route changed. Native implementation LF
SHA256: `0f8f4e130b36f65a5045b5574b9400835910e8bc3679eabb7584c8aa44a9c249`.

The member ledger independently regenerates pinned TS AST inventory and binds
all 23 rows to native definitions and this evidence. This local acceptance
does not support a claim that all required MathJax features are ported. The
complete enabled package/parser/dependency closure, sparse node factory boundary,
general regexp host, shared Common result, both consumers and integration remain
required; arbitrary JS proxy/iterator/species/function metadata is not silently
substituted or delegated as a behavior fallback.
