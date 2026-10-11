# P14 read-only audit: `mtr.ts` and `Mlabeledtr`

Pinned source: `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mtr.ts`,
MathJax 4.1.3, SHA-256
`ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f`.
This is an audit only; no implementation or native acceptance is claimed.

## Complete source member inventory

| Class | Member | Source lines | Kind and behavior |
| --- | --- | ---: | --- |
| `MmlMtr` | `static defaults` | 38–44 | Own static object: shallow spread of `AbstractMmlNode.defaults`, then `rowalign`, `columnalign`, `groupalign` set to `INHERIT`, and `data-break-align` set to `top`, in that insertion order. |
| `MmlMtr` | `kind` getter | 49–51 | Returns `mtr`. |
| `MmlMtr` | `linebreakContainer` getter | 58–60 | Returns `true`. |
| `MmlMtr` | `linebreakAlign` getter | 67–69 | Returns empty string, suppressing the base linebreak indentation reset. |
| `MmlMtr` | `setChildInheritedAttributes` | 76–106 | Normalizes children, derives alignment lists, adds row attributes, then mutates alignment tuples and dispatches inheritance to children. Details below. |
| `MmlMtr` | `verifyChildren` | 113–132 | Enforces mtable parent, repairs non-mtd children, conditionally emits child errors, then calls base child verification. |
| `MmlMtr` | `setTeXclass` | 137–143 | Calls `getPrevClass(prev)`, calls each child’s `setTeXclass(null)`, returns the row. |
| `MmlMlabeledtr` | `kind` getter | 155–157 | Overrides kind with `mlabeledtr`. |
| `MmlMlabeledtr` | `arity` getter | 164–166 | Overrides the inherited unbounded arity with `1`; base verification/inheritance requires at least one child but permits any larger child count. |

Both constructors are inherited; there are no additional constructor bodies,
own fields, or other methods in this file. `MmlMlabeledtr` has no own `defaults`
declaration: JavaScript static lookup inherits `MmlMtr.defaults` through the
constructor prototype chain, so it observes the same defaults object.

## `setChildInheritedAttributes` evaluation and state changes

The first `for...of` reads and captures the current `this.childNodes` array
once. Its array iterator checks the captured array’s live length and fetches
each current slot. For every child whose `isKind('mtd')` is false, the source
evaluates `factory.create('mtd')`, then `this.replaceChild(newMtd, child)`, then
`appendChild(child)` on the returned replacement. This changes both tree edges:
the row now contains the mtd, and the original child becomes an mtd child. A
null child fails at `child.isKind`; a null create/replace result fails before
append. The loop does not re-read `this.childNodes` if callbacks replace that
property, but mutations to the captured array’s slots and length remain
visible.

After normalization the method reads `attributes.columnalign`, calls imported
`split`, then reads and splits `data-break-align`, in that order. `split` is
from `ts/util/string.ts` and is `x.trim().split(/\\s+/)`; it assumes a string
and does not coerce arbitrary values. If the virtual `arity` read is exactly
`1`, it next reads `this.parent.attributes.get('side')`, then unshifts that raw
side value into `calign` and `top` into `balign`. A missing parent/attributes
or non-string split input throws before subsequent actions.

It then calls `addInheritedAttributes` once with the original attributes
handle and an object whose values are read in order: current `rowalign`, literal
`columnalign: 'center'`, literal `data-break-align: 'top'`. The returned
attributes object is local to the method. `addInheritedAttributes` makes a
shallow copy and stores source-origin tuples `[this.kind, value]` for those
three inherited keys. The subsequent child loop performs a fresh read of
`this.childNodes`, capturing that then-current array. For each child, in order,
it writes `attributes.columnalign[1]` to `calign.shift() ||` the existing tuple
slot, writes `attributes['data-vertical-align']` to a new `[this.kind,
balign.shift() || attributes['data-break-align'][1]]` tuple, and invokes
`child.setInheritedAttributes(attributes, display, level, prime)`. These
PropertyList/tuple objects are shallow aliases: the same attributes object and
its mutable column tuple are reused across the child calls; vertical-align is
replaced per child. Raw display/level/prime values pass unchanged.

