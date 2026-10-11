# Root P19 prerequisite audit — implementation not yet authorized

After complete MML action/registry acceptance, root read complete TokenMap.ts,
MapHandler.ts, FunctionList.ts and existing native token/parse/priority seams.
Two constrained Luna audits independently checked actual installed CJS and
required regex/sentinel call sites. They made no edits/builds/tests and selected
no substitute architecture. This is evidence for the next contract, not source
acceptance or permission for agents to invent interfaces.

Pinned SHA256:

- TokenMap.ts: 0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e.
- MapHandler.ts: 7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2.
- FunctionList.ts: d4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f.

The pinned TypeScript AST inventory has 23 TokenMap members, 21 MapHandler
members and four FunctionList members. Initialized module objects/constructor
parameter fields also carry executable behavior; MapHandler's register/getMap
must be explicitly reviewed within its initialized-global body, not omitted
because they are object methods rather than class methods.

TokenMap owns the original AbstractTokenMap/RegExpMap/AbstractParseMap/
CharacterMap/DelimiterMap/MacroMap/CommandMap/EnvironmentMap hierarchy. It does
not own TeX parsing or reduce macros to names. Base construction assigns name
and parser, then registers the same partially initialized object before derived
regex/map/body fields. Native adoption/initialization must retain object identity,
constructor phase and actual virtual reads as the accepted MML binding does.
Private map storage is not ready at base registration time; an eagerly usable
empty map would alter observable constructor behavior. Registration and global
ownership need an acyclic typed interface, not duplicate token registries.

parseResult maps only Missing to true. Abstract parsing reads parserFor then
lookup even if the first result is falsy. Macro parsing reads lookup then another
lookup through parserFor; captured macro and current callable need not agree
after callbacks. Command parsing writes currentCS before reading/calling the
actual macro arguments, and restores it only on normal return. A finally-based
restoration changes source exception state. Environment parsing supplies the
environment callable and original args array as two rest arguments, not spread
arguments. Constructor Object.keys versus Object.entries/slice behavior, fresh
argument arrays and original token/attribute/function identities are required.
The installed CJS iterator/spread lowering also needs review for raw non-array
Macro args already allowed by the accepted foundation, including UTF16 strings,
null errors and generic indexed/length reads; do not silently normalize them.

All RegExpMap source construction sites are in enabled base/ams packages:
BaseMappings uses /[a-z]/i, /[0-9.,]/ and /^\\/; AmsMappings uses /[-*]/i.
BaseConfiguration later replaces letter/digit registrations with the current
initialLetter/initialDigit options, whose defaults are /[a-zA-Z]/ and /[0-9.,]/.
Caller-supplied regex objects retain identity/flags/lastIndex. Contains is a live
test call; lookup calls contains again. Base parse may therefore test twice,
and applicable plus lookup may make another test. A single cached match is
incorrect for g/y regexes. Root must freeze a real typed regex-test host and
prove every required default grammar and stateful callback behavior. Native
Unicode lowercase is not JavaScript RegExp ignoreCase canonicalization.

MapHandler owns one live name registry and SubHandler(s). Replacement keeps
global key position; previously configured handlers retain old map identities.
add copies/reverses names and adds immediately in stable priority order; a
missing name warns/returns with partial state and omits that call's fallback.
remove visits forward and removes one last identity occurrence. SubHandlers
insert the new handler before add, retain empty handlers and iterate handler
config keys in actual Object.keys order. Lookup/applicable/contains must retain
CharacterMap's special null-character check and exact first applicable behavior.

SubHandler.FALLBACK is one identity-bearing mutable symbol. NewcommandUtil's
only external use reads that static field at call time. Descriptions are not
symbol identity. Map scanning stops on exact current sentinel, returns other
truthy values and continues over falsy values. After stopping/exhaustion only
the first prioritized fallback is called; its return is discarded. Missing
fallback throws. Ordinary exceptions propagate. This differs from a token map
callback returning Missing, which parseResult turns into successful true.

FunctionList is a complete prerequisite with constructor/addList/execute/
asyncExecute; accepting only its synchronous subset would leave the source
file incomplete. Callbacks receive their prioritized item record as method
receiver. Execute stops only on exact false and sees list mutations. Async
execution captures the actual items array and runs serially: recognized native
Promise results control continuation, fulfilled values including false are
ignored, direct false stops, exceptions/rejections reject, and non-Promise
thenables are ordinary results. Root must specify native promise scheduling,
owned captured argument/storage lifetimes and callback/exception behavior.
The current priority list's private storage access/lifetime requires a narrow
reviewed boundary; a second list or copied snapshot would change semantics.

Before implementation root will freeze interfaces, module/cycle/ownership,
source-field phases, regex/callable/async services and per-file assignments.
Independent actual-CJS observations must cover complete source members,
registration/configuration order, aliases, callbacks, raw results/arguments,
currentCS and partial failures, regex test counts/lastIndex, live sentinel,
and synchronous/asynchronous list mutation/scheduling/lifetimes. Require
compiling semantic mutants and full core regression. Neither this audit nor
the already accepted MML layer passes TeX/Common/SVG/Folia gates.
