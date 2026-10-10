# E11 MML host usage audit

Read-only audit of the pinned MathJax 4.1.3 tree at commit
`fb987178c2d279a99b0db5ea02e751691703955e`, using the enabled raw-TeX
profile in `tools/mathjax-bundle/entry.cjs` and the installed pinned CJS build
under `tools/mathjax-bundle/node_modules/@mathjax/src/cjs`. This records
source behavior that constrains an MML host; it does not choose native defaults
or define an interface. The profile enables the TeX packages listed in
`scope.json` and SVG output with linebreaking. `texhtml` is present, but its
`allowTexHTML` option defaults to false.

## MML state is nullable and some fields are broader at runtime than their types

`AbstractMmlNode` initializes `prevClass`, `prevLevel`, and its internal
`texclass` to `null`; the `texClass` getter and setter expose that nullable
state (`core/MmlTree/MmlNode.ts:400-408,431-433,497-506`). The public TS
declarations say `number`, but the implementation deliberately uses null as
the unset sentinel. The empty-node branch differs: `texClass` and `prevClass`
are `TEXCLASS.NONE` (-1), `prevLevel` is 0, and `attributes` is null
(`MmlNode.ts:1320-1350`). `MmlMo` has another variant: `_texClass` starts
null, then its getter computes from the operator dictionary and text without
caching when still unset (`MmlNodes/mo.ts:177-201`). `MmlRow` and other
classes may assign a concrete class later. Thus “unassigned”, “MathML empty
node”, and operator-derived class are distinct states.

The `prevLevel` field is not reliably a number at runtime. `getPrevClass`
assigns `prev.attributes.get('scriptlevel')` directly, with only a TS cast
(`MmlNode.ts:716-720`); `MmlMo.adjustTeXclass` similarly assigns
`attributes.getInherited('scriptlevel')` (`MmlNodes/mo.ts:348-363`). These
lookups can also yield `undefined` when the key is absent. `texSpacing` then
uses the JavaScript relational comparison `prevLevel > 0` and also compares
the current scriptlevel (`MmlNode.ts:726-740`), which applies JS coercion to
string, boolean, numeric, null, or undefined values. Do not narrow the stored
value based on the `as number` annotations. The actual scope's normal
inherited script levels are usually numeric, but the public mutation surface
permits other Property values; this audit does not prove all such values
arise from raw TeX.

The MML defaults and policy tables are mutable objects, not immutable schema
constants. `AbstractMmlNode.defaults`, `noInherit`, `stopInherit`,
`alwaysInherit`, and `verifyDefaults` are public static values
(`MmlNode.ts:319-393`); `MATHVARIANTS` is an exported mutable `Set`
(`MmlNode.ts:97-116`), and concrete node classes add their own static
`defaults`. Construction reads the currently registered class's `.defaults`
and the currently registered math class's `.defaults`, then `Attributes`
builds prototype-linked attribute layers (`MmlNode.ts:443-459`;
`Attributes.ts:42-53`). Inheritance consults the live static policy objects
for each call (`MmlNode.ts:765-785`). `MmlFactory.defaultNodes` is also a
mutable static registry reference initialized to `MML`
(`MmlFactory.ts:32-38`; `MML.ts:132-135`). This is identity-bearing state:
the source does not freeze these values before constructing the tree.

`Attributes` stores the original `global` object reference, while it creates
new `defaults`, `inherited`, and explicit-attribute objects with
`Object.create`; its `defaults` is populated by `Object.assign` from the
class defaults (`Attributes.ts:42-53`). The original class-default object is
therefore read at node construction time, whereas the global defaults remain
the prototype parent used by later lookup. It would be inaccurate to replace
all static/default objects with one copied flat map if identity and later
mutation are in scope.

## Text, XML, and HTML nodes have different runtime shape

The default `MML` map registers `TextNode`, `XMLNode`, and `HtmlNode` as
separate constructors (`core/MmlTree/MML.ts:132-135`); `HtmlNode` extends
`XMLNode` and overrides `kind` to `html` (`MmlNodes/HtmlNode.ts:34-40`).
TeX's ordinary path creates actual `TextNode` objects: `NodeFactory.createToken`
calls `factory.create('text', text)` and `createText` calls the MML factory's
`text` registration then `setText` (`input/tex/NodeFactory.ts:96-118`).
Other active TeX methods also create text nodes, for example `BaseItems.ts`
and `ParseUtil.ts`. These are used as children of token nodes, not as
ordinary MML element kinds.

