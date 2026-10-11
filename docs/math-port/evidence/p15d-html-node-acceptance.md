# P15d root acceptance — HtmlNode semantic methods and host calls

Status: local acceptance of the complete pinned HtmlNode.ts over its typed host
surface. Full renderer and Folia replacement remain unaccepted.

Root froze the architecture, interfaces, constructor bridge, generated accessor,
seven-member AST ledger and actual-CJS oracle in f7d8ce7 before dispatch. A Luna
agent could write only html_node.cpp's six methods and its evidence report.
Root reviewed the complete source and submitted CPP, owns all expectations,
builds, corrections, mutation checks and acceptance. CPP/report: b54e07d;
expanded root observations: 809ed02; compiling mutants: 30ffde6.

HtmlNode remains an XMLNode subtype with one authoritative xml/adaptor state.
HtmlAdaptor adds the five required platform-free host operations. No DOM tree,
parser, serializer policy, geometry or backend implementation is introduced.
The existing TextContentBinding invokes the current receiver's virtual method;
it stores no cached text. UTF-16 text and lossless internal WTF-8 markup/kind
strings preserve isolated surrogates and emoji. Strict external UTF-8 encoding
remains a separate boundary.

The source getHTML invokes virtual getXML. setHTML ignores the attribute return,
catches every exception from only that attempt, wraps using fresh span arguments,
and lets wrapping/setXML failures escape. The actual virtual setXML result is
returned as XMLNode*, including another receiver. Nullable payload/host/returns
remain observable. Serialization/text/kind call the current host and payload.
Verification uses the direct parent's virtual isToken, original nullable options,
exact message and short flag; inherited empty mError remains a no-op. Inherited
copy retains factory-before-clone order and does not forward the adaptor.

The typed host exposes payload and attribute-record identity. Children are a
fresh vector value containing the same XmlRef handles; it does not expose erased
JavaScript array-object reflection/identity or arbitrary dynamic DOM properties.
These are explicit host adaptations in html-node-contracts.md, not simulated
browser features or permission to omit later HTML integrations.

## Source observations and root corrections

The final actual pinned CJS oracle has 434 assertions / 75 cases / 52 source-value
records. It checks complete accessors/default state, ignored false/null/undefined
attribute results, foreign/primitive exceptions, catch boundaries, fresh wrapper
records, exact payload and virtual return identity, host callback mutations,
nullable/missing adaptor, omitted/undefined parameter defaults, direct-parent
verification/options mutation, real token text, copy order/errors/adaptor loss
and a copied receiver's live rebound text callback. Unicode markup/kind/text
round trips retain source code units. All expected data comes from actual CJS;
--check reproduces both bindings and oracle output from pinned sources and
the existing full executable-package lock.

The read-only review initially covered 306 assertions/68 cases and identified
missing copy order, copied callback, fresh argument and omitted-parameter witnesses.
Root added these and explicit payload identities. The review's frozen-snapshot
coverage gaps are dispositioned by this final fixture, not treated as acceptance.

Root corrected generator support helpers, scoped a repeated failure variable,
used a boolean UTF-16 comparison compatible with Boost.UT, and registered the
math class already present in the source factory. Initial build/runtime failures
in p15d-focused.log, p15d-focused2.log and p15d-expanded.log are rejected evidence.
These corrections did not alter source expectations. A wrong-kind mutant first
exposed an unsafe test cast; root added actual-source instanceof comparisons and
guarded subsequent observations. Its crash in p15d-mutations.log is rejected.
The second run's stale mutation anchor is also rejected (p15d-mutations2.log).

## Validation and remaining scope

The final unchanged isolated baseline passes all 434 assertions/75 cases.
All 26 mutants compile and fail assertions with exit 1; no crash or compile
failure counts. They cover getters, ignored attribute call, key/payload/catch,
wrapper kind/result/arguments, setter adaptor/dispatch/return/null acceptance,
serialization/text/kind, verification/options, callback binding and inherited
copy order/adaptor omission. Harness byte checks confirm working sources and
fixture unchanged. Results: build/math-port-html-node-mutations/results.json;
final log: build/p15d-mutations3.log. Full final core build/test passes in
build/p15d-core-final2.log. The earlier core run passed a smaller interim fixture
and is not the final acceptance run.

All seven source members are root-reviewed in p15d-html-members.json; the ledger
checker verifies the complete pinned AST inventory. Local source acceptance is
36/246. The remaining 210 sources include other MML classes/factory, TeX packages,
fonts, Common measurement, shared scene and SVG. Enabled texhtml parsing, a
concrete DOM host and HTML measurement/foreignObject output remain separately
required. None is accepted by this semantic node packet. Full G0–G7 stay closed.
