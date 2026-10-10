# E9 factory profile usage audit

This is a read-only source audit of the pinned MathJax 4.1.3 profile at commit
`fb987178c2d279a99b0db5ea02e751691703955e`. The existing
`docs/math-port/evidence/e1-tree-dependencies.md` is the broader tree contract
evidence. E1 already covers the generic NodeFactory/WrapperFactory interfaces,
`AbstractNode`, MML constructor dependencies, class identity, and relevant
source hashes. This E9 packet reconciles with those findings and adds scoped
factory derivations, real call sites, creator-view access, and Object.prototype
behavior. E1 does not provide this profile call-site audit. The source files
below are listed in `docs/math-port/scope.json`; their listed SHA-256 values
are repeated here to make the evidence reproducible. The factory fixture
generator pins the three generic files to the same hashes.

## Factory types and registrations observed

The scoped derivation chain for the native MathML tree is
`MmlFactory -> AbstractNodeFactory<MmlNode> -> AbstractFactory`.
`MmlFactory.ts:33-38` sets `static defaultNodes = MML`; `MML.ts:82-140`
builds the registry from each class's `prototype.kind`. Those entries are
ordinary MathML names (including `math`, `mi`, `mrow`, `inferredMrow`,
`TeXAtom`, `text`, `xml`), not Object prototype names.

The SVG wrapper chain is `SvgWrapperFactory -> CommonWrapperFactory ->
AbstractWrapperFactory -> AbstractFactory`. `CommonWrapperFactory.ts:54-80`
declares an empty `defaultNodes` and the output-jax reference; `svg/WrapperFactory.ts:44-71`
supplies `SvgWrappers`; `svg/Wrappers.ts:61-100` registers the SVG wrapper
classes by static `kind`. These are MathML/output names, plus `scriptbase`,
`text`, `xml`, and the base wrapper's `unknown`; none is `toString`,
`valueOf`, `constructor`, or `__proto__`.

There is a second production use of the generic `AbstractFactory`: TeX
`StackItemFactory` extends it (`input/tex/StackItemFactory.ts:38-60`), sets
`defaultKind = 'dummy'`, and is constructed with `configuration.items`
(`ParseOptions.ts:170`). Its own static field is named `DefaultStackItems`
(`StackItemFactory.ts:45-47`), not `defaultNodes`; the constructor therefore
uses the explicitly passed config map. The profile's stack-item kinds are
registered TeX parser item names. TeX package extension registration adds
items with `setNodeClass` (`Configuration.ts:417-421`).

Separately, `input/tex/NodeFactory.ts` is not a subclass of the generic
factory. It owns parser creator functions (`:54-59`), directly mutates that
table through `set` and `setCreators` (`:149-162`), dispatches `create` to a
kind creator or its `node` fallback (`:171-177`), and uses the MmlFactory to
make actual nodes (`:71-85`). `ParseOptions.ts:165-168` seeds those creators;
`Configuration.ts:417-420` adds package creators. This is an additional
creator-map interface, not a `nodeMap`/`setNodeClass` operation.

## Method call sites in the scoped profile

`AbstractFactory` itself (`core/Tree/Factory.ts:148-201`) initializes
`nodeMap` by iterating `Object.keys(nodes)` and calling `setNodeClass`; the
generic methods are implemented only there. Its `create` looks up `node[kind]`
then `node[defaultKind]` (`:160-161`), `setNodeClass` updates both maps and
captures that class in a creator (`:167-173`), `getNodeClass` reads the
registry (`:178-180`), deletion removes both entries (`:185-188`),
`nodeIsKind` uses `instanceof` (`:193-195`), and `getKinds` returns registry
keys in insertion order (`:200-202`).

The production consumers found in the scoped core/MML/output/TeX sources are:

