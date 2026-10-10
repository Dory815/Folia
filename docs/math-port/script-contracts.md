# P10 script nodes and string utilities — root contract revision 1

Root owns interfaces, static bindings, source lock, oracle expectations,
fixtures, builds, mutations, scope, review and acceptance. Sources are pinned
MathJax 4.1.3; production registry, mo and renderer are separate unfinished
dependencies. No partial registry, kind-string stand-in, custom AST or fallback.

## File responsibilities

| Agent | Only implementation files | Only evidence file |
| --- | --- | --- |
| math_layout | core/mml_tree/mml_nodes/msubsup.cpp | evidence/p10-subsup.md |
| math_fonts | core/mml_tree/mml_nodes/munderover.cpp | evidence/p10-underover.md |
| tex_parser | util/string.cpp | evidence/p10-string.md |

Paths are relative to src/math except evidence paths relative to docs/math-port.
All eight util/string functions and every member of all six concrete classes
must map to code, root-owned bindings, or an explicit typed host adaptation.
Agents must not modify declarations, shared hosts, static records, oracles,
fixtures, CMake, scope, production, another packet; no builds or commits.
Report interface gaps. Implement source methods without omitted features.

## Semantic obligations

Script node inheritance captures the child array once, retains its live slots,
and forwards the same attribute record identity. Missing base/second receiver
fails before evaluating its call arguments; an absent third slot returns after
prior writes. Null arrays/nodes throw FactoryError TypeError, never native UB.
Stage JS call-argument evaluations in order; C++ does not guarantee their order.
Use shared TreeCallGuard for source recursive hooks and virtual helper methods.
Protected hook parameters have no source defaults: do not normalize Missing.

msubsup uses primitive addition (host::add(level,1)), so string levels append 1.
Its prime OR preserves a truthy raw Property and skips the sub getter; otherwise
read the virtual sub getter at the source position. Base/sub/sup and under/over
are virtual native indices; builtin getter bodies are mapped individually.

munderover calls base before force computation and captures constructor ACCENTS
after force. The actual constructor descriptor survives factory re-registration.
Munder shares/inherits the parent's array binding; Mover owns a reversed one.
Static replacement affects later captures, not earlier local handles; in-place
changes affect subsequent reads of the captured array. Sparse/out-of-bounds
entries map to Missing, later attribute keys use source ToString.
Use host::number(level)+1 for level++ only when force or falsey accent requires
increment. Otherwise preserve raw level. force short-circuits attribute reads.
setInheritedAccent reads current childNodes, then explicit presence, then virtual
embellishment. It rereads attributes for each source getter. Strict inequality
keeps NaN unequal and object identity; do not use SameValueZero. Reinheriting
passes a fresh empty attribute record and original display/prime (not initial
under/over false/OR values). Preserve partial writes before exceptions.

util/string consumes UTF-16 units. sortLength sorts by descending unit length
then lexicographic units. quotePattern quotes exactly the source character set.
unicodeChars uses code-point iteration preserving isolated surrogates;
unicodeString accepts numeric spans, rejects fractional/NaN/infinite/out-of-range
values with RangeError, accepts surrogate values and negative zero. This typed
numeric-array binding does not expose arbitrary JS iterators/getters or VM
argument-stack capacity. split(trim(empty)) returns one empty string. isPercent
and split reuse the accepted ECMA whitespace classification.
replaceUnicode follows leftmost non-overlapping source non-Unicode regex matches:
four bare hex digits, or braced one-to-six digits with ECMA spaces, or backslash
plus a non-line-terminator unit. Double backslash becomes one. Other generic
backslash matches yield NaN conversion and RangeError; malformed backslash-U
syntax falls through that generic match. Backslash followed by a line terminator
or at end remains unchanged. Callback output is never rescanned.
toEntity uses only first code point, uppercase minimal hex; empty throws TypeError.
Errors reuse util/option_types::Exception; util must not import MML/platform.

## Root acceptance

Verify pinned source and actual resolved CJS hashes. Generate independent native
expectations from that CJS before agents change implementation; no native output
may define expectations. Cover every source member, raw values, UTF-16 boundaries,
static copies/aliases, class rebinding, live callback order, partial exceptions,
captured array replacement/mutation and inherited accent reinheritance.
Require focused and full core builds plus compiling deliberate mutations that
fail unchanged assertions. No acceptance based on a build or agent claim alone.

Windows Debug native-hook recursion requires the test process's 16 MiB stack
reservation to reach the shared 512-call guard before the OS stack boundary.
The native module cannot configure its caller's stack. G7 must configure and
verify Folia math worker stack availability; this test setting does not prove
the existing app/thread integration. Resource recovery is tested with real
nested hooks, not injected guard counters or reduced-depth witnesses.
