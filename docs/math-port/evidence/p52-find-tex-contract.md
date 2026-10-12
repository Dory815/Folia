# P52 complete actual FindTeX class contract

Root freezes the complete Source class, not a custom subset finder. It extends
the actual core AbstractFindMath with the same Context option graph. Static
OPTIONS lives on the actual constructor once per runtime. Allocation forwards
the runtime's single RegExpBindings through the existing base factory; Source
base initialization completes before the derived initializeFindTeX/getPatterns
phase. All five derived fields begin Undefined, and getPatterns is virtual.

The declaration contains every original concrete method, constructor phase and
field. Only the complete find_tex.cpp algorithms belong to Luna. Root owns
declarations, metadata, allocation, original gold, tests, review and acceptance.
Do not edit any other file, run builds/tests, commit, or delegate another agent.

start/end/hasPatterns/env/sub are their sole field values. End is one ordinary
Context object holding ordinary [close,display,RegExp] arrays. Preserve prototype
keys, duplicate opener overwrite and actual RegExp identity/cursor reuse. No
unordered_map end table, recompiled matcher, second scanner, fake proto record
or copied options is permitted. Local starts is one shared typed UTF-16 vector
(a Source local string array); local parts/subparts are native vectors. They do
not mirror option arrays, and retained starts references keep the actual array.

Use the real looked-up array forEach native method for inlineMath/displayMath.
The shared Context host supplies its captured length and live Has/Get behavior.
Delimiter tuples use the ordinary-array iteration profile of existing native
option bindings: read the first two entries in order, with live length/holes.
Both strings retain UTF-16 exactly. Nonmatching input and all Source switches,
environments, references, control sequences and braces are part of acceptance.

Use actual string_util::sortLength/quotePattern; sorting local strings by the
Source pure comparator may use stable_sort. Pattern creation goes through the
accepted RegExpBindings, with exact Source operation order and flags g.
endPattern's optional parameter remains distinct from the explicit null first
argument of its environment caller. Its return is the canonical native handle.

findEnd returns the actual core protoItem (Property), or PropertyNull; it does
not invent another record schema. Math content/open/close cross into Property
only through lossless WTF-8 property_host::debugText/text. Environment handling
mutates that same record; escaping uses omitted display (null), refs use false.
findMath returns one fresh native ProtoMathItems array identity and receives the
same Context Value input through its inherited std::any bridge. It captures
strings.length once and Gets slots during the loop, not a vector snapshot.
Keep each text slot as its original Value: a missing array slot is Undefined,
which RegExp.exec coerces to its primitive string. Do not reject holes before
the actual scan or eagerly normalize all text. Primitive RegExp input conversion
and the later text.substring method validation have different Source timing.

Scope is the actual ordinary native array/string/RegExp profile, including the
configured delimiter and switch surface. Arbitrary JavaScript evaluation,
monkey-patched string/RegExp prototypes, custom iterable protocols and result-array
JS prototype reflection are not claimed. All supported malformed ordinary-array
inputs must preserve Source exception names/order. No feature of the enabled
Folia MathJax profile may be removed to make tests pass.

Root requires the complete stub graph to compile before body delegation. Gold
executes pinned actual FindTeX and observes results, fields, hooks, identities,
getter effects, exception phases and exact UTF-16 offsets. Completion accepts
the finder only; actual TeX/package parsing, fonts/common layout/shared scene,
SVG parity and Folia native integration still define the full active goal.
