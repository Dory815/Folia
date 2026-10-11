# P13 Mfrac implementation evidence

Source: MathJax 4.1.3 `.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mfrac.ts`,
SHA-256 `4ded5fc9922d462d02818523d676c30299d08cfe7794067bd4e20950c8fccfc7`.
The only implementation file changed for this packet is
`src/math/core/mml_tree/mml_nodes/mfrac.cpp`. No interfaces, fixtures,
expectations, build files, or production registration were changed, and no
build or test was run. Root owns review and acceptance.

| Native member | Pinned source | Behavior mapping |
| --- | --- | --- |
| `MmlMfrac::setTeXclass` | `mfrac.ts:83–89` | Enters a `TreeCallGuard`, calls inherited `getPrevClass(previous)` first, then captures the current `childNodes` array handle. Iteration checks that captured array's live size and reads each current slot before dispatching `setTeXclass(nullptr)`. Null list/slot failures are translated to typed TypeErrors. Returns the fraction itself; it does not return the last child or truncate to arity. |
| `MmlMfrac::setChildInheritedAttributes` | `mfrac.ts:96–139` | Keeps raw display/level/prime inputs. Applies source truthiness and numeric relational conversion, incrementing level numerically only in the source branch. Reads `numalign` then `denomalign` through separate current attribute receivers. Makes two fresh shallow own-property copies, calls `addInheritedAttributes` for numerator and denominator with source key/value order, and completes both records before child callbacks. The denominator's first added key is `denalign`. Child 0 receives false display/current prime; child 1 receives false display/true prime. Each child access re-reads current `childNodes`, so mutations by child 0 affect the denominator lookup. |

The typed adaptations are the root-frozen helpers `arithmeticChild`,
`arithmeticAttributes`, and `arithmeticCopy`. `host::number` and `host::truthy`
bind the JavaScript comparison/increment operations. No protected-hook defaults
are applied here. Shared CJS fixture cases in
`tests/math/reference_arithmetic.cpp` cover raw level/display matrices, alignment
values and copy aliasing, child-list rebinding/growth during TeX-class iteration,
and required-child/null-list errors; these observations remain root-owned and
have not been independently rerun for this packet.

Root review correction: revision 2 renames the copy helper to arithmeticCopy
and binds the pinned CJS `__assign` / Object.assign lowering. The original
agent's own-spread interpretation is superseded by that locked-runtime
contract. Root's virtual-helper prototype/key trace failed before the fix and
passes after it; the MmlNode base required no change.

No additional algorithm or native acceptance is claimed here. In particular,
geometry, TeX parsing, and backend behavior are outside these two methods.