## `verifyChildren` and TeX-class traversal

`verifyChildren` first checks `this.parent`; if truthy, `parent.isKind('mtable')`
is read. On failure it constructs the message using the current `kind`, calls
`this.mError(message, options, true)`, and returns without inspecting or
repairing children. Otherwise it captures `this.childNodes` for a live-length
`for...of`. Each non-mtd child is replaced by a fresh factory mtd and appended
to that mtd. After that graph mutation it tests raw `!options['fixMtables']`;
when falsy, it constructs the error message using current `kind` and calls
`mError` on the original child. Finally, `super.verifyChildren(options)` reads
the row’s then-current child array and dispatches `verifyTree(options)` to its
children. This creates separate first-loop and base-loop array captures.

`setTeXclass` calls inherited `getPrevClass(prev)` before reading the row child
array. Its `for...of` captures that array once, with live length/current slots,
calls every child with a null previous node, ignores each return value, and
returns `this`. A null child throws at the child method access.

## Dependencies and current native boundary

The mtr source directly depends on `AbstractMmlNode` methods/fields:
`attributes`, `arity`, `addInheritedAttributes`, `childNodes`, `factory`,
`getPrevClass`, `kind`, `linebreakContainer`, `linebreakAlign`, `mError`,
`parent`, `replaceChild`, `setInheritedAttributes`, `setTeXclass`, and
`verifyChildren`. It also imports `INHERIT` from `Attributes.ts` and `split`
from `util/string.ts`. The row repair branches depend on registered `mtd` and
`mtable` constructors. Mlabeledtr additionally depends on the class identity
relationship `Mlabeledtr extends Mtr` and its inherited defaults.

The accepted native core already has the corresponding virtual MML seams:
`MmlNode::arity/kind/attributes/setInheritedAttributes/setTeXclass/verifyTree`,
and `AbstractMmlNode`’s protected `getPrevClass`, `setChildInheritedAttributes`,
`addInheritedAttributes`, and `verifyChildren`. `AttributeList` is a
`PropertyList` handle; `PropertyArray` can represent the two-slot inherited
tuples, and `Children` is a shared mutable vector. `AbstractMmlNode::arity()`
is infinity, so the intended Mtr default is unbounded; Mlabeledtr needs the
explicit `1` override. The base `setInheritedAttributes` performs arity
correction before virtual child inheritance and suppresses indent inheritance
when the virtual `linebreakAlign()` is empty.

There are no native `MmlMtr`, `MmlMlabeledtr`, `MmlMtd`, or `MmlMtable`
declarations/implementations in `src/math/core/mml_tree` yet. The base
`noInherit` metadata already contains entries for origins `mtr` and
`mlabeledtr`, but there are no row/table registrations for checked
`isKind`/`factory.create` calls. Current `string_util::split` accepts only
`std::u16string_view`; a faithful method needs a host boundary preserving the
source erased-cast behavior (string inputs work, non-string `.trim()` access
fails) rather than silently coercing `Property` values. The native shared
children vector can model captured array identity and live slot/length
iteration, but each mtr loop must explicitly retain the right capture; the
two loops in `setChildInheritedAttributes` intentionally capture at different
times. Existing interfaces do not model arbitrary monkey-patched array
iterator/`shift`/`unshift` methods, if such host mutation is included in the
enabled profile.

Related pinned sources: `mtable.ts` SHA-256
`8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda` (creates
and validates rows, sets label side, consumes row alignment tuples), `mtd.ts`
SHA-256 `8b888aec4069818407079e84c42247495e04bd93ff52a3fd28bff65ca7635cfb`
(cell child/default/inheritance behavior), `util/string.ts` SHA-256
`024a2ac6f96a314884f2bd71a913060c06fb75d46cbf6c94e15f4e71643a7084`, and
`MmlNode.ts` SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`.
This audit does not freeze an implementation interface or propose an engine;
root owns any architecture and fixture decisions.
