# P13 `MmlMsqrt` method-port evidence

Upstream: MathJax 4.1.3
`.reference/MathJax-src/ts/core/MmlTree/MmlNodes/msqrt.ts`, SHA-256
`8d522b96f39f35c170fe12ab7e59e0999c65551335466010522280bfa3570b7e`.
The MathJax Consortium copyright and Apache-2.0 notice are retained in the
implementation; the full license is at `src/math/LICENSE-MathJax.txt`.

| Native member | Upstream member and lines | Mapping |
| --- | --- | --- |
| `MmlMsqrt::setTeXclass` | `setTeXclass`, 79–83 | Enters `TreeCallGuard`, invokes `getPrevClass(previous)` first, then obtains required child 0 and calls only that child’s `setTeXclass(nullptr)`, finally returns this node. It does not iterate/flatten children or return the child result. |
| `MmlMsqrt::setChildInheritedAttributes` | `setChildInheritedAttributes`, 90–97 | Enters `TreeCallGuard`, obtains required child 0, and calls its inheritance method with the same `AttributeList` handle, unchanged raw `display` and `level`, and `true` for prime; the incoming prime value is unused. |

The shared `host::arithmeticChild` binding turns a null/missing child array or
missing/null child 0 into the contract’s typed TypeError before the child
method is called. Child access follows `getPrevClass` in `setTeXclass`; it is
performed before inheritance dispatch in the protected hook. `AttributeList`
copies preserve the underlying PropertyList handle identity, and Property
copies preserve Missing, null, string, and other raw values. There is no
additional entry-defaulting, geometry, or child traversal in this unit.

Root owns the frozen actual-CJS fixtures, builds, mutation checks, and final
acceptance. This evidence maps source behavior and does not claim acceptance.
