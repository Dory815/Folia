# Gates128–129: MML owners and Empheq enrollment

Gate128 captures 1,957 unchanged inputs and ends with test compilation errors:
four MML probes still expose `_core` as optional<size_t> after the actual owner
became Property, and the new field test lacks two module imports. Gate129 fixes
those probes without changing expected records and builds all 51 targets.
Complete Core, Host, Parser, Foundation, Regexp and LinkedList pass. Every
capture process is terminal; comparison input audits pass before and after.

Gate129 uses the same pinned MathJax 4.1.3 source and comparison rules. Full
rendering matches 70/75 records, actual registered packages are 37/38 and one
warning remains for Bussproofs. Public API is 9/9. Texttip's complete SVG stays
byte-exact. Physics, Colortbl, Mathtools, original/expanded Bussproofs, utility
and module sessions, Tag/SetOptions/defaults, parser projections, map/handler
sessions, host/layout, Attributes and all 220 Mhchem records remain passing.

Actual changes include MML node live fields over the existing factory,
properties, children, parent and class storage, canonical MmlFactory.create
dispatch, and Empheq module exports/utility receiver dispatch in the actual
render session. Empheq's invalid option record now matches Source. The valid
example still fails with `createBoldToken arguments are invalid`. The tooltip
now reaches the actual wrapper factory but fails at its missing Context `wrap`
method. Neither module is claimed complete merely because its tests compile.

The remaining full-renderer records are:

- action-tooltip: missing callable `wrap` on the actual factory prototype;
- bussproofs-tree: canonical operational Stack projection not enrolled;
- empheq-left-brace: actual createBoldToken argument bridge rejects the call;
- texhtml-default-disabled: four extra placement calls, relation merge pending;
- texhtml-explicitly-enabled: font family `undefined` versus Source `Times`.

Next-six remains 18/20, now beyond the earlier missing MML fields: one Tags
method arity and one StackItemFactory graph-alias difference remain. Fonts
remain 13/16, with Tags.enTag.length 0 versus Source 2 as first differences.
The independent TexHtml/XML comparison fails two foreignObject xmlns records.
They already failed in Gate126; the earlier status incorrectly reported all
twelve passing. No comparer or Source record has been changed to accept them.

MML projection still has explicit typed boundaries for foreign children arrays,
arbitrary factory replacement, TextNode value replacement and XML fields.
This is not an assertion of complete JavaScript object semantics or of the
entire MathJax feature surface. Stack, bounded-lifetime and cancellation
packets remain unregistered or unaccepted. Folia and Mermaid are unchanged.

Ignored raw records are gate-specific: `gate-129-inputs-before/after.json`,
`diagnostic-exits-129-root.json`, `common-layout-native/comparison-g129.json`,
`package-captures-g129.json` and `checkpoint-comparisons-g129.json` under
`build/math-port`. Full Source observation SHA-256 remains
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.
Gate129 full native observation SHA-256 is
`e5d54d456723d43c7dce5ec0582d77b66c0fdeb133fd013a0e5fc3b77f2535ea`.
