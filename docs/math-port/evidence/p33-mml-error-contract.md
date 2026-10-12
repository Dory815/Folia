# P33 Root: native MML error construction and spacing dispatch

Root assigns Luna only the bodies of AbstractMmlNode::mError and
AbstractMmlNode::hasSpacingAttributes in src/math/core/mml_tree/mml_node.cpp,
plus evidence/p33-mml-error-implementation.md. No declarations, other bodies,
host services, tests, gold, build settings, Folia, commits or subagents.

The architecture remains the upstream semantic tree: mError constructs semantic
merror/mtext/text nodes using the existing factory and tree operations; it does
not render SVG, repair parser inputs or substitute an unrelated error object.
Pinned original MmlNode.ts SHA-256 is
cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656.
The precise original methods are hasSpacingAttributes at 746-748 and mError at
988-1016. Read them and the current NodeValue/Factory/Property host before work.

mError must preserve all original ordering and virtual boundaries: short-circuit
on the current parent being merror before any allocation/options access; then
create merror and set data-mjx-message before reading options.fullErrors. There
is no options default. Missing/Null options must throw their respective exact
fullErrors access errors after allocation and attribute mutation. Re-read raw
fullErrors at each Source occurrence; callbacks can mutate retained options.
The full/short path creates mtext then text, assigns message or this.kind,
appends text then mtext, re-reads the current parent and replaces this, and only
then re-reads fullErrors and, when falsey, re-reads merror.attributes to set title.
The other path replaces this before appending this into merror. Ignore append/
replace return values as Source does. Retain partial effects on errors.

Use the existing virtual getParent/attributes/kind/factory create/append/replace
boundaries. Do not cache parent/attributes/options property values across Source
accesses. Require nullish node members with NodeValue::requireNode and report
exact reached attribute-null read/set errors. text requires a real TextNode
binding and its virtual setText; a wrong factory type must fail rather than
silently omit text. Convert message/kind to UTF-16 through the existing host.
Use the existing TreeCallGuard; no new recursive-budget mechanism.

hasSpacingAttributes preserves isEmbellished short-circuit followed by virtual
coreMO and the resulting node's virtual hasSpacingAttributes. It must not use
core, skip nullish core results, inspect attributes directly or cache results.
Use existing tagged receiver checks and TreeCallGuard.

The Root-owned original mml-node-oracle.cjs fixture already freezes construction
branches, parent/error identity, message/title/text, root/null-attribute partial
effects and variant verification. --check reproduces 1,084 assertions over 43
cases. Before delegation the actual FoliaTests '*math_port_mml_node_*error*'
run fails all three selected tests against the existing mError stub (exit 1).
That is a real red baseline, not complete semantic acceptance. Root will add
independent raw-option/callback witnesses, review bodies, test the normal graph
and inject compiling faults. Agent completion is not acceptance.
