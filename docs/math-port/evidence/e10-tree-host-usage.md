# E10 tree host usage audit

Read-only audit of pinned MathJax 4.1.3 source at commit
`fb987178c2d279a99b0db5ea02e751691703955e`. This packet records concrete host
behavior and scoped MathML/TeX/SVG consumers; it does not freeze a replacement
interface or claim a general clean port. E1 (`evidence/e1-tree-dependencies.md`)
already records the broad Node, Wrapper, and MmlNode contracts. E10 focuses on
runtime array identity, mutation during traversal/copy, virtual dispatch, and
the node/text/XML and wrapper distinctions.

## Child arrays are live, replaceable objects

`AbstractNode` initializes a live `childNodes` array to `[]` and stores parent
and property state (`core/Tree/Node.ts:159-197`; pinned CJS
`node_modules/@mathjax/src/cjs/core/Tree/Node.js`, constructor). The array is
public and mutable. `setChildren` replaces the array object with a new empty
array and appends the supplied sequence (`Node.ts:253-258`); it does not detach
the old children from their former parents. `appendChild` pushes into the
current array and sets `child.parent = this` (`:263-267`). `replaceChild`
replaces the current array slot and changes parent links (`:272-282`);
`removeChild` splices the current array and clears the child's parent
(`Node.ts:288-303`). Passing null to `appendChild` is not supported: the push
occurs, then dereferencing `child.parent` throws.

The MML implementation can contain explicit null child slots. `MmlNode.copy`
copies non-null children through `appendChild` and pushes null directly into
the copied `childNodes` (`MmlNode.ts:478-489`). This is not the behavior of
generic `AbstractNode.copy`, which skips nulls and packs surviving children
(`Node.ts:308-316`). Generic traversal and `AbstractMmlTokenNode.walkTree`
skip null children (`Node.ts:335-342`; `MmlNode.ts:1090-1098`), while other
methods can assume a populated slot (for example inferred-row access at
`MmlNode.ts:595-599`). The source therefore has more than one null-slot policy;
an adapter cannot flatten the distinction without changing observable copies
or traversal.

The source uses JavaScript `for...of` over the current array in generic
`walkTree` (`Node.ts:335-342`), generic copy (`:311-315`), MML copy
(`MmlNode.ts:478-489`), and token walking (`MmlNode.ts:1090-1098`). Those
iterators retain the array object captured when the loop starts but read its
indexed contents and length as iteration proceeds. Replacing the owner's
`childNodes` reference during a callback does not retarget the active iterator;
mutating the captured array in place can affect later visits, and appending
while walking can extend the active iteration. The callback runs before the
child loop, so it can also replace or mutate the array before iteration starts.
There is no cycle guard. These details are source/runtime behavior, not a
recommendation for mutation during traversal.

MML layout repair also replaces `childNodes` directly with a sliced array when
trimming arity (`MmlNode.ts:804-819`), independently of `setChildren`. Code
holding the old array reference continues to observe the old array. A native
interface that returns or stores child-array handles must distinguish the
current array property from a previously obtained array handle; a single
permanent vector object would not match this replacement behavior.

## Copy uses current kind registration

Generic `AbstractNode.copy` calls the instance's factory with its current
virtual `kind`, then shallow-copies properties and recursively copies
non-null children (`Node.ts:308-316`). It does not preserve the original
concrete constructor identity if the factory's registration for that kind has
changed. The newly created root has no parent; appending copied children sets
their parent to that root. Thus copy is factory-relative and class-map
rebinding can change the class of later copies of existing instances.

`AbstractMmlNode.copy(keepIds=false)` similarly calls the current factory with
`this.kind`, shallow-copies properties, copies the current explicit attribute
values (omitting `id` unless requested), and clones children. If there is one
inferred-row child, it copies that row's children instead. It preserves explicit
null slots by direct push (`MmlNode.ts:467-492`). The copy's registered
constructor/defaults are current at copy time; attributes are then copied onto
that newly constructed node.

