# E1 Tree dependency evidence

Read-only audit of MathJax 4.1.3 source in `.reference/MathJax-src`. This file
records the behavior-bearing dependencies that a later root-owned contract
must account for. It does not define a replacement node model or approve scope.

## Pinned inputs

| Source | SHA256 |
| --- | --- |
| `ts/core/Tree/Node.ts` | `532000186FC65DB8F227B7E69F1ED331EBDD58BCA2D1984AA099D81F3CBCD1D0` |
| `ts/core/Tree/NodeFactory.ts` | `B02C5AC2CABB61755785D35DEF6B57F8246322C626F548DF600CA81AF27B4599` |
| `ts/core/Tree/Wrapper.ts` | `8729A810AABD6EB3AD22021464F2F27D86116FAE9EDC0F818527F82E517BF7D9` |
| `ts/core/Tree/WrapperFactory.ts` | `8C59ED6A5E646E0736730070BA87161D7060F1F4A083E92AFCC8012345537CEA` |
| `ts/core/MmlTree/MmlNode.ts` | `CC0594EB5782928412C4307637866D9F80AC147AF577297EDD48EE88B63B8656` |

## Node and NodeFactory

`Node.ts:30-31` defines `Property` as `string | number | boolean` and
`PropertyList` as a string-keyed object of those values. `Node<N,C>:41-128`
requires the kind and factory, mutable parent and ordered children, property
read/write/list/delete, kind test, child replacement/removal/indexing, deep
copy, recursive search, and traversal with caller data. `NodeClass:138-150`
is a constructor contract taking a factory, optional properties and optional
children.

`AbstractNode:159-197` owns `parent` (initially null), protected `properties`,
ordered `childNodes`, and the readonly factory handle. Its constructor writes
own enumerable input properties through `setProperty` and attaches initial
children. `kind` defaults to `unknown` (`202-204`). Property methods operate on
the own object (`209-241`); `getAllProperties` returns the live object. Kind
testing delegates to factory class identity (`246-248`).

Tree ownership behavior is explicit in `setChildren`, `appendChild`,
`replaceChild`, and `removeChild` (`253-295`): setChildren clears the prior
array then appends; append sets the child’s parent; replacement sets the new
parent and clears the old parent only if it still points at this node; removal
splices the child and nulls its parent. The implementation does not detach a
child from a former parent before appending it, so malformed/multiply-parented
states are possible and observable. `childIndex` is identity-based and returns
null when missing (`300-303`).

`copy` asks the current factory for the same `kind`, shallow-copies properties,
recursively copies non-null children and reparents copies (`308-317`).
`findNodes` includes the receiver by calling `walkTree` (`322-329`);
`walkTree` is preorder, skips null children, passes the same data through, and
returns that data (`335-343`). `toString` is structural/debug text
(`350-352`).

`AbstractEmptyNode:362-414` intentionally overrides child methods: children
are ignored, append returns the supplied node without attaching it,
replace returns the old node without replacement, index always returns null,
walk visits only self, and stringification returns kind. This behavior is
observable independently of the empty subclass's declared arity.

`NodeFactory.ts:34-44,54-67` adds `create(kind, properties, children)`; its
implementation calls the registered per-kind function directly. Registration
and class-identity semantics live in the imported `Factory.ts` (not one of the
five pinned files): its static `defaultNodes`, mutable `defaultKind`, kind to
constructor map, and constructor closures are at `Factory.ts:128-173`;
`create` falls back to the default kind, `nodeIsKind` uses JavaScript
`instanceof`, and `getKinds` preserves map insertion order (`160-202`). The
NodeFactory override at `NodeFactory.ts:61-67` directly indexes its creator
map, so it does not use that generic fallback. A C++ contract must resolve
which factory's creation semantics apply and retain registered class identity
and inheritance tests; a kind-string equality check is not equivalent to
`instanceof`.

## Wrapper and WrapperFactory

`Wrapper.ts:37-70` defines a visitor wrapper as a kind, a reference to one
source node, a wrapper-child array, `wrap(node, ...args)`, and preorder
`walkTree`. `WrapperClass:81-97` is a constructor receiving its wrapper factory,
the node being wrapped, and extension arguments.

