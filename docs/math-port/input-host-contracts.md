# P8a: MML/input primitive host services — revision 1

Owner: root. This is a prerequisite host adaptation, not acceptance of
MmlNode.ts, a parser, or a renderer. It provides the string operations that the
pinned runtime supplies implicitly. It does not evaluate JavaScript and does
not introduce a second dynamic value graph.

## Source consumers and exact boundaries

Pinned MathJax 4.1.3, commit fb987178c2d279a99b0db5ea02e751691703955e:
MmlNode.ts texSpacing compares raw scriptlevel values with zero; MmlNodes/mstyle.ts
uses parseInt for relative scriptlevel; TextNode stores UTF-16 strings;
common/SVG mo wrappers and TexParser use codePointAt; util/string.ts
unicodeChars and common scriptbase use Array.from.
E11 records these dependencies. No algorithm from the rejected prototype is
permitted. This packet implements string primitives only; object-to-primitive
conversion and Property/Options adapters require a separately reviewed caller
contract. Agents cannot infer those conversions or add fallback values.

## Files and frozen exports

- `src/math/util/js_numeric_input.ixx`, module
  `folia.math.util.js_numeric_input`, namespace `folia::math::js_input`:
  `double number(std::u16string_view)` implements Number(string);
  `enum class IntegerRadix { Automatic=0, Octal=8, Decimal=10, Hex=16 };`
  `double parseInt(std::u16string_view, IntegerRadix= IntegerRadix::Automatic)`
  implements parseInt(string, radix) for these four scoped caller forms.
  Other enum values reject with std::invalid_argument; no implicit value coercion.
- `src/math/util/utf16.ixx`, module `folia.math.util.utf16`, namespace
  `folia::math::utf16`: `std::optional<char32_t> codePointAt(std::u16string_view,
  std::size_t)` and `std::vector<char32_t> codePoints(std::u16string_view)`
  preserve unpaired surrogate values and count indices in UTF-16 code units;
  `Input decode(std::string_view)` accepts strict UTF-8;
  `std::string encode(std::u16string_view)` matches TextEncoder replacement of
  each unpaired surrogate. This output conversion is explicitly lossy at an
  external UTF-8 boundary; internal text always remains UTF-16.
- Root-owned `utf16_types.ixx` exports Input {u16string text;
  vector<size_t> byteOffsets;} and DecodeError {size_t byteOffset;}, a
  std::runtime_error subclass. The offset table contains text.size()+1 entries.
  Each code unit maps to its originating UTF-8 sequence start; both units of a
  surrogate pair map to the same start. The final entry equals input byte size.
  Diagnostics use this conversion only, never editor source selection.

Number parsing preserves signed zero and exact binary64 rounding, including
subnormals, overflow, decimal halfway values and unsigned hex/octal/binary
integers beyond 64 bits. Number consumes the entire trimmed string. parseInt
consumes the permitted integer prefix, with source sign, radix rules and NaN
on no digits; decimal/octal/hex results round once, not once per digit. Both follow the
reference runtime's whitespace, not locale-dependent C classification.
The retained MathJax callers use automatic, 8, 10 and 16. This is intentionally
not a generic arbitrary-radix JS runtime service: the pinned V8 parseInt base-36
result for `1` followed by 100 zeroes differs from correctly rounded BigInt
conversion. That behavior is not silently represented as an exact-rounding
contract. Additional radices require separate caller evidence and verification.
Invalid UTF-8 rejects at the first invalid sequence's start (including stray
continuation, overlong, surrogate, truncated and >U+10FFFF sequences). No
replacement decoding and no normalization. encode never modifies input.

## Ownership and dispatch

Root freezes this contract, types and independently generated expectations
before dispatch. math_layout owns only js_numeric_input.ixx and
evidence/p8a-numeric-input.md. tex_parser owns only utf16.ixx and
evidence/p8a-utf16.md. math_fonts owns only the E11 audit until a separate
review task is issued. Agents cannot change signatures, types, contracts,
fixtures/oracle, CMake, scope, production, other packets, or build/commit.
Missing prerequisites must be reported; root resolves them before implementation.

## Acceptance owned by root

`tools/math-port/input-host-oracle.cjs` executes actual Node 24.13.0 Number,
parseInt, codePointAt, Array.from, TextEncoder and fatal TextDecoder operations.
It freezes bit-exact numeric expectations (all NaNs equivalent), UTF-16 units,
codepoints, byte output and boundary-offset assertions independently of native
code. It includes fixed edge cases and deterministic decimal/radix/string
generation. `--check` must prove frozen fixtures unchanged.
Root reviews every branch, runs `cmd /c build_test.bat "*math_port_input_host*"
2>&1`, and uses compiling mutations to prove signed-zero, whole-string parsing,
radix/rounding, surrogate iteration, invalid UTF-8 and offset errors fail.
No numeric tolerance, implementation-derived expected limit, copied heuristic,
or test change to fit native output is accepted. Root then runs the core suite
and records gaps and provenance. This packet cannot pass G1–G7 by itself.

## Reviewed caller obligation before MML implementation

`MmlMaction.nextToggleSelection` passes attributes.selection to parseInt without
an actual string conversion (the TS cast is erased); its default is the number
1 and the enabled action/toggle path consumes it. Root's disposition is to
preserve that path through an explicitly reviewed value-to-string adapter in
the MML contract, using the accepted Number formatter and this string parser.
No interaction/package exclusion is authorized. Raw scriptlevel relational
conversion and mstyle's toString likewise need their own typed caller rules.
The present string-only helper is not acceptance of any of these adapters.
