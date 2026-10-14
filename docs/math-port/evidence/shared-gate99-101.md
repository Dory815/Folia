# Native execution evidence: Gates99–101

Gate99 does not pass the complete build. A draft test in the automatically
collected `tests/math` directory imports an unregistered timer module. The
owner moves that draft to `tools/math-port/drafts` before Gate100. The shared
input capture now also includes automatically discovered math tests and
repository-local quoted includes. Unfinished tests stay outside discovery.

Gate100 and Gate101 both build and link all 28 targets with all 1,761 captured
input hashes unchanged. These are separate from Gate99's actual failure.

## Tests

The Gate100 raw DynamicFile observer exits 0 and its strict Source comparison
passes all records, including raw return identity, live Map replacement,
ordered getters, data/accessor/data transitions, deletion/reassignment and
exception propagation. The copied executable SHA-256 is
`0da7dc2ebf8ef7870f82819131fef317b8d52e32083251ce8e566bc4c222fe63`.
PID 44660 runs from 2026-10-14T16:03:44.2878064Z to 16:03:44.3672360Z.
Complete Font tests pass 179 assertions in 21 tests.

Gate100 Host and focused Core retain two failed assertions in an old test
requiring configurable data-to-accessor replacement to throw `Unimplemented`.
The invalid cross-Context conversion assertions are retained. A separate test
now checks the supported accessor replacement, actual getter and descriptor,
inactive native-cell independence, and replacement with an ordinary data
property. It does not restore the old incomplete behavior.

Gate101's focused native data tests pass 45 assertions in ten tests. Subsequent
complete runs all exit 0:

| Executable | Assertions | Tests | Suites |
| --- | ---: | ---: | ---: |
| Core | 345,659 | 5,325 | 171 |
| Host | 468 | 198 | 10 |
| Parser | 864 | 56 | 7 |
| Regexp | 18,518 | 41 | 3 |
| LinkedList | 27 | 7 | 1 |

The complete Core copied executable SHA-256 is
`c11db91605f294b00fde35bc4581239593b322fa081bc50c574a1e389136c24c`.
PID 19712 runs from 2026-10-14T16:10:56.118Z to 16:14:33.400Z, exit 0.
Foundation remains green at 383 assertions in 58 tests. Actual `x+y` SVG
remains byte-exact at 2,502 characters, with detached Scene checks passing.

## Complete renderer comparison

Gate100 and Gate101 each complete all 75 inputs with exit 0 and 53/75 complete
exact records. Registration remains 23/38 packages with 15 warnings. Native
returns 75 SVG results; Source returns 73 SVG results and two errors. Full
renderer acceptance and Folia integration remain open.

Gate101's copied executable SHA-256 is
`a377c1d2d4454ee207a2f43bb986e235cb20f7ba5ff90b97e65615d229f36980`.
PID 47020 runs from 2026-10-14T16:10:40.070Z to 16:11:18.717Z, exit 0.
The complete native output SHA-256 is
`bc0820695d13f92cd63d2fce8d32df62452704cc7f85482c7b02e9db1ce3b318`.
The pinned Source observation remains unchanged:
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.

The cancel-lines record now matches every Source field. Shared Menclose box
extenders use the unrotated arrow data; nested fill creation keeps its actual
Source call observation; SVG rotation retains Source precision.

The Source SvgMath subclass now owns its percentage-width override and its
canonical prototype method. CommonMath retains shared measurement. Table
viewport geometry uses math units and preserves absent SVG attributes.
Ordered placement translations remain separate even when they sum to zero.
The table corpus remains 12/14: aligned-label output now differs at one
transform separator, and colortbl parsing remains incomplete. The separator
correction is saved after Gate101 and still needs compilation and comparison.

## Remaining packages

Next-six emits all 20 records but remains 0/20 exact. After Textcomp lifetime
and argument-type fixes, the next shared Map difference is Units entry keys.
Cases and Centernot retain separate semantic errors. Font3 remains 10/12;
BBM bold-state propagation and the textmacros Jax record still differ.
No additional package is registered in production based on these partial
results. Full structural diagnostics are now requested alongside the unchanged
strict comparer to resolve all shared metadata differences in one batch.

Luna owners continue package semantics, the complete Physics package, and
Maction/TimerHost respectively. Physics and Maction are not yet registered.
Mermaid remains paused. Actual process provenance and all raw comparisons are
retained in ignored `build/math-port` output.
