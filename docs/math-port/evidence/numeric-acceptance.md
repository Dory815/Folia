# P4/P5 root acceptance

Status: numerical prerequisites accepted locally for the native raw-TeX port;
complete parser, Common layout, shared scene, SVG backend and Folia switch
remain unaccepted.

Root reviewed every lengths.ts export/table and the complete native helper
against the frozen `../numeric-contracts.md` contract. The accepted source is
MathJax 4.1.3 lengths.ts at SHA256
`213f65e9a8fa6d30bda4df4a6c0cc3a1e8d50567267539d85a1f19c1c20cea73`.
P4 adapts ECMAScript Number formatting rather than porting an additional TS
file. The helper enforces IEEE binary64 at compile time; its exact toFixed
algorithm operates on integer significand/exponent data, not rounded
double-times-power arithmetic. to_chars uses explicit general format before
ECMAScript notation transformation. No other toolchain's shortest formatting
conformance is inferred from this build.

Root's source review rejected the initial default to_chars format (it can favor
an exact fixed integer over minimum significant digits) and missing API default
arguments. Both agents corrected only their assigned files. A generator defect
in inherited-name lookup was also fixed by root: C++ identity labels now use
own-only lookup, rather than accidentally reading Object.prototype functions
from the generator's rename map. Expected runtime identities did not change.

## Independent verification

- `numeric-oracle.cjs` verifies clone/source-package equality and creates expected
  results only from the pinned upstream lengths.js and Node 24.13.0 Number.
  `--check` verifies the frozen C++ fixture without rewriting it.
- 2,092 boundary/seeded binary64 samples test exact shortest/fixed strings with
  precisions 0..100, special values, subnormals, negative zero, decimal halfway
  cases and notation boundaries. Length tests check all records, actual prefix
  behavior, every ECMAScript whitespace category, excluded whitespace, malformed
  strings, overflow/underflow, own versus inherited lookups, expression ordering,
  non-unit/zero em and scale, formatter suffixes and default arguments.
- Filtered `cmd /c build_test.bat "*math_port*" 2>&1` passes 15,084 assertions in
  the two new tests, plus all 1,223 assertions in the three existing primitives.
  The full core suite also passes. The final binary64 guard compiles and the
  filtered tests pass after that change. No platform/app integration changed.
- `numeric-mutation-test.cjs` builds isolated copies under ignored
  `build/math-port-numeric-mutations`, first proving a passing baseline. Six
  changes each compile and fail the frozen assertions: wrong notation threshold,
  dropped half rounding, dropped absolute-unit scale, widened em zero threshold,
  dropped percentage reference size, and direct numeric arithmetic bypassing
  the upstream String/prefix path. Working native files are never mutated.

Normal verification:

```powershell
node tools/math-port/numeric-oracle.cjs --check
cmd /c build_test.bat "*math_port*" 2>&1
node tools/math-port/numeric-mutation-test.cjs
```

## Explicit adaptation and remaining boundaries

The read-only numeric tables preserve all records. Upstream exports mutable
objects; root's all-TS search found only readers in lengths/base/AMS/braket/
textmacros and no writes, and the requested native raw-TeX API exposes no JS
module-export mutation. This approved host adaptation does not make general
options, semantic attributes or node properties immutable.

length2em retains nonnumeric Object.prototype results as Property identities
instead of substituting a number. The typed utility API accepts strings/numbers;
future Common wrappers must handle their broader Property inputs explicitly,
including bool/Missing string coercion and numeric consumption of malformed
attribute values. Input offsets/UTF-16 parser semantics are not implemented by
this local UTF-8 whitespace scanner. Arbitrary malformed UTF-8 and custom JS
module/table mutation are not claimed as supported APIs.

The scope has four reviewed source files out of 246; 242 remain unreviewed.
This utility gate does not pass full G0, exhaustive parser/font/layout/scene
coverage, raster verification or production replacement. E7 Options proposals
have 11 scenarios and verified source hashes, but its recursive value contract
and implementation remain pending.
