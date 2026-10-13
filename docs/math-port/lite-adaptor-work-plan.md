# Output DOM and LiteAdaptor source contract

Root owns architecture, original observations and acceptance. Implementation
has not started. Reference: MathJax 4.1.3, source commit
`fb987178c2d279a99b0db5ea02e751691703955e`.

## File responsibilities

| Upstream file | Native files under `src/math` | Responsibility |
| --- | --- | --- |
| `core/DOMAdaptor.ts` | `core/dom_adaptor.ixx/.cpp` | Output DOM interface and actual abstract helper bodies |
| `adaptors/lite/Element.ts` | `adaptors/lite/element.ixx/.cpp` | Element fields, shallow constructor copies, child assignment |
| `adaptors/lite/Text.ts` | `adaptors/lite/text.ixx/.cpp` | Text and comment kinds, raw UTF-16 text |
| `adaptors/lite/Document.ts` | `adaptors/lite/document.ixx/.cpp` | Document root/head/body/defaultView/type identities |
| `adaptors/lite/List.ts` | `adaptors/lite/list.ixx/.cpp` | NodeList/collection/fragment storage and Source iterator behavior |
| `adaptors/lite/Window.ts` | `adaptors/lite/window.ixx/.cpp` | Window document and constructor identities |
| `adaptors/lite/Parser.ts` | `adaptors/lite/parser.ixx/.cpp` | Source HTML parser, entity translation, HTML/XML serializers |
| `adaptors/liteAdaptor.ts` | `adaptors/lite_adaptor.ixx/.cpp` | LiteBase graph mutations, searches, attributes/classes/styles |
| `adaptors/NodeMixin.ts` | `adaptors/node_mixin.ixx/.cpp` | Reference options, font measurements, Unicode CJK counting, worker boundary |

Do not collapse these into a bespoke SVG tree or a single renderer file. The
graph is the Source output-host graph, separate from input MML and the completed
geometric scene. The existing MML XML adaptor does not implement this contract.
Parser/serialization logic belongs to the LiteParser port; LiteBase delegates
to it. Styles reuses the accepted native Styles implementation.

## Ownership and representation

Use one adaptor-owned node store with stable node identity. Public node handles
retain the store; internal parent/children/document links identify records in
that store and must not create C++ reference cycles. Retaining only a child
handle must keep its parent record queryable. A weak parent that disappears
when a caller releases its parent handle does not match Source.

Fields belong to their actual node records, with one authority for attributes,
children, parent, styles, document members and arbitrary properties. If Context
reflection is exposed, use the existing bindings/native-field mechanisms over
those records; do not maintain a second Context tree. No SVG string is a node
handle. Node properties preserve raw values and object identity; they are not
attributes. Shallow constructor/clone copies preserve nested value identity.

Source never-attached `parent` is Undefined; removal sets Null. Preserve that
distinction at the Source observation boundary. UTF-16 indices are used by
`split`, including indices inside surrogate pairs and negative slice indices.
Code-point counting is restricted to NodeMixin measurement, where Source uses
`Array.from`. Do not silently repair ill-formed UTF-16.

Native ownership must allow the observed Source graph states. In particular,
`replace` does not detach the replacement from its previous parent's children
array, and parser head-fragment handling can expose that state. No normalizer
may make the graph a stricter tree than Source.

## Required behavior

Port every member in the original prototype inventory and every Source body
needed by the Folia SVG profile. This profile calls
`liteAdaptor({fontSize:16})`; defaults include `badCSS/badSizes:true`,
`fontFamily:'Times'`, `cjkCharWidth:1`, `unknownCharWidth:.6`,
`unknownCharHeight:.8`, and `canMeasureNodes:false`. Custom instance options and
derived class OPTIONS stay live at the same Source access points.

The following are observed behavior, not implementation suggestions:

- Append/insert remove a node from its previous parent. Insert does that even
  when the old sibling is detached. Remove returns the actual node and sets
  its parent to Null.
