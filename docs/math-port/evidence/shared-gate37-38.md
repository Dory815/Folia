# Shared native checkpoints: Gates 37 and 38

Both combined builds exit 1. All 826 frozen registered/generated input hashes
remain unchanged in each build. Gate37 does not freshly link the library:
an environment temporary is bound to a Property reference, two Unicode tests
are declared outside their suite, and Boost.UT cannot print a UTF-16 vector.
Those three errors are repaired for Gate38.

Gate38 freshly links folia_math and the diagnostics used below. Its core runner
still fails compilation on a different vector of optional UTF-16 strings.
LinkedListDump fails linking because MathLocationService was forward-declared
in one named module and defined in another. Owners have subsequent repairs;
neither repair inherits Gate38 acceptance. Gate36 remains the latest fresh
passing full core run (345,534 assertions, 5,295 tests, 169 suites).

## Fresh Gate38 observations

| Observation | Result |
| --- | --- |
| Base / Frontend / Boldsymbol-Configmacros / AMS / AMS boundary | 150/150 complete original records match |
| HostTests | Exit 0; 430 assertions, 192 tests, 10 suites |
| ParserTests | Exit 1; four assertions fail across two supplied-object identity tests |
| RegexpTests | Exit 1; two tests still expect Unicode `u` to be unsupported |
| Actual production Unicode RegExp observer | Exit 0; 27/37 complete original records match |

The previously failing constructor exceptions are gone. New readonly-field
and setter-order constructor tests pass. The remaining failures are
`base_item_constructor_callbacks_preserve_source_arguments_and_defaults` and
`mstyle_retains_the_supplied_attribute_property_identity`: fresh bootstrap
conversion cannot be applied to already observed supplied records by copying
them. Canonical views over their actual storage are required.

The Unicode differences are six syntax-error messages and four actual Source
pattern records. Three of those patterns exhibit matching behavior but have
different pattern spelling; the actual tag-split pattern fails compilation.
These remain differences in the strict comparison. No record normalization,
private split replacement, or reduced corpus is used to turn them into passes.
The observer invokes the production split primitive also used by LiteParser.
The Source host is Node 24.13.0 / V8 13.6.233.17-node.37 / Unicode 16.0;
native SRELL's Unicode table-version difference remains separately open.

## Provenance and reproduction

Ignored evidence lives in `build/math-font-package-gate-37.log`,
`build/math-font-package-gate-38.log`, `build/math-port/gate-38-inputs-before.json`,
`build/math-port/parser-checkpoint-38-root-fresh/summary.json`,
`build/math-port/diagnostic-exits-38-root.json` and
`build/math-port/unicode-checkpoint-38-root-fresh/summary.json`.
Summaries retain actual executable hashes and process exit codes; old core
and LinkedList executables are not reported as fresh Gate38 results.

`node tools/math-port/run-unicode-checkpoint.cjs LABEL` re-executes the pinned
original oracle and checks the whole baseline SHA-256 before invoking the
native program. The original baseline is
`e6c01910ecb588816a0b1c99f7746b7a9b21bd8abeb1cd84f93a3b7163e5d634`;
Gate38's observer executable is
`c32339dfb22cf41d23e7ce13eb358eb614978881bdfcf46fcd92344ef2dac592`.
The comparison checks row count, uniqueness, order and every complete row;
it returns failure for any mismatch and retains both records.

Common wrappers, actual Lite graph, completed scene and SVG serializer remain
uncompiled implementation work. No native full-pipeline SVG or Folia math
integration is accepted at this checkpoint. Mermaid remains paused.
