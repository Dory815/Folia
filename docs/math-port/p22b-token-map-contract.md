# P22b Root freeze — complete TokenMap.ts class family

Root owns declarations, construction/storage protocols, Source corpus, tests,
mutation matrix and acceptance. Luna may edit only
`src/math/input/tex/token_map.cpp` and
`docs/math-port/evidence/p22b-token-map-implementation.md`. No headers, APIs,
modules, build files, tests, fixtures, expectations, commits, other Source
implementations, integration or additional delegation are authorized.

## Source and ownership

Original MathJax 4.1.3 TokenMap.ts SHA256:
`0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e`.
The complete executable inventory is 22 functions/accessors/constructors plus
the AbstractParseMap.map initialized field, detailed with exact lines/order in
`evidence/p22-token-map-native-boundary-audit.md`. Port every branch; abstract
type/interface declarations remain declarations. Source module responsibilities
live in `input/tex/token_map.ixx/.cpp`, preserving original inheritance:
AbstractTokenMap → RegExpMap or AbstractParseMap → CharacterMap/DelimiterMap
or MacroMap/CommandMap/EnvironmentMap. Do not invent a replacement parser or
collapse the classes into a command switch.

Native declarations bind the original TS TokenMap interface from
token_map_types. `TokenLookup` carries Property, TokenRef and real MacroRef.
Macro inherits the existing NativeObject leaf, and its same owner can cross
ParseAtom as NativeObjectRef; this avoids illegal forward declarations across
named modules. Root extends MapHandler's existing CharacterMap-family projection
for Macro's missing char and null-reference TypeError. No second object/value
track is introduced. Preserve existing callable/receiver and argument protocols.

## Exact construction order and partially initialized state

`TokenMapHost::create<T>` performs the sole final shared allocation with the
private allocation-only tag, then calls Source initialization. Native tag
constructors only establish empty/Missing field storage and final virtual type;
they must not register or run Source initializers. Copy/move of map identities
is disabled. Do not bypass the frozen host, use nonowning shared_ptr aliases,
copy an object, or register at the end of derived construction.

AbstractTokenMap initializer assigns name then parser and immediately calls
MapHandler::registerMap on its existing shared owner. Registration uses the
final virtual name getter once, before regexp or ordinary Map field initialization.
An observer may see missing fields, invoke methods and throw at that point.
Uninitialized map access reports Source TypeError, not empty-map behavior.
After registration AbstractParseMap initializes its single ordered Map storage;
RegExpMap assigns the same RegExpRef. CharacterMap/MacroMap then populate through
virtual add. EnvironmentMap sets its general parser through the virtual setter
only after MacroMap has completed. If any initializer throws after registration,
the registry retains that partially initialized same object.

FunctionMap's default fresh object is selected only for Missing/undefined and
before super initialization as Source does. Explicit null stays null. A derived
initializer must explicitly call each appropriate Source base initializer once.
Use inherited initialization for DelimiterMap/CommandMap. Do not invoke public
virtual parser setter for the base's private `_parser` parameter assignment.

## Native storage/data/dispatch profile

Configured dictionaries and Args retain the existing authoritative Property
graph with callable FunctionRef values. The ordinary Map field is one optional
ordered vector of Property keys/TokenLookup values: get missing→undefined,
has independent of value, SameValueZero key equality, replace-in-place, deletion/
reinsertion order, zero-key canonicalization to positive zero. This is a native
Map binding, not PropertyList/object property rules. No compatibility store.

CharacterMap snapshots Object.keys first, then gets each value at its turn.
String value yields char unchanged and null attributes. Original CJS __read
limits array iteration to two slots: for arrays read only indices within its
snapshot length, including holes/inherited slots; indices outside length become
undefined even if that array's custom prototype has them. For supported ordinary
noniterable data values __read returns the value, then indexed Get supplies 0/1.
Nullish access reports TypeError. Do not substitute unlimited argument expansion
or blindly get outside array length. Token objects and attribute handles retain
identity. Numeric key strings remain distinct from numeric Map keys.

MacroMap snapshots Object.entries key/value pairs before virtual add/population;
later mutation of the JSON object must not change captured values. Resolve
string method names from the supplied functionMap using the accepted property
host, retaining its prototype data lookup. Array form gets slot zero directly,
then Source sparse slice(1); scalar form gets a fresh empty Args array. Use the
accepted raw/typed callable conversions without cloning bodies. Store MacroRef.

