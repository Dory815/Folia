# Root frozen HtmlNode observer binding repair

The packet may change only native C++ emitter strings in
`tools/math-port/html-node-oracle.cjs`, regenerate
`tests/math/reference_html_node.cpp`, and write
`docs/math-port/evidence/p23m-html-observer-implementation.md`.
No production code, other observers, build wiring, pins, locks, Source stimuli,
actual JS overrides, helper categories, expected values, assertion counts or
case names may change. No builds, native tests, commits or delegation. The
agent may run its original oracle solely to generate its own fixture. Root
owns review, original profile reproduction, compile and runtime acceptance.

Root froze 434 assertions, 75 cases, all 52 original expected-value declarations
and 570 ordered profile records before delegation. Profile SHA-256:
`c61f83c6717ec204dff4514315d0c7657a5eb1cc09b086ad4e611c8d743380b9`.
The profile includes original case names, equality/property helper outputs,
each caught result's error/type/primitive value, and the expected-value table.
Object results are covered by original identity/state assertions, not recursive
serialization of mutable nodes. Direct UTF-16 and copy assertions require
unchanged-diff review too. Source HtmlNode.ts SHA-256 remains
`1bf0ba981fc72b63a525424fb51e8bc45231fcf768d618fb8c75517bd72ae665`;
the existing reference loader guards clone/package equality and its dependency
closure. No native output supplies these observations.

Root found a pre-existing generator/fixture drift: the generator emitted
`a.keys().size()` while the committed fixture already used `h::ownKeys(a).size()`
for JS Object.keys. Root aligned that one native emitter expression with the
committed fixture and reproduced the same frozen profile before delegation.
The profile recorder must call `p(expected)` exactly once per original prop
helper; the first recorder draft was rejected because duplicate calls changed
the identity-aware declaration table. That rejected draft produced no profile.

The current core build's genuine observer compile failures include the dense
children factory and nine raw parent writes in this fixture, recorded in
`build/p23l-full-core-check-repaired.log` (SHA-256
`d5adddaa4d3884a1bdba556a1cb981734ce1fa2515257f6c9301517a957d183c`).
This is binding failure evidence, not an algorithm red.

Repair the children factory to produce `tree::ChildArray<MmlNode>` with tagged
NodeValue entries; replace only native raw parent writes with `setParent` and
explicit tagged arguments. Source JS parent writes stay untouched. Preserve
virtual callbacks, captured/current host behavior, ErrorOptions identity,
partial effects and omitted/null/undefined inputs. XmlAdaptor child vectors,
XmlRef payloads, XMLNode* setXML and mError return services are independent of
MML child arrays and must retain their existing interfaces.

Acceptance requires original generator/profile --check reproduction, review
of the native-only diff, independent full-graph compilation, then the repository
core gate. Do not weaken or skip any assertion. Runtime mismatches in still
unported algorithms remain visible and require a separate implementation
packet; this repair adds no whole-file acceptance and no Folia wiring.
