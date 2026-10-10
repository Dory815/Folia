# P8a input-host contract and implementation review

Independent read-only review of root's P8a host contract, generated oracle,
frozen native fixture, and the two implementation modules as they appeared
during review. Pinned upstream is MathJax 4.1.3 at
`fb987178c2d279a99b0db5ea02e751691703955e`; the oracle pins Node
`v24.13.0`. This review does not accept P8a or change its contracts.

## Findings requiring root disposition

### The string-only `parseInt` interface misses an enabled runtime caller

The frozen API takes `std::u16string_view`, and the contract explicitly
defers JS value coercion. In the scoped production tree, however,
`MmlMaction.nextToggleSelection` calls
`parseInt(this.attributes.get('selection') as string)` at
`ts/core/MmlTree/MmlNodes/maction.ts:127-135`. The `as string` is erased.
`MmlMaction.defaults.selection` is the number `1` at lines 36-40; the
enabled TeX `action` package's `\toggle` handler creates a `maction` without
overriding that selection (`ts/input/tex/action/ActionConfiguration.ts:34-50`).
The SVG `toggle` action invokes `mml.nextToggleSelection()` on interaction
(`ts/output/svg/Wrappers/maction.ts:215-249`). A fresh Node probe confirmed
`parseInt(1) === 1` and `parseInt('1') === 1`; the current string-view export
cannot itself represent the actual numeric attribute value. This is an
observed caller gap even though the eventual result for the default number is
simple. Root must decide whether the enabled action path gets a reviewed
Property-to-string adapter, or whether the host product explicitly bounds
that interaction path. Do not describe the current string-only signature as
covering the complete scoped `parseInt` call surface until that is resolved.

Apart from this argument-domain issue, a scan of all 246 `scope.json` source
entries found only automatic, 8, 10, and 16 radix forms:

| Call sites | Radix evidence |
|---|---|
| `mstyle.ts:92,94`, `ColumnParser.ts:433`, `ParseUtil.ts:82`, `AmsMethods.ts:263`, `BaseMethods.ts:1872`, `ColorUtil.ts:192`, `ExtpfeilConfiguration.ts:71-73`, `NewcommandUtil.ts:154`, `UnicodeConfiguration.ts:75,172`, `FontData.ts`, `Wrapper.ts`, `mglyph.ts`, `mtable.ts`, `svg/FontData.ts`, `Entities.ts` | `parseInt(value)` automatic |
| TeX numeric argument, placeholder, style, size and error parsing sites in `AmsMethods.ts`, `BaseMethods.ts`, `ParseUtil.ts`, `TexError.ts`, `NewcommandMethods.ts`, and `PhysicsMethods.ts` | explicit radix 10 |
| `NodeUtil.ts:58`, `ParseUtil.ts:556`, `UnicodeConfiguration.ts:120,146`, `util/Entities.ts:523`, `util/string.ts:101` | explicit radix 16 |
| `UnicodeConfiguration.ts:141` | explicit radix 8 |
| all scoped files | No explicit radix 2 or any other radix found |

This scan is over the manifest's scoped files, not an unrestricted whole
repository search. The `mstyle` caller first converts `scriptlevel` to a
string at `mstyle.ts:89-95`; the action selection path above does not.

## Fresh Node probes

I ran read-only probes under Node v24.13.0. For each exponent in
51, 52, 53, 54, 63, 64, 1022, 1023, 1024, and 2048, and each delta -3 through
+3, I compared `parseInt(n.toString(radix), radix)` with `Number(n)` for
BigInt `n = 2^exponent + delta`, for radices 8, 10, and 16. All 210
comparisons had identical binary64 bit patterns. Decimal halfway probes
confirmed the exact tie (`...3125`) rounds to even 1.0, while the next
decimal value (`...3126`) rounds to the next binary64 value. `-0`,
`-0e999999`, and `-0e-999999` all retained the negative-zero bit pattern.
These results support the selected radix/rounding contract; they do not
execute or validate the C++ implementation.

The contract's base-36 caveat is real: for `s = '1' + '0'.repeat(100)`,
Node `parseInt(s, 36)` and a correctly rounded `Number` conversion of the
exact base-36 BigInt differ in low bits (`603fd5863c3eb043` vs
`603fd5863c3eb047`). This is outside the scoped radix set, so excluding
general radix 36 is justified.

Whitespace probes confirmed U+FEFF and U+00A0 are accepted around a number
and by `parseInt`, while U+0085, U+180E, and U+200B are not whitespace and
produce NaN in those positions. The generated oracle lists the ECMAScript
white-space and line-terminator code points and includes representative
non-whitespace controls, format characters, and surrogates.

For `A😀é`, Node reports UTF-16 units `[0041,D83D,DE00,00E9]`,
`codePointAt` indices 0..3 as `[65,128512,56832,233]`, and UTF-8 bytes
`[65,240,159,152,128,195,169]`. The required source-byte map is
`[0,1,1,5,7]`: both UTF-16 code units for the emoji map to its first byte,
and the final entry equals byte length. At the low-surrogate index,
`codePointAt` returns that low surrogate's numeric code unit, while
`Array.from` visits the valid pair as one scalar. Encoding isolated
surrogates with TextEncoder emits U+FFFD for each isolated unit and preserves
valid pairs. These probes agree with the contract's UTF-16 indexing and
UTF-8 boundary rules.

