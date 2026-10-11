# P17a TeX Token and Macro method evidence

The pinned MathJax 4.1.3 source is `.reference/MathJax-src/ts/input/tex/Token.ts`, SHA-256 `6b4c0e42f277daebe8e676afc3b4d3b6083ebc87939ef8203672a831be1595bf`. The native split preserves the source `Token` class in [token.cpp](../../../src/math/input/tex/token.cpp) and the `Macro` class in [macro.cpp](../../../src/math/input/tex/macro.cpp), both with MathJax's Apache-2.0 attribution; see `src/math/LICENSE-MathJax.txt`.

## Complete member mapping

| Source member | Source lines | Native mapping |
| --- | ---: | --- |
| `Token.constructor(_token,_char,_attributes)` | 31–40 | `Token::Token` stores the three arguments in source order as raw `Property` values. |
| `Token.token` getter | 42–44 | `Token::token()` returns the stored property handle unchanged. |
| `Token.char` getter | 46–48 | `Token::character()` returns the stored property handle unchanged; the native name avoids the C++ `char` keyword. |
| `Token.attributes` getter | 50–52 | `Token::attributes()` returns the stored property handle unchanged, preserving dictionary/array identity. |
| `Macro.constructor(_token,_func,_args=[])` | 60–66 | `Macro::Macro` stores token and exact ParseMethod value, then applies the source default only to a Missing args value. |
| `Macro.token` getter | 68–70 | `Macro::token()` returns the stored property unchanged. |
| `Macro.func` getter | 72–74 | `Macro::func()` returns the same ParseMethod alternative and callable handle. |
| `Macro.args` getter | 76–78 | `Macro::args()` returns the same Property graph; no copy of the underlying argument array is made. |

## Value and identity behavior

`Property` is the frozen shared carrier for exact TeX raw values and strings. Token construction does not validate/stringify `_token`, `_char`, or `_attributes`; explicit Missing, null, booleans, numbers, strings, object/array handles, and malformed-but-representable values remain raw. The getters return `Property` copies whose shared-storage references retain source object identity and later mutations.

Macro preserves ParseMethod identity by copying the variant and its shared callable reference as stored. It does not replace callbacks with names or invoke them. Only `args.missing()` creates a fresh empty `PropertyArray`; explicit null, scalar, object, or existing array values are retained exactly. Each constructor invocation with omitted/Missing args gets a distinct new array; repeated `args()` reads return the same array graph.

The C++ module split follows the root-frozen cycle seam: Token implementation imports its declarations; Macro imports the independent types seam. No parser state, global registry, map behavior, callback execution, or Unicode conversion is introduced. Internal string properties retain the established lossless native representation; external UTF-8 encoding is a separate boundary.

This report maps the eight executable source members only. Root owns actual-CJS differential tests for constructor/accessor values, identity/mutation, fresh defaults, callback and raw non-callable distinctions, and module witnesses. No tests, interfaces, expected results, or build files were edited, and no build or test was run. Implementation completion is not packet acceptance.
