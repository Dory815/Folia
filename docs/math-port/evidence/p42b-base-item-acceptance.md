# P42B BaseItem and StackItemFactory acceptance

Root accepts the seventeen BaseItem methods and the actual StackItemFactory
specialization/identity bridge. With P42A, StackItem.ts's MmlStack/BaseItem
responsibilities are implemented. Source hashes, ownership and explicit typed
package boundaries are in p42b-base-item-contract.md. Stack.ts, the 26 concrete
base reducers, tags/configuration/ParseUtil/UnitUtil and real TeXParser remain.

## Gates and evidence

- Freeze 8db1433 compiles and executes 76 original observations against actual
  BaseItem stubs: all 76 fail assertions (build/p42b-stub-red.log).
- Luna owns only base_item.cpp. Root catches a temporary handle/nonconst-reference
  compilation error in setProperties; Luna fixes it by storing the shared handle
  locally, preserving the same target object. The first completed body then
  matches 73/76 records; the three remaining failures are generic Factory's
  pre-existing missing-fallback message, not BaseItem algorithm differences.
- 7d7cf26 fixes that responsible generic boundary to the pinned production CJS
  undefined/null .apply errors. No StackItemFactory exception translation was
  added. Original Source expectations remain unchanged; Root adds a Null fallback
  observation, constructor phase/class replacement and seven attribute/live-join
  witnesses. A separate reviewer confirms the method order and identifies those
  needed coverage additions.
- 4b72e15 matches all 85 original records (85 assertions/tests), including fresh
  globals/properties, node/scalar/recursive-record aliases, inherited/falsy/empty
  error tuples, shared mutable success/fail, partial over setter failure,
  explicit mrow inference, repeated kind/error queries, live env deletion,
  UTF-16 slicing, attrs rereads/current constants/partial writes, live join,
  virtual constructor opening and one actual pooled StackItem identity.
- All seventeen compiling semantic faults are detected, including two Source
  constructor-phase faults. Baseline and restored sources pass. See
  p42b-base-item-mutations.json and tools/math-port/base-item-mutation-test.cjs.
- The complete normal build_test.bat passes, exit 0
  (build/p42b-full-core.log), including the old 76-case MmlStack gate and all
  existing generic factory, MML, utility and editor/core regressions.
- Both original oracles reproduce with --check. No build products are committed.

Body SHA-256: 9d11b38600bfeabf871ee8422f65e095f398e7b803bc513340eb9ee45682599a.
Constructor binding SHA-256:
9b05948ce4483e7fb99ffbbb0c89ff1efa9c8acbc2fdade22d0d05a162bd268e.

## Scope

The source-file boundaries remain MmlStack/BaseItem, the distinct stack-item
factory and MML graph factory, and borrowed views into one future ParseOptions.
The canonical StackItemContext identity is the actual BaseItem object; no second
item wrapper, registry, graph, parser or VM is installed. Constructor metadata
is captured per actual class; DefaultStackItems is not auto-registered as
defaultNodes. Native typed property/iterator limits and session lifetime bounds
are stated explicitly in the contract; there is no whole-JS reflection claim.

There are now 100 compiled module interfaces. The historical 54/246 ledger is
not a percentage of working TeX or rendering. No full TeX/package parsing, common
mathematical layout, font/glyph service, SVG formula parity or Folia native
integration follows from this packet. Mermaid remains paused and preserved.
