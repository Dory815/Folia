# Gates135–137: Stack/parser integration is not accepted

Gate135 compiles the new Stack array and Context bindings plus parser fields
into the shared graph. It ends with stale typed-helper signatures and test
callers, including an old envStack getter and a duplicated fixture variable.
Gate136 fixes those and includes the canonical FunctionList configuration
identity; its new UT needs a comparison that does not print options::Key.
Gate137 builds all 53 targets with 1,982 captured inputs unchanged.

The new Stack has one replaceable operational array owner. Its first Context
array views the native storage; replacement retains the exact assigned value
and old aliases retain their old store. Parser own fields refer to their real
owners. TextParser envStack replacement is shared with TextMacros brace
operations. ProofTreeItem.innerStack likewise uses one current owner. These
changes are compiled, but the joint implementation does not pass acceptance.

Complete Core has six Stack regressions: three undefined Top results enter
the wrong typed representation, an environment read order changes, and two
native join cases fail Source's captured-length behavior under mutation.
Host has two obsolete Unimplemented expectations after generic ToPrimitive
support; their actual Source behavior must be checked before changing tests.
Parser, Foundation, Regexp and LinkedList pass.

New StackOwners capture fails with a missing canonical StackItem identity.
The original Bussproofs and session captures fail with undefined.innerStack
Push, and expanded Bussproofs also differs. These were previously passing
checks and must be restored. Bussproofs remains unregistered in the renderer.
ParserFields capture fails its expected own-key order. A separate pinned CJS
witness establishes stack before TextParser's text/nodes/envStack, so that
specific fixture assertion is wrong; the native order already agrees.

Full rendering remains 74/75 and is byte-identical to Gate134's complete
native observation. Public API stays 9/9 and full texttip SVG stays exact.
Physics, Colortbl, Mathtools, Tags, Mhchem, Attributes, parser projection,
map-session and host/layout comparisons pass. Fonts remain 13/16 and next-six
18/20, now exposing configMethod.items[0].item.length (Source 2/native 0)
after the real FunctionList identity replaces the former empty carrier.
Two standalone XML namespace differences remain unchanged.

Capture tooling now runs the frozen shared build executables directly,
auditing their hashes before/after and the input manifest. It retains complete
byte output and per-gate provenance without making another executable copy.
Gate137 creates no gate137-runners directory; the former per-gate runner set
was approximately 2.2 GB. The output-capture test passes 73 byte-boundary
checks. A missing helper terminator/import caught during syntax review is
fixed before this evaluation; it does not change compiled inputs.

All evaluation and comparison processes are terminal; input and executable
hash checks pass. Evidence is under build/math-port with Gate137 manifests,
diagnostic-exits-137-root.json, package-captures-g137.json and
checkpoint-comparisons-g137.json. Full native SHA-256 remains
`795255bf0b7f754b17b9a49ae7a107873925825a6572e8b03a8a09ac34b23881`;
Source SHA-256 remains
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.

This is an unaccepted integration checkpoint. Factory fields, XML helper
semantics, lifecycle and cancellation packets remain separate work. Folia
retains its math QuickJS worker and Mermaid remains paused.