Text strings are JavaScript strings, so indexed length, `.length`, slicing,
and regex behavior in these sources count UTF-16 code units. `TextNode`
stores and returns the string unchanged (`MmlNode.ts:1436-1479`). Consumers
use differing string operations: `AbstractMmlTokenNode.isEmpty` checks
`child.getText().length` (`MmlNode.ts:1040-1047`); SVG/common operator wrappers
use `codePointAt(0)` (`output/common/Wrappers/mo.ts:387`; SVG `mo.ts:211`),
and the common script-base wrapper uses `Array.from(base.getText()).length`
when deciding whether a base is one character
(`output/common/Wrappers/scriptbase.ts:585-593`). A host's UTF-8
code-point count is not a substitute for JS string length in the former.

`AbstractMmlTokenNode.getText` first branches on constructor identity:
`child instanceof TextNode` appends `getText()`. Otherwise it tests whether
the property name exists with `'textContent' in child`, and if it does,
invokes that property as a function (`MmlNode.ts:1055-1064`). This is
property-presence semantics followed by a call, not a kind-name test or a
structural fallback. `HtmlNode.textContent()` delegates to
`adaptor.textContent(xml)` (`MmlNodes/HtmlNode.ts:77-81`), so HTML node text is
observable through this exact branch when an HTML node is present. XMLNode has
no `textContent` method. A null child reaches the `in` operator after failing
the `instanceof` test and throws; `getText` has no null guard.

Other MML paths use class identity directly: token emptiness tests
`instanceof TextNode`; token child inheritance and token `walkTree` test
`instanceof AbstractMmlNode` and deliberately skip text nodes
(`MmlNode.ts:1040-1047,1072-1098`). TeX code also uses `instanceof`
`AbstractMmlNode`/`AbstractMmlEmptyNode` (`input/tex/NodeUtil.ts:285`),
`TexParser.ts:230`, and `textmacros/TextParser.ts:159`. `kind` is used for
factory dispatch and other predicates, but substituting `kind == 'text'` or
`kind != ...` for these constructor identity tests is not evidence-equivalent
when a registered constructor is replaced or extended.

An empty MML node exposes `attributes === null` and has no ordinary MML
attribute object (`MmlNode.ts:1226-1230,1345-1350`). Its `childNodes` field is
initialized by the generic `AbstractNode` constructor to an empty array
before the virtual `setChildren` call (`core/Tree/Node.js:42-65`); the
`AbstractEmptyNode` override then ignores child assignment (`Node.ts:362-380`).
A fresh pinned-factory probe confirmed the actual state: `text` has
`childNodes: []`, the property is present, `attributes: null`, and
`parent: null`; `XML` has the same child/attribute/parent state with
`xml: null` and `adaptor: null`; `html` likewise has `childNodes: []`,
`attributes: null`, and `parent: null`. The probe used
`new MmlFactory()` from the installed pinned CJS package and called
`factory.create('text')`, `factory.create('XML')`, and
`factory.create('html')`. In normal TeX token construction, `TextNode` is
stored under a token's initialized children and callers use `getText`, but
its own empty child array is still present and observable.

## XML adaptor calls and copy behavior

`XMLNode.setXML(xml, adaptor = null)` retains both values (`MmlNode.ts:1517-1524`).
`getSerializedXML` calls `this.adaptor.serializeXML(this.xml)` without a null
check (`:1527-1531`). `XMLNode.copy` calls `this.adaptor.clone(this.xml)`,
then calls `setXML(clone)` without passing the adaptor (`:1533-1539`). For a
valid source XML node, copying therefore requires a usable source adaptor for
the clone; the result stores the clone but has `adaptor === null`. Subsequent
serialization on that copy fails. `HtmlNode` inherits this XML copy method;
its own `setHTML` performs `adaptor.getAttribute`, may wrap with
`adaptor.node('span',...)` on exception, and then stores the adaptor through
`setXML` (`MmlNodes/HtmlNode.ts:54-66`). Its serialization and `toString`
methods call adaptor APIs without null checks (`:69-90`).

