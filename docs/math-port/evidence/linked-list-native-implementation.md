# LinkedList and MathList native implementation checkpoint

This checkpoint maps the native implementation to the pinned MathJax
4.1.3 `ts/util/LinkedList.ts` and `ts/core/MathList.ts` sources. The actual
original observer is `tools/math-port/common-layout-reference/linked-list-original-oracle.cjs`;
its frozen output is `build/math-port/linked-list-original.json`. The native
candidate is `tools/math-port/common-layout-reference/linked-list-native-dump.cpp`.
Gate36 registers and compiles the actual production owners and candidate.
Root independently observes all 13 original rows and compares the whole JSON
result, including Source metadata: exact agreement. The focused native suite
also passes 27 assertions in seven tests, including arena release and module
initialization lifetime checks. All 823 shared build-input hashes stay unchanged.

| Source responsibility | Native location | Original candidate row |
| --- | --- | --- |
| Module `END`, circular sentinel, constructor rest values | `util/linked_list.ixx`, `LinkedListModule` / constructors | `end-symbol-and-circular-sentinel`, `push-unshift-and-chain-identity` |
| `push`, `unshift`, `pop`, `shift`, null empty returns and detached links | `LinkedList` methods | `push-unshift-and-chain-identity`, `pop-shift-detach-and-empty-null` |
| `remove` Map membership / SameValueZero and detached nodes | `detail::sameValueZero`, `remove` | `remove-map-membership-and-detach` |
| `clear` endpoint unlinking while preserving interior links | `clear` | `clear-detaches-sentinel-endpoints-only` |
| Forward/reverse live iteration and detached current-node error | `Iterator`, `ReverseIterator` | `forward-and-reverse-iterators-see-live-tail`, `iterator-retains-current-node-after-remove` |
| Virtual `isBefore`, sorted insert, stable sort, merge and callback mutation | `insert`, `sort`, `merge` | `default-insert-dispatches-virtual-isBefore`, `sort-is-stable-and-replaces-sentinel`, `merge-mutates-both-lists-as-source`, `sort-comparator-mutates-original-during-singleton-merge`, `source-iterator-survives-merge-relinks` |
| Source module lifetime and one shared `END` symbol | `util/runtime_modules.ixx`, `LinkedListModule` | `runtime_modules_cache_linked_list_end_once_per_context_and_source_identity`, `runtime_modules_do_not_cache_failed_source_initializer` |
| `AbstractMathList.isBefore` start index then node index | `core/math_list.cpp` | `abstract-math-list-start-i-then-n` |

`RuntimeModules` owns successful module initializations per render Context and
does not cache a failed initializer. Each list receives the module's one END
identity. Iterator termination tests the END data value, so an iterator whose
nodes are relinked into another list stops on that list's sentinel, as the
Source generator does. Arena leases use non-owning node links and an acyclic
arena-forwarding edge after merge; iterators/node handles retain the graph
needed for continued observation without making the node ring an ownership
cycle.

The candidate covers 13 locked original rows, including live comparator
mutation and retained iterators across merge. This is bounded passing native
evidence. General ECMAScript relational
coercion for arbitrary `LinkedList<T>` values is not claimed; the actual
`AbstractMathList` profile uses `MathItem*` and its Source-specific comparator.
That comparator's current differential row uses numeric location fields;
getter order, coercion and malformed location fields still require broader
original observations before complete MathList acceptance.
The CommonOutputJax and full SVG/layout pipeline remain separate acceptance
work.

Ignored results: `build/math-port/linked-list-native-36-root.json` and
`build/math-port/FoliaMathLinkedListTests-36-root.log`. Diagnostic executable
hashes and actual exit codes are in `build/math-port/diagnostic-exits-36-root.json`.
