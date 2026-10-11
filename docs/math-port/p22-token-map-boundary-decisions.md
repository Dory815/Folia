# P22 root boundary decisions — before concrete TokenMap dispatch

These decisions resolve architecture, not implementation or acceptance. Root
must still freeze exact declarations, typed value domains, method inventory
and independent actual-Source witnesses before delegating concrete maps.
The two P22 audits are prerequisites, never agent authority to design APIs.

## Single object identity and source construction order

Use a single native shared allocation for each TokenMap. Native allocation
must establish the final derived type and shared ownership before running
Source-equivalent constructor bodies. An allocation-only tag is constructible
only by the native construction host; callers use that one host path. Native
allocation does not perform registration or package/map initialization.

Source initialization then executes the original base-to-derived sequence on
that same object: assign base name/parser, immediately register this shared
object, return from the base initializer, then initialize regex/map storage
and populate entries, and finally assign EnvironmentMap's parser. The native
base may use enable_shared_from_this only after shared ownership exists.
EnvironmentMap's receiver is obtained from that same owner. Registry entries,
returned object and callback receiver must be identical; no copied proxy,
non-owning shared_ptr control block, delayed end-of-construction registration,
raw-this lifetime trick or second object track is allowed.

Virtual getter/population observations occur on the final derived object;
partially initialized Source slots have an explicit missing/null host meaning,
not C++ undefined behavior. If initialization throws after registration, the
registry retains the partially initialized object exactly as Source does.
Source assignment/throw/registration ordering needs actual-source witnesses.
This allocation/initialization distinction follows the established native
Tree ownership-before-initialization pattern; the Tree factory itself is not
reused as a TokenMap factory and supplies no implicit map registration.

## Lookup, callbacks and storage

TokenLookup must carry the actual Macro shared identity alongside TokenRef and
Property. Root resolves imports/forward declarations before editing the bridge;
consumers must explicitly handle the new alternative and preserve Source
missing/null distinctions. A typed mutable Map store uses SameValueZero keys,
has/get/set/delete and insertion order, rather than PropertyList/object rules.
There is one authoritative map store per constructed map.

Concrete map calls retain the accepted FunctionRef identity and shared callback
protocol. Bare saved parser calls use Missing receiver; EnvironmentMap uses
its exact shared object. Source parseResult converts only undefined to true.
Command currentCS restoration occurs only on normal return. Macro.args remains
aliased and is expanded at invocation time; EnvironmentMap passes that same
array as one argument. Getter timing, Object.keys versus Object.entries,
function-map lookup, sparse slices and argument conversion require explicit
Root host-domain/ordering contracts, not helper calls chosen by an agent.

## RegExp dependency and acceptance

RegExpMap's map algorithm and the full native regexp engine are separate
responsibilities. A typed identity-bearing native test service may be frozen
as the map dependency; its live test calls and state changes must be observed
in actual Source. Root may use controlled services in protocol tests, clearly
identified as such. A test double cannot become a production host or count as
engine acceptance. The ignored repaired candidate cannot be adopted while
engine/UTF-16/property/profile/data/license gates remain open; std::regex,
hardcoded command recognizers and JS runtime fallback remain unauthorized.

Complete package configuration, TexParser/Stack/NodeFactory, Common/scene/SVG,
Mermaid SVG/D2D and Folia release work remains required. Local Source count
stays 53/246. Root's next TokenMap step is declaration and actual-Source oracle
freeze under these decisions, followed by constrained implementation and
independent acceptance; neither audit is permission to fill in shortcuts.

## Root follow-on decisions after P22a acceptance

P22a's canonical function-valued Property and sparse slice binding is now
locally accepted. Configured dictionaries/tuples therefore use the existing
single Property graph; do not introduce a parallel callable JSON array or
copy function bodies to bypass the original slot identities.

Resolve the lookup/atom dependency cycle by making Macro a NativeObject and
adding its real shared MacroRef alternative to TokenLookup through a direct
macro-module import. Types remains a leaf of Macro: ParseAtom may carry the
same Macro owner through NativeObjectRef, with explicit checked projection at
typed Macro consumers. Do not declare Macro in the Types named module and
define it in a different named module, or stringify/copy a Macro into Property.
TokenMap's existing interface is the original TS interface; the concrete
AbstractTokenMap family belongs in its own Source-mapped module and implements
that interface. The interface is not an old/new behavior track.

Construction-host declarations and actual-Source witnesses still must be
frozen before implementation: source field assignments happen after the final
shared allocation, with registry/getter observation before parse-map/regex
field initialization. An uninitialized ordinary map is explicit missing host
state that reports the Source TypeError on access, never a usable empty map.
The stored Map uses ordered Property keys and TokenLookup values with the
accepted SameValueZero host, independent of ordinary object enumeration.

Root must extend MapHandler's character projection for the actual new Macro
alternative before it is introduced. Preserve CharacterMap-family gating;
Macro.char is missing, and null reference property access retains TypeError.
The controlled RegExp protocol may verify map call ordering/identity, while
the unaccepted engine stays a separate required production gate. These decisions
authorize no native implementation without exact API and independent corpus.
