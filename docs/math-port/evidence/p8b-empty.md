# P8b Empty/Text/XML branch implementation evidence

Implementation packet for the root-frozen P8b MML hierarchy contract. Source
is MathJax 4.1.3 at commit `fb987178c2d279a99b0db5ea02e751691703955e`.
This packet implements only `AbstractMmlEmptyNode`, `TextNode`, and `XMLNode`
members declared in `src/math/core/mml_tree/mml_node.ixx`; root owns review,
build, fixtures, and acceptance.

## Empty-node method ledger

`AbstractMmlEmptyNode` uses the single generic node storage inherited from
`tree::AbstractEmptyNode<MmlNode,MmlNode>`: `factory`, `parent`, the shared
`childNodes` handle, and generic properties are not re-declared here. This
matches the pinned source's empty branch inheriting the generic Node fields.
The class's own source fields are typed views of parent/children and do not
create separate storage (`MmlNode.ts:1233-1246`; pinned CJS
`MmlNode.js:875-879`, `Tree/Node.js:42-65`).

| Member | Native behavior | Pinned source |
|---|---|---|
| `isToken`, `isEmbellished`, `isSpacelike`, `linebreakContainer` | `false` | `MmlNode.ts:1250-1280` |
| `linebreakAlign` | empty string | `:1282-1287` |
| `isEmpty`, `arity`, `isInferred`, `notParent` | `true`, `0`, `false`, `false` | `:1254-1307` |
| `Parent` | returns the shared `parent` field | `:1310-1315` |
| `texClass`, `prevClass` | `TEXCLASS.NONE` (-1) | `:1317-1329` |
| `prevLevel` | numeric zero Property | `:1331-1336` |
| `attributes` | null shared pointer | `:1345-1350` |
| setters for class, previous class/level, and attributes | throw host `TypeError`, matching read-only JS getters when assigned through native property adapters | declarations in root contract; getter source above |
| `core`, `coreMO`, `coreIndex`, `childPosition` | this, this, zero, optional zero | `:1352-1378` |
| `setTeXclass`, `texSpacing`, `hasSpacingAttributes` | return input previous node, empty string, false | `:1380-1391,1338-1343` |
| `setInheritedAttributes`, `inheritAttributesFrom`, `verifyTree` | no-op | `:1393-1417` |
| `mError` | null node pointer | `:1419-1428` |
| `setChildren`, `appendChild`, `replaceChild`, `childIndex`, `walkTree`, `toString` | inherited generic empty-node implementations; no duplicate override or storage added | `core/Tree/Node.ts:362-404` |

## TextNode

The `text_` member is `std::u16string`, so set/get retain all UTF-16 code
units, including isolated surrogates. `getText` returns the field by value;
`setText` replaces it and returns this node, matching the source's chainable
setter (`MmlNode.ts:1436-1463`).

`copy` creates through the current factory registration for `kind()` first,
then calls the source's virtual `getText`, then the result's virtual
`setText`. It intentionally does not copy generic properties, `id`, parent,
children, or a `keepIds` request (`:1465-1470`). The copy implementation
retains this source evaluation order and uses the shared `TreeCallGuard` for
bounded native re-entry. A result without the TextNode binding raises
`TypeError` instead of receiving an invented fallback.

JavaScript `toString` returns the raw UTF-16 text. Native `std::string`
therefore uses root's lossless `host::debugText` WTF-8 bridge so a lone
surrogate remains representable. This deliberately does not call external
`utf16::encode`, which applies TextEncoder's U+FFFD replacement and would lose
the original code unit (`:1472-1479`; host bridge contract in
`mml_node_host.ixx`).

## XMLNode

The `xml_` member is the opaque `XmlRef`; `adaptor_` is a borrowed pointer to
the root-supplied `XmlAdaptor`. `getXML` returns the same shared object
identity; `setXML` stores the supplied handle and adaptor and returns this
node (`MmlNode.ts:1487-1524`). Null XML and null adaptor remain distinct
stored states. `getSerializedXML` dispatches to `adaptor_->serializeXML(xml_)`;
a null adaptor maps to host `TypeError`, corresponding to calling the
source's method through null (`:1526-1531`). The caller must keep the borrowed
native adaptor alive for any later operation; this is an explicit native
ownership adaptation in the root contract.

`copy` creates the destination from the current factory registration first,
then invokes the source adaptor's `clone` on the current XML handle, then
calls the new node's virtual `setXML` with only the clone. The omitted second
argument resets the copy's adaptor to null exactly as the source's default
parameter does (`:1533-1540`). Therefore the source node remains serializable,
while the copy's `getSerializedXML` throws `TypeError`; the implementation
does not repair that source behavior. A null source adaptor throws after the
destination factory call. The destination type check is performed after
clone, retaining source call/argument ordering; a wrong registered creator
does not get a fake XML adapter. The operation uses `TreeCallGuard` like the
generic recursive copy boundary.

`toString` returns the literal `XML data` without invoking the adaptor
(`:1542-1549`). No DOM parsing, serialization fallback, or HTML-node behavior
is added in this packet.

## Frozen witness coverage and remaining limits

The frozen `reference_mml_node.cpp` suite is read-only. Its empty/Text/XML
case checks empty-node fixed values, null attributes, `childPosition`,
read-only setter TypeErrors, TextNode UTF-16 retention, current-registry
virtual copy behavior, ignored `keepIds`, and no property copying. The XML
case checks source XML identity, a distinct cloned handle, clone-call order,
source serialization, copy adaptor omission and its later TypeError, literal
`XML data`, and null-adaptor copy failure (`math_port_mml_node_empty_text_readonly_virtual_copy_and_utf16`,
`math_port_mml_node_xml_adaptor_clone_order_and_omission`).

This packet does not implement HtmlNode, TextContentBinding dispatch, token
text aggregation, concrete MML node classes, parser, output wrappers, or
external DOM adaptors. The root oracle fixture covers its registered virtual
TextNode subclass and a test XML adaptor, not arbitrary foreign JS objects.
No build or test was run by this packet; root owns independent compilation,
oracle checks, mutation checks, and acceptance.

## Source identity

| Pinned file | SHA-256 |
|---|---|
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `cjs/core/MmlTree/MmlNode.js` | `86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45` |
| `ts/core/Tree/Node.ts` | `532000186fc65db8f227b7e69f1ed331ebdd58bca2d1984aa099d81f3cbcd1d0` |
