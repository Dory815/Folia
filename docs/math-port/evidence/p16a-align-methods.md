# P16a maligngroup method mapping

This file maps the one executable `MmlMaligngroup` method assigned in P16a. It is not an acceptance report; root owns compilation, actual-CJS comparisons, mutations, and acceptance.

The source is pinned MathJax 4.1.3 `ts/core/MmlTree/MmlNodes/maligngroup.ts`, SHA-256 `4d8ecdae93aaff85887ec4eb09c8821811866a9e9b56e037ac61d0b6d81bf2b1`.

`setChildInheritedAttributes(attributes, display, level, prime)` at TS lines 63-74 performs these steps in order:

1. Reads `this.attributes`, calls its `getAllAttributes()` and passes the original input `attributes` and that result to virtual `this.addInheritedAttributes(...)`.
2. Rebinds the local `attributes` variable to the returned list.
3. Calls the qualified base implementation `super.setChildInheritedAttributes(attributes, display, level, prime)`.

The native method in `src/math/core/mml_tree/mml_nodes/maligngroup.cpp` preserves that order and alias flow: one virtual `attributes()` read; `getAllAttributes()` from its live handle; one virtual merge call with the original input handle and returned record handle; then the qualified `AbstractMmlLayoutNode` child-inheritance method receives the merge result and unchanged display, level, and prime `Property` values. It does not clone either list, implement a second merge algorithm, or add a recursive-call guard; child recursion remains budgeted by the existing `setInheritedAttributes` path as frozen in `simple-mml-contracts.md`.

Native host adaptation: the type-safe interface represents `attributes` as a nullable `shared_ptr<Attributes>`. If it is null, the implementation raises a TypeError for reading `getAllAttributes` instead of dereferencing null. A throwing virtual getter or `addInheritedAttributes` is allowed to propagate; the superclass is not called after either failure. No build or tests were run, per packet boundary.
