# P52 complete FindTeX scanner accepted

Root froze the real FindTeX class, its actual AbstractFindMath base, runtime
bindings and 72 original observations in 4d31505 before delegating its body.
Luna implemented only input/tex/find_tex.cpp. The seven responsibilities are
derived constructor initialization, getPatterns, addPattern, endPattern,
findEnd, findMathInString and findMath. Source OPTIONS belongs to the actual
constructor once per runtime; each finder borrows the same RegExpBindings owner.

The implementation uses the existing Context option graph and actual looked-up
forEach callbacks. End tuples retain the same compiled RegExp instances and
cursors; output uses the existing core protoItem records and one result array.
Environment processing mutates the same record. UTF-16 indices and isolated
surrogates cross the Property boundary through its lossless WTF-8 conversion.
There is no replacement parser, copied option dictionary or SVG implementation
in this scanner.

Root review found two issues before acceptance. The native constructor metadata
had omitted Source arity 1; Root corrected the host and added a direct original
metadata/defaults observation. The initial body read tuple entries without the
ordinary array iterator's live length check. Root added original short/inherited
and getter-truncation observations; Luna corrected both delimiter and end-tuple
reads with sticky iterator completion. Two adjacent-expression observations also
check exact scanner restart positions. Original expectations were never changed
to fit the native body.

All 77 actual pinned FindTeX observations now pass. They cover default and custom
delimiters, opener ordering and duplicate overwrites, ordinary prototype keys,
braces/control sequences, environments including empty names, references and
escapes, switches, malformed input, virtual constructor phases, shared record
identity, captured input length and live slots/getters, primitive regex input
coercion and deferred substring errors, UTF-16 offsets and fresh result identities.
The oracle's --check reproduces the checked-in gold from locked original sources.

All ten compiling semantic faults are detected: wrong opener order, empty
environment exclusion, closing within braces, lost display values, escape
display defaults, skipping adjacent openers, retaining first duplicate end,
ignoring tuple length, bypassing the looked-up forEach, and pre-copying input
values. The restored baseline passes. The mutation manifest records the exact
accepted body hash; generated logs remain under ignored build/.

Independent readonly Luna review found no blocking mismatch within the stated
ordinary-array/primitive-regexp profile. Retained arrow callbacks own the actual
finder and local starts array, rather than dangling borrowed callback state.
Arbitrary JavaScript evaluation, custom iterable protocols and monkey-patched
string/regexp implementations are outside this native host profile.

The complete build_test.bat core build and regression suite passes, exit 0
(build/p52-find-tex-full-core.log). This accepts formula discovery only. Actual
TeXParser/configuration/BaseItems/packages, fonts/common layout/shared scene,
SVG equivalence and replacing Folia's math QuickJS path remain required.
Mermaid remains paused at its separately recorded checkpoint.
