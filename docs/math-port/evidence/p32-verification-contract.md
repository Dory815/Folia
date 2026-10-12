# P32 Root base verification method contract

Root owns the P30 Property options boundary, Source expectations, declarations,
observer templates and acceptance. This packet authorizes Luna to implement
only `AbstractMmlNode::verifyTree` and `AbstractMmlNode::verifyChildren` in
`src/math/core/mml_tree/mml_node.cpp`, plus a bounded implementation report.
No other methods, helpers, types, tests, expected records, build settings or
Folia code belong to the delegated write scope.

Original MathJax 4.1.3 MmlNode.ts is pinned at SHA-256
`cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656`;
the unchanged emitted CJS is
`86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45`.
Actual methods are lines 906–924 and 973–977. Both are ordinary virtual member
implementations; verificationAttributes and error construction retain their
separate original responsibilities.

verifyTree applies its own Source default (Missing becomes local Null), returns
only on exact Null, calls virtual verifyAttributes with the original options
handle, then reads virtual arity once, then reads options.checkArity. If truthy,
nonnegative finite arity is checked: arity 1 allows any nonempty child list;
every other arity requires exact length equality. Each Source length read uses
the current child array and throws the exact null length-access error when
reached. On mismatch it calls virtual mError with the original message/options
and short=true; ignore its return and continue to virtual verifyChildren. Keep
callback ordering, callback changes and partial effects. NaN is not finite
arity for the wrong-count comparison in Source: its >=0 test is false. Do not
replace Source's exact Infinity exclusion with a broader isfinite filter that
changes negative infinity or NaN ordering/property access.

verifyChildren captures the current child array once, performs live indexed
iteration including prototype Get and holes, and invokes every child's virtual
verifyTree with unchanged raw options. It does not skip nullish children,
normalize Missing, copy the array, precompute its length, reload the receiver
array after a child callback, or call a qualified child implementation. Exact
iterator/nullish member errors and already-completed calls remain visible.
Both methods use the existing shared Native TreeCallGuard, with a separate
Native call-budget/recovery witness; this is not a Source syntax cutoff.

Root froze 3,176 complete original records: 1,880 tree cases and 1,296 child
iteration cases, including 748 Source exceptions. Original JSON SHA-256 is
`020e72f32331a16e1ac42d7da65491d958b7b5a6a90923f2a729b02904aa61b7`.
Options include omitted/undefined/null, scalars, arrays and object identities;
arity covers infinities/NaN/signed zero; callbacks mutate flags/arrays, throw,
grow/delete/shrink/rebind/prototype children and share options changes.

Before delegation Root compiled all ninety interfaces and ran the real existing
stubs. All 3,176 assertions failed through those stubs, with exit 1, no crash and
final source/fixture guard passing. The isolated red evidence is under ignored
`build/math-port-mml-verification`. The Native emitter and original records were
fixed before implementation; they may not be changed by Luna.

After implementation Root reviews the two bodies, requires all original
records to match, adds independent compiling faults, checks the actual normal
core build and prior accepted invariants, and records bounded acceptance.
This does not accept mError, subclass verification, the remaining MML/TeX stack,
shared layout, SVG output or Folia. Complete native math remains the goal.
