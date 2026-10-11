# P15d root contracts — HtmlNode and typed host calls

Root read the complete pinned HtmlNode.ts and XMLNode/token methods, plus the
read-only p15d-html-host-audit.md. Source SHA is
1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665.
This packet ports seven HtmlNode members; it does not implement a DOM parser,
texhtml package, HTML measurement, foreignObject output or serialization policy.
Those remain required separate dependencies, including the configurable enabled
texhtml path. Disabled defaults cannot justify dropping them.

## Architecture and host adaptation

HtmlNode remains an XMLNode semantic subtype. html_adaptor is a platform-free
typed host interface extending the existing XmlAdaptor with five required
operations. It stores no second tree or XML state. XMLNode's xml_/adaptor_ remain
the authoritative handles. An XML-only adaptor is not silently substituted.
Opaque shared XmlRef handles preserve object identity and lifetime; actual DOM
construction/parse/measurement belongs to separately reviewed host services.

The existing optional TextContentBinding models the source prototype's method
presence for token.getText. Root's native constructor installs a callback that
invokes this receiver's virtual textContent. It is a method binding, not stored
text or a cache; token extraction must see later payload/adaptor changes and
derived overrides. Source strings used as token text remain UTF-16, including
lone surrogates. Serialized markup and kind labels use the existing lossless
WTF-8 internal host strings; strict external UTF-8 encoding is a separate
boundary. No Windows headers enter this module.

The typed API does not expose arbitrary JS erased objects, own accessor
replacement or non-callable DOM properties. Callbacks expose actual host calls,
arguments, return identities, mutations and exceptions. getAttribute returns a
raw Property because its result is ignored by HtmlNode. node takes the exact
span label, fresh empty attribute record and one-element original payload list.
It may return null; do not reject or normalize that result.
The typed children argument is a fresh vector value carrying the same XmlRef
handles; the API does not expose JavaScript array-object reflection/identity.
Attribute record identity and each payload identity remain observable.

setHTML returns XMLNode* to retain the actual virtual setXML result, including
a different XML receiver. A runtime cast demanding HtmlNode would add a source
failure. XML copy remains inherited: factory creation precedes clone, then
setXML(clone) is called without adaptor. Do not repair the lost adaptor upstream.

## Exact method obligations

- kind returns html. getHTML calls virtual getXML, without direct storage read.
- setHTML tries only getAttribute(html, data-mjx-hdw), ignoring its return.
  Catch every exception from that attempt, including null/unsuitable adaptor.
  The catch invokes node(span, fresh empty attributes, [original html]); a node
  exception escapes. Then call virtual setXML(selected html, original adaptor)
  outside the try and return its actual result. setXML exceptions must escape
  without wrapping. No truthiness test, duplicate attribute call or null guard
  may change which branch executes.
- getSerializedHTML invokes current adaptor.outerHTML(current xml).
- textContent invokes current adaptor.textContent(current xml).
- toString invokes adaptor.kind(current xml) exactly once, then returns
  HTML=<kind>...</kind>. Do not invoke semantic node.kind instead.
- verifyTree checks the direct parent and its virtual isToken. Only a present
  non-token parent calls virtual mError with the exact source message, original
  nullable options handle and shortError=true, then returns. No base verifier,
  adaptor call, traversal or normalization is added. Inherited empty mError is
  genuinely a no-op; do not invent an error tree.

Use existing host::typeError for native null/unsuitable-adaptor method failure.
Do not add TreeCallGuard to these nonrecursive local methods; callbacks and
inherited recursive paths own their existing resource checks.

## Ownership and acceptance

Root owns interfaces, constructor bridge, generated kind accessor, CMake,
source inventory, actual-CJS oracle, expected fixtures, mutation checks and
acceptance. One Luna agent may implement only html_node.cpp's six nongenerated
methods and docs/math-port/evidence/p15d-html-node-methods.md after root freeze.
No other writes, tests, builds, commits, delegation, fallback or scope changes.

Root acceptance requires exact source getters, default XML state, object/adaptor
and virtual return identity, attribute return-vs-throw branches, catch boundaries,
fresh node arguments, null payload/return/adaptor and XML-only adaptor failures,
host/setXML/getXML callbacks and throws, all direct parent branches/options
identity, real token text extraction with UTF-16 and live rebinding, inherited
copy ordering/adaptor loss, focused/full core build and compiling assertion-failing
mutants. Agent completion is not acceptance.
