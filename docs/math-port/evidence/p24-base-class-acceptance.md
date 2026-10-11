# Root acceptance: three base TeX-class propagation bodies

Root reviewed the three changes against the frozen original source and method
contract. Virtual dispatch, raw values, nullish selection and partial effects
remain at the existing AbstractMmlNode boundary. No production declarations or
other bodies changed. This is acceptance of these methods, not of MmlNode.ts as
a whole or of the TeX/layout/SVG path.

The unchanged original class-state and boundary oracles reproduce under
`--check`. Root's supplementary original callback oracle also reproduces:
432 calls, 54 original errors, SHA-256
`e97d16f0fe2646eb4dd0dad12d1915e8ae49c5e2ddfb2cfd52d21d2aa39efb60`.
It observes Null/Undefined/self/peer, virtual setter failures, owner attribute
rebinding, skipped/changed getPrevClass hooks and reads that change subsequent
state. Returns, six raw backing fields, attributes and ordered callback trace
are original observations rather than native-derived expectations.

`node tools/math-port/mml-base-class-gate.cjs` compiles the full current
90-module graph and passes 13,892 assertions in 13 tests. All eleven compiling
mutations fail the original expected observations: wrong return selection,
omitted previous level/transfer, core clear order, truthy vs nullish selection,
qualified instead of virtual hook, prefetched core level, reordered writes,
numeric coercion, swallowed null-attributes failure and normalized Undefined.
The runner verifies working production files remain unchanged.

- Result SHA-256:
  `7c8c33ea4493e2d3af5078ff8f162f231b7c6eac52b1432568f8e6113b740e25`.
- Baseline test log SHA-256:
  `36952e4fa468d49645e381ca6c4561b8d032c995453257975392a679a61ef70b`.
- Existing inheritance/return/predicate/copy/constructor/append regression:
  1,765 assertions / 17 tests; its compiling prime-type mutation is detected.
  Result SHA-256:
  `1f575a5fad3b7d43efd0ecbee1e84862ca48545c79045e5d181adc3f362b05a6`.

`cmd /c build_test.bat 2>&1` was run and fails in other stale observer bindings.
A separate keep-going inventory confirms nine failed test translation units:
mo_order, table, atom_semantics, simple_mml, mfenced, multiscript, arithmetic,
maction and arithmetic_order. Inventory log SHA-256:
`439043c619705141a6310a1acbda543dae63a169637b8c90f92d266d3aabc95a`.
The three separately repaired operator/registry observers compile. No core
test executable ran, and the repository build is not accepted as green.
No Folia integration or whole-file progress tally advance follows.
