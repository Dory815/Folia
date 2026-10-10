# P4/P5 root-frozen numerical prerequisites

These Wave-1 packets are scoped dependencies of Common wrappers and SVG
serialization, not permission to implement unreviewed parser/layout classes.
Root owns this contract, frozen tests, oracle, scope and build integration.

## P4: native number formatting host adaptation

Owner: `math_layout`. Only writable files:
`src/math/util/js_number.ixx` and `evidence/p4-js-number.md` under this directory.
Module `folia.math.util.js_number`; import std; export namespace
`folia::math::js_number`, functions `std::string toString(double)` and
`std::string toFixed(double, unsigned digits=0)`.

MathJax calls JavaScript Number formatting in util/lengths.ts and SVG's fixed
geometry helpers. This module adapts those host-language operations to C++; it
does not execute JavaScript. Reference is the pinned Node 24.13.0 Number behavior,
cross-checked with ECMAScript Number::toString and Number.prototype.toFixed:

- https://tc39.es/ecma262/multipage/ecmascript-data-types-and-values.html#sec-numeric-types-number-tostring
- https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.prototype.tofixed

toString uses shortest round-trippable decimal digits, fixed notation for
1e-6 <= abs(x) < 1e21, scientific otherwise, positive exponent `+`, no exponent
leading zeroes, `NaN`, `Infinity`, `-Infinity`, and zero including -0 as `0`.
Do not use default iostream formatting or std::to_string. std::to_chars shortest
digits may be used with an explicit ECMAScript notation transformation and
must pass the frozen bit-pattern differential corpus.

toFixed validates digits 0..100 before non-finite checks; unsigned >100 throws
std::out_of_range (native RangeError adaptation). Nonfinite values use toString.
For abs(x)>=1e21 use toString; otherwise round the exact binary64 magnitude to
the nearest integer after multiplying by mathematical 10^digits, choosing the
larger integer on a tie. Preserve all requested trailing zeroes. Negative
nonzero input retains `-` even if rounded to zero; negative zero itself has no
sign. Multiplying a double by pow(10,digits) before rounding changes the exact
value and is prohibited. Use exact integer significand/exponent arithmetic;
do not add epsilons, per-example special cases, locale dependence or floating
point environment changes. Helpers remain private to this one module.

## P5: lengths.ts source port

Owner: `math_fonts`. Only writable files:
`src/math/util/lengths.ixx` and `evidence/p5-lengths.md`.
Source `ts/util/lengths.ts`, SHA256
`213f65e9a8fa6d30bda4df4a6c0cc3a1e8d50567267539d85a1f19c1c20cea73`.
Module `folia.math.util.lengths`; import std, constants, property and js_number.
Export namespace `folia::math::lengths` with these exact interfaces:

```cpp
inline const std::array<std::pair<std::string_view,double>,4> UNITS = /* source */;
inline const std::array<std::pair<std::string_view,double>,5> RELUNITS = /* source */;
inline const std::array<std::pair<std::string_view,double>,21> MATHSPACE = /* source */;
Property length2em(std::string_view length, double size=0, double scale=1, double em=16);
Property length2em(double length, double size=0, double scale=1, double em=16);
std::string percent(double m);
std::string em(double m);
std::string px(double m, double M=-BIGDIMEN, double em=16);
```

BIGDIMEN remains in the root constants module, with no duplicate definition.
The native read-only numeric arrays are an explicit host adaptation: root
searched all upstream ts/ references to these names; only lengths and base,
AMS, braket and textmacros mappings read them, and no source writes them.
The raw-TeX native API has no JS module-export mutation API. This does not
authorize changing their records or treating options/attributes as immutable.

All four functions and all table entries are retained. length2em returns Property
because the upstream plain MATHSPACE object exposes inherited Object.prototype
identities for malformed names such as `constructor`, `toString`, and
`__proto__`; a double-only interface would silently lose those observable
results. Represent these through the already-frozen BuiltinProperty identities,
not a numeric fallback. Normal results contain double. Future Common wrappers
must explicitly apply their upstream numeric coercion at the consuming boundary.

Strings at this native boundary are UTF-8. The anchored non-global regex
matches a **prefix**, accepts an optional number and optional unit, does not
recognize exponents, does not require the full string, and has no whitespace
between numeric and unit groups. Preserve optional-number default 1, empty
match fallback, all IEEE comparisons/divisions, expression order, named-space
lookup before the regex, own-only unit membership, and scale only on absolute
units. A scanner reproducing this regex is permitted; a stricter CSS parser is
not. Recognize ECMAScript whitespace (TAB, VT, FF, space, NBSP, BOM, Unicode Zs,
LF/CR/LS/PS); U+180E and U+200B are not whitespace. Decimal prefix conversion
is locale-independent and preserves overflow/underflow and signed zero.
Numeric overload first uses js_number::toString, so exponential numeric inputs
still undergo the same prefix regex, rather than direct arithmetic.

percent/em/px preserve source threshold checks and different suffix stripping
regexes. They use P4's exact toFixed; thresholds, nonfinite values, minimum
clamping including M=0/NaN, -0 and exponents are frozen in upstream fixtures.

## Acceptance before either agent may claim completion

Root first generates `tests/math/reference_numeric.cpp` with
`node tools/math-port/numeric-oracle.cjs`. It takes every expected result from
upstream lengths.js or Node Number, never native implementations. Native agents
can read but cannot edit the oracle/tests/contracts/build files. No shared builds,
commits, new dependencies, Windows APIs, production edits or source shortcuts.
P5 may write against P4's frozen signature while P4 implementation is pending;
root does not accept P5 until both modules compile and pass independently.
If the contract is insufficient, report the exact source behavior instead of
changing it. Completion includes per-method mapping, adaptation rationale and
uncovered behavior. Root reviews source/diff and runs the filtered core suite;
exact strings and table identities, full-precision numeric comparison, seeded
input/bit-pattern cases and isolated comparator mutations must pass.