The MML empty-node branches have specialized copy behavior. `TextNode.copy`
uses the current `text` factory registration and copies only the text
(`MmlNode.ts:1436-1479`). `XMLNode.copy` uses the current registration for
`XML` and clones the opaque XML object (`:1487-1539`). It calls `setXML(clone)`
without forwarding `this.adaptor`, leaving the copy's adaptor at its default
null. `HtmlNode` inherits this XML copy method while changing kind to `html`
(`MmlNodes/HtmlNode.ts:34-40,54-66`), so the current `html` registration is
used and the adaptor is likewise omitted. These are separate source paths and
the native copy contract needs to preserve their current behavior or record an
explicitly accepted adaptation.

The default MML registry includes ordinary nodes, `TextNode`, `XMLNode`, and
`HtmlNode` as distinct entries (`MML.ts:132-135`). TeX creator code exercises
the text path: `NodeFactory.createToken` creates a `text` node, and
`createText` also calls the current MmlFactory `text` registration
(`input/tex/NodeFactory.ts:96-118`). The XML/HTML types are present in the
same MML registry and have specialized common/SVG wrappers. The `texhtml`
handler can construct an `html` node and call `setHTML`
(`input/tex/texhtml/TexHtmlConfiguration.ts:44-86`), but it first checks
`parser.options.allowTexHTML` (`:46-52`); the production default profile keeps
that policy disabled, so this is not a default-profile positive case. This
audit did not find a direct `XMLNode` construction in enabled TeX handlers.
The raw-TeX evidence therefore establishes the text path directly, HtmlNode
conditionally, and not an XMLNode TeX construction.

## Virtual methods called during construction

The generic `AbstractNode` constructor initializes its fields, then calls
`this.setProperty` for every own property and calls `this.setChildren` when the
initial child list is nonempty (`Node.ts:186-197`). JavaScript dispatches both
calls to the most-derived method during base construction. No scoped MML node
overrides `setProperty`, but `AbstractEmptyNode` overrides `setChildren` as a
no-op (`Node.ts:362-380`), and `AbstractMmlNode` overrides `setChildren` to
route children into an inferred row when arity is negative (`MmlNode.ts:595-600`).

The MML constructor itself invokes dynamic members before completing its own
initialization: it reads `this.arity`, creates and parents an inferred row,
calls `this.setChildren`, reads `this.kind`, then gets class and global
defaults and applies explicit attributes (`MmlNode.ts:443-459`). Concrete
classes override kind/arity and specialized tree operations; for example
`MmlInferredMrow.kind` is `inferredMrow` (`MmlNodes/mrow.ts:191-202`). These
calls rely on JS's fully dynamic dispatch while constructors are executing.
The native constructor boundary must initialize such virtual-dependent state
after complete derived-object construction. The generic constructor's
property/child calls also need equivalent dispatch if it is used with
non-MML derived node types that pass those arguments.

For wrappers, `AbstractWrapper` itself stores only `factory` and `node` in the
pinned CJS constructor. Its declared `childNodes` field has no initializer, so
`'childNodes' in wrapper` is initially false; `walkTree` explicitly checks
that presence before descending (`core/Tree/Wrapper.ts:115-169` and pinned
CJS `core/Tree/Wrapper.js`). `CommonWrapper` initializes children later by
mapping the source node's `childNodes` and wrapping each entry
(`output/common/Wrapper.ts:538-555`). Its constructor also calls
`getStyles`, `getStyleData`, `getVariant`, `getScale`, `getSpace`, and
`this.wrap` before it finishes, so JavaScript can dispatch those calls to
overrides on a concrete wrapper subclass while the common base constructor is
running. Native wrappers need a deferred initialization step before relying on
derived virtual behavior.

The base wrapper's `wrap(node)` accepts one node and forwards only that node
to the factory (`core/Tree/Wrapper.ts:152-154`), dropping extra arguments even
though the interface advertises them (`:58-70`). The concrete `CommonWrapper`
override accepts a `parent`, calls the factory with `parent || this`, and when
an explicit parent was supplied pushes the result into that parent's
`childNodes` (`output/common/Wrapper.ts:562-569`). It also records the wrapper
in the output jax's node map. This parent argument is the second constructor
argument forwarded through `AbstractWrapperFactory.wrap`; it is not a generic
base-wrapper behavior. During normal common-wrapper construction, the mapped
child list is assigned after wrapping, while nested `wrap` calls can mutate
the parent list. The `map` callback does not filter null entries, so an MML
explicit null slot reaches the wrapper factory as a null node and can throw.
Do not infer that base wrappers own the CommonWrapper child tree behavior.

