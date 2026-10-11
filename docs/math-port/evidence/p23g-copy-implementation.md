# P23g MML copy implementation evidence

This packet implements only `AbstractMmlNode::copy(bool)` in
`src/math/core/mml_tree/mml_node.cpp`. It does not accept the full MML family,
the whole MML source file, TeX, layout, SVG, or Folia integration.

## Pinned-source mapping

The implementation was compared with `MmlNode.ts:467–492` and the installed
MathJax 4.1.3 CJS `MmlNode.js:149–191`. The frozen pins are:

- TypeScript `MmlNode.ts`: `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`
- CJS `MmlNode.js`: `86c5eb572dcf2652c3c52a60b6c720d231e17f600cc9aeef3ace4aeafc81d45`
- Focused Source-order and observer audit: `p23g-copy-source-audit.md`

| Native range | Source responsibility |
|---|---|
| `mml_node.cpp:196–209` | Enter `TreeCallGuard`; create through the current factory and kind before reading source state; dynamically check that the product has the native normal-MML property storage; replace that target's property-store handle with a fresh shallow own-key copy. Nested `Property` values keep identity. An incompatible native factory family raises an explicit TypeError before source property copying; this is a typed native boundary, not arbitrary-JavaScript-factory equivalence. |
| `mml_node.cpp:211–228` | Test source Attributes truthiness, read the Attributes handle again, capture its explicit property store and `Object.keys` snapshot. For each retained key, read the destination Attributes receiver first, then Get the source value live and call `set`; omit `id` unless this invocation has `keepIds=true`. |
| `mml_node.cpp:232–241` | Check current source child-array presence/length and capture its handle. For a singleton, Get child zero to read `isInferred`; if true, Get slot zero again and capture that value's current `childNodes`. This preserves both nullish dereference points and callbacks that change the singleton slot. |
| `mml_node.cpp:242–258` | Iterate the captured handle using live length and indexed Get. A Node receives virtual `copy()` with no forwarded `keepIds`, then the copied result is passed to the destination's virtual `appendChild`. Falsey hole/Undefined/Null values are written as explicit Null to the destination's current ChildArray, bypassing append and parent assignment. |

The source corpus and its full method-by-method order notes are in
`p23g-copy-source-audit.md`. This implementation does not use `NodeUtil` as a
substitute for the source algorithm and does not share Attributes layers.

## Boundaries and validation status

Property values and child slots remain in the existing `PropertyList`,
`Attributes`, `NodeValue`, and `ChildArray` stores. The native family check is
needed because protected C++ storage cannot be assigned on an arbitrary
foreign product as JavaScript can. It occurs immediately after factory
creation and before source state is read. The recursive call continues to use
the existing tree call guard.

No build, test, mutation, fixture generation, or commit was run for this
delivery. Root owns comparison against the frozen original-Source cases,
semantic mutations, and acceptance. `superscript-first-copy` remains outside
this packet's setup coverage because its original input construction depends
on the not-yet-migrated `NodeUtil.setChild` path. Other MML methods and
consumers remain unaccepted.