| Operation | Call-site evidence | Observed use |
|---|---|---|
| `create` | `MmlNode.ts:450,468`; `MmlNodes/mo.ts:238`; many `BaseMethods.ts` and `BaseItems.ts` parser calls; `MathDocument.ts:1056-1062`; `CommonWrapper.ts:1421,1436`; `svg/Wrappers/maction.ts:285` | MML constructors, inferred rows, text/error nodes, parser output, copy, and tooltip math roots. NodeFactory's separate dispatch is `NodeFactory.ts:171-177`. |
| `wrap` | `core/Tree/WrapperFactory.ts:74-76`; `CommonWrapper.ts:563`; `common.ts:466,493`; `svg/Wrappers/maction.ts:298`; `CommonWrapper.ts:216`-adjacent text wrapping | The wrapper factory uses `node.kind`, passes that same node as argument one, and delegates to generic `create`. Output traversal wraps roots and children. |
| `getNodeClass` | `MmlNode.ts:455-456`; `MmlNodes/mo.ts:238`; `common.ts:887-890` | MML node class `defaults`, the MathML `math` class for `instanceof`, and wrapper class styles. |
| `nodeIsKind` | `Node.ts:246-248`; calls throughout scoped TeX and MML/output files, e.g. `BaseItems.ts:243,248,406,431`, `MmlNodes/mtable.ts:141-175`, `LinebreakVisitor.ts:221-240`, `CommonWrapper.ts:658-715` | Dispatches kind checks across parser stack items, MML nodes, and wrappers. Only string kinds are passed at these call sites. |
| `getKinds` | `Visitor.ts:145-153`; `common.ts:887-890` | Builds visitor handlers and iterates registered wrapper classes for styles. |
| `setNodeClass` | generic constructor `Factory.ts:152-154`; TeX item registration `Configuration.ts:417-421` | Default MML and wrapper registries are constructor-seeded. TeX stack-item package additions/replacements are runtime registrations. |

The audit found no scoped call to generic `deleteNodeClass`. It found no
scoped write to generic `nodeMap` or to the generic `node` creator object.
However, the protected TypeScript `node` is a normal JS object: `MmlFactory`
exposes it through `MML` (`MmlFactory.ts:40-45`), and
`CommonWrapperFactory` exposes it through `Wrappers` (`common/WrapperFactory.ts:83-87`).
The revision-2 native contract correctly models these as live independent
creator-view handles, separate from the class map. A scoped-source search found
no reads of either factory getter and no writes through them: the only SVG
`Wrappers` text match outside the class declarations is the UI module's
`MathJax._.output.svg.Wrappers.maction` class export, not a factory getter
(`ts/ui/no-dark-mode/no-dark-mode.ts:79`, outside this TeX/SVG profile). Thus
their live-view semantics remain required API behavior even though the
profile's internal rendering call sites do not exercise direct mutation.

The TeX-local NodeFactory's `.factory` table is directly mutated by its own
registration methods as described above. No profile package writes that field
directly, but package configuration passes creator maps to `setCreators`.
`setCreators` uses `for...in` rather than `Object.keys` (`NodeFactory.ts:158-161`),
so inherited enumerable entries on an extension map can be observed; whether
the native configuration boundary permits such maps is not established by the
raw TeX profile.

## Object.prototype names and reflective behavior

The generic source really does inherit from `Object.prototype`: `node` is
initialized with `{}` (`Factory.ts:143`), and `create` checks it by ordinary
property lookup (`:160-161`). A direct execution of the pinned CJS source with
a minimal factory confirmed the non-node outcomes: `create('toString')`
returns a string; `create('constructor')` returns an ordinary object;
`create('valueOf')` throws `TypeError`; and `create('__proto__')` throws
`TypeError`. These are upstream JavaScript outcomes; `AbstractFactory.create`
does not validate that the returned value is a FactoryNode. `getNodeClass`
for all four is missing, so `nodeIsKind` reaches `instanceof` with an invalid
RHS and throws `TypeError`. The revision-1/2 native contract's TypeError when a
typed product API encounters a non-node result is a deliberate host adaptation
bounded to the typed raw-TeX service, not MathJax's behavior for the generic
untyped API.

No scoped MML kind (`MML.ts:82-140`), SVG wrapper kind
(`svg/Wrappers.ts:61-100`), or configured TeX item kind was found to request
those names. This is positive evidence only for the pinned built-in profile.
It does not prove the absence of an extension-created kind with one of those
names or a caller that directly invokes the exposed `MML`/`Wrappers` maps.
`__proto__` is especially distinct: assigning it on the ordinary creator
object changes that object's prototype, so later deletion of a class mapping
does not necessarily remove the inherited creator. The generic oracle's
`prototype_creator_retained` fixture already covers this operation. Revision 2
correctly keeps the creator view live and separate from `nodeMap_`, preserving
this prototype creator after class deletion. Arbitrary descriptors and
arbitrary multi-level JS prototype chains are outside the native bridge, as the
contract states.

