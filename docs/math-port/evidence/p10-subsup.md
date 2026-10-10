# P10 `msubsup` implementation evidence

Source provenance is MathJax 4.1.3 `ts/core/MmlTree/MmlNodes/msubsup.ts`
(SHA-256 `37fd01fa0c44ceab5b4ed30ba3d95ab43ffb8aed55db27c7290d6d786131f6e7`)
and its production-resolved CommonJS output
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNodes/msubsup.js`
(SHA-256 `7971b391a8d7c92e0d6115b17c4c394dafc828942fc00591ba6064b871610c18`).
The native implementation is [msubsup.cpp](../../../src/math/core/mml_tree/mml_nodes/msubsup.cpp).
It retains MathJax's Apache-2.0 notice and points to
`src/math/LICENSE-MathJax.txt`.

## Member mapping

| Source member | Source lines | Native implementation |
| --- | ---: | --- |
| `MmlMsubsup.kind` | 45–50 | `MmlMsubsup::kind()` returns `"msubsup"`. |
| `MmlMsubsup.arity` | 54–57 | `MmlMsubsup::arity()` returns `3`. |
| `MmlMsubsup.base` | 61–64 | `MmlMsubsup::base()` returns `0`. |
| `MmlMsubsup.sub` | 68–71 | `MmlMsubsup::sub()` returns `1` and remains virtual. |
| `MmlMsubsup.sup` | 75–78 | `MmlMsubsup::sup()` returns `2` and remains virtual. |
| `MmlMsubsup.setChildInheritedAttributes` | 84–112 | Captured child vector, live slot reads, ordered child calls, JS `+`, and short-circuit prime behavior. |
| `MmlMsub.kind` | 126–130 | `MmlMsub::kind()` returns `"msub"`. |
| `MmlMsub.arity` | 135–139 | `MmlMsub::arity()` returns `2`. |
| `MmlMsup.kind` | 156–160 | `MmlMsup::kind()` returns `"msup"`. |
| `MmlMsup.arity` | 164–168 | `MmlMsup::arity()` returns `2`. |
| `MmlMsup.sup` | 175–178 | `MmlMsup::sup()` returns `1`, overriding the parent getter. |
| `MmlMsup.sub` | 183–186 | `MmlMsup::sub()` returns `2`, overriding the parent getter. |

Static `defaults` fields are intentionally not duplicated here: the root-owned
`msubsup_bindings.cpp` forwards each class to its frozen defaults binding.
Constructors are inherited from the parent exactly as in the source and are
implemented by the shared root-owned factory binding.

## Inheritance hook behavior

`setChildInheritedAttributes` enters the shared `TreeCallGuard`, captures the
current `childNodes` vector once, then reads its slots at each source access.
Replacing the node's `childNodes` property during a callback therefore does
not retarget this call, while in-place mutations of the captured vector remain
visible. It calls base, subscript, then (only if present) superscript, keeping
the input attribute record and the original display/level/prime properties.
The second and third calls use display `false` and compute level with
`host::add(level, 1)`, preserving JavaScript primitive addition, including
string concatenation. For each prime argument it preserves a truthy raw prime
value; otherwise it invokes the virtual `sub()` getter at that source point
and compares it with the relevant child index. A truthy prime skips that
virtual getter.

The source's receiver-before-arguments behavior is retained: a missing/null
base or subscript receiver raises the native typed `TypeError` before level
addition or the `sub()` getter. A missing/null third child returns only after
the earlier child calls, preserving their partial effects. Native vector
bounds/null checks replace JavaScript's `undefined` receiver exception and
prevent C++ undefined behavior; they do not normalize the malformed tree.

## Boundaries

This packet implements the three concrete script-node classes and their
inherited-attribute hook only. Defaults, factory registration, generic MML
behavior, and `munderover` remain owned by their respective root/agent
packets. No claim of packet acceptance or full MathJax port is made here; the
root owns compilation, oracle comparison, mutation checks, and acceptance.
