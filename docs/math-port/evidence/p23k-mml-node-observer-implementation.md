# P23k MmlNode native observer migration

## Scope and result

Updated only the generated C++ observer layer in
`tools/math-port/mml-node-oracle.cjs` and regenerated
`tests/math/reference_mml_node.cpp`. The authorized oracle run reported
**1,084 assertions / 43 cases**, unchanged from the frozen contract. The
JavaScript source classes, stimuli, Source calls, expected values, case names,
assertion accounting, and pinned inputs were left intact. No native build or
test was run; this is an observer migration, not an implementation acceptance.

## Observer mapping

| Legacy observer operation | Current native observation |
| --- | --- |
| Dense `vector<MmlNode*>` child fixture | `ChildArray<MmlNode>` populated with tagged `NodeValue`s. |
| Child index via `operator[]` and pointer traversal | `size()`/`get()` through `valueAt`; explicit `nodeOrNull()`/`requireNode()` only when the observed operation needs a node. `kindAt` retains Null and Undefined distinctions. |
| Child enumeration as raw-pointer range | Index traversal of the ChildArray, preserving its tagged entries. |
| Raw `parent` field reads/writes | `getParent()` and `setParent(NodeValue)`, with `sameNode` for node identity and `isNull` for explicit Null observations. |
| Raw-pointer `setTeXclass`, `core`, `coreMO`, `appendChild`, `replaceChild` signatures | Current tagged `NodeValue` method signatures; emitted callback signatures also carry `NodeValue`. |
| Vector mutation (`push_back`, positional erase) | ChildArray `push_back(NodeValue(...))` and index-based `erase`. |
| Direct pointer equality against returned node values | `sameNode` helpers, retaining exact tagged node identity. |

The test-local `children`, `valueAt`, `at`, `sameNode`, `isNull`, `isUndefined`,
and `kindAt` helpers are native observation adapters. They do not change the
JavaScript oracle or add a second child/parent representation. Explicit
`PropertyNull` fixtures remain tagged Null values.

## Verification boundary

The authorized generator run completed and printed
`{"source_members":111,"executable_members":85,"assertions":1084,"cases":43,"mode":"generate"}`.
Root owns the independent Source-profile reproduction, fixture diff review,
and core build/test gate. The generated C++ still contains the original
expected assertions; this report makes no claim that unported runtime methods,
the full MML graph, or Folia math rendering have passed.