`nodeIsKind` is not string comparison: `Factory.ts:193-195` uses JS
`instanceof` against the *current* class returned by `getNodeClass`. MML
`Node.isKind` delegates to it (`Node.ts:246-248`), and `MmlMo.coreParent`
performs the additional direct `parent instanceof factory.getNodeClass('math')`
check (`MmlNodes/mo.ts:235-248`). Thus native predicates need inheritance
semantics and registry rebinding must affect later checks; equality of kind
strings is inadequate. No scoped code replaces a registered constructor's
`prototype`, customizes `Symbol.hasInstance`, or reflects on `instanceof`
beyond these checks. Compiled RTTI predicates model the built-in class tree,
but they do not establish equivalence for those arbitrary JS mutations.

## Static constructor metadata consumed

The MML family uses class static `defaults`: `AbstractMmlNode` initializes
attributes from `factory.getNodeClass(this.kind).defaults` and then from the
registered `math` class's `defaults` (`MmlNode.ts:443-459`).
`MmlNodes/mo.ts:238-244` additionally needs the returned `math` constructor
itself for an `instanceof` check. Node-specific classes define defaults in
their corresponding `MmlNodes/*.ts` files; `MmlInferredMrow.defaults` aliases
`MmlMrow.defaults` (`MmlNodes/mrow.ts:191-201`). A family metadata adapter
therefore needs both actual class identity and the per-class defaults map,
including aliases/inheritance; a single fixed global property table would not
represent these reads.

The wrapper family consumes constructor static `styles`: `common.ts:887-900`
iterates kinds and passes each registered class to `addClassStyles`. The
default base wrapper also declares static `kind`, `styles`, and other metadata
(`common/Wrapper.ts:268-300`); SVG wrapper classes set static `kind` from MML
prototype kinds, with special cases such as `scriptbase` (`svg/Wrappers/*.ts`).
The generic factory contract's opaque static metadata handle must therefore
support at least identity, MML defaults, wrapper styles, and the `math`
constructor role. No finding here assigns a final native metadata schema; that
remains with the family contracts.

## Live creator views and replacement dispatch

Revision 2's `creatorView()` is the right surface for the source getters:
`MmlFactory.MML` and `CommonWrapperFactory.Wrappers` return `this.node` itself,
not a copy and not `nodeMap`. The E1 audit independently describes `MmlFactory`
as exposing the creator map (E1, “Ownership, registration and dependency
cycles”). A native class-map mutation must not implicitly rewrite a
caller-held creator view, and changing a view entry must not change
`getNodeClass`; this independence comes from the root contract and is not
exercised by the pinned profile's internal getter call sites.

For `setNodeClass`, source `Factory.ts:167-173` first writes the class into the
Map, reads that class into local `KIND`, and writes a fresh creator closure
under the kind. That closure invokes the captured constructor. Calling
`setNodeClass` again replaces both current registrations; a previously saved
creator function still holds the prior captured class. The revision-2 contract
faithfully freezes that distinction for class-map and creator-view handles.
No subclass in the scoped generic MML/SVG factory chain overrides
`create`, `wrap`, `getNodeClass`, `nodeIsKind`, `setNodeClass`, or
`deleteNodeClass`; the profile's only inherited `setNodeClass` registration
call outside factory initialization is `Configuration.add` for StackItemFactory
items (`Configuration.ts:417-421`). That operation may replace a class by kind
as configurations are added, so the captured-constructor replacement rule is
live for this path.

The separate TeX parser NodeFactory is not governed by this generic creator
closure: `NodeFactory.set` replaces a direct function-table entry, and
`create` reads the current function entry for each call
(`input/tex/NodeFactory.ts:149-177`). No evidence in the pinned profile
mutates an already-returned creator through `MML`/`Wrappers`, replaces
constructor prototypes, or adds a custom `Symbol.hasInstance`. Those extension
behaviors remain unknown beyond the bridge's declared typed host boundary.

## Constructor-order obligation for a C++ port

