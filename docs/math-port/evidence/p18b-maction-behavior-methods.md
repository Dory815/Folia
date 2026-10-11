# P18b maction behavior methods

This packet implements the four behavior methods assigned to
`MmlMaction`. The pinned upstream TypeScript source is
`.reference/MathJax-src/ts/core/MmlTree/MmlNodes/maction.ts`
(SHA-256 `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895`).
The corresponding installed CJS source is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlNodes/maction.js`
(SHA-256 `eddcdd2111b4b7cf5ad72937b0a4ff7beb555b98308c6771a013a18121af8ce7`).
The generated CJS preserves the method bodies and order described below.

## Method ledger

| Native member | Upstream source | Ported behavior and ordering |
| --- | --- | --- |
| `verifyAttributes(PropertyList)` | `maction.ts:98–106` | Enters the bounded tree-call guard, invokes `AbstractMmlNode::verifyAttributes` by qualified dispatch first, then reads current `actiontype` once. Only when it is strictly unequal to the string `toggle` does it query current `hasExplicit("selection")`; only a true result causes a fresh attributes read and `unset("selection")`. |
| `setTeXclass(MmlNode*)` | `maction.ts:114–122` | Reads current `actiontype` and uses strict equality with lowercase `tooltip`; no lowercase conversion occurs here. On a match it reads the current child array and slot 1, and calls that child with null if the slot is non-null. It then calls virtual `selected()` exactly once, dispatches the incoming previous node to that captured selection, calls `updateTeXclass` with the same captured selection after the callback, and returns the child's result. |
| `nextToggleSelection()` | `maction.ts:127–135` | Reads current selection, converts it using the contract's global `parseInt(String(value))` host path, adds one, and applies `Math.max(1, value)` with NaN propagation. It then reads current child-array length even if the result is NaN. A strict greater-than comparison wraps to one; finally it re-reads attributes and stores the raw double result. Fractional and NaN values are retained as JavaScript arithmetic would retain them. |
| `setChildInheritedAttributes(AttributeList, Property, Property, Property)` | `maction.ts:141–160` | Reads `actiontype` once and calls the shared Unicode `lowerString` host operation. A non-tooltip value invokes the qualified base hook with the four original raw arguments and returns. Tooltip mode reads child slot 0, and if non-null dispatches inherited attributes with all original arguments. After that callback it rereads `childNodes` and slot 1; a non-null child receives the same attribute handle and literal `false, 1, false`. Null child arrays throw at the corresponding indexed read; absent/null slots are skipped. |

## Adaptations and limits

All four recursive-capable public/virtual method entries use the existing
`TreeCallGuard<MmlNode>` budget. `action_host.ixx` supplies Unicode lowercase
and the existing arithmetic/string host supplies global-`parseInt` coercion;
the implementation adds no ASCII lowercase logic or source-specific matching.
`Attributes` and `Property` are shared handles, so passing the raw attribute
value preserves the input identity and mutations. Null C++ child-array handles
are translated to a TypeError at the source access site; a present array slot
containing a null node follows optional-chain skip behavior in the child
inheritance hook and is an error only where the TypeScript immediately calls a
method on it.

No interface or helper was missing. This report maps the source behavior; root
owns the independent oracle, build, mutation checks, and packet acceptance.
