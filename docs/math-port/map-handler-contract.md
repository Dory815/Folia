# P20a root-frozen complete MapHandler.ts packet

Source MathJax 4.1.3 fb987178c2d279a99b0db5ea02e751691703955e,
ts/input/tex/MapHandler.ts SHA256
7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2.
All 21 inventoried members/initializers (15 callable members) are required.
Root owns architecture, declarations, host adapters, Source tests/fixtures,
manifests, mutations, review, ledger and acceptance. Luna owns ONLY
src/math/input/tex/map_handler.cpp and
docs/math-port/evidence/p20a-map-handler-implementation.md.

## Supplied declarations and dependency boundaries

Root's token_map_types.ixx declares the exact native TokenMap protocol, not
concrete map algorithms. Mapped values use Property or shared TokenRef (Macro
inherits Token); ParseMethod uses the accepted shared FunctionValue identity;
ParseResult retains Property/StackItemRef/ParseSymbolRef. TokenMap derives the
identity-only NativeObject. CharacterMapIdentity is an RTTI family marker for
the future actual CharacterMap/DelimiterMap hierarchy; no boolean type tag or
token spelling may replace instanceof. This protocol does not accept TokenMap.ts,
RegExpMap, a regex engine or the complete parser. Tests supply maps satisfying
the protocol to test MapHandler, just as Source's TokenMap interface allows.

MapHandler.register is registerMap in C++ (register is a keyword). The module
registry owns shared map references, keys are raw Property values, Map key
equality is SameValueZero (NaN equals NaN; signed zero is equal; object handles
retain identity). register evaluates name once before overwriting; getMap
absence corresponds to Source undefined. No extra reset/alternate registry.
Constructor registration order is a separate required TokenMap packet.

Root's MethodConfig is a native typed dictionary of existing FunctionValue
slots with shared storage, own-undefined shadowing and inherited methods.
It carries no evaluator, arbitrary JS accessor or custom iterator. Its single
entries collection is authoritative. HandlerConfig is accepted PropertyList
with own key order and PropertyArray map-name values. These adapters cover
enabled native package configuration, including callable/scalar fallback
slots and prototype lookup; arbitrary JS program injection is outside the
raw-TeX API. Do not narrow public TeX regex syntax or registered package scope.

## Required exact behavior

Preserve all branches and order in Source MapHandler, SubHandler and SubHandlers.
SubHandler.add snapshots the supported PropertyArray, reverses that snapshot,
looks up each map and adds it at the supplied priority. Missing map warns once
and returns immediately: earlier additions survive, later maps and fallback
are omitted. The warning must be exactly the UTF-16 prefix
"TexParser Warning: " plus Source interpolation. Array holes/inherited indexed
values must yield the Source iterator values; non-array native input is TypeError
in this supported protocol. remove iterates the original supported array live,
uses this.retrieve and removes the last matching identity as PrioritizedList
already specifies. Fallback truthiness uses Property semantics or nonnull
FunctionRef identity, not merely successful variant extraction.

SubHandler.parse iterates the live PrioritizedList directly, calls each
map.parse regardless of contains, checks strict equality to mutable FALLBACK
before truthiness, breaks on FALLBACK and returns the first other truthy result
unchanged. For native results, null shared pointers represent JS null; other
references preserve identity. FALLBACK starts as one Symbol('fallback') identity.
Do not normalize void/false here (parseResult belongs to TokenMap).

After no applicable parse, collect the fallback iterator snapshot and invoke
ONLY its first record with the actual record receiver, parser slot, and token
ParseAtom slot through the accepted functions::invoke protocol. Return Source
undefined regardless of callback result. Empty fallback or noncallable first
item raises TypeError. Preserve callback state/throw identity and mutation.
There is no fabricated safe default or second fallback callback path.

lookup calls this.applicable then the chosen map's lookup, else Source null.
contains calls this.applicable, and for CharacterMapIdentity only reads lookup
value's char and excludes exactly null, not undefined/false/empty. TokenRef
reads character(); Property values use property_host::get(value,"char"),
including nullish TypeError. Non-character map lookup must not be evaluated.
toString collects live map.name values then uses Source array join with ", ";
null/undefined names become empty fields. retrieve compares names with JS strict
equality (NaN does not equal NaN), unlike registry Map key equality.

SubHandlers.add snapshots own keys via HandlerConfig.keys(), reads each value
and fallback slot live, obtains/creates handlers through virtual get/set then
calls subHandler.add. remove does not create handlers or erase their keys.
set replacement preserves original Map key position; get absence is undefined.
retrieve iterates current Map values live, returns first found map, else null.
keys is a live retained-map iterator; appends before exhaustion are seen, and
an exhausted iterator stays done even after later additions. It retains storage
after the SubHandlers object is released. No eager keys/vector snapshot.

warning is a native host seam: the supplied sink receives the complete UTF-16
console message; absent sink writes the same message to stdout. Sink exceptions
propagate and partial configuration state survives. Protected configuration_
and fallback_ allow observational test probes; no alternate state is introduced.

## Independent acceptance before local completion

tools/math-port/map-handler-source.cjs executes actual hash-pinned CJS and
freezes p20a-map-handler-source.json: 24 probes/117 observations initially,
expanded by root to 35 probes/153 observations before final native testing.
Boundary additions preserve independent actual-Source execution.
They cover registry overwrite/SameValueZero/name access, reversed/partial adds,
priorities/duplicates/remove, falsy and identity-bearing results, FALLBACK,
live mutation, actual fallback receiver, empty/noncallable errors, character
null distinction, source own/prototype config lookup and live Map keys.
Root native bindings must replay these stimuli against the actual port;
abstract map bodies in tests are fixtures, not a replacement parser.

Root will run the generated native suite, compiling assertion-failing mutants,
relevant accepted callable/FunctionList/Token regressions and
cmd /c build_test.bat 2>&1. C++23 modules/import std; test TUs use shared
Boost.UT support header without import std. All source members must be mapped
and reviewed before accepting MapHandler.ts. No token map, regex, TeX/Common/SVG
or Folia switch is accepted by this packet. Missing protocol work returns to root;
Luna cannot edit declarations, fixtures, tests, CMake, scope or another file.
