# P13 `MmlMroot` method ledger

Pinned source is MathJax 4.1.3
`ts/core/MmlTree/MmlNodes/mroot.ts`, SHA256
`b4b8f9deddb22efe7580067d8845d7cb225046e244eb93da8b0e931eff76e99a` from
`docs/math-port/evidence/p13-arithmetic-members.json`. The method witnesses in
that inventory are `setTeXclass` lines 82–87 (member SHA256
`f8c9ad6bda0a8073a1f177c100f72e61fc8b2132dac6eb83dd01ba3883860a10`) and
`setChildInheritedAttributes` lines 94–107 (member SHA256
`385e9ebd02b4a565a8716bf5e0d604a3d6c56b89ebd440f2d1c696856a660b01`).

| Source method | Native mapping |
| --- | --- |
| `setTeXclass(prev)` | `src/math/core/mml_tree/mml_nodes/mroot.cpp:12-24`. Under the shared call guard, transfer prior class/level first; resolve and call child 0 with null previous; only after that virtual call, resolve the current child 1 and call it with null previous; return this. `host::arithmeticChild` checks null arrays, missing slots, and null children with the host TypeError. It is invoked again for child 1, so callback replacement of `childNodes` is observed. |
| `setChildInheritedAttributes(attributes, display, level, prime)` | `src/math/core/mml_tree/mml_nodes/mroot.cpp:26-39`. Resolve child 0 and call it with the original attributes handle, raw display/level, and true prime. Resolve child 1 after that callback and before computing its level; use `host::add(level, 2)` to preserve JavaScript `+` behavior, then call with the same attributes handle, false display, computed level, and original prime. |

The protected hook intentionally does not apply defaults; its caller owns the
outer entry defaults. `PropertyList` copies are shared object handles, so
passing `attributes` by value preserves source aliasing across both child
calls. The two `arithmeticChild` calls preserve source receiver lookup timing,
and the second child's receiver is captured before `host::add` can perform
property-to-primitive conversion. No geometry, TeX parsing, or tree policy is
implemented here.

No tests or builds were run for this assignment. Root owns independent source
review and acceptance; the ledger is not a passing-test or parity claim.
