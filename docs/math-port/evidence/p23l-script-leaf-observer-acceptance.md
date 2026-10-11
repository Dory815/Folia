# Root script and leaf observer acceptance

Root accepts only the native observer binding repairs in the P23l contract.
This does not accept previously ported methods against the replacement sparse
graph at runtime, increase the whole-file port tally, or accept Folia integration.

Both original locked oracles and their frozen Source profiles reproduce without
changes: script has 1,689 assertions / 18 cases, including 1,683 / 17 original
observations; leaf has 669 / 27, including 663 / 26 original observations.
The 1,816 script profile records retain SHA-256
`cc570115c252cd201bd2a59030cb8cc747ddbf3eb3106481d72df9d0b6cac898`;
the 658 leaf records retain
`2014a953aba93aae64e7a12be82e08d5cfc4fd44f8bf44ef6e8e83350ab1fce4`.
Root reviewed the native-only emitter diff, the unchanged direct expectations
and all five BMP matcher bitsets. No Source helper category, stimulus, original
virtual callback, expected result, assertion, case, pin or lock was removed.

The first combined compilation rejected two remaining captured-array index
writes in script. The agent repaired these to `original->set(2, NodeValue(late))`,
preserving assignment to the captured array rather than the rebound current
array. Root then repeated both original profile checks and compilation.

`node tools/math-port/script-leaf-observer-compile.cjs` independently compiles
both observers over all 90 actual module interfaces and the full implementation
graph. Root fixed the generic runner's compile-only target wiring to include
additional fixtures as well as its primary fixture; the result explicitly
records two compiled fixtures and `semantic_tests_executed: false`.
`build/math-port-script-leaf-observer-compile/compile-result.json` SHA-256:
`66ced0cd67c0a58656248c7ea9fd5d56a6f6c61849acb4fe3017d3dd3aa73f84`.
Its successful baseline build log SHA-256:
`54a3f24ca2d1086a17f9f76445e1bd1d06733dbd6484d7f0ddb012f7ea216ccf`.

The subsequent repository `cmd /c build_test.bat` still fails before any core
tests run. Twelve other generated observers contain stale child-array, tagged
node, virtual-method or parent bindings: html_node, mo_order, atom_semantics,
mo_values, simple_mml, mfenced, mml_registry, mathchoice, mo, arithmetic, maction and
arithmetic_order. The earlier contract's seven-file frontier was the output of
an earlier partial build, not the complete inventory. Neither script nor leaf
appears among this build's failing translation units. Preserve all remaining
failures and repair their observer boundaries in separate packets; do not add
production compatibility containers or weaken expected Source behavior.
The final failed core build log is `build/p23l-full-core-check-repaired.log`,
SHA-256 `d5adddaa4d3884a1bdba556a1cb981734ce1fa2515257f6c9301517a957d183c`.