`AbstractWrapper:107-169` stores the source node and protected factory; its
kind getter forwards `node.kind` (`130-132`). `wrap` delegates to the same
factory (`152-154`). `walkTree` calls the callback on self and descends only
when a `childNodes` member exists, skipping null children (`159-169`). It does
not own or mutate the semantic node tree; wrappers are a parallel visitor view
that may be lazy or cached by concrete wrappers/factories.

`WrapperFactory.ts:37-50` defines `wrap(node, ...args)` over the generic factory;
`AbstractWrapperFactory:62-76` dispatches by `node.kind` into `create` and
forwards arguments. The generic `Factory` registration map and static class
identity machinery are shared with NodeFactory. Concrete wrappers can override
`wrap` or traversal behavior and add arbitrary state/arguments; this generic
surface cannot be reduced to a fixed node-to-wrapper table without checking
the concrete profile.

## MmlNode semantic contract

`MmlNode.ts:24-34` imports Attributes, generic node/factory types, the
MmlFactory, and `DOMAdaptor`. `AttributeList:37-39` is distinct from
PropertyList: each key maps to a pair `[originatingNodeKind, Property]`.
`TEXCLASS`, `TEXCLASSNAMES`, spacing arrays, `MATHVARIANTS`, and indentation
attribute names are static semantic tables (`45-126`), not per-node fields.
`MMLNODE:131` is a union of MmlNode, TextNode and XMLNode.

The `MmlNode` interface (`138-276`) extends generic Node and adds these
behavior-bearing fields/getters: `isToken`, `isEmbellished`, `isSpacelike`,
`linebreakContainer`, `linebreakAlign`, `isEmpty`, `arity`, `isInferred`,
logical `Parent`, `notParent`, actual `parent`/`childNodes`, `texClass`,
`prevClass`, `prevLevel`, and `attributes`. It also requires specialized copy,
embellished-operator `core`/`coreMO`/`coreIndex`, `childPosition`, TeX-class
assignment/spacing, spacing-attribute detection, inherited-attribute
propagation, `inheritAttributesFrom`, structural verification and `mError`.
These values are partly derived and overridable; they are not annotations that
can be filled once during parsing.

`AbstractMmlNode:312-459` extends AbstractNode and carries shared defaults,
inheritance policy, verification defaults, prior-class/level state, an
Attributes object, and its own factory/tree references. The static defaults
(`319-325`) mark `mathbackground`, `mathcolor`, `mathsize`, and `dir` as
`INHERIT`. Static policy tables (`335-393`) encode source-kind to target-kind
exclusions (`noInherit`), child-boundary stopping (`stopInherit`), attributes
that always inherit without a node default, and verification options.

Construction (`443-459`) inserts an `inferredMrow` for any negative arity,
attaches children, constructs Attributes from the registered class's static
`defaults` plus the `math` class's defaults as globals, then applies explicit
attributes. Therefore MML construction depends on concrete class registration
being available before its constructor runs and on `kind` being dynamically
resolved to select the class defaults. `copy(keepIds)` (`467-492`) recreates
through the factory, copies node properties, copies only explicit attributes
(dropping `id` unless requested), unwraps a sole inferred row, recursively
copies children, and preserves null child slots; inherited state is recreated,
not copied.

Core/parent/child semantics (`497-689`) include separate stored `texclass`,
default predicates, arity `Infinity`, logical Parent skipping `notParent`
ancestors, child setters routed through inferred rows, flattening of inferred
rows under arbitrary-arity parents, conversion to explicit `mrow` for fixed
arity parents (including transferring attributes and properties), core-node
accessors and logical child-position lookup. Concrete MML node classes
override `kind`, arity, predicates and structural behavior.

TeX spacing (`694-749`) mutates prior class/level on participating nodes,
transfers state from embellished cores, uses `TEXSPACE`/`TEXSPACELENGTH`,
suppresses nonnegative spaces at positive script levels, and consults the core
operator's spacing attributes. Inheritance (`765-898`) walks origin-tagged
attributes, consults the static policy tables and per-class defaults, sets
displaystyle/scriptlevel/prime state, repairs fixed arity, handles linebreak
indentation, and calls virtual child propagation. Helpers
`setChildInheritedAttributes` and `addInheritedAttributes` are protected
extension points; subclasses can change how attributes or derived display and
script levels reach descendants.

