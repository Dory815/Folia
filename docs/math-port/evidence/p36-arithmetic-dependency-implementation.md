# P36 arithmetic dependency implementation

Implemented the three authorized MathJax 4.1.3 method bodies.

| Source method | Source range | Native method | Native range |
| --- | --- | --- | --- |
| `mroot.ts` `setTeXclass` | 82–87 | `mroot.cpp` `MmlMroot::setTeXclass` | 12–18 |
| `mroot.ts` `setChildInheritedAttributes` | 94–107 | `mroot.cpp` `MmlMroot::setChildInheritedAttributes` | 20–28 |
| `mfrac.ts` `setTeXclass` | 83–89 | `mfrac.cpp` `MmlMfrac::setTeXclass` | 13–24 |

Mroot class assignment calls `getPrevClass`, then reads and dispatches child
slot zero, then reads the current child array again for slot one after the first
child callback. Mroot inheritance sends unchanged attributes/display/level and
literal `true` prime to slot zero, then dispatches through a fresh slot-one
lookup with `false`, `host::add(level, 2)`, and the original prime value.
Mfrac class assignment captures the array once, uses its live length and
indexed Get, calls each child's virtual `setTeXclass(null)`, and returns the
fraction.

The existing arithmetic host access and addition APIs cover the required
operations. No unresolved host boundary is known. No build or tests were run;
Root owns verification and acceptance.
