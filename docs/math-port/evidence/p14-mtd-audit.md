# P14 `MmlMtd` dependency audit

Read-only audit of MathJax 4.1.3 `ts/core/MmlTree/MmlNodes/mtd.ts` against the
current native MML boundary. The pinned source SHA256 is
`8b888aec4069818407079e84c42247495e04bd93ff52a3fd28bff65ca7635cfb` (also
recorded in `docs/math-port/scope.json`). No implementation or build was
performed. This audit does not freeze an interface or accept a future port.

## Source class and members

`MmlMtd` extends `AbstractMmlBaseNode` (`mtd.ts:33`). Its source members are:

| Member | Source lines | Semantics and dependency |
| --- | --- | --- |
| static `defaults` | 37–45 | Shallow own-property spread of `AbstractMmlBaseNode.defaults`, followed in insertion order by `rowspan: 1`, `columnspan: 1`, `rowalign`, `columnalign`, `groupalign` set to the shared `INHERIT` sentinel, and `data-vertical-align: 'top'`. Imports `PropertyList` and `INHERIT` from `Attributes.ts`; the defaults must be a distinct object from the base defaults while retaining the source values. |
| `kind` getter | 50–52 | Returns `'mtd'`; used by defaults/no-inherit policy, error messages, factories and class-kind checks. |
| `arity` getter | 59–61 | Returns `-1`, selecting an inferred `mrow` wrapper in `AbstractMmlNode.initializeMml` and routing later `setChildren`/append operations through it. Its child 0 is that inferred row, not the cell's first authored expression node. |
| `linebreakContainer` getter | 68–70 | Returns true; inherited base attribute propagation treats the cell as a linebreak container. |
| `linebreakAlign` getter | 75–77 | Returns `'columnalign'`; generic inherited-attribute logic reads this attribute and adds indentation fields when truthy/falsey handling requires it. |
| `verifyChildren(options)` | 84–94 | Reads the actual `parent` field. If non-null, checks `parent.isKind('mtr')`; on failure calls `mError(kind + ' can only be a child of an mtr or mlabeledtr', options, true)` and returns without traversing children. If the parent is null or passes, calls `super.verifyChildren(options)`. `options` and the short-error flag are passed through unchanged. |
| `setTeXclass(prev)` | 99–103 | Calls `getPrevClass(prev)` first, then reads current `childNodes[0]`, calls that node's `setTeXclass(null)`, ignores its return, and returns this cell. A previous-node callback can therefore affect which child is read. |

There are no additional `MmlMtd` instance fields. The class depends on
`AbstractMmlBaseNode` for tree storage, `getPrevClass`, `mError`, and inherited
verification; on `MmlNode`/Tree `Node` for `parent`, `childNodes`, and
`isKind`; and on `Attributes` for default/inherited/global resolution.
`mtd.ts` directly imports `PropertyList`, `AbstractMmlBaseNode`, `MmlNode`, and
`INHERIT`. The relevant pinned dependency hashes are:

| Source | SHA256 |
| --- | --- |
| `core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `core/MmlTree/Attributes.ts` | `c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a` |
| `core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `core/Tree/Factory.ts` | `e61d2f5868287e2f382503cf690cd241698bf212957c1795d0cd35ce3bef049e` |
| `core/MmlTree/MmlNodes/mtr.ts` | `ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f` |
| `core/MmlTree/MmlNodes/mtable.ts` | `8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda` |

## Order-sensitive verification and tree behavior

The parent test is a direct-parent test, not `Parent` (the getter that skips
nodes marked `notParent`). In upstream `Node.isKind` delegates to the factory's
`nodeIsKind`, which is `node instanceof getNodeClass(kind)`. The source
condition says `'mtr'`, but accepts `mlabeledtr` through constructor
inheritance: `MmlMlabeledtr extends MmlMtr` in `mtr.ts:151`. A string comparison
against the reported kind (`'mlabeledtr'`) would incorrectly reject that valid
parent. The invalid-parent path must not call `super.verifyChildren`; the
valid and detached-parent paths must.

`mtr.ts` and `mtable.ts` establish why this test and the defaults are coupled:
`Mtable.verifyChildren` groups direct cells into rows, and `Mtr.verifyChildren`
wraps non-cell children into `mtd` cells. `Mtr` calls each child cell's
`setInheritedAttributes` after applying row/column alignment values; its row
class includes the `mlabeledtr` subclass. `Mtd` itself relies on inherited
`AbstractMmlNode.setInheritedAttributes`. `AbstractMmlNode.stopInherit()` lists
`mtd.columnalign`, `mtd.rowalign`, and `mtd.groupalign`, so these values land on
the cell and are removed from the attribute object passed farther into cell
contents. `INHERIT` defaults resolve through the `Attributes` global layer.
The `columnalign` line-break alignment feeds generic linebreak inheritance.

For `setTeXclass`, upstream calls `getPrevClass(prev)` before indexing child 0.
`getPrevClass` reads the previous node's TeX class and scriptlevel, either of
which is virtual in the native boundary; only after these reads should the
current child array be consulted. A missing child array/slot/null child fails
at the source access, after any previous-state writes. The inferred-row
construction means that a normal `mtd` child 0 is its inferred row; the row's
own TeX traversal handles expressions inside the cell. The cell returns
itself, not the row result.

On invalid-parent verification, source argument evaluation builds the message
from the current `kind`, passes the original `options` object and `true`, then
returns regardless of `mError`'s return value. A null parent skips this check
and proceeds to inherited child verification. With a valid row parent, the
superclass iterates `childNodes` and calls each child's `verifyTree(options)`;
malformed null child arrays/elements reach the corresponding source failure.
The native `AbstractMmlNode::mError` and `verifyChildren` provide typed host
operations, but there are no native table/row classes yet to exercise
replacement and traversal in their real parent context.

## Current native boundary and gaps

The repository currently has `AbstractMmlBaseNode`, generic
`AbstractMmlNode::initializeMml` inferred-row support, `Attributes::INHERIT`,
`stopInherit` policy, class-binding/`isInstance` infrastructure, checked
`verifyTree`/`mError`, and shared `TreeCallGuard`. Those are useful prerequisites
for this source file. However, `src/math/core/mml_tree/mml_nodes` contains no
native `MmlMtd`, `MmlMtr`, `MmlMlabeledtr`, or `MmlMtable` declarations,
registrations, defaults, or traversal implementations. The native tree cannot
yet construct the actual Mtd inferred-row shape, satisfy the row-instance
check, or exercise row/table repair and error replacement. `scope.json` marks
`mtd.ts` not started.

The protected native `verifyChildren(PropertyList)` hook exists on
`AbstractMmlNode`; native `MmlNode::mError` takes an `ErrorOptions` adapter
rather than the source `PropertyList` directly. A future binding must preserve
the same options handle through that adapter, the `shortError=true` behavior,
and the source return/early-exit order. Existing generic verification has
checked null-child handling; the new class needs its own ordered direct-parent
test before invoking it. The native class defaults must be independently
initialized with the six source additions above, and `kind/arity/
linebreakContainer/linebreakAlign` must reflect the source before inherited
initialization or verification uses them.

No implementation responsibility or scope disposition is proposed here. Root
must freeze the Mtd and table-family contracts and CJS witnesses before coding;
this audit does not claim complete MathML table support or acceptance.