Verification (`906-977`) is option-gated: checks attributes against per-kind
defaults plus allowed names, validates mathvariants unless ignored, checks
arity with special handling for unary nodes and Infinity, then recursively
checks children. `mError` (`987-1012`) suppresses nested errors inside merror,
creates merror and sometimes mtext/text via factory, puts the message in a
data attribute/title or child text, and replaces this node in its actual
parent. It consequently depends on factory registration and mutable parent
ownership.

Subclass layers are semantic, not just code reuse:

- `AbstractMmlTokenNode:1020-1099` has token defaults (`mathvariant=normal`,
  `mathsize=INHERIT`), isToken=true, computes emptiness/text from TextNode and
other `textContent` children, propagates inherited attrs and walking only to
AbstractMmlNode children.
- `AbstractMmlLayoutNode:1109-1158` has inferred-row arity -1 and derives
spacelike/embellished/core/TeX class behavior from its first child.
- `AbstractMmlBaseNode:1169-1222` derives embellished/core from the base and
applies base-aware TeX class rules while treating later script children as
separate lists.
- `AbstractMmlEmptyNode:1233-1429` overrides the generic child, attribute,
spacing, parent/core, inheritance, verification and error behavior for
attribute-less leaf nodes.
- `TextNode:1436-1480` owns text; `setText` is mutable and copy creates a new
registered `text` node. `XMLNode:1487-1549` owns an opaque XML object plus a
DOMAdaptor, serializes and clones it through that adaptor, and copies through
the registered `XML` node factory entry.

## Ownership, registration and dependency cycles

The generic semantic tree has one node object per tree position, parent
back-pointers and ordered child arrays; wrappers point at those nodes and have
their own wrapper-child arrays. Factories are retained by nodes and used for
deep copy, inferred-row repair and errors. Attributes are per-MmlNode, but
their global layer points to defaults on the registered `math` class.

The registry uses constructor objects, not only names. `Factory.ts` constructs
per-kind creator closures, exposes static `defaultNodes`, and implements
`instanceof`; `MmlFactory.ts:24-46` seeds that registry from `MML` and exposes
its creator map as `MML`. The MML map (`MML.ts:24-82+`) imports each concrete
node class and keys entries using each class prototype's dynamic `kind`
getter. The class's static `defaults` are read through the constructor object.
Thus kind registration, inheritance/prototype identity, static defaults, and
factory lifetime are all observable dependencies.

TypeScript source imports form these cycles: `Node.ts -> NodeFactory.ts ->
Node.ts`; `Wrapper.ts -> WrapperFactory.ts -> Wrapper.ts`; and `MmlNode.ts ->
MmlFactory.ts -> MmlNode.ts` (with `MmlFactory -> MML -> MmlNode` and concrete
MmlNodes). Several references are used only as types and may be erased by the
TypeScript compiler, but the source-level contracts are mutually recursive.
The C++ module graph must avoid illegal cycles while keeping factory/node and
wrapper/factory interfaces mutually referential, and concrete MmlNode
registration must not introduce a module cycle. This audit does not prescribe
how to break them.

## Values beyond the shared scalar Property contract

The source's `Node.Property` itself is scalar only (`Node.ts:30`), so ordinary
node properties and MathML attribute values fit the scalar Property contract.
The following are separate structured values/interfaces and cannot be
flattened into one `Property` without losing source semantics:

- `AttributeList` maps names to `[originating node kind, Property]` tuples
  (`MmlNode.ts:37-39`).
- `noInherit` is a three-level kind/kind/attribute table;
  `stopInherit` and `alwaysInherit` are policy maps; they are class-static
  structured tables, not user Property values (`335-381`).
- `MMLNODE` is a tagged semantic union containing ordinary MmlNode, TextNode
  and XMLNode (`131`).
- `XMLNode.xml` is an opaque object and its adaptor is a generic host object
  (`1491-1523`); serialization/cloning requires DOMAdaptor behavior
  (`1526-1539`). It is not a Node `Property`.
