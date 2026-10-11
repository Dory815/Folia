# P23l leaf native observer migration

Updated only the C++ emitter in `tools/math-port/leaf-oracle.cjs` and regenerated
`tests/math/reference_leaf.cpp` with the authorized oracle run. The generator
reported **669 assertions / 27 cases** across the ten locked source modules,
matching the frozen P23l contract and its 658-record Source profile. The five
full BMP pattern bitsets remain generated from the original pinned classes.

The emitter now creates test child arrays as `ChildArray<MmlNode>` populated
with tagged `NodeValue`s. Its indexed helper uses ChildArray `get`/`size`, with
pointer projection only for observations that dereference a child; tagged
identity comparisons use `sameNode`. The Probe's TeX-class callback and
override use `NodeValue`, and explicit JS null arguments are represented by
`NodeValue(PropertyNull{})`. Property-valued `notParent` observations retain the
original `eq` helper and use `h::truthy` to adapt to the Source boolean
observation.

Source array operations remain array operations in the native observer:
`unshift(null)` maps to `ChildArray::splice` with a one-element Null addition,
and `push` maps to `push_back(NodeValue(...))`. The live alias/callback tests
continue to mutate the captured ChildArray handle and rebind the current handle
separately. `ACCENTS` and the script inheritance hook signatures are unchanged.

No JS source classes, stimuli, calls, pins, expected values, helper categories,
case names, assertion counts, or profile golds were changed. No native build or
test was run; Root owns independent profile reproduction, fixture review, and
full-graph compilation. This repairs only the observer binding and makes no
claim about runtime semantic acceptance.