The JS base constructor `AbstractNode` installs fields, copies properties via
`this.setProperty`, and conditionally calls `this.setChildren(children)`
(`core/Tree/Node.ts:186-197`). JS dispatches those calls to the most-derived
implementation even while the base constructor is running. For MML, the
derived `AbstractMmlNode` constructor makes further virtual-dependent reads:
it reads `this.arity`, creates and parents an inferred row when negative,
calls `this.setChildren`, reads `this.kind`, obtains the two static defaults
maps, and initializes attributes (`MmlNode.ts:443-459`). `MmlInferredMrow`
overrides `kind` (`MmlNodes/mrow.ts:191-202`), and concrete MML classes
provide `arity`, kind, and specialized `setChildren` behavior.

The observed ordering means a C++ implementation cannot invoke those operations
from a base constructor and expect derived virtual dispatch. It needs a
complete derived object before the MML initialization sequence runs; the
contract's adopt-then-initialize ordering also keeps children/parent links
from pointing at a destroyed partial object. Preserve MML ordering: initialize
the inferred row first if needed, then set passed children, then load defaults
for the final virtual kind and `math`, then apply explicit attributes. This is
an obligation inferred from source behavior and the frozen factory contract,
not a claim that node-family constructors are already ported.

The generic `Factory.ts` constructor itself reads `this.constructor.defaultNodes`
when no map is supplied (`:148-154`). C++ base construction cannot discover a
derived static field automatically. Current default production factories
avoid ambiguity by requiring injected `MML`/`SvgWrappers` maps at construction
in the native adaptation. `StackItemFactory` also receives `configuration.items`
explicitly (`ParseOptions.ts:170`) and has a differently named
`DefaultStackItems` field, so it does not satisfy the generic static convention
without an explicit adapter.

## Source references and hashes

Hashes below come from the pinned `scope.json` manifest; the first three also
match `tools/math-port/factory-oracle.cjs` source pins. Entries marked
`unreviewed` in the manifest are evidence sources, not accepted native ports.

| Source | SHA-256 |
|---|---|
| `ts/core/Tree/Factory.ts` | `e61d2f5868287e2f382503cf690cd241698bf212957c1795d0cd35ce3bef049e` |
| `ts/core/Tree/NodeFactory.ts` | `b02c5ac2cabb61755785d35def6b57f8246322c626f548df600ca81af27b4599` |
| `ts/core/Tree/WrapperFactory.ts` | `8c59ed6a5e646e0736730070ba87161d7060f1f4a083e92afcc8012345537cea` |
| `ts/core/MmlTree/MmlFactory.ts` | `5e7338cd3eed5a4002243b7371dc5098c6433ff4ba4345cb2d50cf0385b2abdb` |
| `ts/core/MmlTree/MML.ts` | `c64bcfdf65452559995a92c13db75133b1500ef59efc2be5c9489ceacc4d4964` |
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `ts/input/tex/StackItemFactory.ts` | `229bf0f6bd58642bff86102bb8fb6fd2b9a61d3f52ad31ce738f3f2cefa2e405` |
| `ts/input/tex/NodeFactory.ts` | `bf20e8d4f7ed43450df42d65556ccbceea1469541b214c37544f180aa205c840` |
| `ts/output/common/WrapperFactory.ts` | `a798803bab094ae1e72cd7f96eb37aca936ac458da145c1ebb22079a0280b6cd` |
| `ts/output/svg/WrapperFactory.ts` | `6d1a4a793905ef6aadf5481781f7c88f32e9512ac49dab443097f43789da3d00` |
| `ts/output/svg/Wrappers.ts` | `8a4fd2c7a4f92b1fac22c4e5f79072b8bf24d9942933478c0bdac1ab48fcae23` |
| `ts/output/common.ts` | `3686a8e803e26ae59d03c096ac2e6e5b1ab1332a5c4bcaa083cb9a02c2c61096` |
| `ts/core/Tree/Visitor.ts` | `29c372210da67492445b92a8a3a754c6d30fa91cc38c7f25eb5b4ffc51129106` |

## Open evidence limits

The pinned profile audit does not establish behavior for third-party JS
extensions that mutate factory maps, constructor prototypes, static metadata,
or `Symbol.hasInstance`. It also does not inventory arbitrary custom values
that callers can pass through the public factory API. The source gives no
evidence that those dynamic features are used by the shipped profile, but
scope membership alone is not proof that every embedding excludes them. E1 is
present and consistent with this audit; its broad tree evidence should not be
read as proof that these generic-factory extension behaviors are absent.
