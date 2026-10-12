# P37 operator, TeXAtom and under/over responsibility contract

Root design for the next packet; no implementation or acceptance is claimed.
Pinned reference remains MathJax-src 4.1.3 at
fb987178c2d279a99b0db5ea02e751691703955e.

## Ownership

- Root owns public declarations, mo_host tagged indexed/member primitives,
  munderover's local tagged slot/member primitives, observers, CMake, mutation
  gates, scope extensions and acceptance.
- `math_layout` owns only `operator_class.cpp`'s shared adjustment algorithm
  and the two stubs in `tex_atom.cpp`. Mo and TeXAtom must use this one effective
  Source algorithm; do not preserve the overwritten TeXAtom identity stub or
  duplicate the algorithm. Shared guard ownership follows the existing P15bc
  contract; delegating adjustment wrappers must not add another guard.
- `mml_verify_nodes` owns only five stub bodies in `mo_classes.cpp`:
  coreParent, coreText, isAccent, setTeXclass, adjustTeXclass.
- `mml_row_mi` owns only getForms in `mo_lookup.cpp`, checkPseudoScripts and
  checkMathAccent in `mo_scripts.cpp`, and the two semantic stubs
  setChildInheritedAttributes/setInheritedAccent in `munderover.cpp`.

No implementation begins while the P36 runner is live. Agents may not change
helpers, interfaces, constructor/static bindings, existing implemented bodies,
gold, tests, or generated data. Root completes primitive bindings before dispatch.
Agents read each full Source method, report precise correspondence and API gaps,
and do not run builds/tests. All files use LF.

## Semantic obligations

Keep each method in its owning upstream node and keep the effective operator
adjustment shared. No parser, measurement or backend logic belongs here. Use
existing tagged NodeValue, shared sparse ChildArray, raw Property, virtual
getters/setters and captured constructor metadata.

Preserve raw versus logical parents, registered-constructor instanceof rather
than kind strings, repeated virtual checks/reads, exact child-slot read timing,
and constructor ACCENTS/index/pattern services. Core traversal must not collapse
null and undefined slots or replace Source member errors with arbitrary fallback.
Do not cache a parent/core/attribute across a Source reread. Callback mutations and
throws retain earlier writes and graph effects. Guard entry/traversal using the
established Native resource boundary; no unguarded recursion or infinite malformed
graph walk.

Operator class assignment preserves getList evaluation, exact Missing texClass
property, spacing shortcut, raw fence and exact form values, and virtual class
writes. Shared adjustment preserves raw class/previous/autoOP, short return for
NONE, source chained assignment order, inherited scriptlevel read, leading and
trailing BIN rules, parent walk and terminal child identity. End-slot indexing
must use the array captured before the current length expression.

Forms preserve nonSpaceLength/firstNonSpace/lastNonSpace dispatch and Source
position/form ordering. Script/pseudo/accent checks preserve captured method
receiver before argument evaluation and the pinned pattern/UTF-16 semantics.

Under/over inheritance captures the Source nodes array, handles slot 0 before
later literal slots, keeps raw prime-or-presence values, and uses the actual
base coreMO movablelimits attributes and constructor accent index order.
Accent inheritance preserves explicit/default/strict comparison, optional
child dispatch, level addition and partial effects. A current-array helper
cannot replace a Source previously captured nodes array.

## Independent acceptance

Existing original observer gold currently reproduces with --check:

- Mo: 23577 assertions / 14 tests; order: 538 / 50.
- Script: 1683 original assertions / 17 tests plus six Native assertions in
  one separate budget test (1689 / 18 total).
- Atom/semantics: 6888 / 74, including the separately documented Native guard
  assertions; effective algorithm is tested through both Mo and TeXAtom.

Additional raw operator values/pattern fixtures remain unchanged and join the
gate when relevant. Root must first prove real stub failures, then review the
full methods and require green real implementations plus independent compiling
semantic faults. Source oracle existence and declaration compilation do not
establish parity. Surviving faults, crashes and compile failures reject the gate.
Known observer gaps are supplemented from the actual original Source before
native comparison. No unsupported feature or error fallback is accepted as a
completed native MathJax renderer. Mermaid remains paused.

Root's preimplementation red gate builds all 90 existing modules and the four
original fixtures plus two separate Native entry assertions. It exits 1 through
real Unimplemented bodies and verifies working source/fixture bytes unchanged.
The failing paths execute only 7320 assertions (3266 failed) rather than
the full green corpus; this is red evidence only. Both Native entry assertions
also fail with Unimplemented. No implementation is inferred from that build.

Primitive bindings retain tagged Get until the requested member dereference.
Null arrays and Null ACCENTS reads report the actual requested numeric property.
The missing slot itself remains Undefined and retains the subsequent member error.
