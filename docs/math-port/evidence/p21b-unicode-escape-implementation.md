# P21b — Unicode escape grammar implementation

Implemented only in the frozen ignored successor
`build/math-port-regexp-unicode-escape-source/src/parse.rs`. The resulting
SHA-256 is
`34a075eaf65c01f5c4f3fbe527018f7573cf3dc034072b419bcab82abcbd864a`.
The edit retains the P20d pre-prefix rollback repair and changes no adapter,
matching, indexing, Unicode data, or other parser branch.

`consume_character_escape` (1551–1560) now passes both explicit grammar
contexts to `try_escape_unicode_sequence`: the computed Unicode mode is used
both to permit `\\u{...}` and to decide whether escaped surrogate pairs are
combined. When a legacy ordinary escape rejects a braced form, the helper
restores its cursor to its entry point (after `u`); the existing identity
escape path then consumes only `u`, leaving following source for the enclosing
parser. Unicode-mode failures still flow through the existing invalid-escape
diagnostic.

`try_escape_unicode_sequence` (1768–1868) separates `allow_braced` from
`combine_surrogate_pairs`. Braced syntax enters only when `allow_braced` is
true (1776). It validates every collected braced digit with
`char::is_ascii_hexdigit` (1781), before the existing radix conversion and
code-point range check. The primary four-digit escape applies the same ASCII
check while collecting its four units (1813). The speculative low-surrogate
trial checks each of its four units as well (1850); invalid input follows the
existing closure failure path, which restores to before the second `\\u`
prefix and returns the first high-surrogate unit. Thus a leading plus, other
non-ASCII numeral, or any non-hex character cannot be accepted by Rust's
radix conversion in any of the three paths.

The two named-capture identifier call sites remain explicitly independent of
the regexp's flags: the initial identifier unit (1890) and subsequent units
(1915) pass `(true, true)`. They retain braced Unicode escapes and surrogate
pair combination in legacy mode while receiving the same strict ASCII
HexDigit validation. No concrete pattern or code point is special-cased.

The packet requested no builds or tests. Root owns the four-corpus replay,
Node differential validation, locked upstream test suite, mutation review, and
acceptance; this report makes no claim that those gates have passed.
