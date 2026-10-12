# P40 NodeUtil implementation

Implemented the 21 newly released semantic bodies in
`src/math/input/tex/node_util.cpp`, corresponding to the pinned
`NodeUtil.ts` ranges below. The previously implemented child-array methods and
all Root-owned declarations/helpers remain unchanged.

| Native method | Pinned source responsibility |
| --- | --- |
| `createEntity` | `NodeUtil.ts:57-59` — hexadecimal `parseInt` followed by code-point conversion |
| `getChildren`, `getText` | `67-79` — live child-array handle and virtual text access |
| `appendChildren` | `87-93` — live iteration and virtual append dispatch |
| `setAttribute`, `setProperty`, `setProperties` | `100-146` — attribute/property routing, ordered property snapshot, and source callback order |
| `getProperty`, `getAttribute`, `removeAttribute`, `removeProperties` | `149-185` — virtual property access and attribute operations |
| `copyAttributes` | `229-240` — shared Attributes assignment before own-property entry snapshot and ordered virtual setters |
| `isType`, `isEmbellished`, `getTexClass`, `getCoreMO`, `isNode`, `isInferred` | `243-300` — node predicates and virtual class/core queries; `isNode` uses the Root canonical argument adapter and the two source MML base identities |
| `getForm`, `getOp`, `getMoAttribute` | `306-353` — live forms iteration, global `MmlMo::operatorTable` lookup, and optional operator-property lookup before fresh attribute fallback |

Each semantic entry has a `TreeCallGuard`. The bodies reuse Root's attribute,
node-text, forms-receiver and canonical-node helpers plus existing Property,
Attributes, MML and UTF-16 conversion APIs. During independent review Root added
the scoped propertyAt helper for exact nullish property-key errors and corrected
the texClass write failure and compiled-iterator null boundary. Acceptance and
actual build/test results are in p40-parser-utilities-acceptance.md.

The implementation follows the released typed boundaries: NodeUtil child
indices are `size_t`, and parser heterogeneous node values use the canonical
`NodeValue` or `MmlNode*` carrier. The contract's package profile uses integral
indices; it does not claim arbitrary JavaScript named-property reflection.
No build or tests were run, per instruction.