- Generic visitor callbacks and factory/wrapper extension arguments are typed
  `any` (`Node.ts:128`, `Wrapper.ts:64,70,95`, `WrapperFactory.ts:50`); their
  concrete uses must be inventoried from enabled consumers before freezing
  C++ signatures.

## Source-map/method coverage references

Primary implementation sections: `Node.ts:41-128,138-150,159-414`;
`NodeFactory.ts:34-67`; `Wrapper.ts:37-169`;
`WrapperFactory.ts:37-76`; `MmlNode.ts:37-131,138-304,312-1013,
1020-1549`. Supporting registration evidence is `Factory.ts:29-203`,
`MmlFactory.ts:24-46`, and `MML.ts:24-82+`.

## Exhaustive MmlNode interface and implementation ledger

The tables below are source-member ledgers, not a proposed C++ class layout.
“State/dependency” describes observable reads, writes, delegation or factory/
parent dependencies. Interface-only declarations are separated from members
with bodies so signatures cannot be mistaken for implementations. Line
numbers refer to the pinned `ts/core/MmlTree/MmlNode.ts`.

### Interface-only members

`MmlNode:138-276` declares these members without implementations:

| Member | Lines | Contract dependency |
| --- | --- | --- |
| `isToken`, `isEmbellished`, `isSpacelike`, `linebreakContainer`, `linebreakAlign`, `isEmpty` | 142-147 | Derived predicates and container behavior. |
| `arity`, `isInferred`, `Parent`, `notParent` | 152-160 | Child constraints and logical parent distinct from actual parent. |
| `parent`, `childNodes` | 165-170 | Mutable parent edge and ordered children. |
| `texClass`, `prevClass`, `prevLevel` | 178-180 | TeX class and spacing traversal state. |
| `attributes` | 185 | MML attribute layers. |
| `copy(keepIds?)` | 193 | Deep copy with optional id retention. |
| `core`, `coreMO`, `coreIndex` | 199-211 | Embellished operator core lookup. |
| `childPosition` | 216 | Index under logical parent. |
| `setTeXclass(prev)` | 224 | TeX class propagation. |
| `texSpacing()` | 229 | TeX inter-atom spacing. |
| `hasSpacingAttributes()` | 234 | Explicit spacing/form query. |
| `setInheritedAttributes(attributes, display, level, prime)` | 245-250 | Origin-tagged attribute and style-context propagation. |
| `inheritAttributesFrom(node)` | 258 | Reconstruct inherited context for inserted nodes. |
| `mError(message, options, short?)` | 268 | Factory-created error replacement. |
| `verifyTree(options?)` | 275 | Option-controlled recursive validation. |

`MmlNodeClass:283-304` is interface-only as well: optional constructor-static
`defaults` (`287`) and a constructor receiving `MmlFactory`, attributes and
children (`299-303`). The base constructor consumes these at
`MmlNode.ts:454-457`.

### Module-scope initialized semantic tables in MmlNode.ts

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| `TEXCLASS` | 45-55 | Named numeric TeX atom-class constants consumed in class assignment and spacing. |
| `TEXCLASSNAMES` | 57-66 | Index-aligned class-name list for the numeric classes. |
| `TEXSPACELENGTH` | 71-76 | Index mapping from spacing-table magnitude to MathML mathspace keyword. |
| `TEXSPACE` | 82-91 | TeX class-pair spacing matrix. |
| `MATHVARIANTS` | 97-116 | Set of accepted `mathvariant` strings used by attribute verification. |
| `indentAttributes` | 121-126 | Names of indentation attributes used by linebreak-related consumers. |

