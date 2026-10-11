# P24 operator and registry observer bindings

This packet changes only native C++ emitted by the three frozen oracles and
their generated observer fixtures. No MathJax source behavior, stimuli,
expected values, case names, helper categories, pins, scope entries, or runtime
modules were changed.

## Native binding map

`mo-values-oracle.cjs` changes only its emitted `children` helper. It now
constructs `tree::ChildArray<MmlNode>` from `initializer_list<NodeValue>`, which
matches the factory's `m::Children` alias. All 74 assertions and 14 case bodies
in the generated fixture otherwise remain byte-for-byte generated from the
same raw definition observations.

`mml-registry-oracle.cjs` updates the same helper and its emitted native
`node(...)` tree projection. The projection captures `n->childNodes`, records
its initial length, sizes the resulting `PropertyArray` before mapping, then
uses `ChildArray::forEach` and tagged `NodeValue` reads. This preserves
Array.map's captured receiver and length, skips absent slots, sees inherited
slots through `has`, and keeps output holes. A present non-node value is
explicitly dereferenced with `requireNode()` where the recursive projection
needs a node. Expected declarations, registry cases, direct source values and
all case bodies were left alone.

`mo-oracle.cjs` updates its children helper, changes only the one native dense
list passed to `create(mrow)` to populate a `ChildArray` with `push_back`, and
changes the native parent observation to compare the raw `coreParent()` service
with `style->getParent().nodeOrNull()`. The local `items` vector still holds
factory product pointers; the independent `ACCENTS`/Property vector, operator
dictionary, `coreParent()` service, and expected UTF-16/BMP values were not
converted or changed.

## Regeneration evidence

Each original oracle was run once in generation mode:

- mo-values: 74 assertions, 14 cases.
- MML registry: 43 entries, 172 construction cases, 1,849 instance-matrix
  observations, 175 total cases, and 387 source-value entries.
- Mo: 23,577 assertions, 14 cases, and all 1,252 original dictionary entries.

These counts match the frozen profiles in the P24 contract. No native build or
test was run. Root owns independent source-profile reproduction, direct
expectation review, full graph compilation and the repository gate.

## Source pin boundary

The oracle dependency structure and source checks were not changed. Mo and
mo-values inherit the literal version/hash and package/runtime checks from
`mo-pattern-reference.cjs`. Registry validates the MML/MmlFactory/maction TS
and installed package text against hashes selected from the current
`scope.json`; its transitive MML class loaders also use scope-selected hashes.
Thus registry's integrity check depends on the separately frozen scope
manifest, while the Mo dictionary oracle has its dedicated literal source and
runtime lock. This packet does not replace or broaden those pin contracts.
