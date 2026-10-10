# P8a numeric input evidence

This packet implements only the primitive string-to-number host operations in
`src/math/util/js_numeric_input.ixx`, under the frozen
`docs/math-port/input-host-contracts.md` revision 1. It is a native host
adaptation for operations supplied by the JavaScript runtime; it does not port
an algorithm from MathJax. Pinned source commit: MathJax 4.1.3,
`fb987178c2d279a99b0db5ea02e751691703955e`.

## Branch and method ledger

| Export/branch | Behavior |
| --- | --- |
| `number(u16string_view)` trim | Trims the ECMAScript WhiteSpace and LineTerminator code points explicitly, including BOM; does not use locale classification. Empty-after-trim returns positive zero. |
| Number special values | Recognizes exact `Infinity` with an optional sign. Other spellings, including `NaN`, return quiet NaN. |
| Number non-decimal integer | Accepts only unsigned `0x`, `0o`, and `0b` prefixes, requires at least one valid digit, and consumes the full remainder. A sign before such a prefix is invalid. |
| Number decimal grammar | Checks full consumption of optional sign, integer/fraction digits, and optional exponent. Invalid characters or incomplete exponents return NaN. `std::from_chars` performs locale-independent binary64 decimal conversion; range errors become signed zero or signed infinity according to the decimal magnitude. Exponent accumulation saturates at a checked input-length-derived bound, not a fixed numeric cutoff. |
| `parseInt` radix validation | Accepts only `Automatic`, `Octal`, `Decimal`, and `Hex`; every other enum value throws `std::invalid_argument` before parsing. |
| `parseInt` prefix/sign | Trims only leading ECMAScript whitespace, consumes an optional sign, selects decimal by default and hexadecimal for an automatic `0x` prefix, and recognizes an optional `0x` prefix for explicit hexadecimal. Other explicit radices do not consume prefixes. |
| `parseInt` digit scan | Reads the maximal digit prefix in the selected radix; invalid/no digits yield NaN. Trailing input is permitted. A negative zero result preserves its sign. |
| Integer rounding | All accepted non-decimal Number inputs and parseInt results accumulate exact unsigned integer limbs, retain the top 53 bits plus guard/sticky information, and round once to nearest-even. A carry into the exponent and overflow to infinity are handled before constructing binary64 bits. No per-digit floating rounding occurs. |

## Correspondence to pinned callers

The contract's source consumers are the implicit JS `Number(string)` and
`parseInt(string, radix)` operations. Direct parseInt sites in the pinned tree
include `mstyle.setChildInheritedAttributes` (lines 82–98),
`maction.nextToggleSelection` (around line 130), common `mglyph` (line 220),
common `mtable.getAlignmentRow` (line 1342), common `Wrapper` (line 950), and
common `FontData` (lines 843 and 1095). The inspected calls use the JS default
radix. The frozen interface additionally supports the explicitly scoped radix
8, 10, and 16 forms from the host contract; it does not expose arbitrary bases.

`MmlNode.texSpacing` compares raw script-level properties with `> 0` (pinned
`MmlNode.ts`, lines 726–740). That comparison applies JavaScript relational
coercion to Property values; this module does not implement that operator or
object-to-primitive conversion. `mstyle` first calls `.toString()` on its
explicit scriptlevel before parseInt. The input API accepts the resulting
UTF-16 string, while dynamic property coercion remains a separate host concern.
No direct `Number(...)` call was found in the inspected MML/common directories;
`number()` provides the frozen Number(string) primitive API and its oracle
corpus rather than claiming a specific direct MathJax call site there.

## Exact adaptation and limitations

`std::from_chars` is used only after explicit ECMAScript grammar validation and
is independent of the process locale. Integer prefixes use native exact
bit accumulation instead of converting through decimal text or repeatedly
rounding a floating accumulator. The implementation does not mutate the
floating-point environment and has a compile-time guard for IEC 559 binary64.
Decimal conversion relies on the platform's
`from_chars` binary64 implementation for correctly rounded conversion; the
root-owned Node oracle is the acceptance check for this compiler/runtime.

For a decimal that `from_chars` reports out of range, the implementation needs
only the sign of `explicitExponent + digitsBeforeDot - firstNonzeroDigit - 1`
to distinguish underflow from overflow. It accumulates the exponent magnitude
with checked multiply/add and saturates at `text.size() + 4096`, saturating the
bound itself at `size_t` maximum if that addition would overflow. The absolute
mantissa offset is at most `text.size() - 1`; therefore a saturated exponent
is strictly larger than any offset that could cancel it. Values below the
saturation bound are accumulated exactly. This cap only classifies a
`from_chars` range error: the complete validated decimal spelling is still
passed to `from_chars` (with a leading plus removed because its grammar omits
that sign), without truncation or syntax limits.

This is not a full JavaScript numeric runtime. There is no arbitrary-radix
parseInt, general value/object coercion, Number object semantics, or implicit
conversion of `Property` values. Inputs are already UTF-16 strings. Invalid
UTF-16 code units that are not recognized ECMAScript whitespace make Number
input invalid; parseInt stops at them as non-digits. The source MML relational
comparison and `.toString()` dispatch are not implemented by these two
functions.

No build or test run was performed in this packet; root owns compilation,
oracle comparison, mutation checks, and acceptance. The frozen
`tests/math/reference_input_host.cpp` and its generator were read-only.

## Source hashes

| Pinned source | SHA-256 |
| --- | --- |
| `ts/core/MmlTree/MmlNode.ts` | `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656` |
| `ts/core/MmlTree/MmlNodes/mstyle.ts` | `7442e2885d2f4605d42f5b4472b3183f9cbadbd8ed5c0aeddf03b8f23c1d8616` |
| `ts/core/MmlTree/MmlNodes/maction.ts` | `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895` |
| `ts/output/common/Wrappers/mglyph.ts` | `3a4d41fe473fbb370b7a906b8631584bb8f2cee210c9a4e89041edbe7c928a1a` |
| `ts/output/common/Wrappers/mtable.ts` | `a1e060baab84d7b842c811c560ab73f32ca91614e3a5b02f7f1712db73b0100c` |
| `ts/output/common/Wrapper.ts` | `d1db6112db1cb3546658eb8868b5997a4f69e588632ecbe4e08a71949544a828` |
| `ts/output/common/FontData.ts` | `7d8eeca536fa1b7c14517590b9c5a372ad10b34667d196d09b030f88cda780b2` |