For the enabled raw-TeX profile, `texhtml`'s handler returns false unless
`parser.options.allowTexHTML` is true (`input/tex/texhtml/TexHtmlConfiguration.ts:44-52`),
and its package option defaults to false (`:92-99`). With the default profile,
this is not an active positive HTML construction path. The handler's enabled
path obtains an adaptor, parses serialized markup, creates an `html` node,
calls `setHTML`, and places it under `mtext` (`:69-86`). The default MML
registry and common/SVG consumers still contain XML/HTML support, but this
does not prove XML/HTML node creation from enabled raw TeX. No enabled TeX
caller in the inspected profile directly constructs an `XMLNode`.

## Constructors distinguish omitted arguments from explicit null

JavaScript default parameters apply only to `undefined`, not `null`.
`AbstractMmlNode(factory, attributes = {}, children = [])` forwards no nullable
normalization (`MmlNode.ts:443-459`). Passing `null` as `attributes` reaches
`Attributes.setList(null)`, whose body is `Object.assign(this.attributes,
list)`; the JS built-in ignores a null source, so this behaves like an empty
explicit list (`Attributes.ts:67-71`). Passing `null` children on an ordinary
fixed-arity MML node reaches `setChildren`, then `for...of children`, and
throws (`MmlNode.ts:595-600`; generic `Node.ts:253-258`). An inferred-row
node first routes it to its inferred row, where the same null iteration
failure occurs. Omitted children use `[]`; explicit null does not.

The empty-node constructor path has a different argument boundary. A normal
`factory.create('text')` supplies default `{}` properties and `[]` children;
the generic `AbstractNode` constructor initializes `childNodes` before it
enumerates properties and checks child length (`core/Tree/Node.ts:186-197`).
Explicit null `properties` fails at `Object.keys(properties)` and explicit
null `children` fails at `children.length` on this generic path. These are
not MML `attributes` arguments: TextNode/XMLNode inherit `AbstractEmptyNode`
and expose `attributes === null` as a getter. By contrast, the ordinary
AbstractMmlNode constructor calls `super(factory)` without forwarding its
attributes or children to `AbstractNode`, then handles those parameters in
its own body as described above.

The CJS lowering confirms class-field sequencing relevant to virtual calls.
`AbstractMmlNode` calls `super(factory)` first; after that returns, generated
assignments set `_this.prevClass = null`, `prevLevel = null`, and
`texclass = null`, before its own inferred-row/children/attribute setup
(`cjs/core/MmlTree/MmlNode.js:133-146`). Generic `AbstractNode` invokes
`this.setProperty` for properties and calls `this.setChildren` only for a
nonempty initial child list (`core/Tree/Node.ts:186-197`); these are dynamic
calls during base construction. MML's own constructor then calls dynamic
`arity`, `kind`, `setChildren`, and factory registration lookups
(`MmlNode.ts:443-459`). Derived field initializers run after `super` returns
under the emitted CJS class-construction sequence, so later initializers may
overwrite state assigned during the base call. Any native port must respect
the observed ordering rather than assume all most-derived fields are already
initialized during base construction.

## Scoped callers that consume these distinctions

| Caller | Concrete dependency |
|---|---|
| TeX `NodeFactory` | Creates TextNode by factory registration and calls `setText`; its `createText` explicitly returns null for a null text argument (`input/tex/NodeFactory.ts:96-118`). |
| TeX `NodeUtil`, `TexParser`, text macros | Use `instanceof` MML base classes for argument/node behavior, not only `kind` (`input/tex/NodeUtil.ts:285`; `TexParser.ts:230`; `textmacros/TextParser.ts:159`). |
| `MmlMi` | Calls `getText`, uses token content to set inherited mathvariant and may assign `texClass` (`MmlNodes/mi.ts:74-102`). |
| `MmlMo` | Reads/writes `texClass`, reads raw scriptlevel values for previous-level spacing, and derives class from the operator dictionary (`MmlNodes/mo.ts:177-201,320-339,348-363`). |
| Common `Wrapper` | Its `getText` recognizes `TextNode` by `instanceof`; text wrapper code calls `TextNode.getText` (`output/common/Wrapper.ts:1188-1198`; `output/common/Wrappers/TextNode.ts`). |
| Common/SVG `mo` wrappers | Call `getText`, `codePointAt`, and read `texClass`; TeXAtom wrapper indexes `TEXCLASSNAMES` by `texClass` (`output/common/Wrappers/mo.ts:328-387`; `output/svg/Wrappers/mo.ts:211`; `output/svg/Wrappers/TeXAtom.ts:137`). |
| Common MathML visitors | `JsonMmlVisitor` and serialized/MathML visitors call `getText` and XML serialization (`core/MmlTree/JsonMmlVisitor.ts:73,97`; `SerializedMmlVisitor.ts:52,61`; `MathMLVisitor.ts:59`). |
| HTML support | `HtmlNode.textContent` and XML adaptor methods are available, but raw-TeX positive creation is gated off by the default option (`MmlNodes/HtmlNode.ts:54-80`; `texhtml/TexHtmlConfiguration.ts:44-99`). |

