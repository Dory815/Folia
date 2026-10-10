# P10 string utility implementation evidence

This is an implementation ledger, not acceptance. The source is MathJax 4.1.3
`ts/util/string.ts` (SHA-256
`024a2ac6f96a314884f2bd71a913060c06fb75d46cbf6c94e15f4e71643a7084`); the
installed CJS counterpart is
`tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/string.js` (SHA-256
`e1f09bca257dd268b9483b7310e4c77039dd022f01c5e9f5a62f47a981921e9c`).
The implementation is in `src/math/util/string.cpp` and uses the frozen public
signatures in `string.ixx`.

| TypeScript member | Native method | Ported behavior and host adaptation |
|---|---|---|
| `sortLength` (line 31) | `sortLength` | Descending UTF-16 code-unit length, then equality and lexicographic UTF-16 order. Returns the length difference as `double`. |
| `quotePattern` (47) | `quotePattern` | Adds a backslash for exactly `^ $ ( ) { } . + * ? - \| [ ] : \`. Slash and all other units pass through. Operates on UTF-16 units, including isolated surrogates. |
| `unicodeChars` (57) | `unicodeChars` | Uses `utf16::codePoints`; valid pairs become one point and isolated surrogates remain their numeric code-unit values. Returns the contract's dense `vector<double>`. |
| `unicodeString` (67) | `unicodeString` | Validates each `double` as finite, integral, and in `[0, 0x10FFFF]`, then emits UTF-16. Surrogates and negative zero are accepted. Invalid values throw `Error("RangeError", "Invalid code point …")`. A numeric span is the frozen typed boundary; arbitrary JS iterators/getters and engine argument-stack limits are outside this binding. |
| `isPercent` (77) | `isPercent` | Removes trailing ECMA whitespace with `js_input::isWhitespace`, then tests for `%`. This matches `/%\\s*$/` for string inputs. |
| `split` (87) | `split` | Trims and separates on ECMA whitespace using `js_input::isWhitespace`. Empty or all-whitespace input returns one empty string, matching `trim().split(/\\s+/)`. |
| `replaceUnicode` (97) | `replaceUnicode` | Scans left to right for the source regex alternatives: bare `\\U` plus four hex units; braced `\\U{…}` with 1–6 hex digits and optional ECMA whitespace; otherwise backslash plus one non-line-terminator UTF-16 unit. The only generic match accepted is `\\\\`, which yields one backslash. Other generic matches and malformed `\\U` forms throw `RangeError` for NaN conversion. Valid escapes use the existing hex `parseInt` host helper and fromCodePoint-equivalent validation. Output is appended once and never rescanned. A terminal backslash or a backslash before LF, CR, U+2028, or U+2029 remains unchanged. |
| `toEntity` (111) | `toEntity` | Uses `utf16::codePointAt(text, 0)`, emits uppercase minimal hexadecimal `&#x…;`, and ignores remaining units. Empty input throws `TypeError`, matching the source's undefined `.toString()` access. |

`replaceUnicode` preserves the source regex's leftmost, non-overlapping behavior.
For example, bare `\\U12345` consumes only the four hex digits and leaves `5`;
an invalid braced form falls through to the generic `\\U` match and throws.
An escaped astral character is handled as one UTF-16 unit by the fallback,
then throws, as the non-Unicode regex dot does in JavaScript. `parseInt` uses
explicit hexadecimal radix, and code-point validation admits surrogate values
while rejecting values above U+10FFFF.

Direct MathJax call sites identified in the pinned source tree:

- `sortLength` and `quotePattern`: `input/tex/FindTeX.ts:26,122,152,165,224`;
  `quotePattern` also appears in `input/asciimath/FindAsciiMath.ts:26,100-101`.
- `unicodeChars`: `output/svg.ts`, `output/chtml.ts`, `output/common/Wrapper.ts`,
  `output/common/Wrappers/mo.ts`, and `core/MmlTree/MmlNodes/mo.ts`.
- `unicodeString`: `core/MmlTree/MmlNodes/mo.ts:41,540`.
- `isPercent`: `output/common/Wrappers/mtable.ts` and
  `output/chtml/Wrappers/mtable.ts`.
- `split`: XML, table, menclose, maction, multiscripts, and SVG wrappers under
  `output/common`, `output/chtml`, `output/core`, and `output/svg`.
- `replaceUnicode`: `input/tex/BaseMethods.ts:45,1130`.
- `toEntity`: `core/MmlTree/SerializedMmlVisitor.ts:28,162-173`.

Implementation imports only the existing UTF-16 and numeric-input helpers and
the frozen `options::Exception` alias, plus `js_number::toString` for the
finite invalid-code-point suffix so its text follows ECMAScript Number
formatting (including large values such as `1e20`). NaN and infinities use
their ECMAScript spellings directly. It adds no MML, renderer, platform, or
JavaScript-runtime dependency. I did not run builds or tests; root owns those
checks and acceptance. No known gap remains against the frozen typed contract.
