# P14 MmlMtable dependency audit

This is a read-only prerequisite audit of the pinned MathJax 4.1.3 source. It
does not propose or freeze a replacement interface, implement MmlMtable, or
claim acceptance. Only this evidence file was written; no builds or tests were
run.

## Pinned sources

| Source | SHA-256 | Role in MmlMtable |
| --- | --- | --- |
| `ts/core/MmlTree/MmlNodes/mtable.ts` | `8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda` | Audited class and all seven declared behavior methods/accessors plus instance fields. |
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` | Base node lifecycle, inherited-attribute and verification hooks, child edits, MML factory access, class identity, and error insertion. |
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` | Generic parent/child array and factory behavior used by MML edits. |
| `ts/core/MmlTree/Attributes.ts` | `c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a` | Explicit/inherited/default attribute reads, inherited writes, and explicit removal. |
| `ts/util/string.ts` | `024a2ac6f96a314884f2bd71a913060c06fb75d46cbf6c94e15f4e71643a7084` | `split()` used to parse row alignment. |
| `ts/core/MmlTree/MmlNodes/mtr.ts` | `ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f` | Runtime `mtr` and `mlabeledtr` classes required by table row creation and `isKind('mtr')`. |
| `ts/core/MmlTree/MmlNodes/mtd.ts` | `8b888aec4069818407079e84c42247495e04bd93ff52a3fd28bff65ca7635cfb` | Runtime cell class created while repairing invalid row children. |

## Class/member ledger

There is one class, `MmlMtable extends AbstractMmlNode`, at `mtable.ts:39`.
It declares no constructor. Its executable members and instance fields are:

| Member | Source lines | Behavior and dependencies |
| --- | --- | --- |
| static `defaults` | 43–64 | Creates a new shallow object spread from `AbstractMmlNode.defaults`, then writes the listed scalar table defaults in order. The nested `data-break-align` comment documents a top/bottom/center list but the stored default is the string `top`. |
| instance `properties` | 69–71 | Creates a per-instance object with own `useHeight: true`. This is the generic Node `properties` bag, distinct from MML attributes. |
| protected `texclass` field | 76 | Initializes source TeX class to `TEXCLASS.ORD` during derived class field initialization. It is not an attribute or a generic property. |
| getter `kind` | 81–83 | Returns `mtable`. |
| getter `linebreakContainer` | 90–92 | Returns true. |
| getter `linebreakAlign` | 99–101 | Returns empty string, disabling the base automatic indent reset/addition. |
| `setInheritedAttributes` | 106–125 | Iterates the imported mutable `indentAttributes` array. For each name, tests `attributes[name]` by JS truthiness; on truthy value it captures the `this.attributes.setInherited` receiver/method before reading `attributes[name][1]` as an argument, then invokes it. It next checks explicit presence and, if present, removes the attribute. It calls the base method only after all names. Each property/method access is live and ordered. |
| `setChildInheritedAttributes` | 134–160 | First repairs non-`mtr` children by iterating the current child array with for-of semantics and replacing each with a newly created `mtr`, then appending the old child into it. Recomputes display from explicit/default displaystyle, adds inherited column/row/break alignment values, reads explicit cramped, splits the rowalign string, then loops the current child array and mutates the shared `attributes.rowalign[1]` tuple with each shifted row alignment before dispatching child inheritance. Each child receives the same mutable attributes object, computed display, raw level, and boolean cramped as prime; the `_prime` input is intentionally unused. |
| `verifyChildren` | 167–204 | Captures `factory` once. A numeric-index loop re-reads `this.childNodes.length` and the indexed child each iteration. Existing `mtr` starts a new grouping run. A non-row child is either removed into the current row (then index decremented) or replaced by a new row; an `mtd` child is moved directly, while any other node is wrapped in a factory-created `mtd`. When `options['fixMtables']` is falsey, the just-moved child is removed from its wrapper parent, its `parent` field is set directly to the table, an empty `mtd` is appended for original `mtd` children, and `child.mError(...)` is appended to the last cell. It finally calls the base `verifyChildren(options)`. |
| `setTeXclass` | 209–215 | Calls `getPrevClass(prev)`, captures the child array for for-of iteration with live length/index semantics, calls each child with null previous, then returns the table itself. It does not return a child result. |

