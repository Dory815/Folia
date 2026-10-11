# P13 fraction/radical semantic nodes — root-frozen contract, revision 2

Sources are MathJax 4.1.3 mfrac.ts, msqrt.ts and mroot.ts from the pinned clone.
This packet ports complete source files over the accepted MML hierarchy. It
contains no TeX recognition, fraction/radical geometry or backend operations.
Common wrappers and shared scene placement remain separate required packets.

Root owns interfaces, generated defaults/constant getters/class-field phases,
native operation bindings, source hashes, oracles, tests, CMake, mutations,
ledger and acceptance. Freeze and commit all these before agent dispatch.
Agents may change only their two assigned files; no builds, shared edits,
test/expectation changes, commits, scope changes or production registration.

| Agent | Sole source | Sole evidence | Methods |
| --- | --- | --- | --- |
| tex_parser | mml_nodes/mfrac.cpp | evidence/p13-mfrac.md | setTeXclass, setChildInheritedAttributes |
| math_layout | mml_nodes/msqrt.cpp | evidence/p13-msqrt.md | setTeXclass, setChildInheritedAttributes |
| math_fonts | mml_nodes/mroot.cpp | evidence/p13-mroot.md | setTeXclass, setChildInheritedAttributes |

All six methods use TreeCallGuard. Required child/attribute access raises the
shared typed TypeError before later actions; no native invalid dereferences.
Calls retain JS receiver/argument evaluation order and virtual dispatch.
Protected inheritance hooks have no entry defaults: preserve raw display,
level and prime, including Missing/null/strings/arrays. Outer superclass entry
defaults already belong to the accepted MmlNode implementation.

Fraction retains its inherited null class. Radicals initialize the same
inherited source texclass field to ORD after superclass initialization, never
a numeric mirror or separate state. Defaults use the pinned shallow-copy helper;
sqrt/root have distinct records despite equal values. Constant accessors stay
in root-generated binding CPP files.

Mfrac.setTeXclass first transfers previous class/level, then captures the array
iterator once. Its length/elements are live during callbacks; array rebinding
does not replace the captured iterator. Each child receives null previous.
Do not truncate to declared arity, skip null children, return the last child,
normalize the fraction class, or stop iteration at its original length.

Mfrac inheritance uses JS truthiness and `level > 0`, then numeric `level++`
only when that branch executes. It reads numalign then denomalign separately,
creates a fresh shallow copy for each addInheritedAttributes call, and constructs
both records before child inheritance. Source denominator key is `denalign`
(not denomalign). Preserve raw alignments and exact six added entries. Child
0 gets false display/current prime; child 1 gets false display/true prime.
Re-read childNodes for each call after callbacks. The pinned CJS distribution
lowers source spreads to `__assign({}, attributes)` using Object.assign.
`arithmeticCopy` preserves shallow nested aliases and that inherited prototype
setter. An own source `__proto__` array becomes the new copy's prototype and is
absent from its own keys. Root revision 2 corrects the original own-spread
contract after an actual-CJS virtual-helper witness rejected it. MmlNode's
existing assign-based helper already matches this compiled profile; no base
semantic change was needed. This transpilation detail is part of the locked
runtime compatibility contract, not a claim of uncompiled TS spread semantics.

Msqrt setTeXclass transfers previous state then calls only child 0 with null
previous and returns this. The child is normally inferredMrow; do not flatten
it. Its inheritance passes the original attributes handle and raw display/
level, always true prime. Mroot calls child 0 then reads child 1 again; root
index inheritance has false display, JS `level + 2` (string concatenation
differs from fraction increment), and original prime. Both root children share
the original attributes handle, including first-child mutations. Do not read
child 1 early or evaluate its level addition before capturing its receiver.

Root actual-CJS witnesses must cover all AST members, defaults/phase/identity,
raw level and display branches, attribute copy/alias rules, traversal/call
order, child-array mutation, malformed state and complete outer inheritance.
Root reviews every method and all retained branches, reproduces data and
fixtures, runs core regression and requires compiling/assertion-failing
isolated mutants. Full G0–G7 and production integration remain unfinished.