## Interface constraints and unknowns for root review

- Represent unset class values separately from numeric TeX classes, and do not
  collapse the `AbstractMmlEmptyNode` overrides or `MmlMo`'s computed getter
  into the generic nullable case.
- Preserve `prevLevel` as a `Property` until the source's JS `> 0` comparison
  semantics are applied. The `number` cast is erased in the compiled CJS.
- Keep constructor/class identities observable for the scoped `instanceof`
  tests. Preserve the separate `textContent` property-presence check and
  invocation when HTML nodes are admitted.
- Keep mutable static registry/default/policy identity if the native host
  exposes runtime mutation. These values are not frozen by the upstream
  implementation; this audit does not decide which mutation APIs the raw-TeX
  product will expose.
- Preserve null `attributes` and an initialized empty `childNodes` array on
  TextNode/XMLNode/HtmlNode where the current interface admits these types, or
  document a bounded host adaptation. The default TeX profile actively uses
  TextNode; it does not establish active XMLNode/HtmlNode construction.
- Keep XML adaptor operations explicit. `copy` omits the adaptor on its result
  by source behavior; no silently repaired copy behavior is evidenced here.

Unknowns: this audit did not execute arbitrary host mutation of exported
static policy objects, custom class registration, or raw-TeX HTML with
`allowTexHTML` enabled. The profile's tests and enabled-package callers are
not a proof that such host-facing APIs are excluded. It also does not infer
that every Property-typed scriptlevel reaches the raw comparison through the
default parser.

## Source hashes

Pinned TS source hashes below were checked against the checkout and
`docs/math-port/scope.json` where included. CJS line references refer to the
installed `@mathjax/src` 4.1.3 package used by the bundle, not to the TS line
numbers.

| Source | SHA-256 |
|---|---|
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `ts/core/MmlTree/Attributes.ts` | `c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a` |
| `ts/core/MmlTree/MmlFactory.ts` | `5e7338cd3eed5a4002243b7371dc5098c6433ff4ba4345cb2d50cf0385b2abdb` |
| `ts/core/MmlTree/MML.ts` | `c64bcfdf65452559995a92c13db75133b1500ef59efc2be5c9489ceacc4d4964` |
| `ts/core/MmlTree/MmlNodes/mo.ts` | `2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605` |
| `ts/core/MmlTree/MmlNodes/mi.ts` | `3bbddaeac72d2c0f2e5d8a0440c28188f98d1bf1481a0ec6872e103a50f146c2` |
| `ts/core/MmlTree/MmlNodes/HtmlNode.ts` | `1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665` |
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
| `ts/input/tex/NodeFactory.ts` | `bf20e8d4f7ed43450df42d65556ccbceea1469541b214c37544f180aa205c840` |
| `ts/input/tex/textmacros/TextParser.ts` | `538059c8482022c1a4bc6f8e763fdc21993fa6353f1956655ae11c9271788a30` |
| `ts/input/tex/texhtml/TexHtmlConfiguration.ts` | `e31f1539452eba33048d8e7bcf87fcf9690f23be1412799ddf2a6f43beab4a3b` |
| `ts/output/common/Wrapper.ts` | `d1db6112db1cb3546658eb8868b5997a4f69e588632ecbe4e08a71949544a828` |
