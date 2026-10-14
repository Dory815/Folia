# Native execution evidence: Gates95–97

Gate95 was cancelled before its build command started because an owner reported
an incomplete Common Maction change. Its recorded build exit is null. Gate96
then failed compilation at an optional shared-pointer access in table labels.
After that access was corrected, Gate97 built and linked all 27 targets with
all 1,628 captured input hashes unchanged.

## Complete observations

Gate97 completes all 75 actual inputs with exit 0. Exactly 52 complete records
match Source, including MML, Common boxes, method calls, paint creation, font
loads and SVG. Registration remains 23/38 packages with 15 warnings. Native
returns SVG for all 75 inputs, whereas Source returns 73 SVG results and two
action-related errors. Full renderer acceptance remains open.

The pinned Source observation SHA-256 is unchanged:
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.
The copied executable SHA-256 is
`79603318d2c2257e08769ac27691a4e421ff280dcea76744c3325947e8bb8449`.
PID 70512 runs from 2026-10-14T05:29:59.256Z to 05:30:40.382Z, exit 0.
The actual output SHA-256 is
`2691c6690ef4c9a749bd396c2f25bf9bfc1a2c00839e4f9f92b1aea22a2f2d63`.

The table/spacing corpus improves to 12/14 complete exact records. The
remaining differences are label-node creation order and colortbl parsing.
The table run uses the same copied executable, PID 25744, from
2026-10-14T05:30:50.140Z to 05:30:58.657Z, exit 0. Its 14-case corpus hash is
`d0f4e0454d64b36a7660cee776d474dd9de81d10a0018060a26325c88363748a`.

## Tests and open work

Foundation passes 383 assertions in 58 tests. The actual document probe exits
0, preserves byte-exact `x+y` SVG and verifies completed Scene access after
service release. The last full Core run remains Gate94 with one failing
assertion; Gate82 remains the last entirely passing full Core run.

Gate97 reruns the remaining dynamic-provider test and still fails one of its
six assertions. The failing assertion assumes the first entry in an ordered
native map is the `arrows` file. Its expected file value has since been
corrected to the actual selected key; that correction still needs a new build
and test run. Provider retry behavior passes the other five assertions.

Next-six package observation still stops before emitting its full 20 records
because a token-map callback lacks its canonical function binding. Those
packages are not newly registered in production. Raw font-field semantics,
Maction behavior, Menclose placement and remaining package owners are still
under development. Folia integration remains pending; Mermaid remains paused.

Actual process provenance, raw output, strict comparisons and build-input
audits remain in ignored `build/math-port` output. No comparison tolerance,
record filtering or change to the pinned Source observation is introduced.
