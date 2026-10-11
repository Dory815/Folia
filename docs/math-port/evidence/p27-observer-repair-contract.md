# Root remaining six-observer compile repair packet

Only the Native C++ emitter sections and necessary emitted CPP type/API
expressions in six oracle files may change. Root owns regenerated fixtures,
original Source replay, invariant checks, independent whole-graph compilation,
repository inventory and acceptance. No production algorithms/declarations,
Source JS probes/evaluation, inputs, expected values, loaders/pins/scope,
case filtering, fallback, shared emitter refactor, build/test/gold generation,
commit or delegation by the implementers. This is compile repair only.

Root replayed every original --check and froze both scripts and fixtures in
ignored build/p27-observer-freeze before delegation. Original profiles:

| Family | Cases | Assertions | Additional frozen observations |
|---|---:|---:|---|
| mo-order | 50 | 538 | ordering/identity/partial-failure records |
| arithmetic | 7 | 13501 | 1440 raw inheritance rows and attr/value pools |
| arithmetic-order | 22 | 56 | separate callback order/postorder tree snapshots |
| table | 135 | 4700 | 240 inheritance rows, 160 explicit/default/cramped rules, 428 values |
| atom-semantics | 74 | 6888 | 924 raw shared-operation rows, 157 values |
| multiscript | 226 | 1304 | 480 inheritance rows, 1170 values |

All six have independent Native emitters; no shared owner conflict. Luna
math_layout owns mo-order/arithmetic/arithmetic-order emitter code and writes
p27-arithmetic-operator-observer-implementation.md. Luna tex_parser owns
table/atom-semantics/multiscript emitter code and writes
p27-table-script-observer-implementation.md. They do not generate CPP fixtures.
Source JS prefixes before const out=[ must be byte-identical after LF
normalization. Native-only changes later in mixed JS/C++ emit sections must
leave Source evaluation and every input/expected/raw table field intact.
Never globally replace text that occurs in both JS and CPP templates; isolate
the native section. Root rejected this regression in P26.

Existing NodeValue/ChildArray services are the established boundary. Adapt
actual virtual signatures, receiver arrays and result locals, preserving Null
versus Undefined and exact identity. Known factory products, standalone test
vectors and intentionally raw methods (e.g. Mo coreParent and Maction selected)
remain their declared types. Do not normalize all tagged values to pointers.
Use getParent/setParent for raw parent observations/writes; use logical Parent
only where the original does. Preserve chained assignment write order.

Original snapshot map projections capture the receiver and initial length,
Has/Get each index and retain holes in PropertyArray. This applies to table,
atom-semantics and multiscript snapshots. Arithmetic-order's snapshot has an
original for-of: record the node's fields/attributes first, capture its child
array at that point, Get every index with live length, dereference nullish
values with the proper error, then append the node in postorder. No skip-null
path, dense vector or one loop substituted for both behaviors.

Mo-order Parent probe now returns NodeValue; coreParent remains raw. Its
Snapshot.isEmbellished mutation uses raw getParent and array.set for slots 1/2,
matching this.parent.childNodes, not logical Parent. Arithmetic virtual
setTeXclass/getPrevClass take tagged values, attribute hooks retain AttributeList
(Property) and all 1440 original rows/pools; results preserve tag and identity.
Table's null-first assignment uses array.set with explicit Null. Multiscript's
appendChild returns NodeValue and direct slot Get/Set preserves the observed
order/parent state; mError/factory products and unrelated ns vectors stay raw.
Atom's adjustTeXclass dispatch and its native shared-guard witness must adapt
typed results while retaining original depth/identity assertions.

Root requires original --check with identical profiles/data, prefix and value
table guards, independent compilation of all six concrete fixtures against the
full current 90-module graph and a fresh normal build/inventory. These steps
can clear stale compile failures; they do not accept missing sparse consumer
algorithms, runtime semantics, full TeX/parser/layout/output or Folia.
