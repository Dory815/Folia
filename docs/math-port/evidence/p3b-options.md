# P3b Options native port evidence

This packet records the native port of MathJax 4.1.3 `ts/util/Options.ts`.
The upstream source is pinned at SHA256
`b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3`.
The implementation is in `src/math/util/options.ixx`; the root-owned graph
contract is in `options-contracts.md` and `option_types.ixx`. The frozen oracle
fixture `tests/math/reference_options.cpp` was read but not modified. This
completion is evidence for root review, not an acceptance claim.

## Method ledger

| Upstream member and lines | Native implementation | Preserved behavior / evidence |
| --- | --- | --- |
| `OBJECT`, `isObject` 25, 33–39 | `isObject` | Rejects non-object values and functions, then performs ordinary `constructor` reads with the original short-circuit order. If the first read is not Object, it reads `constructor` again before comparing with Expandable; accessors and mutations therefore remain observable. Arrays and instances are classified through their observed constructor rather than structural kind alone; own constructor shadowing is honored. |
| `APPEND` 59, `REMOVE` 73 | exported UTF-16 views | Preserve the exact sentinel strings `[+]` and `[-]`. Sentinel detection uses exact own-key count, key identity and array tests. |
| `OPTIONS.invalidOption`, `OPTIONS.optionError` 78–92 | exported `InvalidOption`, `Diagnostics`, single module-definition `OPTIONS` | Defaults to warn; the installed callback reads the live global mode when called, throws `Exception("Error", message)` for fatal mode, and otherwise sends the exact `MathJax: `-prefixed UTF-16 diagnostic to the replaceable sink. The default sink writes UTF-8 to stderr. `OPTIONS` is one mutable global definition in the module interface; this preserves its public identity and avoids an unresolved MSVC symbol from an exported inline initializer that refers to itself. |
| `Expandable` 97 | Context intrinsic prototype/constructor | Native identity is Context-local and comes from the frozen intrinsic graph. |
| `expandable` 114–116 | `expandable` | Creates a new object with Expandable prototype; copies own enumerable string/symbol values via ordinary get/set, invoking getters and losing source descriptors as `Object.assign` does. |
| `makeArray` 125–127 | `makeArray` | Returns the same array identity when already an array; otherwise creates a one-element array containing the original value. |
| `keys` 136–143 | `keys` | Preserves the falsey early return; otherwise returns enumerable string keys in Context JS order followed by every own symbol, including non-enumerable symbols. Primitive string indices use Context UTF-16 boxing behavior. |
| `copy` 152–170 | `copy` / `run_traversal` copy frames | Reads own descriptors without invoking getters, recursively replaces array/plain/Expandable descriptor values, omits non-enumerable descriptors, retains enumerable descriptor flags/accessor identities, and selects the root Expandable prototype by an ordinary constructor read. It builds the source-compatible descriptor basket through ordinary assignment, preserving `__proto__` behavior instead of skipping that key. `Object.defineProperties` descriptor conversion is staged before definitions. If recursion or a callback removes a later key after the initial key list was captured, the next descriptor lookup throws TypeError as reading `prop.value` would in the source. |
| `insert` 182–274 | `insert` / `run_traversal` insert frames | Follows source key order and validation-before-read sequence; validates with `dst[key] === undefined` and live constructor lookup; invokes the mutable `optionError` callback and skips invalid entries when it returns. Clearing that callback produces a native TypeError at its call site. Mutates mergeable destinations, copies eligible objects otherwise, replaces arrays through an empty array plus nested insert, and assigns scalar/function/instance values directly. Setter/getter side effects flow through Context get/set. |
| `insert` sentinel branch 214–247 | `has_valid_sentinel_shape`, insert frame, `spread_arrays` | Recognizes only the source's exact one-key or two-key forms. The two-key default sort conversion throws TypeError for a Symbol. Removal is first and uses native arrayFilter/indexOf strict equality (so NaN is not removed and object references compare by identity); it reads `sval[REMOVE]` for the condition and again inside each filter callback, preserving getter side effects and mutations. For append, it completes the left array spread before reading `sval[APPEND]`, then spreads that value into a new dense array, materializing holes as undefined. The source arrays are not modified. |
| `defaultOptions` 285–291 | `defaultOptions` | Applies definitions in argument order through `insert(..., false)` and returns the exact destination handle. |
| `userOptions` 302–308 | `userOptions` | Applies definitions in argument order through `insert(..., true)` and returns the exact destination handle. |
| `selectOptions` 318–329 | `selectOptions` | Copies requested own string properties only, shallowly, to a new ordinary object; own undefined is retained, inherited keys are omitted, getters are invoked, and ordinary `__proto__` assignment semantics remain. |
| `selectOptionsFromKeys` 340–345 | `selectOptionsFromKeys` | Uses enumerable own string keys from the selector object (`Object.keys` behavior), then delegates to shallow own-only selection; selector symbols are ignored. |
| `separateOptions` 362–378 | `separateOptions` | Iteratively partitions enumerable own string keys. Selector values are ordinary inherited-aware reads; undefined means continue/remainder, while null and other defined values select. The input is not mutated and the final remainder is returned first. |
| `lookup` 390–392 | `lookup` | Uses own-property presence, then an ordinary read: inherited values are ignored, own undefined is returned as undefined, and getters run. An undefined default argument follows JavaScript default-parameter behavior and becomes null. |

