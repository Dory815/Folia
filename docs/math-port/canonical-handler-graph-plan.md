# Canonical handler graph responsibilities

The locked font/textmacros observations expose a real Source graph missing
from the native Context projection. SubHandlers currently binds an actual
owner but exposes an empty object. Source exposes its Map, SubHandler owners,
prioritized map records, actual TokenMap/Token/Macro objects and fallback
FunctionList. Reconstructing those fields inside an observer is not a fix.

Root assigns these independent Luna implementation packets:

| Owner | Files and responsibility |
| --- | --- |
| map_storage_design | New `util/ordered_map_storage.ixx`; `map_bindings`, `string_map`, `token_map`, `map_handler`: one generic insertion-ordered dictionary storage and live Context Map projection |
| token_identity_design | New `input/tex/token_map_context_bindings` pair; narrow Token/Macro member access: canonical emitted classes, prototypes, live data fields, callbacks and real map contents |
| font_packages | New `input/tex/handler_context_bindings` pair and optional `util/prioritized_list_bindings` pair: actual SubHandlers/SubHandler fields and stable prioritized array records; existing FunctionList binding for fallbacks |
| parser_projection | ParserRuntime integration, canonical configuration/ParseOptions aliasing and explicit parser-domain conversions; no edits to the other packets' storage/binding implementations |
| Root | Interface review, module enrollment, shared build, complete Source acceptance and production registration |

The generic dictionary owns optional entry slots. Deletion/clear releases the
removed keys and values while retaining cursor positions. Replacing an entry
preserves its position; delete/reinsert appends. Iterators retain the store,
skip deleted entries and observe later insertions. Read the current entry
before a callback can reallocate storage. Context Map operations and typed
native operations address this same store. A view retains storage and explicit
per-Context converters; it owns no second dictionary or eager entry snapshot.
Canonical Map identity comes from that storage, not a temporary adapter.

PrioritizedList is an array of stable record owners, not a Map. Its array view
addresses the actual storage and each record's actual item/priority. Native
and Context mutations share those records. SubHandler._fallback reuses the
actual FunctionList owner and its existing binding.

TokenMap is a TypeScript interface, so it gains no invented constructor or
prototype. Only emitted Source classes are bound. Token and Macro are
independent classes. Preserve original constructors, field descriptors, method
names/lengths, callback identities and inheritance. Explicit parser-domain
conversion hooks belong at the integration boundary; binding code dispatches
existing algorithms and does not reproduce parsing logic.

Acceptance requires complete pinned font/next-six packageData comparisons,
existing Map/TokenMap/parser tests, and focused Source/native witnesses for
bidirectional mutation, identity/aliasing, SameValueZero keys, update order,
delete/reinsert/clear during live iteration, stable prioritized record identity,
callback identity and owner release. Unsupported conversions must be reported
explicitly, never replaced with empty objects or missing values. No packet
accepts the complete renderer on its own.
