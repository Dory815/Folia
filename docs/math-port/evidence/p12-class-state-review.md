# P12 read-only class-state review

This review compares the pinned MathJax 4.1.3 source with the root-frozen
native class-state implementation and contract. No source, declaration, test,
oracle, or contract files were changed.

## Source and native mapping

| Behavior | MathJax source | Current native path | Review |
| --- | --- | --- | --- |
| Raw `texClass` getter/setter | `MmlNode.ts:497–506` | `mml_node.ixx:87,103–104`; `mml_node.cpp:198–199` | One `Property` field, initialized to `PropertyNull`; getters/setters preserve the raw variant/reference. |
| `setTeXclass` | `MmlNode.ts:694–697` | `mml_node.cpp:304–307` | Calls `getPrevClass`, reads `texClass()` once, and selects `this` unless nullish; no truthiness conversion. |
| `updateTeXclass` | `MmlNode.ts:703–709` | `mml_node.cpp:310–318` | Reads and transfers raw class/level values; clears core level before core class, then reads/transfers core class. Virtual getters/setters preserve the sequence. |
| `getPrevClass` | `MmlNode.ts:716–722` | `mml_node.cpp:321–327` | Reads previous `texClass`, stores it, gets `attributes()` once, then reads `scriptlevel`; the class write occurs before a null-attributes failure. |
| `texSpacing` | `MmlNode.ts:726–744` | `mml_node.cpp:330–350` | Read sequence and coercion behavior match the contract; details below. |
| Delimited-row class initialization | `mrow.ts:154–183` | `mrow.cpp:106–133` | Uses nullish checks for `open`, `close`, and `texClass`; initializes INNER only when class is null/undefined. |
| Empty-node read-only state | `MmlNode.ts:1320–1349,1383–1390` | `mml_empty.cpp:20–30,41–42` | Numeric NONE/zero getters and null attributes are retained; attempted class/level setters fail with typed errors; spacing is empty. |

The pinned `MmlNode.ts` SHA-256 is
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`;
`mrow.ts` is
`af58f0c1aa39a71d6c3dc5adf358a8cefeecb3f76cb483ec940f3a7ea72b4b9`.

## `texSpacing` read and conversion order

The native body preserves the observable source order:

1. Read `prevClass()` once for the nullish test. Read it a second time only
   when the first value is neither null nor missing; otherwise use numeric
   `TEXCLASS.NONE`.
2. Read `texClass()` once, then apply JavaScript truthy `|| TEXCLASS.ORD`.
   Both NONE exits compare the resulting raw values strictly against numeric
   NONE, so a string `"-1"` is distinct.
3. Convert the previous and current raw values to source property keys and
   read the row and column separately. This preserves string keys, inherited
   property lookup, missing rows, and invalid columns.
4. Only after the column read, evaluate `prevLevel() > 0`. Read
   `attributes()` and its `scriptlevel` only when the first comparison is
   falsey. A missing column does not skip those level reads: an invalid
   column plus nonpositive previous level and null attributes still throws;
   an invalid column plus positive previous level short-circuits the
   attributes read.
5. If either level is positive, test `space >= 0`; if that does not return,
   convert for `Math.abs(space)` and perform the final length-table lookup.
   Thus a missing column proceeds through `NaN` and the `"NaN"` lookup unless
   the source-ordered attribute access throws first.

The class and previous-class fields are not mirrored as numeric optionals.
`Property` copies preserve shared object/array handles, and setters retain
Missing, null, numeric, boolean, string, and reference variants. The relevant
native virtual getters are non-const, and the implementation calls those
getters rather than reading the backing fields in the semantic methods.

## Findings and boundary

I found no class-state read-count, raw-value, transfer-order, strict-NONE,
nullish-mrow, empty-node setter, or invalid-column short-circuit mismatch in
the reviewed paths. Root's frozen class-state oracle and assertions remain
the acceptance authority; this audit makes no build or acceptance claim.

One adjacent implementation detail deserves root review against the contract's
no-`const_cast` rule: `AbstractMmlBaseNode::isEmbellished() const` in
`src/math/core/mml_tree/mml_branches.cpp` currently uses `const_cast` to call a
child's non-const virtual getter. It is outside the class-state methods audited
above and does not provide an alternate `texClass`/`prevClass` storage path,
but it is present in the neighboring MML interface.