### `AbstractMmlNode` implemented members

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| static `defaults` | 319-325 | Shared defaults; mathbackground/mathcolor/mathsize/dir have `_inherit_`. |
| static `noInherit` | 335-360 | Kind-origin/kind-target/attribute exclusion table read by propagation. |
| static `stopInherit` | 367-371 | Per-kind attributes removed before child propagation. |
| static `alwaysInherit` | 377-381 | Allows propagation even without a kind default. |
| static `verifyDefaults` | 386-393 | Default arity/attribute/mathvariant/error/fix options. |
| `prevClass` field | 403 | Previous TeX class; initially null. |
| `prevLevel` field | 408 | Previous script level; initially null. |
| `attributes` field | 413 | Per-node Attributes instance. |
| `childNodes` field | 418 | Ordered MmlNode children. |
| `parent` field | 423 | Actual parent pointer. |
| readonly `factory` field | 428 | MmlFactory dependency for lookup and creation. |
| protected `texclass` field | 433 | Backing state for `texClass`; initially null. |
| `constructor` `(factory, attributes={}, children=[])` | 443-459 | Reads dynamic arity/kind; creates inferred row when needed; attaches children; gets this class defaults and `math` global defaults via factory; constructs Attributes and applies explicit attrs. |
| `copy(keepIds=false)` | 467-492 | Factory-creates same kind; shallow-copies properties and explicit attrs, omitting id by default; unwraps sole inferred row; deep-copies children and preserves null slots. Inherited attributes are rebuilt, not copied. |
| `get texClass()` | 497-499 | Reads protected `texclass`. |
| `set texClass(value)` | 504-506 | Writes protected `texclass`. |
| `get isToken()` | 511-513 | Base false; token subclass overrides. |
| `get isEmbellished()` | 518-520 | Base false; layout/base and concrete subclasses override. |
| `get isSpacelike()` | 525-527 | Base false; layout/concrete subclasses override. |
| `get linebreakContainer()` | 532-534 | Base false; concrete containers override. |
| `get linebreakAlign()` | 540-542 | Base `data-align`; concrete containers may override. |
| `get isEmpty()` | 547-552 | Scans children, skips null, false for any nonempty child. |
| `get arity()` | 560-562 | Base `Infinity`. |
| `get isInferred()` | 567-569 | Base false. |
| `get Parent()` | 575-581 | Follows actual parents while `notParent` is true. |
| `get notParent()` | 586-588 | Base false; inferred/transparent nodes override. |
| `setChildren(children)` | 595-600 | For negative arity delegates to inferred row; otherwise generic clear/append. |
| `appendChild(child)` | 607-633 | Negative arity redirects; inferred rows flatten for Infinity, otherwise convert to explicit mrow and transfer children/attributes/properties; generic append sets parent. |
| `replaceChild(newChild, oldChild)` | 639-645 | Routes through inferred row for negative arity; otherwise generic replacement and parent repair. |
| `core()` | 650-652 | Base self. |
| `coreMO()` | 657-659 | Base self. |
| `coreIndex()` | 664-666 | Base zero. |
| `childPosition()` | 671-689 | Skips `notParent` ancestors, identity-searches logical parent's children, null if detached/not found. |
| `setTeXclass(prev)` | 694-697 | Stores previous state; returns self if texClass exists, otherwise passes through prev. |
| protected `updateTeXclass(core)` | 703-710 | Copies prior class/level and texClass from core; clears core prior class/level. |
| protected `getPrevClass(prev)` | 716-721 | Reads prior texClass and resolved scriptlevel into this node. |
| `texSpacing()` | 726-741 | Reads TeX class matrix and script levels; suppresses nonnegative spaces for scripts. |
| `hasSpacingAttributes()` | 746-748 | For embellished nodes delegates to `coreMO`; base otherwise false. |
| `setInheritedAttributes(attributes={}, display=false, level=0, prime=false)` | 765-835 | Applies defaults and policy tables; writes inherited attrs, display/script level and prime property; repairs fixed arity; derives linebreak indents; invokes child propagation. |
| protected `setChildInheritedAttributes(...)` | 846-855 | Calls each child's virtual propagation method. |
| protected `addInheritedAttributes(current, attributes)` | 866-881 | Copies origin-tagged attributes; adds eligible values tagged with this kind. |
| `inheritAttributesFrom(node)` | 889-898 | Reads display/script/mathsize/prime from template and invokes this node's propagation. |
| `verifyTree(options=null)` | 906-927 | Null no-op; otherwise attributes, optional arity/error repair, then children. |
| protected `verifyAttributes(options)` | 934-966 | Optionally checks explicit names against defaults/allowlist and mathvariant against static set unless ignored; may call mError. |
| protected `verifyChildren(options)` | 973-977 | Recursively calls child `verifyTree`. |
| `mError(message, options, short=false)` | 987-1012 | Suppresses nested merror; factory-creates error/text nodes; writes message attrs/text; replaces in actual parent and may nest original node. |

