# P44 UnitUtil/UnitMap contract

Root freezes complete input/tex/UnitUtil.ts, MathJax 4.1.3 source SHA-256
90f461310aa6d07b494cda98dc5398e9737231f046e20099ea0f5c19fa24c66e.
Luna may implement only unit_util.cpp: UnitMap constructor/updateDimen/set/get/
delete (native erase) and UnitUtil matchDimen/dimen2em/em/trimSpaces. muReplace
belongs to that same Source implementation unit. No util/lengths replacement.

Root owns declarations, default singleton/table binding, native value/pattern
host, original oracle/tests and independent faults. UnitMap's ordered map is
private; public num/unit/regex handles remain live, generated in original Source
order. set updates without changing key order, returns this and rebuilds even
on replacement. Successful erase rebuilds; failed erase retains pattern handles.
Constructor duplicate keys take the last value but keep first insertion order.
get uses truthiness then pt fallback; removing pt permits Missing -> NaN in
multiplication, rather than a hardcoded .1 fallback.

Source match chooses current dimenRest/End by rest truthiness, normalizes first
comma only, returns match captures 1/4 and full-match UTF-16 length, then converts
mu through current UNIT_CASES factor and UnitUtil.em, retaining original matched
length. End is anchored with JS whitespace; Rest consumes at most one literal
ASCII space after units. Invalid dimen2em uses value||'1' and get(null), yielding
pt only through the live map. em uses Source abs cutoff, accepted js_number
toFixed(3), then Source trailing /\.?0+$/ replacement. trimSpaces preserves
non-string identities; it trims JS whitespace and restores one trailing ASCII
space only for the exact Source escaped-ending condition.

## Native engine profile

UnitPattern is a native UTF-16 binding for Source-generated regular expressions,
not a MathJax algorithm. std::wregex receives one wchar per original UTF-16 unit
on both 16/32-bit wchar platforms; captures are converted back losslessly.
ECMAScript \s and \d are explicitly expanded so locale classifications cannot
change JS whitespace or ASCII digits. Root tests Unicode whitespace, line
terminators, surrogate tail lengths, literal non-BMP unit names, alternation
priority/backtracking and immutable old pattern handles.

The fixed Source num plus ordered literal unit names (default and custom) are
the required engine grammar. This is not acceptance of arbitrary evaluated JS
RegExp subclasses, flags, Unicode property escapes, lookbehind, custom match
callbacks, nonliteral regex-injection unit names or changed num grammars.
Compilation errors reject with an explicit native SyntaxError; no fallback.
Source update side effects remain before a compilation failure. Native numeric
host consumes complete fixed numeric captures plus mu formatter numeric strings,
not arbitrary parseFloat prefixes from an altered num grammar.

An exhaustive pinned ts caller audit finds no external UNIT_CASES.set/delete or
public regex/num writes. Enabled TexParser GetDimen, ColumnParser, ParseUtil,
BaseItems/BaseMethods, AMS/mathtools/newcommand/bussproofs/unicode use the four
UnitUtil functions. The actual UnitMap mutation API is nevertheless preserved
and independently compared within its declared pattern profile. Parser context
ownership moved to tex_parser_context in 62e0755; no context state was duplicated.

Initial gold contains 182 original records and must be checked against compiled
stubs before implementation. Complete TeX/configuration/Tags/BaseItems parsing,
fonts/common layout, SVG and Folia wiring remain required. Mermaid stays paused.
