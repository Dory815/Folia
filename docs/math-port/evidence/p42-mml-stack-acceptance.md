# P42A MmlStack acceptance

Root accepts the MmlStack responsibility from pinned StackItem.ts only.
BaseItem, StackItemFactory, Stack and concrete reducers remain prerequisites.
Source SHA-256 and exact ownership/profile are in p42-mml-stack-contract.md.

## Evidence

- Freeze b6cdde3 compiles and executes the original 74-case stub gate: 73
  assertions fail and the initial field/array observation passes. This is a
  semantic red gate, not a compiler-error gate (build/p42-stub-red.log).
- The first Luna body compiles but fails 21 tests. Root and a separate read-only
  reviewer identify Peek's Null receiver evaluation order and toMml's noncanonical
  std::any scalar argument. These were corrected in the semantic body; the
  unsolicited MML recursion guard was removed from these non-tree operations.
- Root adds inherited named-slot and private-field-vs-protected-getter witnesses.
  Three Native probes also needed explicit temporaries to preserve Source
  left-to-right observation around mutation. Original return/error/trace
  expectations were not altered to accommodate Native results.
- Final c63a694 body matches all 76 original records (76 assertions/tests).
  The oracle --check regenerates those same records from the hash-checked
  original CJS and pinned TS; build/p42-body-baseline.log passes, exit 0.
- All twelve independent compiling faults are detected: dropped Null/Missing
  Push values, bypassed virtual Size, lost First receiver capture, empty First
  index-vs-named storage, wrong Last index, Null Peek default, fractional slicing,
  Null slice receiver order, Clear aliasing, ignored forceRow, Null inferred
  default, and copied toMml child array. Baseline and restored source pass.
  See p42-mml-stack-mutations.json and tools/math-port/stack-item-mutation-test.cjs.
- The complete normal build_test.bat passes, exit 0
  (build/p42-full-core.log), including all existing math/core tests.

Semantic body SHA-256:
3fa5ea725c2c3d0606820c0c281cd9a20e44125f079467fe959a0aaad95e5bc8.
Root owns the oracle/hosts/acceptance; Luna neither builds nor accepts its work.

## Boundaries

The same existing MML graph is retained. Node lists preserve array aliases,
sparse holes, inherited indexed/named numeric slots and tagged Null/Missing
nodes. Borrowed service edges read the actual current configuration and factory.
No separate parser, semantic tree, VM or renderer was introduced. Array
descriptors/intrinsic overrides and whole-JavaScript object reflection are not
this typed enabled-package profile; native container capacity remains a host
bound. This does not accept the rest of StackItem.ts or any TeX -> SVG formula.

The target now has 97 module interfaces; that count describes compilation
structure, not upstream-file completion. The historical scope ledger is not
advanced by this partial Source-file packet. Full parser/package coverage,
mathematical fonts, common wrappers/layout, SVG and Folia replacement are open.
Mermaid remains paused with its checkpoint and working-tree artifacts preserved.
