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
| mml_attributes | New `core/mml_tree/attributes_bindings` pair and narrow actual layer accessors: full emitted Attributes class and its four live layers; parser_projection attaches it to canonical MML nodes |
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

TypeScript `abstract` does not remove a constructor from emitted JavaScript.
The emitted AbstractTokenMap and AbstractParseMap constructors must retain their
real allocation/registration behavior and prototypes. A narrow concrete native
carrier may supply the absent abstract operations with Source-equivalent
behavior; rejecting construction merely because C++ marks a base abstract is
not parity. A writable map field must accept another same-Context Map while
preserving the replacement's identity and live storage. Typed conversions occur
at use, without copying its entries into a second authoritative dictionary.

One emitted PrioritizedList class is shared by its typed native views and the
FunctionList subclass. Its mutable DEFAULTPRIORITY belongs to that Context's
class state. Binding calls pass that default into the existing native add and
addList algorithms, including constructor calls; they never change a global
default or duplicate the algorithm.

Attributes exposes actual global/defaults/inherited/attributes fields in Source
assignment order. Replacing a layer does not silently rebuild the other layers'
prototype links. Methods operate on live records: setList must preserve getter
and target-write interleaving and partial effects when a later getter throws.
All borrowed-runtime callbacks and conversion hooks check the explicit execution
lease before using their providers.

Acceptance requires complete pinned font/next-six packageData comparisons,
existing Map/TokenMap/parser tests, and focused Source/native witnesses for
bidirectional mutation, identity/aliasing, SameValueZero keys, update order,
delete/reinsert/clear during live iteration, stable prioritized record identity,
callback identity and owner release. Unsupported conversions must be reported
explicitly, never replaced with empty objects or missing values. No packet
accepts the complete renderer on its own.
