# P23c generic Node implementation notes

This packet implements only `AbstractNode`'s six frozen generic methods in
`src/math/core/tree/node.ixx`. It is not a full Node/MML migration or an
acceptance claim. The controlling contract is
`docs/math-port/p23c-generic-node-contract.md`; upstream is MathJax 4.1.3
commit `fb987178c2d279a99b0db5ea02e751691703955e`.

## Method ledger

| Native method | Pinned Source method | Implemented mapping |
|---|---|---|
| `node.ixx:70–82 setChildren` | `ts/core/Tree/Node.ts:253–258` | Rebind receiver to a fresh ChildArray before validating/iterating input. Capture the supplied shared handle; iterate its live length using indexed `get`, so holes yield Undefined and inherited numeric slots are read. Dispatch each value through virtual `appendChild`, which resolves the receiver's current child array each time. A null handle throws after the receiver has been rebound. |
| `node.ixx:84–92 appendChild` | `Node.ts:260–267` | Push the exact tagged ChildValue first. Then `requireNodeForWrite("parent")`, virtual `setParent(this)`, and return the original tag. Undefined/Null therefore remain in the array if the parent write throws. |
| `node.ixx:94–117 replaceChild` | `Node.ts:269–283` | Invoke virtual `childIndex(oldChild)` first. If found, obtain the receiver's current array and write the tagged replacement at that index; only then require the new receiver for `parent` write, call `setParent(this)`, read old child's parent, and clear it iff equal to this. Return the exact replacement on either found/not-found path. Self-replacement follows the same writes and can end with parent Null. |
| `node.ixx:119–132 removeChild` | `Node.ts:286–295` | Invoke virtual `childIndex`; on a match splice the receiver's then-current array first, then require the original argument for `parent` write and call `setParent(Null)`. Return original ChildValue on both found and not-found paths. A found Undefined/Null is removed before the parent assignment fails. |
| `node.ixx:134–138 childIndex` | `Node.ts:298–302` | Delegate to ChildArray `indexOf`, preserving strict node identity, distinct Undefined/Null tags, hole skipping, inherited numeric properties, and array order. Return `nullopt` for no match. |
| `node.ixx:140–168 copy` | `Node.ts:305–317` | Enter the existing recursive TreeCallGuard; ignore generic `keepIds`; create through the current factory/kind; replace target properties with a fresh shallow Object.assign-style record; capture the source child array only after creation/property copy; traverse captured handle with live length/Get; skip falsey Undefined/Null/hole reads; virtual-copy truthy Node slots and append the resulting product through the destination's virtual append. Return the factory-owned product. |

Source method bodies were compared directly with the pinned `Node.ts` lines
above. The source SHA-256 is
`532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0`.

## Preserved ordering and ownership

`setChildren` captures the input handle independently of replacing
`this->childNodes`; it does not snapshot contents. Its `size()` check happens
again on each loop iteration, while `get(index)` distinguishes own values,
holes, and inherited values. The call remains virtual, so a child callback may
rebind the destination array without retargeting the input iteration handle.

`appendChild` mutates the receiver array before checking whether the tagged
input can receive a `parent` write. `replaceChild` and `removeChild` defer
looking up the receiver array until after the virtual `childIndex` call. A
successful virtual callback may have changed both that array and the target
index's contents. Replacement writes the slot before either parent access;
removal splices before attempting the parent write. Parent reads/writes use the
frozen virtual `getParent`/`setParent` interface; there is no mirror parent
field.

The `copy` property path builds a fresh `PropertyList`, assigns the source's
own enumerable values in its established Object.assign key order, and then
replaces the factory node's current property record. Nested Property values
retain their shared identity. It does not alias the source root record or
merge with constructor properties. Child products remain owned by the
FactoryServices session; ChildArray slots contain only borrowed node identity.

Native boundary: after factory creation and the fresh shallow property copy,
`copy` uses `dynamic_cast<AbstractNode<Product, Base>*>` to verify that the
returned product contains this native family's property storage. A foreign
Product-derived family that does not satisfy that typed contract raises a
native `FactoryError` TypeError before the property assignment. Source
JavaScript can assign `properties` on arbitrary object-compatible factory
results; this C++ representation cannot model every such foreign property
layout. The checked failure is therefore an explicit native family boundary,
not evidence of Source-equivalent behavior. No setter mirror was added.

## Boundaries and remaining checks

The native child-list API is a `shared_ptr<ChildArray>` handle, so a null
handle is the representable missing-list case at these six C++ entry points.
Source JavaScript can distinguish a missing (`undefined`) argument from Null;
this packet does not widen the frozen handle API. The null-handle branches are
explicit TypeErrors rather than dereferences of a null C++ shared pointer.
The six implementations do not add generic property descriptors, array
species, proxy behavior, product ownership, or a dense compatibility path.

Only `node.ixx` was changed for implementation. No build, test, mutation,
fixture, generated output, or commit was run here, and no other child
consumers were migrated. Root owns source-level red/green acceptance and the
remaining child/MML/NodeUtil graph.
