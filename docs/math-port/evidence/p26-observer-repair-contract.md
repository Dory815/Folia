# Root three-observer compile repair packet

Only native C++ observation bindings in simple-mml-oracle.cjs,
mfenced-oracle.cjs and maction-oracle.cjs may change. Root regenerates their
reference fixtures, replays original Source and independently compiles all
three against the current complete 90-module graph. No production algorithm,
declaration, compatibility storage, expected value, CJS stimulus/operation,
source loader, case removal, scope pin, build/test command or commit belongs
to this delegated repair. This is compile acceptance only, with zero semantic
test executions; sparse production consumers still have explicit stubs.

Root reran each generator and verified that all original tables, stimuli and
expected suffixes match the existing fixtures exactly after line-ending
normalization. Before repair, maction/mfenced --check pass. simple-mml --check
already fails because its native emitter lags the fixture's tagged AttributeList
hook; the actual original observation suffix is nevertheless identical.
Do not treat that existing emitter drift as a Source behavioral change.

Frozen original profiles: simple 106 cases/7,667 assertions/1,605 raw rows/296
value records; mfenced 225 cases/984 assertions/509 value records; maction 934
cases/935 assertions including the real-tree test/2,318 value records.
Root snapshots live in ignored build/p26-observer-freeze. Before-repair complete
Source suffix hashes (starting at the shared sourceValues table):

| Family | SHA-256 |
|---|---|
| simple-mml | d559fb7c3bcac48e9628532c61ab89eec1ff8d5905214dd4d1b44bfb078a6501 |
| mfenced | a9ea6ff4fa422fc192c29976cf7e7a56dd88cbcd48e0bbdab55605baeccf1f05 |
| maction | cb65ee8cafb0bdb232032b314f56aeea1fa64f70d4d0005b1d2286b2c31298fe |

Adapt probes to existing tagged NodeValue virtual methods and ChildArray.
No shadow pointer child vector, global NodeValue conversion, skip-null fallback
or qualified base call replacing the original virtual callback. A known factory
product can use its raw node pointer; a tagged observed value must follow the
specific original helper's nullish behavior. Preserve callbacks, mutation order,
rebinding, error paths and partial state exactly.

JS map observations capture the receiver and initial length, perform Has/Get,
retain holes in PropertyArray, and preserve explicit Undefined/Null through the
original callback. Original for-of reads every index with live length, producing
Undefined for holes and observing prototype values. Do not use one dense loop
for both. Simple tree/state, mfenced state/tree/held arrays, and maction
children/realNode all need their actual Source observation semantics. Do not
create a new generalized sparse test host or reintroduce dense compatibility.

Luna math_layout owns the simple/mfenced native emitter sections only and writes
p26-simple-mfenced-observer-implementation.md. Luna tex_parser owns the maction
native emitter header only and writes p26-maction-observer-implementation.md.
They may propose required generated C++ call-site type adjustments, but cannot
change JS Source computation or input/expected values. Root owns regenerated
fixtures and verification. Neither agent builds, tests, generates gold, commits
or delegates. Header naming must remain collision-safe for MSVC module builds.

Acceptance requires original replay --check and the same profile counts,
unchanged sourceValues tables and raw stimulus/expected fields, independent
full graph compilation, and a fresh keep-going repository inventory. This can
reduce stale compilation failures; it cannot accept any missing math feature
or pass full core/runtime/backend/Folia gates.