All twelve exported functions are implemented. The contract's native
`Exception` names and UTF-16 messages are used for source errors; recursive
copy/insert stops at the agreed depth 512 with `RangeError` and
`Maximum call stack size exceeded`. Nested copy and insert operations use an
explicit `std::vector<TraversalFrame>` rather than C++ recursion, so the frozen
depth limit is independent of the Windows Debug call-stack size.

The traversal frame state corresponds to the upstream call/continuation order:

| Upstream operation | Frame transition | Resumed behavior |
| --- | --- | --- |
| `copy` descends into an array value | Copy frame `copy_wait_child` → Insert frame at `depth + 1` | Retains the replacement array in the copied descriptor, then performs the original enumerable check and basket assignment. |
| `copy` descends into an object value | Copy frame `copy_wait_child` → Copy frame at `depth + 1` | Uses the returned copied object as the descriptor value, then stores the descriptor in the basket. |
| `insert` recursively merges `dval` and `sval` | Insert frame `insert_wait_child` → Insert frame at `depth + 1` | Keeps already-applied destination mutations and resumes the next captured source key after the child completes. |
| `insert` replaces a source array | Set `dst[key] = []`, read `dst[key]`, then Insert frame at `depth + 1` | Preserves setter/getter effects and resumes after nested insertion. |
| `insert` copies an object value | Insert frame `insert_wait_child` → Copy frame at `depth + 1` | Assigns the child result to `dst[key]` only after the copy completes. |
| `copy` finishes source keys | Copy `copy_define_read`, then `copy_define_apply` | Reads all descriptor values into a staging vector before defining any result property. |

Each frame captures its source key list on entry, as the source `for...of
keys(...)` call does. The frame stack is heap-owned and unwinds automatically on
an exception, while prior destination writes remain visible just as with an
abrupt synchronous nested call. Root resolved the re-entry boundary in revision 2:
each invocation has its own traversal stack, and arbitrary recursive callbacks
authored outside the port are not part of the raw-TeX API. Any reachable ported
callback requiring utility re-entry needs a separate session/depth contract
before its G0 acceptance; no generic JS callback execution is claimed here.

## Frozen profile evidence

The agent did not modify the initial 414-assertion fixture. Root extended it and
independently accepted 593 assertions in 21 tests; see `options-acceptance.md`
for final results. It exercises classification
and constructor shadowing; falsey and boxed-string `keys`; symbols and property
ordering; descriptor-preserving copy and null/undefined TypeErrors; nested
deep-copy identity; array copy and holes; append/remove identity and NaN;
malformed sentinels and Symbol-sort TypeError; warn/fatal order and inherited
defaults; function destinations and accessors; own/shallow selection and
remainder-first separation; default merge order and the `__proto__` descriptor
basket; and Context lifetime/isolation.

The implementation relies only on the frozen Context surface:
`ownKeys`/`enumerableStringKeys`/`ownSymbols`, `descriptor`, `get`/`set`,
`define`, `descriptorObject`/`descriptorFromObject`, prototype and intrinsic
identity, and the native array methods. No parser, renderer, test, oracle,
contract, build, production integration, or dependency file was changed. No
build was run under this packet.

## Remaining review points

The acceptance tests have not been run by this agent, as required by the
packet. Root should compile this module with the shared option graph and run the
frozen `*math_port_options*` tests. Root should also inspect the deliberate
host adaptations: standard C++ callback lifetime for the global diagnostics,
the depth-cap boundary, and native array iteration/filter behavior as provided
by the cooperating `option_array.cpp` unit. Those checks are not exclusions or
accepted gaps; they remain part of root validation.