All parse dispatch reads happen in Source order. Base parserFor calls contains
and reads virtual parser only when applicable; base parse calls parserFor then
lookup regardless of parser truthiness. Map results and methods use JS truthiness;
only undefined parse results become true. Saved bare methods have Missing receiver.
RegExpMap lookup calls contains again, so inherited parse can consume two live
test calls even when the first succeeded. RegExpRef is a controlled, identity-
bearing native dependency; this packet implements no regex engine or fallback.

Macro/Command/Environment parse performs two virtual lookups: first captures the
Macro, second (inside parserFor) selects its method. Do not reuse the first lookup
or refresh the captured Macro. Read its token/args live after selecting the method.
Ordinary raw Property/TokenRef alternatives need Source property/missing-field
handling where reachable; null/falsy references short-circuit without a getter.
Base Macro mapped atoms retain NativeObjectRef identity, not copied token text.

Command: save currentCS, assign backslash+input token using Source default
ToPrimitive/string concatenation, then read backslash+Macro token, expand current
Args, call bare parser, restore currentCS only on normal return, then parseResult.
Body/argument/getter exceptions leave the changed currentCS and preserve thrown
identity; no RAII finally restoration. Delimiter calls the original base parse
with backslash+token and retains subsequent virtual dispatch.

Environment: after guards read general virtual parser, then captured Macro token,
then pass envParser and same Macro Args array as two unspread rest slots. Its
receiver is the exact shared EnvironmentMap owner. Flatten the method rest slot
to the canonical FunctionRef/Property expected by the established Function bridge.
Use explicit locals for ordered getter evaluation; C++ argument evaluation order
cannot stand in for JavaScript left-to-right evaluation.

The host profile is ordinary configured own/prototype data dictionaries/arrays,
existing typed virtual accessors/handlers and configured native callables. This
does not bind arbitrary JS proxies/custom iterator/species/function descriptors.
The actual Folia-enabled package/parser dependency closure remains required; any
configured callsite needing another host primitive must be reported to Root.
No profile exception may become a silent behavior fallback. Regex engine,
parser/Stack/NodeFactory/Common/scene/SVG/Folia and parallel Mermaid remain gated.

## Independent frozen acceptance before delegation

Root's actual Node 24.13.0/CJS oracle guards TokenMap, Token and MapHandler bytes,
then executes original constructors/methods with controlled subclass obligations,
accessor traces and a clearly controlled RegExp dependency. It generates
**208 base-dispatch cases, four regex call-sequence cases and 220 flags in twelve
groups**, independently of native code. Source fixture SHA256:
`470dba782f01758d0027a1c50bb59b52368ea367d58da8cd9880046f9c738f05`.
`token-map-oracle.cjs --check` checks fixture and generated native test byte content.
Root controls `token-map-native.inc` and the generated Boost.UT test; agents may
not amend either. Case output includes errors, values/identities and exact events.

Coverage includes every class and executable member/initialized field, falsey/
truthy mapping/parser/results, undefined-only parseResult, partial registration/
constructor failure, overwrite and ordered SameValueZero keys, character arrays
with inherited slots beyond/within length, missing attrs, WTF-8 surrogate units,
different enumeration mutation timings, raw/typed functions, sparse Args, inherited
method names, mutable/two-lookup Macro dispatch, delimiter concatenation, command
exception state, environment same-owner receiver/unspread Args and regex live calls.

Native stubs must compile and fail before delegation. The initial build failed
because the Root BaseMap test inherited a protected allocation constructor;
Root added an explicit test-only forwarding constructor. That compilation error
is not an accepted red test. The corrected final focused build compiled and
executed all thirteen tests; each failed at a named P22b implementation stub,
exit 1. Raw log `build/p22b-token-map-final-red.log` SHA256:
`79fc5fd2a1325163672ebb404732219809e7e95cc98ff9e9c1a6f084c4d0eae3`.
Boost.UT's exception summary underflows its passed counter; this is thirteen
real stub failures, not a passed assertion total. Source regeneration/check is
green independently. This complete packet is frozen before implementation.
Final acceptance requires unchanged independent Source parity, compiling semantic
mutants rejected by actual assertions without crashes, relevant complete core
build/tests and independent diff/source review. Agent completion passes no gate.

Frozen mutation families: late registration; map initialized before registration;
copied/wrong receiver or retained body; lost initialization partial state; name
read twice; parser getter timing; lookup skipped/refreshed/snapshot; regex double
test erased; wrong Map key equality/zero/order/value-presence; eager Character
values vs delayed Macro entries; pair reads beyond length; lost/null attrs;
function-map/default/slice alias changes; parser-for missing vs null; undefined
or false result collapse; Args snapshot/compaction; missing backslash; command
restoration on throw; spread environment Args; method/receiver identity loss.