### `AbstractMmlTokenNode` implemented members

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| static `defaults` | 1024-1028 | New spread object from AbstractMmlNode defaults; sets mathvariant=normal, mathsize=`INHERIT`. |
| `get isToken()` | 1033-1035 | True. |
| `get isEmpty()` | 1040-1047 | False for non-TextNode children or nonempty TextNode content. |
| `getText()` | 1055-1065 | Concatenates TextNode text and child `textContent()` where available. |
| protected `setChildInheritedAttributes(...)` | 1072-1083 | Only propagates to AbstractMmlNode children, excluding TextNode. |
| `walkTree(func, data)` | 1090-1098 | Visits self then only AbstractMmlNode children; returns data. |

### `AbstractMmlLayoutNode` implemented members

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| static `defaults` | 1113 | Aliases AbstractMmlNode defaults (same object). |
| `get isSpacelike()` | 1118-1120 | First child value; assumes a child exists. |
| `get isEmbellished()` | 1125-1127 | First child value; assumes a child exists. |
| `get arity()` | 1132-1134 | -1, requiring an inferred mrow. |
| `core()` | 1139-1141 | First child. |
| `coreMO()` | 1146-1148 | Delegates to first child's coreMO. |
| `setTeXclass(prev)` | 1153-1157 | Delegates to first child, updates self from it, returns delegated result. |

### `AbstractMmlBaseNode` implemented members

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| static `defaults` | 1173 | Aliases AbstractMmlNode defaults. |
| `get isEmbellished()` | 1178-1180 | First child value. |
| `core()` | 1185-1187 | First child. |
| `coreMO()` | 1192-1194 | Delegates to first child's coreMO. |
| `setTeXclass(prev)` | 1199-1221 | Starts own class ORD; delegates embellished/mi base and copies core class, otherwise processes base separately/adopts TeXAtom class; processes script children separately; returns base result or self. |

### `AbstractMmlEmptyNode` implemented members

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| `parent` field | 1240 | MmlNode-typed actual parent slot; inherited empty child methods do not maintain it. |
| `childNodes` field | 1245 | MmlNode child array, but inherited empty methods prevent attaching children. |
| `get isToken()` | 1250-1252 | False. |
| `get isEmpty()` | 1257-1259 | True. |
| `get isEmbellished()` | 1264-1266 | False. |
| `get isSpacelike()` | 1271-1273 | False. |
| `get linebreakContainer()` | 1278-1280 | False. |
| `get linebreakAlign()` | 1285-1287 | Empty string. |
| `get arity()` | 1292-1294 | Zero. |
| `get isInferred()` | 1299-1301 | False. |
| `get notParent()` | 1306-1308 | False. |
| `get Parent()` | 1313-1315 | Returns actual parent without skipping. |
| `get texClass()` | 1320-1322 | TEXCLASS.NONE. |
| `get prevClass()` | 1327-1329 | TEXCLASS.NONE. |
| `get prevLevel()` | 1334-1336 | Zero. |
| `hasSpacingAttributes()` | 1341-1343 | False. |
| `get attributes()` | 1348-1350 | Null; no Attributes object. |
| `core()` | 1355-1357 | Self. |
| `coreMO()` | 1362-1364 | Self. |
| `coreIndex()` | 1369-1371 | Zero. |
| `childPosition()` | 1376-1378 | Zero, regardless of parent membership. |
| `setTeXclass(prev)` | 1383-1385 | Returns prev unchanged. |
| `texSpacing()` | 1389-1391 | Empty string. |
| `setInheritedAttributes(...)` | 1398-1403 | No-op. |
| `inheritAttributesFrom(node)` | 1410 | No-op. |
| `verifyTree(options)` | 1417 | No-op. |
| `mError(...)` | 1422-1428 | Null; no error creation or replacement. |

