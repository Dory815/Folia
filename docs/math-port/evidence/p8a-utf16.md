# P8a UTF-16 primitives implementation evidence

Implementation packet for the frozen P8a API in
`docs/math-port/input-host-contracts.md` revision 1. Source inspection used the
pinned MathJax 4.1.3 checkout at commit
`fb987178c2d279a99b0db5ea02e751691703955e`. This report records the implementation
and source correspondence; root owns builds, frozen-fixture validation, and
acceptance.

## Caller evidence

| Pinned source location | Observed behavior relevant to this packet |
|---|---|
| `ts/core/MmlTree/MmlNode.ts:1436-1469`, `TextNode.getText`, `TextNode.setText` | TextNode stores and returns its JavaScript string verbatim. The host's UTF-16 representation therefore retains code units, including isolated surrogates; it must not normalize when storing internal text. SHA-256 `cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`. |
| `ts/input/tex/TexParser.ts:284-285` | The parser reads `this.string.codePointAt(this.i)` at a UTF-16 string index, then converts the returned scalar/code-unit value with `String.fromCodePoint`. SHA-256 `9afdfff070ec7ed7115a1afe97048b3d1f1befae5b75e8ec963e0456685ee167`. |
| `ts/util/string.ts:57-58`, `unicodeChars` | `Array.from(text)` iterates Unicode code points (pairing valid UTF-16 surrogate pairs) and each one-code-point string is converted with `codePointAt(0)`. This source utility—not `textmacros/TextParser.ts`—is the direct Array.from caller. SHA-256 `024a2ac6f96a314884f2bd71a913060c06fb75d46cbf6c94e15f4e71643a7084`. |
| `ts/output/common/Wrappers/mo.ts:387,669-670` | Operator wrapper obtains a first code point via `codePointAt(0)` and uses `Array.from(c).length` to require one Unicode code point for stretching. SHA-256 `67c7ed182bc8a38139da98e4d42d0350d2386de29a7e5f95095b4161dcab1bcf`. |
| `ts/output/svg/Wrappers/mo.ts:211` | SVG operator wrapper falls back to the first text code point when choosing a stretch variant. SHA-256 `b982073346322e8bff645d39f94eac35c613f8b1e45e857de1fb6a2d88950873`. |
| `ts/input/tex/textmacros/TextParser.ts` | Inspection found no direct `Array.from` or `codePointAt` call in this file. Its hash is `538059c8482022c1a4bc6f8e763fdc21993fa6353f1956655ae11c9271788a30`; the P8a contract's “textmacros/TextParser uses Array.from” wording appears to attribute `util/string.ts` behavior to the wrong caller. |

## Branch-to-implementation ledger

Implementation is confined to `src/math/util/utf16.ixx`; exported signatures
and `Input`/`DecodeError` are the root-frozen declarations in
`src/math/util/utf16_types.ixx`.

| API / branch | Implementation behavior |
|---|---|
| `codePointAt(text,index)`, index at or past `text.size()` | Returns `nullopt`, matching JavaScript `String.prototype.codePointAt` returning `undefined` outside the string. Index units are UTF-16 code units. |
| `codePointAt`, high surrogate followed by low surrogate | Combines the adjacent pair into one supplementary scalar using the standard UTF-16 formula. |
| `codePointAt`, ordinary BMP unit, isolated high surrogate, or isolated low surrogate | Returns that code unit's numeric value as `char32_t`; isolated surrogates are preserved rather than replaced. |
| `codePoints(text)` | Iterates code units using `codePointAt`; advances two units only when a valid pair produced a supplementary scalar. Every isolated surrogate remains one returned value, matching `Array.from(text).map(c => c.codePointAt(0))`. |
| `decode(bytes)`, ASCII and valid 2/3/4-byte UTF-8 | Decodes strictly to UTF-16; supplementary values emit a high/low pair. No locale or platform codec is used. |
| `decode`, invalid lead, stray continuation, C0/C1, F5-FF | Throws the frozen `DecodeError` at the invalid sequence's first byte. |
| `decode`, missing or malformed continuation | Throws `DecodeError` at the sequence lead byte, including when the malformed byte occurs after a valid continuation prefix. |
| `decode`, overlong 3/4-byte form, UTF-8 encoded surrogate, value above U+10FFFF | Rejects E0/ED/F0/F4 restricted second-byte ranges at the lead offset. The 2-byte lead range excludes overlong C0/C1 forms. |
| `decode`, UTF-8 BOM U+FEFF | Preserves it as an ordinary UTF-16 code unit; no BOM stripping is performed. |
| `decode`, byte offset map | Appends the start byte offset once for every emitted UTF-16 code unit. Both units of a supplementary pair use the same source-byte offset. Appends `bytes.size()` as the terminal entry, giving exactly `text.size()+1` offsets, including for empty input. |
| `encode`, BMP scalar and valid surrogate pair | Emits canonical UTF-8 bytes; a valid pair is combined and encoded as one 4-byte sequence. |
| `encode`, isolated high or low surrogate | Emits UTF-8 for U+FFFD, matching `TextEncoder`; the input `u16string_view` is never changed. This boundary conversion is lossy by contract. |
| `encode`, U+FEFF | Emits its UTF-8 bytes and does not strip it. |

## Host adaptation and limits

The implementation uses explicit UTF-8 validation and UTF-16 code-unit
operations, so its behavior does not depend on the active Windows code page or
locale. `DecodeError` retains the exact start byte offset while preserving the
root-frozen exception text. `std::u16string` is the internal string form; only
`encode` replaces isolated surrogates because UTF-8 cannot represent them and
the frozen external boundary follows TextEncoder.

This module does not provide arbitrary JavaScript string coercion, a general
TextDecoder options surface, normalization, editor offsets, parser behavior, or
XML/entity handling. It implements only the frozen four-function contract.
The implementation was not built or executed in this packet; root's frozen
Node-generated expectations and independent acceptance checks remain pending.
