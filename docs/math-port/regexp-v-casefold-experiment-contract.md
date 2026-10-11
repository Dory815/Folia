# P21d root freeze — Unicode-set point folding and algebra

This ignored candidate experiment is a prerequisite for the required native
TeX regexp host. It accepts no engine, TokenMap, MathJax file or Folia route.
Root owns architecture, copy preparation, interfaces, independent Node corpus,
replay/upstream tests/mutations and acceptance. Luna owns only
build/math-port-regexp-v-casefold-source/src/parse.rs,
build/math-port-regexp-v-casefold-source/src/unicode.rs and
docs/math-port/evidence/p21d-v-casefold-implementation.md. No other edits,
builds/tests, expectations, resets, commits, delegation or integration.

## Exact boundaries

The successor retains all Unicode16/P21b algorithms and generated data.
Predecessor parse.rs LF SHA256:
34a075eaf65c01f5c4f3fbe527018f7573cf3dc034072b419bcab82abcbd864a.
unicode.rs LF SHA256:
4aebcc04392ddf6820b74bc470e3e9bfc7ea7bf486e0b203e3f589136c741635.
Data table raw SHA256:
475cb03589f0deb859b9a2ccfb34e3f33026a1e89500524024ce575a135e5daf.
Matching/indexing, flags adapter, IR/emitter/optimizer, general interval
operations, legacy/u folding, generated tables, alias resolution, string
alternatives and grammar spelling are outside this packet. String alternatives
continue through the existing StringSet path. Do not alter the q parser.

The primary normative reference is ECMA-262 2025 CompileToCharSet / set algebra:
https://tc39.es/ecma262/2025/multipage/text-processing.html#sec-runtime-semantics-compiletocharset.
The behavioral reference remains pinned actual Node 24.13.0, not a rewritten
normative oracle. Root also inspected its V8 source at
https://raw.githubusercontent.com/nodejs/node/v24.13.0/deps/v8/src/regexp/regexp-parser.cc.
Its full hash is 443127384a169615abb1e384be68d8102f76754c7294823fed4e3c8cefb3572c.
LookupPropertyValueName closes a Unicode property before complementing in v+i;
LookupSpecialPropertyValueName preserves intrinsic Any/ASCII range definitions.
This distinction is observed in actual Node and must remain a typed property
catalog responsibility, never a concrete full-pattern or subject check.

## Frozen private interface and operation responsibility

Add private PropertyCaseFolding enum {Unicode, Intrinsic} in unicode.rs.
Change PropertyEscapeKind::CharacterClass to the record variant
CharacterClass { intervals: &'static [Interval], folding: PropertyCaseFolding }.
All existing script/category and ordinary binary results are Unicode policy.
Only the existing UnicodePropertyBinary::Ascii / Any enum definitions select
Intrinsic. No new name matching or alias/table edits. The existing Assigned
table is Unicode policy. StringSet is unchanged. Adapt only the two files'
existing pattern matches to these fields.

In v+i, fold literal singleton/range operands before union/intersection/
subtraction, and close ordinary character/property escape positive sets before
complementing. Nested sets must already be prepared when their negation is
applied. Preserve raw range endpoints until range validation/construction.
Use existing unicode::add_icase_code_points; do not rewrite table algorithms.
Carry icase into the ClassSet union/intersect/subtract preparation and both
range insertion sites as needed. Typed intrinsic property ranges remain exact
in v+i. Remove the final whole-point-set closure in ClassSet::node so algebra
cannot re-add removed equivalents or widen intrinsic property ranges.

Top-level v property escapes use the same prepared property responsibility;
negative properties complement after permitted closure. Preserve the ordinary
u property branch's existing complement-before-closure order and preserve all
legacy behavior. Keep d/D,s/S,w/W top-level and nested semantics consistent;
nested word positives close before W complement. Do not globally disable v
case folding. Source string alternatives, constructor syntax diagnostics,
empty-string sets, WSpace alias and astral zero-width protocol stay separate.

## Independent acceptance before dispatch

Root generated **227,712 actual Node observations / 593 patterns** before
implementation. SHA256:
0d65e4dcf82471c630fcea1f370e444af451c86b4b9cecb886ff80cbe87db063.
Groups: property/nested complement 27,648; operand set algebra 194,688; literal,
range and escape controls 5,376. Flags cover v/vi, global/sticky v+i, and u/ui
controls; texts include case-fold equivalence classes, surrogates, astral pairs,
whitespace and start offsets. Actual constructor/capture/range/lastIndex are
recorded; no candidate contributes expected data.

Root replays the four original corpora, Unicode16 property corpus and the new
corpus against P21c in both adapters, classifies every retained difference and
requires no regression in the correct adapter. Complete unchanged upstream
utf16 tests remain recorded, including incompatible old version/legacy cases;
no failures are waived or counted as passing. Effective compiling semantic
mutants must be detected by actual-Node observations. This packet cannot adopt
the engine while any engine/host/profile gate remains open.
