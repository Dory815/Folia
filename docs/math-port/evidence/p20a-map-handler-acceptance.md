# P20a root local acceptance — complete MapHandler.ts

Root accepts the complete pinned MathJax 4.1.3 MapHandler.ts responsibility
under ../map-handler-contract.md. Local complete Source acceptance becomes
**52/246**. This accepts no concrete TokenMap class, regexp engine, complete
TeX parser, Common layout, SVG output or Folia switch; full G0–G7 remain open.

Root froze declarations, ownership, equality, callable/receiver, config,
warning and iterator contracts before Luna implementation. Luna owned only
map_handler.cpp and its implementation report. Root owns the independent
actual-Source observations, native stimuli, manifest, mutations and review.

The implementation preserves the module registry's SameValueZero keys and
single name read, reverse snapshot additions, partial additions on missing
maps, priorities and last-identity removal. Parsing uses the live priority
list, calls map.parse without a contains precheck, compares mutable FALLBACK
before truthiness and retains result identity. Fallback invocation snapshots
actual FunctionList records, calls only the first with that record as receiver,
and ignores the callback return. Empty/noncallable fallbacks retain TypeError;
callback and warning exceptions preserve identity and partial state.

CharacterMap family identity is RTTI, with only exact null character values
excluded; non-character lookups are not evaluated. Lookup, applicable,
retrieve, nullish-name join, own-key snapshots and prototype fallback slots
follow Source. SubHandlers retains one ordered map: removing contents retains
handler identity/key position, retrieving values observes live appends, and
keys iterators retain storage and remain exhausted after completion.

Root source review corrected two implementation errors before acceptance:
default warning output assumed Windows wchar_t representation, and FALLBACK
comparison assumed the public static field remained a Symbol. Output now uses
portable std::cout and the accepted lossless WTF-8 host conversion; result
comparison handles native null references, raw Property strict equality and
nonnull shared identities across the complete ParseResult protocol.

map-handler-source.cjs executes actual hash-pinned CJS. Root froze the initial
24/117 observations before dispatch and expanded independent Source probes to
**35 cases/153 observations** before final tests. It checks registry keys,
priorities/duplicates, partial failure, all falsy/truthy result families,
mutable FALLBACK, live mutations, fallback receiver/result/throw identity,
character-null distinctions, configuration prototypes, retained live keys,
handler preservation, live remove arrays, mutable default priority and UTF-16
warning arguments including an isolated surrogate. map_handler_source.inc is
generated from these observations; --check reproduces both committed artifacts.

The native suite passes **198 assertions in 22 tests**: 153 Source observations,
35 vector-size checks and ten native protocol/result/lifetime witnesses.
Interface-fixture token maps replay observed Source call shapes; they do not
implement, substitute for or accept any TokenMap parser algorithm. Protected
member pointers inspect actual production base handlers without replacing
factory-created instances or casting them to fixture subclasses.

All **29** isolated mutants compile and exit 1 through failed assertions.
They alter registry equality/name reads, order/partial adds/priorities,
removals, contains prechecking, FALLBACK/null/NaN comparisons, live iteration,
fallback defaults/results, lookup/character-family/null semantics, join,
warning prefix/stdout, handler preservation, key order/exhaustion/retention,
prototype shadowing and remove snapshots. Results are
p20a-map-handler-mutations.json. Working implementations and fixtures were
byte-checked unchanged; compile failures, crashes and empty baselines reject
the run.

Mutation review first caught unsafe fixture indexing when expected elements
were absent; root added guarded observational reads. A handler-replacement
mutant then passed the earlier assertions; root added an actual-CJS identity
probe and reran all mutants. Neither event was counted as acceptance, and no
existing Source expectation was relaxed.

p20a-map-handler-members.json maps all **21 inventoried members/initializers**
plus the object's **two actual method bodies** (register/getMap), whose source
hashes are checked separately. HandlerConfig/FallbackConfig aliases bind to
the frozen native dictionaries. The checker verifies every Source member hash
and target against fresh TypeScript 5.9.3 inventory.

Root reran relevant FunctionList, callable/receiver, argument-expansion,
Token/Macro and primitive oracles in --check mode; existing fixtures remain
unchanged. Final `cmd /c build_test.bat 2>&1` completes with exit 0, including
the final 198 assertions and accepted callable/token/primitive regressions.
Ignored log: build/p20a-final-full-core.log. Mutation log:
build/p20a-map-handler-mutations.log. No platform or application path changed.
