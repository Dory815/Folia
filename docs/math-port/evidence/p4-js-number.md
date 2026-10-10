# P4 JavaScript Number formatting host adaptation

Implementation: `src/math/util/js_number.ixx`, module `folia.math.util.js_number`.
The only exported operations are `toString(double)` and `toFixed(double,
unsigned = 0)`. This file adapts the JavaScript Number operations that MathJax
uses at numeric-string boundaries; it does not provide a JavaScript runtime.

## Method ledger

| Export | Contract behavior | Native method |
| --- | --- | --- |
| `toString(double)` | ECMAScript shortest round-trip decimal; special values and signed zero; fixed/scientific notation threshold, exponent sign, and exponent spelling | `format_shortest` handles nonfinite/zero values, obtains shortest digits with `std::to_chars`, parses the decimal significand and exponent, then applies ECMAScript's `1e-6 <= abs(x) < 1e21` presentation rule. |
| `toFixed(double, unsigned)` | Validate 0–100 first; nonfinite and `abs(x) >= 1e21` use Number string conversion; otherwise exact decimal rounding, ties upward, fixed fractional digit count, and sign rules | `toFixed` decodes the binary64 sign/exponent/fraction and delegates finite in-range magnitudes to `format_fixed_magnitude`. |

Private arithmetic helpers:

| Helper | Responsibility |
| --- | --- |
| `BigUnsigned` | Little-endian base-2^32 unsigned integer used only for exact scaled magnitudes. |
| `multiply_small` | Exact multiplication by 5 for each requested decimal digit. |
| `shift_left` / `shift_right` | Exact powers-of-two multiplication/division; right shift is floor division. |
| `bit` | Tests the half-denominator bit used by round-to-nearest, ties-up. |
| `increment` | Adds one to the truncated quotient when the discarded remainder is at least one half. |
| `decimal` | Converts the integer exactly by repeated base-1,000,000,000 division. |
| `format_fixed_magnitude` | Combines decimal scaling, exact rounding, and requested fractional zero padding. |
| `format_shortest` | Normalizes the implementation's shortest `to_chars` result to ECMAScript notation. |

## Exact `toFixed` arithmetic

For a finite nonzero binary64 magnitude, the decoded value is
`significand * 2^binary_exponent`. Scaling by `10^digits` is represented as
`significand * 5^digits * 2^(binary_exponent + digits)`. When the final power
of two is nonnegative, the integer is formed by exact multiplication and left
shift. When it is negative, the big integer is divided by the corresponding
power of two; its bit immediately below that denominator determines whether
the remainder is at least one half. Incrementing the quotient therefore
implements the specified tie-up choice without evaluating `double * pow(10,n)`.
The final integer is converted to decimal and padded/pointed without further
floating-point operations.

The sign is taken from the original nonzero input after magnitude rounding,
so a negative value that rounds to zero retains `-`. `value == 0` is handled
before decoding and returns unsigned zero text for either signed zero.

## Branches and adaptations

- `toFixed` rejects `digits > 100` before checking NaN or infinity.
- `toString` emits `NaN`, signed infinities, and `0` for both zero signs.
- `toFixed` delegates nonfinite inputs and magnitudes at least `1e21` to
  `toString`; all remaining finite values use exact significand arithmetic.
- The binary64 normal path restores the implicit leading significand bit and
  uses exponent `storedExponent - 1023 - 52`; the subnormal path uses the raw
  fraction with exponent `-1074`.
- `std::to_chars` is used only for the shortest round-trippable decimal digits.
  The adapter parses its optional exponent and decimal point, removes redundant
  leading/trailing zeroes, then applies ECMAScript's notation thresholds and
  exponent spelling. It does not use iostream formatting or locale state.
- A compile-time assertion requires `double` to be IEC 559 binary64 (`sizeof`
  8, 53 significand bits, `max_exponent` 1024), as required by the bit decoder.
- A small private base-2^32 integer is sufficient: the contract bounds the
  decimal precision to 100 and routes values `>= 1e21` away from exact fixed
  formatting. Intermediate arithmetic is exact and bounded to a few hundred
  bits.

## Evidence and remaining verification

The frozen contract is `docs/math-port/numeric-contracts.md` (P4). The frozen
reference corpus is `tests/math/reference_numeric.cpp`; root reports that all
15,084 numeric and lengths assertions passed on its current toolchain, along
with the full core suite. Neither frozen file was changed. This result supports
the tested toolchain only; it is not a blanket claim of identical shortest
`to_chars` behavior across standard libraries.

The contract's differential corpus remains the acceptance test for any
implementation-specific `std::to_chars` shortest-digit differences on other
toolchains. No other numeric formatting gaps are claimed resolved beyond the
frozen corpus and current-toolchain run reported above.
