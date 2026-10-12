# P37 operator and script implementation

Implemented the five assigned bodies from pinned MathJax-src 4.1.3
(fb987178c2d279a99b0db5ea02e751691703955e). No helper or interface was changed.

| Source method | Source range | Native method | Native range |
| --- | --- | --- | --- |
| `mo.ts` `getForms` | 475–498 | `mo_lookup.cpp` `MmlMo::getForms` | 115–143 |
| `mo.ts` `checkPseudoScripts` | 518–529 | `mo_scripts.cpp` `MmlMo::checkPseudoScripts` | 28–53 |
| `mo.ts` `checkMathAccent` | 549–568 | `mo_scripts.cpp` `MmlMo::checkMathAccent` | 78–115 |
| `munderover.ts` `setChildInheritedAttributes` | 102–136 | `munderover.cpp` `MmlMunderover::setChildInheritedAttributes` | 48–87 |
| `munderover.ts` `setInheritedAccent` | 167–188 | `munderover.cpp` `MmlMunderover::setInheritedAccent` | 99–127 |

`getForms` keeps the raw parent alongside each logical `Parent` traversal and
dispatches the row's virtual nonspace/first/last queries in source order.
Pseudo-script detection captures the constructor pattern, then uses the
logical parent and preserves the two separate attribute writes. Math-accent
detection captures the logical parent and one child array, reads at most three
slots subject to the array's current length, and preserves the source over
branch's `under.coreMO()` access.

Under/over inheritance captures the initial child array, calls base slot zero
before reading later slots, retains raw truthy `prime` values, and reads
constructor accent entries at their source points from the captured array
handle. It captures the base receiver before evaluating its prime expression.
Accent inheritance preserves explicit/default/strict-value checks, fresh
attribute reads, and the captured optional child dispatch. `getScriptlevel`
continues to perform numeric increment.

The existing tagged `NodeValue`, `ChildArray`, constructor services, and
virtual MML APIs cover the assigned paths. One edge depends on the existing
class mapping: if a node reports `isKind("mrow")` without deriving from
`MmlMrow`, the interface has no generic `nonSpaceLength`/`firstNonSpace`/
`lastNonSpace` dispatch; the implementation reports the missing native method
as a TypeError. No build or tests were run; Root owns verification and
acceptance.