### `TextNode` implemented members

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| protected `text` field | 1440 | Owned UTF-16 JavaScript string, initially empty. |
| `get kind()` | 1445-1447 | `text`. |
| `getText()` | 1452-1454 | Returns stored text. |
| `setText(text)` | 1460-1463 | Replaces text and returns self. |
| `copy()` | 1468-1470 | Creates registered `text` node via factory and sets copied content. |
| `toString()` | 1477-1479 | Returns text verbatim. |

### `XMLNode` implemented members

| Member | Lines | State/dependency changed or read |
| --- | --- | --- |
| protected `xml` field | 1491 | Opaque object reference, initially null. |
| protected `adaptor` field | 1496 | DOMAdaptor host reference, initially null. |
| `get kind()` | 1501-1503 | `XML`. |
| `getXML()` | 1508-1510 | Returns stored XML object identity. |
| `setXML(xml, adaptor=null)` | 1517-1524 | Stores object/adaptor handles and returns self. |
| `getSerializedXML()` | 1529-1531 | Calls adaptor serialization on stored XML; no null-adaptor guard. |
| `copy()` | 1536-1540 | Calls adaptor clone and creates registered XML node; source passes only cloned XML to setXML, so the copy's adaptor remains null. |
| `toString()` | 1547-1549 | Fixed `XML data`, not serialized XML. |

## Exhaustive Node, factory and wrapper member ledgers

Root's read-only TypeScript AST inventory reports 23 executable methods,
accessors and constructors in `Node.ts`, one in `NodeFactory.ts`, four in
`Wrapper.ts`, one in `WrapperFactory.ts`, and 85 in `MmlNode.ts`. The MmlNode
implementation tables above map all 85; the following tables map the other
files' executable entries and fields individually. These are coverage ledgers,
not acceptance decisions.

### `AbstractNode` and `AbstractEmptyNode` implementations in Node.ts

| Owner member | Lines | State/dependency changed or read |
| --- | --- | --- |
| `AbstractNode.parent` field | 166 | Actual parent pointer, initially null. |
| `AbstractNode.properties` field | 171 | Own mutable property object. |
| `AbstractNode.childNodes` field | 176 | Ordered child vector, initially empty. |
| `AbstractNode.` `constructor(factory, properties={}, children=[])` | 186-197 | Stores factory; assigns each own input property; attaches initial children. |
| `AbstractNode.get kind()` | 202-204 | Returns `unknown`; concrete classes override. |
| `AbstractNode.setProperty(name, value)` | 209-211 | Writes own property. |
| `AbstractNode.getProperty(name)` | 216-218 | Reads property by JS property lookup (including prototype behavior). |
| `AbstractNode.getPropertyNames()` | 223-225 | Returns own Object.keys ordering. |
| `AbstractNode.getAllProperties()` | 230-232 | Returns live property object. |
| `AbstractNode.removeProperty(...names)` | 237-241 | Deletes each own property. |
| `AbstractNode.isKind(kind)` | 246-248 | Delegates to factory constructor-identity test. |
| `AbstractNode.setChildren(children)` | 253-258 | Clears children then appends each new child. |
| `AbstractNode.appendChild(child)` | 263-267 | Appends by identity, sets `child.parent=this`, returns child; does not detach old parent. |
| `AbstractNode.replaceChild(newChild, oldChild)` | 272-283 | Identity lookup; swaps if found, sets new parent, clears old parent only if still pointing here; always returns new child. |
| `AbstractNode.removeChild(child)` | 288-295 | If found, splices and nulls parent; always returns requested child. |
| `AbstractNode.childIndex(node)` | 300-303 | Identity index; null when absent. |
| `AbstractNode.copy()` | 308-317 | Factory-creates same kind, shallow-copies properties, recursively copies non-null children; copied root has no parent. |
| `AbstractNode.findNodes(kind)` | 322-330 | Preorder traversal including self; uses each node's `isKind`. |
| `AbstractNode.walkTree(func, data)` | 335-343 | Preorder callback; skips null children; returns unchanged data. |
| `AbstractNode.toString()` | 350-352 | Structural string of kind and joined children. |
| `AbstractEmptyNode.setChildren(_children)` | 373 | No-op. |
| `AbstractEmptyNode.appendChild(child)` | 378-380 | No-op returning input child. |
| `AbstractEmptyNode.replaceChild(_newChild, oldChild)` | 385-387 | No-op returning old child. |
| `AbstractEmptyNode.childIndex(_node)` | 392-394 | Always null. |
| `AbstractEmptyNode.walkTree(func, data)` | 401-404 | Calls callback only on self; returns data. |
| `AbstractEmptyNode.toString()` | 411-413 | Returns kind only. |

