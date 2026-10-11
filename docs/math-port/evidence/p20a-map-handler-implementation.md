# P20a MapHandler implementation evidence

This report maps the implementation in `src/math/input/tex/map_handler.cpp` to
the frozen MathJax 4.1.3 source `ts/input/tex/MapHandler.ts`, SHA-256
`7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2`. The
source was read from the pinned checkout. This is an implementation ledger,
not a test or acceptance claim.

## Member mapping

| Source member/state | Source lines | Native implementation | Port notes |
|---|---:|---|---|
| module `maps` | 33 | private `mapRegistry()` | Ordered vector retains key/value identity and insertion position; `property_host::sameValueZero` supplies Map key equality. |
| initialized module object `MapHandler` (`register`, `getMap`) | 35–55; methods 42–44, 52–54 | `MapHandler::registerMap`, `MapHandler::getMap` | `registerMap` reads `map.name()` once before lookup/write; replacement retains key position. `getMap` uses SameValueZero and returns null `TokenMapRef` for absence. Null native map registration raises TypeError rather than dereferencing. |
| `SubHandler.FALLBACK` | 61 | static `SubHandler::FALLBACK` definition | One shared `ParseSymbol` identity with description `fallback`; identity is checked before parse-result truthiness. |
| `_configuration` | 63–64 | `SubHandler` inherited `configuration_` declaration | Uses the accepted `PrioritizedList<TokenMapRef>` state. |
| `_fallback` | 65 | `SubHandler` inherited `fallback_` declaration | Uses accepted `FunctionList` state and actual item records. |
| `SubHandler.add` | 74–90 | `SubHandler::add` | Validates supported array protocol, snapshots/spreads then traverses in reverse, performs incremental adds, warns and returns on the first missing map, then conditionally adds truthy fallback. |
| `SubHandler.remove` | 98–108 | `SubHandler::remove` | Iterates the original array live; resolves names by strict equality and removes the map identity; applies fallback truthiness. |
| `SubHandler.parse` | 116–129 | `SubHandler::parse` | Iterates the live prioritized list, calls parse without a contains precheck, checks strict equality against the mutable FALLBACK value before truthiness, then snapshots the fallback list and invokes only its first record with parser/atom slots. Callback result is discarded and native Missing represents source undefined. |
| `SubHandler.lookup` | 139–142 | `SubHandler::lookup` | Uses `applicable`; absence returns `PropertyNull`. |
| `SubHandler.contains` | 151–156 | `SubHandler::contains` | Uses the RTTI `CharacterMapIdentity` marker; only that family performs lookup, and only `PropertyNull` in the character field means false. |
| `SubHandler.toString` | 161–167 | `SubHandler::toString` | Collects names through the live list, then joins using source array-join nullish handling and `", "`. |
| `SubHandler.applicable` | 175–182 | `SubHandler::applicable` | Live list traversal, first map whose `contains` succeeds. |
| `SubHandler.retrieve` | 190–197 | `SubHandler::retrieve` | Live list traversal; map name is compared with source strict equality (so NaN does not match itself). |
| `SubHandler.warn` | 204–206 | `SubHandler::warn` | Prefixes the complete UTF-16 warning once, calls the injected sink or converts the message through the shared UTF-16 text adapter before writing to stdout; sink exceptions propagate. |
| `SubHandlers.map` | 210 | `SubHandlers::map_` declaration | Ordered storage vector models native Map key order and shared retained storage. |
| `SubHandlers.add` | 219–233 | `SubHandlers::add` | Snapshots own keys, performs handler and fallback reads in source order, creates through `get`/`set`, passes the shared warning sink, and delegates. |
| `SubHandlers.remove` | 241–248 | `SubHandlers::remove` | Snapshots keys; does not create handlers or delete entries; stages map/fallback reads in source order. |
| `SubHandlers.set` | 256–258 | `SubHandlers::set` | Replaces values without changing key position, appends new keys. |
| `SubHandlers.get` | 266–268 | `SubHandlers::get` | Returns the current value or null shared pointer for absence. |
| `SubHandlers.retrieve` | 276–284 | `SubHandlers::retrieve` | Reads values live by index; returns the first found map or null shared pointer. |
| `SubHandlers.keys` | 291–293 | inline `SubHandlers::keys()` plus `KeyIterator::next()` | Retains shared map storage, observes appends before exhaustion, and remains done after exhaustion. |

## Shared helpers and host adaptations

The registry uses `Property` keys because the frozen native declaration retains
raw property values. Equality delegates to the accepted SameValueZero adapter;
the per-handler `retrieve` operation deliberately uses strict equality instead.
This preserves the source distinction for NaN while retaining object identity
through shared property storage.

`PropertyArray::spread()` supplies the supported iterable-array snapshot used by
`add`; indexed access to the original `PropertyArray` remains live in `remove`.
Holes and inherited indexed slots therefore follow the shared PropertyArray
protocol. Non-array values raise TypeError at the typed boundary, as specified
by the frozen contract.

FALLBACK comparison uses strict equality for the full mutable `ParseResult`
value, rather than assuming it remains a `ParseSymbolRef`. Null native shared
references compare as JavaScript null across native result alternatives;
non-null references compare by identity, Property numbers retain NaN and signed
zero behavior, and differing non-null alternatives do not compare equal.

The parse fallback call uses `functions::invoke(record, args)` rather than
calling the stored function body directly. The shared helper supplies the
actual prioritized-list record as the callback receiver and validates callable
identity. The argument slots are `reference_wrapper<ParserContext>` and
`ParseAtom(Property(token))`, matching the accepted ParseMethod bridge. The
return value is intentionally ignored.

Character-map detection is RTTI-based. A `TokenRef` supplies its virtual
`character()` value; a property result uses `property_host::get(..., "char")`,
which retains nullish-read TypeError behavior. No concrete token map or regular
expression implementation is included in this packet.

`HandlerConfig.keys()` provides the own-key snapshot; `MethodConfig.get()` is
used for the fallback slot. Reads are staged because C++ function-argument
evaluation order cannot express the source's left-to-right property access.
The `WarningSink` is the frozen native console seam; no additional evaluator or
configuration state is introduced.

## Boundaries and review status

All declarations used here already exist in the root-frozen interfaces. This
unit does not implement concrete token maps, registry construction order,
regexp matching, arbitrary JavaScript property accessors/iterators, parser
integration, or Folia wiring. No build or native oracle was run by this packet;
the root agent owns source review, differential fixtures, mutants, builds, and
acceptance.
