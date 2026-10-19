# OutputJax FunctionList Context binding evidence

Status: implementation is registered in `folia_math` and was compiled and
linked in Root's Gate 26. Root's focused OutputJax comparison and test run
reported one observer mismatch (`base-defaults` serialized the FunctionList
wrapper instead of its `items` array) and one test-fixture exception (the
inherited setter was passed as a getter). Both local corrections are now in
the candidate/test sources and await the next Root build. Constructor metadata
corrections made after Gate 26 are likewise not yet compiled. This is not an
acceptance report.

## Source behavior and current boundary

The pinned `PrioritizedList.ts` constructs one ordinary `items` array on each
instance. `FunctionList.ts` inherits that property, adds definitions by iterating
the supplied list, iterates `this.items` in `execute`, and captures
`const items = this.items` once in `asyncExecute`. Both paths read the array's
live length/indexed values as they progress. Source code and the already-port
ed native core preserve the ordered list and stable item-record identities,
but the native `PrioritizedList<T>` currently stores only a
`shared_ptr<vector<shared_ptr<PrioritizedListItem<T>>>>`. It has no Context
object identity or ordinary-array representation.

The Context indexed-view protocol now exposes the list's one existing
`StorageRef` as a memoized native array identity. The instance `items` field
returns a view for its current storage, and a retained view stays attached to
that storage after a list rebind. The protocol's inherited descriptor path
also consults native views when the view is used as a prototype, and native
arrays validate the ordinary non-enumerable, non-configurable `length`
descriptor.

## Single-storage implementation

The dedicated `folia.math.util.function_list_bindings` unit owns
the Context-facing FunctionList class/prototype and identity adapters, not a
second FunctionList algorithm or item collection. Each native FunctionList
has one Context instance object; `add`, `addList`, `remove`, `execute`, and
`asyncExecute` dispatch to the same existing typed object and canonical
function bodies. In particular, Context `execute` must call each stored
`FunctionRef`'s canonical Context function object with the Context item-record
object as `this` and pass the same argument values, matching Source's
`item.item(...data)` receiver. The binding must not copy callbacks or introduce
a second method implementation.

The pinned CJS build assigns `PrioritizedList` and `FunctionList` prototype
methods as ordinary properties. Their methods are enumerable, writable, and
configurable; the native Context prototype descriptors now use those same
flags. This differs from modern native ES class method descriptors.
The emitted `FunctionList(list)` constructor has length one and an ordinary
writable, non-enumerable, non-configurable `prototype` property; the binding
preserves that function metadata.

The registry maps the identity of each existing
`PrioritizedList<FunctionValue>::StorageRef` to one Context array `ObjectRef`
and maps each existing `FunctionListItem` shared identity to one Context
ordinary object `ObjectRef`. The array adapter delegates length, indexed
Has/Get/Set/Delete/Define, descriptors, keys, and array intrinsics directly to
that captured `StorageRef`; the record adapter delegates `item` and
`priority` reads/writes to that exact shared item record. It must never
materialize or refresh a parallel vector or Context array.

`FunctionListContextBindings` exposes the owner-preserving `bind`, `resolve`,
`make`, and `view` operations. The Context `items` field reads the current
storage identity and returns its memoized array. Assigning a view produced by
this registry rebinds the FunctionList to the view's captured storage. A
previously retained view keeps its captured storage when the Source property
is rebound. Typed `add`, `remove`, and synchronous `execute` operate on the
selected storage; mutations through the view and typed methods are immediately
visible through one another. Construction binds the list instance before
applying optional definitions.

The item record view is likewise identity-memoized. Reordering/removing a
record must not invalidate a previously returned record object; its `item` and
`priority` remain the fields of that same retained record. Creating a new
record for serialization or copying its fields would violate Source identity.

The existing native indexed-view protocol covers live `length`, own-index
presence, prototype-aware indexed `Get`, `Set`, `Define`, `Delete`,
descriptors, own-key enumeration, and length changes. Standard array methods
route through the same Context operations. The FunctionList provider is the
only FunctionList-specific layer; the general Context host has no
FunctionList-specific branches.

## Focused native coverage and remaining Source gaps

The focused checks exercise Context operations and the typed list together.
They cover the following identity and mutation cases:

1. Repeated `Context.get(functionList, "items")` returns the same array
   identity while the storage is unchanged.
2. `add` and `remove` through the typed FunctionList are visible immediately
   through the retained Context array; `push`, index replacement, and deletion
   through that Context array are visible to subsequent `execute` calls.
3. An item record yielded/read from the array retains identity across later
   insertion/reordering/removal, and its `item`/`priority` fields are live.
4. Replacing `items` with another registry-backed array changes the list's
   current storage and produces a different view. The previously retained
   array still refers to its original storage and does not redirect to the new
   list storage.
5. Mutations during synchronous iteration retain the existing
   `PrioritizedList` iterator's captured-storage/live-length semantics.
6. The general indexed-view protocol reports descriptors/own keys/length
   consistently and consults native-view prototypes for inherited setters and
   read-only descriptors.
7. Context `execute` dispatch reaches the same FunctionList, calls the
   canonical Context function for each retained callback with its item record
   as `this`, forwards arguments, stops only on strict `false`, and propagates
   the callback exception unchanged. A callback that mutates the shared
   filter record must affect the next callback and `executeFilters`' returned
   `data`.

The fresh G27 focused suite passes 98 assertions in 21 tests, and the complete
14-row OutputJax candidate matches the frozen Source oracle. This checkpoint
does not cover async Context Promise objects or foreign array/record
replacement; those remain explicit gaps, as do non-number priorities and
native MathItem/MathDocument/DOMAdaptor Context identities.

## Current limitations

The typed storage stores non-null typed records, while JavaScript arrays can
hold holes, null, arbitrary objects, and explicit `undefined`. Those states
cannot currently be represented by `PrioritizedList`; the `StorageView`
rejects them explicitly. Context conversion of arbitrary native callback
results and `asyncExecute` Promise semantics also remain bounded gaps. The
typed priority field accepts only native numbers; Source JavaScript can store
other values and apply relational coercion during sorting, which this record
type does not represent. The OutputJax typed path cannot convert native
`MathItem`, `MathDocument`, or `DOMAdaptor` objects into Context identities
until their actual owner bindings exist. These gaps are explicit; this report
does not claim full Source parity.

Source references: pinned `ts/util/PrioritizedList.ts` (`constructor`,
`[Symbol.iterator]`, `add`, `remove`) and `ts/util/FunctionList.ts`
(`constructor`, `addList`, `execute`, `asyncExecute`). Native references:
`src/math/util/prioritized_list.ixx`,
`src/math/util/function_list.ixx/.cpp`, `src/math/util/option_types.ixx`,
`src/math/util/option_storage.hpp`, and `src/math/util/option_object.cpp`.

SHA-256: `FunctionList.ts`
`D4106C42268E0D7CB3549F8FFDBEB5BAB87EAE6DAFFAE7314D5E724045A6918F`;
`PrioritizedList.ts`
`A700459F984BA1A5F18BF0E7DC183C242961B94C33587A9AA81BBAACF97180C8`;
native `function_list.ixx`
`BAF4B9F14F2261253C912C020847159D464FAEA23868701B2290208F9C06FFB2`;
native `function_list.cpp`
`30EA61F280D0B18E6349CA56D2E109457CC1E994F1A8BE567640D2BD30C2EE08`.
