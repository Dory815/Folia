# Typed TeX primitives and MML construction

Root reviewed and promoted the Luna candidates for TexError, TexScanner,
ParseUtil expansion, MmlFactory/NodeFactory and structural node traits. All
use native owned values and request-local document references. They import
neither the withdrawn object runtime nor SVG/layout services.

Structural arity traits cover the 43 pinned MML kinds. MML construction inserts
inferred rows, flattens inferred children for arbitrary arity, and converts them
to explicit rows for finite arity. Destination vectors are reserved before
moving existing children. TeX error construction uses that common rule rather
than a separate error-specific inferred-row implementation. Source named node
properties, family defaults/validation and layout semantics remain pending.

TexScanner ports the Source lexical methods with UTF-16 cursor observations.
ParseUtil ports only addArgs, substituteArgs and checkMaxMacros: escaped hashes,
control-word boundary spacing, buffer limits and increment/error ordering.
TexError formats owned strings and supplies an explicit native UTF-8 what()
policy; it exposes no generic property or coercion API.

## Native result

```text
cmake -Wno-dev -G Ninja -S . -B build/core
cmake --build build/core --parallel 4 --target FoliaMathTests
build/core/FoliaMathTests.exe
```

Final build and execution both returned 0: **63 tests / 789 assertions**.
The suite includes the earlier foundation and SVG consumer tests.

The first scanner test build needed an explicit import of TexError and a UTF-16
comparison that avoids Boost.UT's narrow-stream printer. The first execution
returned 1 for two manually copied ParseUtil expectations: the argument was
`beta` while expected text still said `one`. Root corrected those test values
to the corresponding Source substitution. No production algorithm change was
needed for that failure. The following build and complete focused run passed.
No abnormal native exit occurred. Logs remain ignored under
`build/math-port/native-raii-tex-*`.

The draft Source witnesses used the pinned actual TexParser lexical methods,
TexError formatter, ParseUtil methods and MML factories, with hashes recorded
in their packets. These are component observations, not full TeX rendering.
The parser/stack/packages, node properties, complete fonts, shared layout,
Renderer ownership loop, 150 full SVG pairs and Folia integration remain open.
