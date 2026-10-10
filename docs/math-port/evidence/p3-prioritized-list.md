# P3a PrioritizedList evidence

Reference: MathJax-src 4.1.3 `ts/util/PrioritizedList.ts`, SHA256
`a700459f984ba1a5f18bf0e7dc183c242961b94c33587a9aa81bbaacf97180c8`.
Native target: `src/math/util/prioritized_list.ixx`, module
`folia.math.util.prioritized_list`.

| Upstream member | Native member | Fidelity and adaptation |
| --- | --- | --- |
| `PrioritizedListItem<DataClass>` | `PrioritizedListItem<T>` | Preserves mutable `priority` and `item` fields. |
| `DEFAULTPRIORITY = 5` | `inline static double& DEFAULTPRIORITY` | Mutable and shared through one module-level backing double, including across `T` specializations. This preserves TypeScript's erased-generic class property. |
| Constructor initializes `items = []` | Default constructor with empty `items_` | Empty list behavior is identical. |
| `[Symbol.iterator]()` | `begin()`, dynamic `end()`, and nested input `iterator` | Range iteration yields mutable item records. The default-constructible iterator stores owner identity, an integer cursor, and a shared pointer cache acquired on first dereference and retained until increment. Removing the current record therefore does not destroy a reference already yielded during the loop body. End comparison reads current size, treats end sentinels as equal, and preserves `items[i++]` behavior across mutations. The explicit JS `next()` result object is adapted to C++ range iteration; there is no separate `next()` API. |
| `add(item, priority = DEFAULTPRIORITY)` | `add(T item, double priority = DEFAULTPRIORITY)` | Scans backward while `priority < existing.priority` and inserts after ties. This retains the exact IEEE comparison behavior for NaN and infinities and preserves stable order for equal priorities. Returns the item by value as requested by the frozen native contract. |
| `remove(item)` | `remove(const T&)` | Scans from the end, uses `==` (NaN remains unequal to itself), erases only the last match, and returns `*this` for chaining. Scalar same-type equality models JS strict equality for the supported scalar types; `std::shared_ptr` equality compares stored pointer identity, not callback contents. |

Storage uses a vector of shared owning pointers to item records. The ordered
array can shift on insertion/removal without invalidating references to entries
that remain present; an iterator cache also keeps a removed current record alive
through the current loop body, matching the JavaScript value already returned
by `next()`. Iterators never retain vector iterators.

Known adaptations and limits: this C++ template cannot reproduce JavaScript's
cross-type coercion rules, which `!==` never performs in this source anyway.
Callers should use scalar `T` or identity handles such as `std::shared_ptr<T>`;
custom equality types are responsible for implementing identity equality.
The native API does not expose the source's protected backing `items` array or
its iterator-result object. No comparator/oracle behavior, option composition,
or other `PrioritizedList` behavior is claimed by this packet. Root-owned
reference fixtures and build validation remain the acceptance evidence.