- LiteBase overrides the abstract replace helper. It returns the old node;
  replacement from another parent can remain in both child arrays.
- Deep clone sets cloned element parents, but cloned text/comment parents
  remain Undefined. Arbitrary properties and the Styles cache are not copied.
- `childNodes` returns a new shallow array. Empty first/last/out-of-range child
  accesses are Undefined; detached sibling queries return Null.
- `setAttribute('style',...)` clears the Styles cache. Removing that attribute
  leaves an existing cache intact. Cached `getStyle` can therefore return the
  old value while `allStyles` returns Undefined.
- `insertRules` replaces the child array with new unparented text; old children
  keep their previous parent. Preserve Source concatenation and spacing.
- HTML/XML serialization uses Source PCDATA/self-closing rules. XML default
  namespace insertion affects the serialized attribute list, not node state.
  Attribute order and escaping are observable.
- Search covers Source's limited selectors and traversal order, includes the
  supplied root when matching, respects stop counts, and has distinct missing
  results (id/unsupported selector Null, missing class/tag Undefined).
- `LiteList` copies its input array. `getElements` consumes the actual `.nodes`
  field. The pinned iterator's normal `next()` receiver throws a TypeError;
  retain that original observation rather than pretending iteration succeeded.

NodeMixin's CJK RegExp uses `gu` and includes supplementary-plane ranges. The
current native RegExp profile rejects `u`; implementing measurement is therefore
a real dependency. Port the complete range/counting operation with explicit
Source equivalence, or extend the canonical Unicode RegExp host with its own
acceptance. Do not delete the flag, measure UTF-16 units, use font glyph widths,
or switch to DirectWrite during SVG parity work. The original observer includes
range endpoints, supplementary characters, combining marks and lone surrogates.

LiteParser also uses Unicode RegExp patterns and Entities translation. Named
entity loading and retry behavior are part of its dependency closure, including
texhtml usage. Do not replace it with a standard HTML parser whose repairs
change Source structure. Follow actual Source malformed-input behavior.

`createWorker` is an asynchronous host service backed by `node:worker_threads`
in Source. No worker lifecycle is accepted by this packet's current observer.
Classify its reachability from the selected packages and provide a separately
reviewed native service if reachable. Returning Null or ignoring requests is
not a port. Browser measurement/adaptors are outside this LiteAdaptor profile.

## Original observations and acceptance

`tools/math-port/lite-adaptor-original-oracle.cjs` checks nine TS source hashes
and package version, records the loaded CJS hashes, and generates 35 original
rows. Root executed it twice into ignored `build/math-port` outputs and compared
the parsed whole documents with `assert.deepStrictEqual`: exact repeat. This
is original baseline generation, not native implementation or acceptance.
Upstream TypeErrors in the iterator row are captured as actual errors.

Acceptance requires:

1. A native observer runs the same construction/mutation/serialization and
   measurement operations on actual production objects. Strict whole-record
   equality covers all 35 rows; no retained original strings serve as native
   results, and no identities are replaced by hard-coded booleans.
2. Focused tests exercise retained child/parent handles, clone aliases, UTF-16
   split, namespace serialization without mutation, stale style caches,
   traversal/stop order and live options. Observe Source errors precisely.
3. Root independently exercises deliberate graph-detach, cache-reset, code-unit
   counting and escaping faults; the comparator/tests must catch them.
4. Every inventory member has a source-to-native ledger entry and every
   reachable parser/entity/worker dependency has a real implementation or a
   named incomplete gate. Inventory alone is not acceptance.
5. Root registers the modules, compiles a stable shared snapshot, runs focused
   parity/tests, and checks the ordinary core build. This packet introduces no
   Windows or Direct2D dependencies into math/core.

After host acceptance, CommonOutputJax and Source wrappers use these actual
handles. SVG scene construction still needs its own per-kind port and original
comparison. Direct2D will consume that common scene; geometry must never be
recovered by reparsing the host's serialized SVG.
