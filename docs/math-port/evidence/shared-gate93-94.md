# Native execution evidence: Gates93–94

The pinned complete75 observation and all comparison rules are unchanged.
Gate93 ends with a parser-interface string type error. Gate94 builds and
links all 27 targets; all 1,628 captured input hashes remain unchanged.

## Complete observations

Gate94 completes all 75 inputs with exit 0. Fifty records match every Source
field, including MML, Common wrappers and boxes, method calls, paint creation,
font loads and serialized SVG. Native still registers 23/38 packages and emits
15 configuration warnings. The complete renderer is not accepted.

The Source observation SHA-256 remains
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.
The copied executable SHA-256 is
`2622c878cc9b2c37c734cc7bdc64e2ed527ed7b666d22c022889149e465e619f`.
PID 62276 runs from 2026-10-14T04:43:59.575Z to 04:44:41.622Z, exit 0.
The exact report is retained in ignored output as
`common-layout-g94-comparison.json` with all 27 failures (25 records and two
package-registration checks).

The focused table/spacing corpus improves from 4/14 to 11/14 complete exact
records. Its remaining differences are the floating-point operation order for
column rules, labeled-row placement, and the not-yet-registered colortbl input
package. The comparison retains those differences without numeric tolerance.

## Tests

- Foundation passes 383 assertions in 58 tests across 12 suites.
- Host passes 465 assertions in 197 tests, including the previously failing
  inherited Symbol.iterator lookup. Parser, Regexp and LinkedList pass.
- Full Core runs to completion with one assertion failure in
  `failed_dynamic_provider_preserves_source_setup_placeholder_and_sync_retry_is_a_noop`.
  Its other five previously failing dynamic-provider tests now pass. Core is
  not accepted; Gate82 remains the latest entirely passing complete Core run.
- The actual document probe exits 0, retains byte-exact `x+y` SVG (2,502
  characters), and verifies completed Scene access after service release.
- Old-six and next-seven parser corpora pass 24/24 and 23/23. Font3 still needs
  its corrected function-length observer rerun. Next-six exits during its
  first input before it emits a complete native record; its 20 cases are not
  accepted and its packages are not yet enrolled in the production session.

Executable copies, actual process IDs, UTC intervals, exit codes, raw output
and all test logs remain in ignored `build/math-port` output. The native
document and Foundation provenance files end in `-94-provenance.json`; the
five complete test-run provenance records are in `diagnostic-exits-94-root.json`.

## Responsible boundaries corrected

Common layout uses the actual direction constants for roots and bevelled
fractions; mtext follows the Source `Parent` accessor; four border widths are
retained instead of replacing previously read sides. SVG-owned placement now
preserves root/fraction operation order, omitted optional call arguments,
primitive creation attributes, border/background order, and Source text-node
measurement. The table labels node is created by its SVG constructor, exposed
through the existing Context field, and captured from its actual current DOM
state into an owned Scene using `Builder::adoptExisting`.

Dynamic font records use one canonical binding for generated and extension
files. Array-backed native properties inherit the actual Array.prototype.
The remaining dynamic-provider assertion, raw-field behavior, package bodies,
action rendering, font-load paths and other exact comparison differences remain
open. Folia integration remains pending. Mermaid remains paused.