The 23 executable AST entries are the methods/accessors/constructor in this
table; the three fields are additional AST members.

### `AbstractNodeFactory` implementation in NodeFactory.ts

| Owner member | Lines | State/dependency changed or read |
| --- | --- | --- |
| `AbstractNodeFactory.create(kind, properties={}, children=[])` | 61-67 | Dispatches through `this.node[kind]` with properties/children; this override directly indexes the creator map and has no fallback. |

This is the file's one executable AST member. `NodeFactory.create` is an
interface declaration at `NodeFactory.ts:34-44`; `AbstractNodeFactory` extends
`AbstractFactory` at `54-57`.

### `AbstractWrapper` implementation in Wrapper.ts

| Owner member | Lines | State/dependency changed or read |
| --- | --- | --- |
| `AbstractWrapper.node` field | 115 | Reference to source semantic node. |
| `AbstractWrapper.childNodes` field | 120 | Wrapper-child array, declared without initializer here. |
| `AbstractWrapper.factory` field | 125 | Protected WrapperFactory reference. |
| `AbstractWrapper.get kind()` | 130-132 | Forwards `node.kind`. |
| `AbstractWrapper.` `constructor(factory, node)` | 141-147 | Stores factory and node references. |
| `AbstractWrapper.wrap(node)` | 152-154 | Delegates to its factory; generic return type is a cast. |
| `AbstractWrapper.walkTree(func, data)` | 159-169 | Visits self; if a childNodes property exists, traverses it preorder skipping nulls; returns data. |

The four executable AST methods are getter `kind`, constructor, `wrap`, and
`walkTree`; the three fields are additional AST members.

### `AbstractWrapperFactory` implementation in WrapperFactory.ts

| Owner member | Lines | State/dependency changed or read |
| --- | --- | --- |
| `AbstractWrapperFactory.wrap(node, ...args)` | 74-76 | Calls generic create with `node.kind`, node and forwarded extension arguments. |

This is the file's one executable AST member. `WrapperFactory.wrap` and the
wrapper constructor contracts are declarations at `WrapperFactory.ts:37-50`
and `Wrapper.ts:81-97`.

## Interface declaration inventory (declarations only)

These TypeScript interface members are contracts, not executable AST members.
Names are listed explicitly for comparison with the implementation tables.

| Interface | Members | Lines |
| --- | --- | --- |
| `Node<N,C>` | readonly `kind`, readonly `factory`, `parent`, `childNodes`; `setProperty`, `getProperty`, `getPropertyNames`, `getAllProperties`, `removeProperty`, `isKind`, `setChildren`, `appendChild`, `replaceChild`, `removeChild`, `childIndex`, `copy`, `findNodes`, `walkTree` | Node.ts:41-128 |
| `NodeClass<N,C>` | constructor `(factory, properties?, children?)` | Node.ts:138-150 |
| `NodeFactory<N,C>` | extends `Factory`; `create(kind, properties?, children?)` | NodeFactory.ts:34-45 |
| `Wrapper<N,C,W>` | readonly `kind`, `node`, `childNodes`; `wrap(node,...args)`, `walkTree(func,data?)` | Wrapper.ts:37-70 |
| `WrapperClass<N,C,W>` | constructor `(factory,node,...args)` | Wrapper.ts:81-97 |
| `WrapperFactory<N,C,WW,WC>` | extends `Factory`; `wrap(node,...args)` | WrapperFactory.ts:37-50 |
| `MmlNode` | Individually listed in the interface-only ledger above | MmlNode.ts:138-276 |
| `MmlNodeClass` | Optional static `defaults`; constructor `(MmlFactory,attributes?,children?)` | MmlNode.ts:283-304 |
