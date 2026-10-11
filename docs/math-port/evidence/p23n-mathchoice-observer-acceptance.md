# Root MathChoice observer binding acceptance

Root accepts this native test-binding repair, not MathChoice algorithm runtime
equivalence on the sparse graph. The original Source oracle and profile both
reproduce unchanged: 412 assertions, 410 case names, 409 complete original calls
and 1,081 expected-value records. Profile SHA-256 remains
`3dc783dce354ba83003d6949ade951047d83912982f3a2409d42085a3b871cea`.
Root reviewed the native-only emitter diff and unchanged direct defaults/identity
assertions. No source stimuli, helper category, error, expected value, pin or
assertion changed. Raw notParent remains a Property; factory creation keeps its
independent pointer interface, while MML replacement/parents use tagged values.

The isolated full-graph compile over all 90 interfaces succeeds:
`node tools/math-port/mathchoice-observer-compile.cjs`.
Its compile-result.json explicitly reports no semantic tests, SHA-256
`9d3aeacaf088134b8fec70ba29d87650bc944592ad3dbfe0eef0ae650029dd02`;
the successful baseline build log SHA-256 is
`d8c21184b104f7aed29d97314fb6bcfc6ea1bb6326cb5e5b062b94e8db62acca`.

Root then reran the repository build with Ninja -k 0 to finish the complete
FoliaTests compile inventory despite failures. MathChoice no longer fails;
the remaining 12 failed observer translation units are mo_values, table,
mo_order, atom_semantics, mfenced, simple_mml, mml_registry, multiscript, mo,
arithmetic, maction and arithmetic_order. The gate still fails before any core
tests execute. Log build/p23n-core-inventory.log SHA-256:
`4bbb00d79397949b0e1a84b6226e602d4e71bfe9d18ca60a876da76c1dcc5a4d`.
No whole-file tally advance or Math/Folia acceptance follows from this packet.