## Evaluation order, mutable aliases, and tree changes

`setInheritedAttributes` evaluates each loop body in this order: read the
incoming tuple by name and test its truthiness; if truthy, evaluate the
`this.attributes` receiver and `setInherited` method first, then evaluate the
arguments (`name`, a second `attributes[name]` read, and that value's index 1)
before invoking the captured method. Next evaluate the current
`this.attributes` receiver and `hasExplicit` method before its `name`
argument; if true, evaluate the current receiver and `unset` method before
the `name` argument. Only after the array loop does the base method run. The
incoming `attributes` record may have inherited properties, and a truthy but
non-indexable value yields an undefined slot in JavaScript. Nullish tuple
indexing throws. `indentAttributes` is a source array whose iterator captures
the array object and checks its live length; the current native declaration
is a `std::vector<std::string>` global, not a JS array handle.

The row-repair for-of loop captures the child array object, but sees its live
length and current slots as it advances. `replaceChild` mutates that captured
array in place, and `appendChild` changes the old child's parent. For each
non-row child, the `replaceChild` receiver/method is evaluated before its
arguments; `this.factory.create('mtr')` runs before the old-child argument.
The returned row is then the receiver for `appendChild(child)`. The second
for-of loop begins after the first and therefore captures whatever
`this.childNodes` refers to then. It computes display from `getExplicit` and
short-circuits `getDefault` when the explicit value is truthy. The call to
`addInheritedAttributes` captures its receiver/method before its arguments;
the additions object then reads the current attributes receiver for
`columnalign`, uses literal `rowalign: 'center'`, and reads the receiver again
for `data-break-align`. It next reads explicit `data-cramped` and obtains the
`rowalign` string through another current attributes receiver before calling
`split`. It passes one `attributes` object to every row. Its `rowalign` value
is a mutable two-slot attribute tuple; the indexed
write and later row callbacks share that tuple. For the assignment to
`attributes.rowalign[1]`, JavaScript evaluates and captures the left-hand
`attributes.rowalign` base/index before it evaluates the right side; it then
calls `ralign.shift()`, and only on a falsey result reads
`attributes.rowalign[1]` again for the fallback. A callback that changes the
tuple can therefore influence subsequent rows.

`verifyChildren` is deliberately different: the numeric loop checks the
current table child-array property each iteration, so array replacement can
change subsequent iterations. Removing a child decrements the index so the
new occupant at that position is processed. `mtr` remains the current group
until an actual `mtr` row is encountered. The invalid-child path performs
multiple writes to `parent` and nested child arrays; the unusual `fixMtables`
false branch, including the direct `child.parent = this` write and the final
cell selection at lines 188–199, must be carried as written rather than
restructured into a cleaner tree repair. The base verification call observes
the resulting tree and can recurse into nodes created by these edits.

For call evaluation order in `verifyChildren`, `factory` is captured once
before iteration. The `replaceChild` receiver/method is evaluated before its
arguments; `factory.create('mtr')` is evaluated before the old `child`
argument. The returned row becomes the receiver for the following
`appendChild`, whose child argument is the original node or the freshly
created `mtd` (whose factory call is evaluated only in the non-`mtd` branch).
The `fixMtables` property is read only after that move. In its false branch,
`child.parent` is read as the `removeChild` receiver before the child argument;
the direct parent assignment follows. The later `mError` receiver is the child
and is evaluated before its message expression reads `this.kind`; the final
cell `appendChild` evaluates the current `mtr.childNodes` array for the base,
then re-reads `mtr.childNodes` to compute its length/index, then invokes the
selected cell with `merror`. These ordered reads matter if factory creation,
node methods, or getters have observable effects.

`setTeXclass` and both for-of loops capture an array handle and observe its
live length/slots. A callback that rebinds `this.childNodes` does not change
the already captured array in those loops; a callback that mutates the
captured array does. A normal C++ range-for over a copied vector would miss
these distinctions.

## Direct dependencies and required host operations

* `AbstractMmlNode` supplies `getPrevClass` (`MmlNode.ts:716–723`), base
  `setInheritedAttributes` (`765–834`), `setChildInheritedAttributes`
  (`846–854`), `addInheritedAttributes` (`866–884`), base `verifyChildren`
  (`973–977`), `mError` (`987–1011`), `factory`, attributes, and the
  parent/child links. The base inherited hook also performs generic arity
  repair and linebreak inheritance before dispatching this override.
* The generic tree supplies `replaceChild`, `appendChild`, `removeChild`, and
  child storage (`Node.ts:263–303`). MML overrides of append/replace
  (`MmlNode.ts:607–644`) route through inferred rows;
  mtable's own infinity arity normally avoids the special negative-arity path.
  Tree operations update parent pointers, while the source error branch also
  writes a parent pointer directly.
* Attributes need the four separate operations `get`, `getExplicit`,
  `getDefault`, `setInherited`, `hasExplicit`, and `unset`, preserving the
  source's repeated receiver lookups. Existing native `Attributes` has these
  methods, with four live layers (`Attributes.ts:89–167`). The imported
  `indentAttributes` array is declared in `MmlNode.ts:121–126`.
* The factory must create `mtr`, `mtd`, and nodes required by `mError`
  (`merror`, `mtext`, `text`) with the source's overload argument shapes.
  Class identity must recognize `mlabeledtr` as an instance of `mtr`; the
  source tests `child.isKind('mtr')`, not a separate string check for
  `mlabeledtr`.
* `split()` is `string.ts:87–88` (`x.trim().split(/\s+/)`) and must retain the
  pinned trim/ECMAScript-whitespace behavior. `mtable.ts` applies a TypeScript
  cast only: it does not coerce a non-string rowalign value before calling
  `split`.
* `verifyChildren` needs source property lookup and truthiness for
  `options['fixMtables']`, including inherited names and falsey values, plus
  the exact `mError(message, options, short)` behavior.
* All recursive virtual calls and tree edits need the shared tree-call
  resource guard. That guard must not alter order or skip iterations.

## Current native boundary and unresolved prerequisites

The accepted native tree already has `PropertyList`/`PropertyArray` storage,
generic `parent` and `childNodes`, generic factory creation, tree edits,
MML attributes, `getPrevClass`, `addInheritedAttributes`, `verifyChildren`,
`mError`, `TreeCallGuard`, and UTF-16 `string_util::split`. `MmlNode::isKind`
uses factory class identity; the native class binding uses `dynamic_cast`, so
a registered MmlMlabeledtr derived from MmlMtr can satisfy `isKind("mtr")`.
The generic MML verification and error machinery is present.

There is currently no native `MmlMtable`, `MmlMtr`, `MmlMlabeledtr`, or
`MmlMtd` implementation or registration in `src/math/core/mml_tree/mml_nodes`.
Consequently the table's factory children and real parent/child behavior cannot
be exercised yet. The accepted `indentAttributes` binding is a mutable
`std::vector<std::string>`, not the source's shared JS-array identity and live
iterator. `PropertyList` models enumerable own/inherited values but not
accessors or arbitrary executable getters, so callbacks caused by custom
property descriptors cannot be reproduced. Current `mError` takes
`ErrorOptions` rather than the upstream `PropertyList`; it can represent null
for `mError`, but `verifyChildren` currently accepts a non-null `PropertyList`
and cannot represent source null-options failure at `options['fixMtables']`.
Message construction and evaluation order still require explicit host mapping.
`FactoryServices` accepts `std::span<const std::any>`, so source
`factory.create('mtd', {},
[child])` requires a precise native argument adapter. These are dependency and
contract questions for root, not approved interface changes in this audit.

The inherited-attribute hook's JavaScript numeric-index tuple reads and the
rowalign tuple's in-place slot write must use the shared Property graph, not
copied scalar fields. The native `split` accepts a UTF-16 view; because source
casts are erased, a future implementation must distinguish a true string from
other `Property` variants before calling it rather than first coercing through
`String(value)`.
