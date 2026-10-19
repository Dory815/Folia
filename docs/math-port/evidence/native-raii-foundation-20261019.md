# Native RAII foundation: first native observation

This is a partial implementation observation, not full MathJax or application
acceptance. The controlling architecture and gates are the native RAII documents.

The single root `folia_math` graph was replaced with typed MML ownership,
session/equation values, immutable Scene, pure BBox/constants, native length
conversion/decimal formatting, and the SRELL scanner service. Its source and
module closure no longer includes the withdrawn Context/object runtime.

The MML vocabulary matches all 43 pinned `ts/core/MmlTree/MML.ts` registrations.
`MmlDocument` owns stable nodes and is the primary named module; `MmlNode` is
its exported partition. Detachment/reparenting does not transfer ownership out
of the document. Mathematical family algorithms are still missing.

`TexSessionState` owns persistent macro values, labels, IDs and counters.
Request-local `TexEquationState` owns current equation scratch. Macro expansion
is deliberately not implemented in the session module; it belongs to ParseUtil.
Source end-to-end persistence/error witnesses remain required.

Scene snapshots the ordered group hierarchy, permits signed mathematical
metrics, preserves typed paint states, rejects cycles, and shares only immutable
glyph resources. It performs no TeX or mathematical placement.

## Commands and results

MSVC 14.50.35717, Debug, existing `build/core`, Ninja, at most four jobs:

```text
cmake -Wno-dev -G Ninja -S . -B build/core
cmake --build build/core --parallel 4 --target FoliaMathTests
build/core/FoliaMathTests.exe
```

Configuration returned 0. The first two builds returned 1 due to test-source
compilation errors: Boost.UT comparison lookup for two string views and integer
spellings of floating-point literals. Root corrected these tests. The third
build returned 0; all production units compiled. Native execution returned 0:
38 tests / 378 assertions across the foundation and the separately committed
SVG consumer tests. No abnormal native exit occurred.

Build/test logs are ignored in `build/math-port/native-raii-*.log`.
All console tests link `folia_console_test_runtime`.

Independent development-time Source witnesses cover the new decimal conversion
cases and regex expectations. Those are paired with native UT assertions; they
do not substitute for the complete 150-record SVG comparison.

## Incomplete gates

The full parser, 38 package handlers, MML family semantics, immutable font
repository, CommonWrapper measurements/placement, Renderer, C ABI and real
application worker acceptance remain incomplete. No claim is made about
request-level allocation stability or complete SVG parity yet. Old test counts
are not reused. Folia still uses its current math worker.