## Override inventory in the audited tree profile

Search of the scoped generic Node, MML node classes, common wrappers, and SVG
wrappers found these relevant overrides:

| Method | Implementations and behavior |
|---|---|
| `setProperty` | Only `AbstractNode` in the inspected hierarchy (`Node.ts:209-211`). It writes to the live property object. MML concrete nodes call it for internal markers; no scoped override was found. |
| `setChildren` | `AbstractNode` replaces array and appends; `AbstractEmptyNode` ignores input (`Node.ts:253-258,362-380`); `AbstractMmlNode` routes to inferred row or delegates (`MmlNode.ts:595-600`). |
| `copy` | Generic `AbstractNode`, `AbstractMmlNode`, `TextNode`, and `XMLNode` (`Node.ts:308-316`; `MmlNode.ts:467-492,1468-1470,1536-1539`). Their null/property/attribute/opaque-data policies differ as described above. |
| `walkTree` | Generic `AbstractNode`, `AbstractEmptyNode`, and `AbstractMmlTokenNode` (`Node.ts:335-342,401-404`; `MmlNode.ts:1090-1098`). Token walking visits only `AbstractMmlNode` children, excluding TextNode/XMLNode. |
| `wrap` | Generic `AbstractWrapper` (drops extras) and `CommonWrapper` (forwards parent and updates mapping/tree) (`Wrapper.ts:152-154`; `output/common/Wrapper.ts:562-569`). |

The audited sources show no further override of these exact methods in the
concrete MathML or SVG wrapper files. This is an inventory of the pinned
profile sources examined, not proof that applications cannot add subclasses.

## Native interface constraints and limits

Evidence relevant to root's upcoming interface decision:

- Expose the current child-array identity and replacement, and preserve null
  entries where MML copy creates them. Traversal and copy each use a captured
  array object, skip policy is method-specific, and callbacks can mutate the
  backing array.
- Keep copy factory-relative: it dispatches using current kind/current
  registration. Preserve distinct generic, MML, text, and XML/HTML copy
  behavior, including the source's XML adaptor omission if strict parity is
  required.
- Defer virtual-dependent constructor initialization until the complete
  derived product exists. MML and CommonWrapper constructors both invoke
  derived behavior as they initialize.
- Keep the generic base wrapper's missing child-array state separate from
  CommonWrapper's mapped child wrappers, and preserve the different `wrap`
  argument forwarding rules.

These constraints are based on the listed source paths and pinned CJS output.
The audit does not establish behavior for arbitrary third-party subclasses,
prototype monkey-patching, all MathML input paths, or every possible malformed
tree. It makes no general clean-port or compatibility claim.

## Source hashes

These SHA-256 values are from `docs/math-port/scope.json`; Node and Wrapper
also appear in E1. References above use line numbers from the pinned TS source.

| Source | SHA-256 |
|---|---|
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `ts/core/Tree/Wrapper.ts` | `8729a810aabd6eb3ad22021464f2f27d86116fae9edc0f818527f82e517bf7d9` |
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `ts/core/MmlTree/MML.ts` | `c64bcfdf65452559995a92c13db75133b1500ef59efc2be5c9489ceacc4d4964` |
| `ts/core/MmlTree/MmlNodes/HtmlNode.ts` | `1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665` |
| `ts/input/tex/NodeFactory.ts` | `bf20e8d4f7ed43450df42d65556ccbceea1469541b214c37544f180aa205c840` |
| `ts/input/tex/texhtml/TexHtmlConfiguration.ts` | `e31f1539452eba33048d8e7bcf87fcf9690f23be1412799ddf2a6f43beab4a3b` |
| `ts/output/common/Wrapper.ts` | `d1db6112db1cb3546658eb8868b5997a4f69e588632ecbe4e08a71949544a828` |
