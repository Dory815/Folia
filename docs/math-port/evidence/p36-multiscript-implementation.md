# P36 multiscript implementation report

Implemented only the two assigned bodies in `src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp`. The already accepted `MmlMprescripts::verifyTree` and `MmlNone::verifyTree` bodies are unchanged. No build or tests were run, as required by the P36 packet.

## Source correspondence

`MmlMmultiscripts::setChildInheritedAttributes` (`mmultiscripts.ts:65–105`):

- Enters `TreeCallGuard`, then obtains current child slot 0 through `multiscriptChild(..., "setInheritedAttributes")` and calls the child's inherited-attribute method with the original four arguments before any other work.
- The loop condition obtains the current child array length each time. Each body obtains the current indexed child and checks its virtual `isKind("mprescripts")`.
- Only the first marker switches to pre-script mode. At an even marker index, it creates `none`, re-reads the current child array for in-place splice, then performs the tagged parent write, and advances the local index an extra step. A null factory result remains a `NodeValue` through splice and fails only at the parent write.
- Other children receive the original attributes, `false`, `host::add(level, 1)`, and the original truthy `prime` value or the boolean parity result. The non-marker count advances after the virtual call.
- After scanning, parity uses the current array. Repair creates `none` and dispatches virtual `appendChild`; it then separately reads the current array for the indexed receiver and re-reads it for length. An empty final array is represented by a Missing slot before tagged `setInheritedAttributes` access. The final inheritance call uses original `prime`.

`MmlMmultiscripts::verifyChildren` (`mmultiscripts.ts:112–142`):

- Enters `TreeCallGuard` and reads `fixMmultiscripts` once into a raw `Property`; later guards use its source truthiness.
- Each loop condition reads current length and each indexed access reads the current child slot. The child `isKind` call is virtual.
- Duplicate marker diagnostics read `child.kind()` before owner `kind()`, then call the child's virtual `mError` with unchanged options and `short=true`. The first even-position marker and final parity diagnostics use the exact Source messages and default non-short `mError` call. Returning error callbacks allow scanning to continue.
- Calls qualified `AbstractMmlNode::verifyChildren(options)` last, corresponding to the inherited implementation selected by the TypeScript `super` call.

## Tagged API status and remaining uncertainty

The current `multiscript_host.ixx` now supplies receiver checks, raw indexed `NodeValue` access, exact member access, in-place splice, and nullable zero-argument factory creation. These bodies use those APIs directly. No unresolved host helper is required for the two methods. The Native tree cannot express a JavaScript negative array property key as an indexed child; the zero-length final-array case is handled as a Missing slot, matching the subsequent undefined receiver failure. This report records the implementation mapping only; Root owns independent Source parity and mutation acceptance.