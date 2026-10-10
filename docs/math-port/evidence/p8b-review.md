# P8b independent review: Empty/Text/XML and current MML bindings

This review checks the frozen P8b contract, the current native implementation
snapshot, the pinned MathJax 4.1.3 runtime, and the root-generated oracle. It is
an evidence report only; root retains implementation review and acceptance.

## Source and oracle identity

`tools/math-port/mml-node-oracle.cjs --check` passed against the current frozen
fixture and reported 111 inventoried members, 85 executable members, 1,061
assertions, and 40 cases. The script verifies the pinned TypeScript and CommonJS
hashes before checking the generated fixture. The resolved CommonJS file is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNode.js`.

| Source | SHA-256 |
|---|---|
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `cjs/core/MmlTree/MmlNode.js` | `86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45` |
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |

## Empty/Text/XML findings

The pinned `AbstractMmlEmptyNode` overrides the MML-specific getters and
methods at `MmlNode.ts:1233-1428`; it inherits the generic empty-node child
operations from `Tree/Node.ts:362-397`. The native subclass reuses
`tree::AbstractEmptyNode<MmlNode,MmlNode>` and does not create a second parent,
child array, or property store. This matches `Tree/Node.js:42-65`, whose
constructor initializes the generic fields before an empty node is used.

The fixed values, null attributes, no-op inheritance/verification, and
read-only setters in `mml_empty.cpp` match the pinned source. A fresh strict
CommonJS probe assigning to `attributes`, `texClass`, and `prevLevel` raised
`TypeError` for each getter-only property. The native direct setter adapters
also throw the host's `TypeError`; the frozen test checks the error name.

`TextNode` stores UTF-16 code units and its copy creates the current `kind`,
then invokes the source virtual `getText`, then the destination virtual
`setText`. The oracle's registered `VText` witness observes one source getter
and one destination setter, and checks a lone high surrogate plus a valid
surrogate pair. A fresh CJS probe observed the same order (`getText`, then
`setText:hello`). `host::debugText` preserves isolated surrogates using WTF-8,
which is the right native representation for `TextNode::toString`.

The typed native setter takes `std::u16string`; JavaScript's erased runtime
does not enforce the TS `string` annotation. A fresh CJS probe showed
`setText(42)` stores and returns the number `42`, and `toString()` also returns
that number. This behavior is outside the current typed MML host contract, but
the limit should remain explicit: the current native API does not preserve
arbitrary non-string runtime values in TextNode's `text` field.

Fresh CJS probes exercised a null `childNodes` handle on a finite-arity node.
`setInheritedAttributes()` and `verifyTree({checkArity:true})` both throw at
the arity check with `TypeError: Cannot read properties of null (reading
'length')` (`MmlNode.ts:798-814` and `:907-921`). Root has since added a frozen
native witness (`finite_arity_null_checks_before_virtual_children`); current
native code now checks the handle before dispatching to virtual child
operations, and the fixture requires no such child callback to occur.

`XMLNode` source order and adaptor omission match the current implementation.
The CJS probe observed one `clone` call during copy and no serialization call;
the copied node then raised `TypeError` from `getSerializedXML`, while the
source remained serializable. The frozen case checks identity, distinct clone,
source serialization, copied-node failure, `XML data`, and blank-source copy
failure. The `XmlRef`/borrowed `XmlAdaptor*` mapping is the explicit root-frozen
host adaptation; no arbitrary DOM object behavior is modeled here.

The frozen case `text_toString_raw_field_and_xml_factory_clone_order` now
checks TextNode's lone-surrogate `toString` bridge and XML `create`-before-
`clone` trace. A fresh instrumented CJS probe independently confirmed the
runtime trace `create`, then `clone`; it also confirmed a wrong TextNode factory
result still evaluates the source `getText` once before the non-callable
`setText` TypeError. The source expressions are at `MmlNode.ts:1465-1470` and
`:1533-1540`.

One remaining partial-mutation discrepancy is in short-form `mError`. Native
code checks `error->attributes()` before writing `data-mjx-message`, replaces
the old node through virtual `parent->replaceChild`, then dereferences
`error->attributes()` again to set `title` without checking it
(`mml_node.cpp:520-538`). A fresh pinned-CJS probe used a parent subclass whose
`replaceChild` sets the new error node's `attributes` to null. Source throws a
TypeError while setting `title`, after replacement has already happened. The
native path would dereference a null shared pointer after the same replacement.
Add a null check after `replaceChild` (preserving the already-mutated parent)
and a frozen virtual-parent witness before treating this branch as faithful.

## Executable-member review

I compared the executable members in the pinned source against the current
three implementation units and their ledgers. `mml_node.cpp` covers the
normal-node constructor phase, copy, nullable classes/attributes, predicates,
logical parent, inferred-row child methods, TeX-class transfer and spacing,
inheritance, verification, and `mError`. `mml_branches.cpp` covers token
identity/text/walk methods, layout child-zero delegation, and base-node
class/script dispatch. `mml_empty.cpp` covers every empty-node override plus
TextNode and XMLNode methods. Static defaults and tuple helpers are root-owned
in `mml_bindings.cpp`. The pinned source file's 85 executable members are
accounted for at the declaration/definition level; semantic parity still
depends on the native differential and mutation gates.

The review checked source-array behavior: `copy` and ordinary `for...of` paths
retain a captured shared-array handle and observe its live length; inferred-row
flattening snapshots `forEach` length; base scripts snapshot after base
processing. Inheritance trims by replacing the child array without detaching
removed children, preserves display/level/prime at child dispatch, and routes
through virtual child methods. TeX-class updates copy previous fields before
clearing core fields in assignment order. Error construction preserves
message creation, parent replacement, append, and short-title ordering except
for the post-replacement attributes dereference documented above. No other
concrete ordering defect was found within the frozen typed host interface
during this read-only pass.

## Host boundary observations

`mml_node_host.ixx` preserves null separately from Missing and uses a
lossless WTF-8 bridge. Empty/Text/XML code relies on that bridge only for
TextNode debug output and uses typed opaque XML handles for adaptor calls. The
native Error adapter maps source `TypeError` cases to `FactoryError` named
`TypeError`. No discrepancy was found in the covered, typed inputs.

`XMLNode::setXML` and `TextNode::setText` are C++-typed interfaces. Their source
methods accept erased runtime values without validation because TypeScript
types disappear in CJS. That means the C++ interfaces are bounded host
adaptations, not a complete emulation of arbitrary calls from JavaScript.
The supported raw-TeX integration should only pass UTF-16 text and registered
XML handles as the frozen contract specifies.

Other explicit boundaries are the root contract's dense `vector<MmlNode*>`
child arrays (no distinction between sparse holes and null entries), typed
registered class metadata instead of arbitrary JS constructors, and null
native pointers standing for source `undefined` where methods such as `core()`
return a missing child. The tests check TypeError names for read-only setters;
their host messages are not byte-for-byte V8 property-assignment messages.

## Current member-definition inventory

All declared `AbstractMmlEmptyNode`, `TextNode`, and `XMLNode` methods have
definitions in `mml_empty.cpp`. The token/layout/base branch definitions are
present in `mml_branches.cpp`, and the four defaults accessors are in
`mml_bindings.cpp`. All 85 executable source members have native definitions
in the current snapshot. This is a static declaration/definition inventory
only; a successful native link has not been established by this review.

This review ran the oracle consistency check and read-only CJS probes. Root
reported a filtered native build passed the earlier 1,052-assertion, 38-case
snapshot. The current checked fixture has 1,061 assertions and 40 cases; I did
not run a build for that later snapshot. This review does not accept P8b.
