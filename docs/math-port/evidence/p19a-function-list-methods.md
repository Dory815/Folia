# P19a — FunctionList source members

Pinned source: `.reference/MathJax-src/ts/util/FunctionList.ts`, SHA-256
`d4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f`.
The actual CJS is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/FunctionList.js`,
SHA-256 `571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278`.

| Pinned source member | TS lines | Native implementation | Mapping |
|---|---:|---|---|
| constructor | 47–52 | `FunctionList::initializeFunctions` | Factory helper allocates the derived object first; a non-null input calls the virtual `addList` only after allocation. Null skips; present empty list still dispatches. |
| `addList` | 59–70 | `FunctionList::addList` | Reads the current definition-vector length/entry each turn. Direct items call virtual `add(item)`; tuple entries read item then optional priority and call `add(item, priority)` only when present. Null list/tuple errors; extra tuple fields are ignored; no callable validation occurs here. |
| `execute` | 78–90 | `FunctionList::execute` | Copies argument slots once, then uses the inherited `PrioritizedList` begin/end iterator, matching source `for...of`. The iterator retains the original storage handle and the yielded record identity across callback mutations; a nullable record is checked before invocation. Each invocation receives the captured arguments. Only exact false stops the list. |
| `asyncExecute` | 105–133 | `FunctionList::asyncExecute`, private `AsyncFrame`, `resumeAsync` | Copies arguments and captures item storage before output-promise creation; starts synchronously at index −1. Matching session promises use `input.then(execute).catch(fail)` with the intermediate promise retained by the chain. Fulfillment is ignored; false resolves false, exhaustion resolves true; initial errors reject the output and continuation errors flow through the intermediate rejection and catch. |

The typed host carries each callable as its `FunctionRef` identity or preserves
a non-callable `Property` until `functions::invoke`. Invocation captures the
callable before callback mutation and passes the original prioritized record.
Tuple definitions retain shared tuple identity while their values are read at
the add site. Missing priority uses the virtual one-argument `add` path, so the
current mutable `DEFAULTPRIORITY` is read for each definition.

Synchronous execution delegates array capture, live-length traversal, and
yielded-record retention to the inherited iterator rather than duplicating its
iteration algorithm. It snapshots arguments before validating the captured
storage handle, so a null item array produces a TypeError before iterator
construction. Async frames retain the captured storage array, immutable argument-slot copy,
index, weak queue, and output promise. Each resume locks the queue only for that
synchronous turn. Promise state belongs to the queue arena; continuations hold
the frame without a recursive `std::function` self-cycle or borrowed
`FunctionList` pointer. The code preserves the distinct `then` and `catch`
links; it does not attach rejection handling directly to the input promise.

Native host boundary: definitions are typed vector/tuple handles and priorities
are optional numbers as frozen by the contract. Arbitrary JS iterables,
proxies, accessors, non-number priority coercion, and foreign thenables are not
implemented as a general JS runtime. The actual source oracle, mutation checks,
builds and acceptance remain root-owned; this report is not an acceptance
claim. No build or test was run for this packet.
