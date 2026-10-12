# P50 native scanner RegExp contract

Root owns the native language host; Luna ports actual FindTeX algorithms only
after this dependency is accepted. FindTeX remains its own Source owner extending
core AbstractFindMath. This host does not recognize TeX, delimiters, or equations.

The pinned FindTeX creates only non-Unicode global expressions. Pattern storage,
input, result captures, match.index and lastIndex use UTF-16 code units. A match
retains its input string and distinguishes an absent capture from an empty one.
The caller may retain the same shared RegExp and mutate its numeric lastIndex.
Global failure resets lastIndex; plain scans ignore and retain lastIndex.
Successful zero-width exec does not advance lastIndex. Numeric cursor conversion
uses ToLength, including NaN, negatives, fractions, infinity and large numbers.
Search starts in the complete input, preserving absolute indices and context.

Existing SRELL is used with its custom regex_traits extension point and
utf_traits_core<char16_t>, rather than default scalar-aware char16 traits. No
vendored code is modified. Its ECMAScript whitespace and default dot line
terminators match the original. Source quotePattern legitimately escapes colon
and hyphen; the compiler adapter must preserve non-Unicode identity escapes
instead of rejecting those configured delimiters. This is general literal
escape adaptation, never a source-text special case for one formula.

This gate accepts the generated scanner grammar and plain/global/sticky numeric
cursor behavior against the actual JavaScript RegExp primitive and pinned
FindTeX pattern generators. Other valid flags are explicitly unimplemented;
invalid or duplicate flags are SyntaxError. It does not claim all ECMAScript
RegExp grammar, Unicode properties, named groups, dynamic coercion, string replace
callbacks or a complete native TeX parser. Those real caller requirements must be
added and verified when their actual Source owners are ported, never silently
approximated. The full MathJax/package/layout/SVG/Folia goal stays unchanged.

Root freezes declarations and independent Node observations before completing
the host. Tests include surrogate halves, captured groups, quoted punctuation,
actual begin/ref/end patterns, whitespace, zero-width matches, global failures,
and nonzero absolute offsets. The module and implementation remain portable and
use import std; SRELL lives only in the implementation global module fragment.