The fatal `TextDecoder` probe rejected a malformed continuation sequence
`61 E1 80 20` and a truncated sequence `61 F0 9F 98`. Node's exception is a
`TypeError` and exposes no byte offset. Therefore the contract's custom
`DecodeError.byteOffset` at the invalid sequence's lead byte is a deliberate
native diagnostic convention, not an offset copied from the JS exception;
the fixture independently freezes that convention by placing each malformed
tail after a known valid prefix.

## Oracle and fixture coverage

`node tools/math-port/input-host-oracle.cjs --check` passed under the pinned
runtime and reported 3,793 assertions: 1,007 Number inputs, 932 parseInt
assertions, 112 UTF-16 strings, 68 valid UTF-8 decodes, and 60 invalid UTF-8
cases. It reads the frozen C++ fixture, regenerates from actual Node built-ins,
and compares normalized line endings; it does not import either C++ module.
Expectations compare exact double bits, with NaNs treated as equivalent.
The UTF tests include isolated/pair-surrogate cases, every UTF-16 index
through end-plus-one, UTF-8 sequence boundary examples, BOM preservation,
randomized valid code points, invalid-lead/continuation/overlong/surrogate/
truncated/out-of-range inputs, and byte offsets after valid prefixes.

The recently added BigInt boundary cases cover `Number` at powers 2^51,
2^52, 2^53, 2^54, 2^63, 2^64, 2^1022, 2^1023, 2^1024, and 2^1100 with
nearby deltas, using hex/octal/binary prefixes; parseInt cases cover decimal,
hex, and octal strings around 2^52, 2^53, 2^54, 2^63, 2^64, 2^1023, and
2^1024. This usefully targets precision transitions and overflow.

Remaining coverage gaps or limits visible from the generator:

- The 1,500-zero prefix is a `Number("0x...20000000000001")` case. The
  parseInt vector has no comparable thousands-of-leading-zero case, so the
  integer-prefix/limb parser's leading-zero path is not independently frozen
  at that size.
- Deterministic generated decimal fractions use at most 100 digits and
  exponents in [-700,700]; dedicated fixed values cover selected subnormal
  and overflow boundaries, but this is not an exhaustive all-binary64 parser
  proof.
- UTF-16 `codePointAt` tests cover all nonnegative indices up to size plus one.
  The C++ signature intentionally cannot represent JS negative, fractional,
  NaN, or infinite index coercions; scoped callers use integer index zero or
  parser positions. This is an API boundary, not a failed oracle case.
- `TextDecoder` only supplies rejection in Node. Exact lead-byte error
  offsets are host-defined by P8a's independently specified rule, not
  observable from this JS operation.
- Static generated outputs depend on Node's specified ECMAScript behavior
  plus the runtime pin; the `--check` guard proves fixture stability, not
  correctness of unrepresented values or platform `from_chars` behavior.

## Implementation review status

The visible implementation surfaces match the frozen signatures and avoid
JS-engine/runtime dependencies. The UTF-16 implementation explicitly
combines only valid surrogate pairs, preserves lone surrogate values in
`codePointAt`/`codePoints`, maps each decoded code unit to the UTF-8 lead
offset, rejects malformed UTF-8 at the sequence start, preserves BOM, and
uses U+FFFD only in `encode`. The numeric implementation lists the ECMAScript
whitespace set, handles signed/unsigned radix prefixes and negative zero,
uses a limb integer with guard/sticky round-to-nearest-even logic for integer
radix conversion, and uses `from_chars` for decimal input. Source inspection
alone cannot establish bit-exact parity at every midpoint, overflow,
underflow, or standard-library implementation boundary.

No native build or test was run during this review. Root remains responsible
for branch-level review, compiling tests and mutations, the full core suite,
and accepting or revising the stated numeric and UTF-8 error-offset limits.

## Evidence references

| Pinned source | SHA-256 | Relevant lines |
|---|---|---|
| `ts/core/MmlTree/MmlNodes/maction.ts` | `fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895` | defaults and `nextToggleSelection`, 36-40, 127-135 |
| `ts/input/tex/action/ActionConfiguration.ts` | `0e647387a28c524bd2d4b7a57758b78bfc95555297874ecf3e46da8a1d163b1d` | `\toggle` creates maction, 34-50 |
| `ts/output/svg/Wrappers/maction.ts` | `63da035f36f695187d005da6400f0aa3a28c75cbf22acba8daead1ecf11d5d6d` | toggle invokes selection advance, 215-249 |
| `ts/core/MmlTree/MmlNodes/mstyle.ts` | `7442e2885d2f4605d42f5b4472b3183f9cbadbd8ed5c0aeddf03b8f23c1d8616` | explicit scriptlevel string conversion and automatic parseInt, 89-95 |
| `ts/input/tex/unicode/UnicodeConfiguration.ts` | `63e751b6f50d5b9db08bed4385d192fcf53efe1cabc2dd0f9ca04f9c93302597` | auto, octal, decimal, and hex parseInt forms, 75, 120, 141, 146, 172 |
| `tools/math-port/input-host-oracle.cjs` | Root-owned current file; regenerated fixture verified with `--check` | numeric, Unicode and boundary generators, 1-70 |
